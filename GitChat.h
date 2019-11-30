#ifndef MAINWINDOW_CHAT_H
#define MAINWINDOW_CHAT_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <thread>
#include <atomic>

QT_BEGIN_NAMESPACE
namespace Ui { class GitChat; }
QT_END_NAMESPACE

class Git;

class GitChat : public QMainWindow
{
    Q_OBJECT
public:
    GitChat(Git& git);
    ~GitChat();
private:
    void AddIncomingMessage(QString message);
    void AddOutgoingMessage(QString message);
public:
    void StartGitListeningThread();
private:
    Git& m_git;
    std::thread m_gitThread;
    std::atomic_bool m_running{false};
    QStandardItemModel m_model;
    Ui::GitChat *ui;
};
#endif // MAINWINDOW_CHAT_H
