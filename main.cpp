#include "GitChatConfig.h"
#include "GitChat.h"
#include "Git.h"
#include <QApplication>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <optional>
#include <QUuid>

int main(int argc, char *argv[])
try {
    QApplication a(argc, argv);

    QFile f("qdarkstyle/style.qss");
    if (!f.exists())
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to set stylesheet, file not found");
        msgBox.exec();
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    GitChatConfig chatConfig;
    std::optional<GitChat> chat;
    std::optional<Git> git;

    chatConfig.connect(&chatConfig, &GitChatConfig::gitLogin, [&]
    (QString gitUrl, QString login, QString password){
        try {
            git.emplace(gitUrl, login, password);
            chat.emplace(git.value());
            chatConfig.close();
            chat->show();
        } catch (const std::exception& err) {
            QMessageBox msgBox;
            msgBox.setFixedSize(500,200);
            msgBox.critical(nullptr, "Error: ", QString{"Error: "} + err.what());
        }
    });

    chatConfig.show();
    return a.exec();
}
catch(const std::exception& err)
{
    QMessageBox msgBox;
    msgBox.critical(nullptr, "Error", err.what());
}
