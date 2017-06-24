#include <QApplication>
#include "chatserverwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ChatServerWindow w;
    w.show();

    return a.exec();
}
