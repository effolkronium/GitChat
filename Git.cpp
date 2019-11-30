#include "Git.h"
#include <QDir>
#include <QProcess>
#include <QUuid>
#include <QMessageBox>
#include <stdexcept>

Git::Git(QString gitUrl, QString login, const QString& password)
    : m_gitCredUrl([&]{
        return std::move(gitUrl.replace("https://", "https://" + login + ":" + password + "@"));
      }()),
      m_gitLogin{std::move(login)},
      m_gitRepoPath{QDir::tempPath()  + QDir::separator() + QUuid::createUuid().toString()},
      m_gitMessagesPath{m_gitRepoPath + QDir::separator() + "messages"}
{
    CloneRepo();
    InitRepo();
}

bool Git::IsNewRepo() const
{
    return !QFile{m_gitMessagesPath}.exists();
}

void Git::InitRepo()
{
    if(IsNewRepo())
    {
        QFile file(m_gitMessagesPath);
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    ExecuteGit("add messages");
    ExecuteGit("commit -m\"Git::InitRepo()\"");
    ExecuteGit("push");
}

void Git::PushMessage(QString message)
{
    std::lock_guard guard{m_mutex};
}

std::queue<std::pair<QString, QString>> Git::GetAllMessages()
{
    std::lock_guard guard{m_mutex};
}

std::queue<std::pair<QString, QString>> Git::GetNewMessages()
{
    std::lock_guard guard{m_mutex};
}

void Git::PullMessages()
{

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
