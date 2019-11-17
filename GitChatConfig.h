#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class GitChatConfig; }
QT_END_NAMESPACE

class GitChatConfig : public QMainWindow
{
    Q_OBJECT

public:
    GitChatConfig(QWidget *parent = nullptr);
    ~GitChatConfig();

private:
    Ui::GitChatConfig *ui;
};
#endif // MAINWINDOW_H
