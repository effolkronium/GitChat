#include "GitChat.h"
#include "Git.h"
#include "ListViewDelegate.h"
#include <QScrollBar>
#include <chrono>
#include "./ui_GitChat.h"

using namespace std::literals;

GitChat::GitChat(Git& git)
    : QMainWindow(nullptr)
    , m_git(git)
    , ui(new Ui::GitChat)
{
    ui->setupUi(this);

    ui->listView->setResizeMode(QListView::Adjust);
    ui->listView->setWordWrap(true);
    ui->listView->setModel(&m_model);
    ui->listView->setMinimumSize(200,350);

    ui->listView->setItemDelegate(new ListViewDelegate());

    // create some data and put it in a model
    for(int i = 0; i < 256; ++i)
    {
        QStandardItem *item1 = new QStandardItem("This is item one");
        item1->setData("Incoming", Qt::UserRole + 1);
        m_model.appendRow(item1);
        QStandardItem *item2 = new QStandardItem("This is item two, it is a very long item, but it's not the item's fault, it is me typing all this text.");
        item2->setData("Outgoing", Qt::UserRole + 1);
        m_model.appendRow(item2);
        QStandardItem *item3 = new QStandardItem("This is the third item");
        item3->setData("Incoming", Qt::UserRole + 1);
        m_model.appendRow(item3);
    }

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
    m_gitThread = std::thread{[this]{
        while(m_running)
        {
            std::this_thread::sleep_for(2s);
        }
    }};
}

void GitChat::AddIncomingMessage(QString message)
{
    auto item = std::make_unique<QStandardItem>(message);
    item->setData("Incoming", Qt::UserRole + 1);
    m_model.appendRow(item.release());
}

void GitChat::AddOutgoingMessage(QString message)
{
    auto item = std::make_unique<QStandardItem>(message);
    item->setData("Outgoing", Qt::UserRole + 1);
    m_model.appendRow(item.release());
}
