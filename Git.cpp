#include "Git.h"
#include <QDir>
#include <QProcess>
#include <QUuid>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <stdexcept>
#include <iostream>

using namespace std::literals;

Git::Git(QString gitUrl, QString login, const QString& password)
    : m_gitCredUrl([&]{
        return std::move(gitUrl.replace("https://", "https://" + login + ":" + password + "@"));
      }()),
      m_gitLogin{std::move(login)},
      m_gitRepoPath{"C:\\_\\1"},
      //m_gitRepoPath{QDir::tempPath()  + QDir::separator() + QUuid::createUuid().toString()},
      m_gitMessagesPath{m_gitRepoPath + QDir::separator() + "messages"}
{
  //  CloneRepo();
   // if(IsNewRepo())
  //      InitRepo();

    m_isRunning = true;
    m_newMsgThread = std::thread{[this]{
        while(m_isRunning)
        {
            std::this_thread::sleep_for(10s);
            if(m_toSend.empty())
                continue;

            std::lock_guard guard{m_mutex};

            {
                QFile file(m_gitMessagesPath);
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
            }

            ExecuteGit("add messages");
            ExecuteGit("commit -m\"Git::PushMessage\"");

            while(0 != ExecuteGit2("push"))
                if(0 != ExecuteGit2("pull"))
                    FixConflicts();
        }
    }};
}


Git::~Git()
{
    if(m_newMsgThread.joinable())
        m_newMsgThread.join();

    return;
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
    QFile file(m_gitMessagesPath);
    file.open(QIODevice::WriteOnly);
    file.close();
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
        QFile file(m_gitMessagesPath);
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

                if(line.startsWith("<") || line.startsWith("=") || line.startsWith(">"))
                    continue;

                textWithoutConflits += line += '\n';
            }

            if(!in.seek(0))
                throw std::runtime_error{"if(!file.seek(m_messagePos))"};

            in << textWithoutConflits;
        }

        if(!file.resize(file.pos()))
            throw std::runtime_error{"if(!file.resize(file.pos()))"};
    }

    ExecuteGit("add messages");
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
                m_presendMsgGuids.erase(findIt);
            }
        }

        if(!isMsgWasPresended)
            messages.emplace(std::move(fields[1]), std::move(fields[2]));
    }

    m_messagePos = file.pos();

    return messages;
}

void Git::PullMessages()
{
    ExecuteGit("pull");
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
    auto gitCommand = "git -C \"" + m_gitRepoPath + "\" "
            + "-c user.name='" + m_gitLogin + "' -c user.email='<>' "
            + command;
    if(0 != QProcess::execute(gitCommand))
        throw std::runtime_error{("The command \"" + gitCommand + "\" has failed").toUtf8()};
}

int Git::ExecuteGit2(const QString& command)
{
    auto gitCommand = "git -C \"" + m_gitRepoPath + "\" "
            + "-c user.name='" + m_gitLogin + "' -c user.email='<>' "
            + command;
    return QProcess::execute(gitCommand);
}
