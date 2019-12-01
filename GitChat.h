#ifndef MAINWINDOW_CHAT_H
#define MAINWINDOW_CHAT_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <thread>
#include <atomic>
#include <queue>

using MessageType = std::queue<std::pair<QString, QString>>;
Q_DECLARE_METATYPE(MessageType);

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
    void AddIncomingMessage(const QString& author, const QString& message);
    void AddOutgoingMessage(const QString& author, const QString& message);
public:
    void StartGitListeningThread();
signals:
    void SignalAddMessages(MessageType messages);
    void SignalGitThreadError(const QString& message);
private slots:
    void on_lineEdit_returnPressed();

private:
    void GitThreadError(const QString& message);
    void AddMessages(MessageType messages);
private:
    Git& m_git;
    std::thread m_gitThread;
    std::atomic_bool m_running{false};
    QStandardItemModel m_model;
    Ui::GitChat *ui;
};
#endif // MAINWINDOW_CHAT_H
