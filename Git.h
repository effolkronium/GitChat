#ifndef GIT_H
#define GIT_H

#include <string>
#include <QString>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <atomic>
#include "thread_safe_queue.hpp"

class Git
{
public:
    Git(QString gitUrl, QString login, const QString& password);
    ~Git();
public: // should be thread safe to call simultaneously
    QString GetLogin() const { return m_gitLogin; }
    void PushMessage(const QString& author, const QString& message);
    std::queue<std::pair<QString, QString>> GetNewMessages();
public:
    void CheckRepoWriteRight();
    void CloneRepo();
    void InitRepo();
    void PullMessages();
    void ExecuteGit(const QString& command);
    int ExecuteGit2(const QString& command);
    void FixConflicts();
    bool IsNewRepo() const;
private:
    QString m_gitCredUrl;
    QString m_gitLogin;
    QString m_gitRepoPath;
    QString m_gitMessagesPath;
    qint64 m_messagePos = 0;
    std::mutex m_mutex;
    std::mutex gc_mutex;
    std::mutex m_presendMsgGuids_mutex;
    std::set<QString> m_presendMsgGuids;
    thread_safe_queue<std::tuple<QString, QString, QString>> m_toSend;
    // guid, author, message

    std::thread m_newMsgThread;
    std::atomic_bool m_isRunning{false};
};

#endif // GIT_H
