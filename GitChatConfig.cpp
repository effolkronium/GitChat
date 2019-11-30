#include "GitChatConfig.h"
#include "./ui_GitChatConfig.h"

GitChatConfig::GitChatConfig(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GitChatConfig)
{
    ui->setupUi(this);
}

GitChatConfig::~GitChatConfig()
{
    delete ui;
}

void GitChatConfig::on_pushButton_clicked()
{
    emit gitLogin(ui->editBoxGitURL->text(),
                  ui->editBoxUserName->text(),
                  ui->editBoxPassword->text());
}
