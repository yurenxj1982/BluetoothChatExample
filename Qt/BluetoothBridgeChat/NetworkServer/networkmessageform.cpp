#include "networkmessageform.h"
#include "ui_networkmessageform.h"
#include "networkmessageservice.h"

NetworkMessageForm::NetworkMessageForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetworkMessageForm)
{
    ui->setupUi(this);
}

NetworkMessageForm::~NetworkMessageForm()
{
    delete ui;
}


void NetworkMessageForm::onShowMessage(const QString & message, const QString & msgSender, int msgType)
{
    QListWidgetItem * item = new QListWidgetItem;

    QString label;
    if(!msgSender.isEmpty())
    {
        label.append(QString("[%1]: ").arg(msgSender));
    }

    label.append(message);
    item->setText(label);
    switch (msgType)
    {
    case NetworkMessageService::ConnectedMessage:
        item->setTextAlignment(Qt::AlignHCenter);
        item->setTextColor(QColor(Qt::green));
        break;
    case NetworkMessageService::DisconnectedMessage:
        item->setTextAlignment(Qt::AlignHCenter);
        item->setTextColor(QColor(Qt::red));
        break;

    case NetworkMessageService::LocalMessage:
        item->setTextAlignment(Qt::AlignTrailing);
        break;
    case NetworkMessageService::RemoteMessage:
        item->setTextAlignment(Qt::AlignLeading);
        break;

    }

    ui->msgList->addItem(item);
}
