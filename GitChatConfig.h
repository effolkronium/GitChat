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
signals:
    void gitLogin(QString gitUrl, QString login, QString password);
private:
    Ui::GitChatConfig *ui;
private slots:
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
