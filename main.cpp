#include "GitChatConfig.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GitChatConfig w;
    w.show();
    return a.exec();
}
