#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QCommonStyle>
#include "remoteservcieselectdialog.h"

ChatWindow::ChatWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow),
    m_socket(nullptr)

{
    ui->setupUi(this);
    ui->serviceSelectButton->setIcon(QIcon(":/search.png"));
    ui->serviceSelectButton->setStyleSheet(
                "border: none;"
                "min-width : 24px;"
                "min-height: 24px;"
                "icon-size: 24px"
                );

    connect(ui->serviceSelectButton, &QPushButton::clicked, this, &ChatWindow::onStartSelectServcie);

    ui->connectButton->setIcon(QIcon(":/disconnected.png"));
    ui->connectButton->setDisabled(true);
    ui->connectButton->setStyleSheet(
                "border: none;"
                "min-width : 24px;"
                "min-height: 24px;"
                "icon-size: 24px"
                );

    connect(ui->connectButton, &QPushButton::clicked, this, &ChatWindow::onConnectToService);

    ui->disconnectButton->setIcon(QIcon(":/connected.png"));
    ui->disconnectButton->setVisible(false);
    ui->disconnectButton->setStyleSheet(
                "border: none;"
                "min-width : 24px;"
                "min-height: 24px;"
                "icon-size: 24px"
                );

    connect(ui->disconnectButton, &QPushButton::clicked, this, &ChatWindow::onDisconnectToService);

    ui->sendButton->setDisabled(true);
    ui->messageEdit->setDisabled(true);
    connect(ui->sendButton, &QPushButton::clicked, this, &ChatWindow::onSendMessage);
    connect(ui->messageEdit, &QLineEdit::editingFinished, this ,&ChatWindow::onSendMessage);

    connect(this, &ChatWindow::messageReceived, this, & ChatWindow::appendMessage);


    ui->messageList->setWordWrap(true);
    ui->messageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->messageList->setStyleSheet(
                "QListWidget::item {"
                "max-width: 10em;"
                "}");
}

ChatWindow::~ChatWindow()
{
    m_socket->deleteLater();
    m_socket = nullptr;

    delete ui;
}

void ChatWindow::onStartSelectServcie()
{
    RemoteServcieSelectDialog dialog(BLUETOOTH_SERVICE_UUID);
    if(dialog.exec() == QDialog::Accepted)
    {
        m_currentSelectService = dialog.selectedService();
        ui->serviceLabel->setText(m_currentSelectService.device().name());
        ui->connectButton->setEnabled(true);
    }

}

void ChatWindow::onConnectToService()
{
    if(!m_currentSelectService.isValid())
    {
        return;
    }

    if(m_socket)
    {
        m_socket->deleteLater();
        m_socket = nullptr;
    }


    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    connect(m_socket, &QBluetoothSocket::readyRead, this, &ChatWindow::onReadyRead);
    connect(m_socket, &QBluetoothSocket::connected, this, &ChatWindow::onServiceConnected);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &ChatWindow::onServiceDisconnected);

    m_socket->connectToService(m_currentSelectService);

}

void ChatWindow::onDisconnectToService()
{
    if(!m_socket)
    {
        return;
    }
    m_socket->disconnectFromService();
}

void ChatWindow::onReadyRead()
{
    if(!m_socket)
    {
        return;
    }

    while (m_socket->canReadLine())
    {
        QByteArray line = m_socket->readLine();
        emit messageReceived(QString::fromUtf8(line.constData(),line.length()),
                             m_socket->peerName(),
                             SIDE_REMOTE
                             );
    }
}

void ChatWindow::onServiceConnected()
{
    ui->connectButton->setVisible(false);
    ui->disconnectButton->setVisible(true);
    ui->sendButton->setEnabled(true);
    ui->messageEdit->setEnabled(true);

    emit messageReceived(QString("%1已连接").arg(m_socket->peerName()));
}

void ChatWindow::onServiceDisconnected()
{
    m_currentSelectService.unregisterService();
    ui->serviceLabel->clear();

    ui->disconnectButton->setVisible(false);
    ui->connectButton->setVisible(true);
    ui->connectButton->setEnabled(m_currentSelectService.isValid());
    ui->sendButton->setDisabled(true);
    ui->messageEdit->setDisabled(true);

    emit messageReceived("已断开");

    if(m_socket)
    {
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void ChatWindow::onSendMessage()
{
    if(!m_socket)
    {
        return;
    }

    QString msg = ui->messageEdit->text();
    ui->messageEdit->clear();
    QByteArray text = msg.toUtf8() + '\n';
    m_socket->write(text);

    emit appendMessage(msg, "Me", SIDE_LOCAL);

}

void ChatWindow::appendMessage(const QString & message, const QString & msg_sender,  int side)
{
    QString label;

    if(!msg_sender.isEmpty())
    {
        label = QString("[%1] : %2").arg(msg_sender).arg(message);
    }
    else
    {
        label = message;
    }

    QListWidgetItem * item = new QListWidgetItem(label);

    switch(side)
    {
    case  SIDE_INFO:
        item->setTextAlignment(Qt::AlignHCenter);
        item->setTextColor(QColor(Qt::red));
        break;
    case  SIDE_LOCAL:
        item->setTextAlignment(Qt::AlignTrailing);
        break;
    case  SIDE_REMOTE:
        item->setTextAlignment(Qt::AlignLeading);
        break;
    }




    ui->messageList->addItem(item);
}
