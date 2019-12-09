#include "GitChat.h"
#include "Git.h"
#include "ListViewDelegate.h"
#include <QScrollBar>
#include <QMessageBox>
#include <chrono>
#include "./ui_GitChat.h"

using namespace std::literals;

GitChat::GitChat(Git& git)
    : QMainWindow(nullptr)
    , m_git(git)
    , ui(new Ui::GitChat)
{
    ui->setupUi(this);

    qRegisterMetaType<MessageType>();

    ui->listView->setResizeMode(QListView::Adjust);
    ui->listView->setWordWrap(true);
    ui->listView->setModel(&m_model);
    ui->listView->setMinimumSize(200,350);

    ui->listView->setItemDelegate(new ListViewDelegate());

    connect(this, &GitChat::SignalAddMessages, this, &GitChat::AddMessages);
    connect(this, &GitChat::SignalGitThreadError, this, &GitChat::GitThreadError, Qt::ConnectionType::BlockingQueuedConnection);

    StartGitListeningThread();
}

GitChat::~GitChat()
{
    m_running = false;
    if(m_gitThread.joinable())
        m_gitThread.join();

    delete ui;
}

void GitChat::StartGitListeningThread()
{
    m_running = true;
    m_gitThread = std::thread{[this]{
        try {
            while(m_running)
            {
                auto messages = this->m_git.GetNewMessages();
                emit SignalAddMessages(std::move(messages));
                std::this_thread::sleep_for(4s);
            }
        } catch (const std::exception& err) {
            emit SignalGitThreadError(QString{"Error: "} + err.what());
        }
    }};
}

void GitChat::AddMessages(std::queue<std::pair<QString, QString>> messages)
{
    while(!messages.empty())
    {
        auto& message = messages.front();
        if(m_git.GetLogin() == message.first)
            AddOutgoingMessage(message.first, message.second);
        else
            AddIncomingMessage(message.first, message.second);

        messages.pop();
    }

    ui->listView->scrollToBottom();
}

void GitChat::GitThreadError(const QString& message)
{
    QMessageBox msgBox;
    msgBox.setFixedSize(500,200);
    msgBox.critical(nullptr, "Error: ", message);
    std::terminate();
}

void GitChat::AddIncomingMessage(const QString& author, const QString& message)
{
    auto item = std::make_unique<QStandardItem>(QString{"\""} + author + QString{"\""} + QString{": "} + message);
    item->setData("Incoming", Qt::UserRole + 1);
    m_model.appendRow(item.release());
}

void GitChat::AddOutgoingMessage(const QString& author, const QString& message)
{
    auto item = std::make_unique<QStandardItem>(QString{"\""} + author + QString{"\""} + QString{": "} + message);
    item->setData("Outgoing", Qt::UserRole + 1);
    m_model.appendRow(item.release());
}

void GitChat::on_lineEdit_returnPressed()
{
    m_git.PushMessage(m_git.GetLogin(), ui->lineEdit->text());
    AddOutgoingMessage(m_git.GetLogin(), ui->lineEdit->text());
    ui->lineEdit->clear();
}

void GitChat::on_GitChat_destroyed()
{
    std::exit(0);
}
