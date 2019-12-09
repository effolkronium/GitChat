#include "Git.h"
#include <QDir>
#include <QProcess>
#include <QUuid>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <stdexcept>
#include <iostream>
#include <QSaveFile>

using namespace std::literals;

Git::Git(QString gitUrl, QString login, const QString& password)
    : m_gitCredUrl([&]{
        return std::move(gitUrl.replace("https://", "https://" + login + ":" + password + "@"));
      }()),
      m_gitLogin{std::move(login)},
      m_gitRepoPath{QDir::tempPath()  + QDir::separator() + QUuid::createUuid().toString()},
      m_gitMessagesPath{m_gitRepoPath + QDir::separator() + "messages"}
{
    CloneRepo();
    if(IsNewRepo())
        InitRepo();

    QMessageBox m;
    m.show();
    m_isRunning = true;
    m_newMsgThread = std::thread{[this]{
        while(m_isRunning)
        {
            std::this_thread::sleep_for(4s);
            if(m_toSend.empty())
                continue;

            std::lock_guard guard{m_mutex};

            {
                QSaveFile file(m_gitMessagesPath);
                if(!file.open(QIODevice::Append)) {
                    throw std::runtime_error{file.errorString().toUtf8()};
                }

                if(m_messagePos > file.size())
                    throw std::runtime_error{"New message db is smaller then old"};

                QTextStream in(&file);

                while(!m_toSend.empty())
                {
                    std::tuple<QString, QString, QString> nextMsg;
                    m_toSend.wait_and_pop(nextMsg);
                    in << '\n' << std::get<0>(nextMsg) << "," << std::get<1>(nextMsg) << "," << std::get<2>(nextMsg);
                }

                file.flush();
            }

            ExecuteGit("add messages");
            ExecuteGit("commit -m\"Git::PushMessage\"");

            std::cout << "\nPRE_PUSH\n";

            while(0 != ExecuteGit2("push"))
            {
                std::cout << "\nPRE_PULL\n";

                std::this_thread::sleep_for(1s);
                if(0 != ExecuteGit2("pull"))
                {
                    std::cout << "\nFIX_CONFLICTS\n";
                    FixConflicts();
                }
            }
        }
    }};
}


Git::~Git()
{
    m_isRunning = false;
    if(m_newMsgThread.joinable())
        m_newMsgThread.join();

    try {
        QDir{m_gitRepoPath}.removeRecursively();
    } catch (const std::exception& err) {
        std::cerr << "Git::~Git() has failed: " << err.what();
    }
}

bool Git::IsNewRepo() const
{
    return !QFile{m_gitMessagesPath}.exists();
}

void Git::InitRepo()
{
    {
    QSaveFile file(m_gitMessagesPath);
    file.open(QIODevice::WriteOnly);
    }
    ExecuteGit("add messages");
    ExecuteGit("commit -m\"Git::InitRepo()\"");
    ExecuteGit("push");
}

void Git::PushMessage(const QString& author, const QString& message)
{
    auto newGuid = QUuid::createUuid().toString();
    {
        std::lock_guard _{m_presendMsgGuids_mutex};
        m_presendMsgGuids.insert(newGuid);
    }
    m_toSend.push({newGuid, author, message});
}

void Git::FixConflicts()
{
    {
        QSaveFile file(m_gitMessagesPath);
        if(!file.open(QIODevice::ReadWrite |  QIODevice::Append)) {
            throw std::runtime_error{file.errorString().toUtf8()};
        }

        if(m_messagePos > file.size())
            throw std::runtime_error{"New message db is smaller then old"};

        {
            QTextStream in(&file);

            if(!in.seek(0))
                throw std::runtime_error{"if(!file.seek(m_messagePos))"};

            QString textWithoutConflits;

            while(!in.atEnd()) {
                QString line = in.readLine();

                if(!line.startsWith("{"))
                    continue;

                textWithoutConflits += line += '\n';
            }

            if(!in.seek(0))
                throw std::runtime_error{"if(!file.seek(m_messagePos))"};

            in << textWithoutConflits;

            in.flush();
        }

        if(!file.resize(file.pos()))
            throw std::runtime_error{"if(!file.resize(file.pos()))"};

        file.flush();
    }


    std::this_thread::sleep_for(1s);

    std::cout << "\nPRE_ADD_MSG_CONF\n";
    ExecuteGit("add messages");

    std::cout << "\nPRE_FIX_CONFLICT_COMMIT_CONF\n";
    ExecuteGit("commit -m\"Git::FixConflicts\"");
}

std::queue<std::pair<QString, QString>> Git::GetNewMessages()
{
    std::lock_guard guard{m_mutex};

    ExecuteGit("pull");

    QFile file(m_gitMessagesPath);
    if(!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error{file.errorString().toUtf8()};
    }

    if(m_messagePos > file.size())
        throw std::runtime_error{"New message db is smaller then old"};

    QTextStream in(&file);

    if(!in.seek(m_messagePos))
        throw std::runtime_error{"if(!file.seek(m_messagePos))"};

    std::queue<std::pair<QString, QString>> messages;
    while(!in.atEnd()) {
        QString line = in.readLine();

        if(line.size() == 0)
            continue;

        QStringList fields = line.split(",");
        if(fields.size() != 3)
            throw std::runtime_error{"Corrupted messages file"};

        bool isMsgWasPresended = false;
        {
            std::lock_guard _{m_presendMsgGuids_mutex};
            auto findIt = m_presendMsgGuids.find(fields[0]);
            if(m_presendMsgGuids.end() != findIt)
            {
                isMsgWasPresended = true;
            }
            else
            {
                m_presendMsgGuids.emplace(fields[0]);
            }

        }

        if(!isMsgWasPresended)
            messages.emplace(std::move(fields[1]), std::move(fields[2]));
    }

    m_messagePos = 0;

    return messages;
}

void Git::CloneRepo()
{
    if(QDir{m_gitRepoPath}.exists())
        throw std::runtime_error{("The directory " + m_gitRepoPath + " already exist").toUtf8()};

    QDir().mkdir(m_gitRepoPath);

    ExecuteGit(QString{"clone "} + m_gitCredUrl + " " + m_gitRepoPath);
}

void Git::ExecuteGit(const QString& command)
{
    std::lock_guard guard{gc_mutex};

    auto gitCommand = "git -C \"" + m_gitRepoPath + "\" "
            + "-c user.name='" + m_gitLogin + "' -c user.email='<>' "
            + command;
    if(0 != QProcess::execute(gitCommand))
        throw std::runtime_error{("The command \"" + gitCommand + "\" has failed").toUtf8()};
}

int Git::ExecuteGit2(const QString& command)
{
    std::lock_guard guard{gc_mutex};

    auto gitCommand = "git -C \"" + m_gitRepoPath + "\" "
            + "-c user.name='" + m_gitLogin + "' -c user.email='<>' "
            + command;
    return QProcess::execute(gitCommand);
}
