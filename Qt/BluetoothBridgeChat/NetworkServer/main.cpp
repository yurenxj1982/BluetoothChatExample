#include <QApplication>
#include "networkmessageform.h"
#include "networkmessageservice.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    NetworkMessageForm w;
    NetworkMessageService service;
    QObject::connect(&service, &NetworkMessageService::showMessage, &w, &NetworkMessageForm::onShowMessage);
    w.show();

    return a.exec();
}
