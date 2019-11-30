#ifndef GIT_H
#define GIT_H

#include <string>
#include <QString>
#include <mutex>
#include <queue>

class Git
{
public:
    Git(QString gitUrl, QString login, const QString& password);
public: // should be thread safe to call simultaneously
    void PushMessage(QString message);
    std::queue<std::pair<QString, QString>> GetAllMessages();
    std::queue<std::pair<QString, QString>> GetNewMessages();
private:
    void CloneRepo();
    void InitRepo();
    void PullMessages();
    void ExecuteGit(const QString& command);
    bool IsNewRepo() const;
private:
    QString m_gitCredUrl;
    QString m_gitLogin;
    QString m_gitRepoPath;
    QString m_gitMessagesPath;
    std::mutex m_mutex;
};

#endif // GIT_H
