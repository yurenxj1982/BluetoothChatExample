#include "chatserverwindow.h"
#include "ui_chatserverwindow.h"

ChatServerWindow::ChatServerWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatServerWindow)
{
    ui->setupUi(this);

    connect(ui->sendButton, &QPushButton::clicked, this, &ChatServerWindow::onSendClicked);
    connect(ui->messageEdit, &QLineEdit::editingFinished, this, &ChatServerWindow::onSendClicked);

    connect(this, &ChatServerWindow::messageReceived, this, &ChatServerWindow::onMessageReceived);

    ui->sendButton->setDisabled(true);
    ui->messageEdit->setDisabled(true);
}

ChatServerWindow::~ChatServerWindow()
{
    stopServer();
    delete ui;
}

void ChatServerWindow::startServer(const QBluetoothAddress & localAdapter)
{
    if(!m_server.isNull())
    {
        return;
    }

    QBluetoothServer * server = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol);
    connect(server, &QBluetoothServer::newConnection, this, &ChatServerWindow::onClientConnected);
    m_server = QSharedPointer<QBluetoothServer>(server);

    bool result = m_server->listen(localAdapter);

    if(!result) {
        showMessage(QString("未能在%1上绑定服务").arg(localAdapter.toString()));
        return;
    }

    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                               classId);

    classId.prepend(QVariant::fromValue(QBluetoothUuid(QUuid(BLUETOOTH_SERVICE_UUID))));

    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds,classId);

    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, "Bluetooth Chat Server");
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                               "Bluetooth Chat Server Demo");
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, "yurenxj1982@sohu.com");

    m_serviceInfo.setServiceUuid(QBluetoothUuid(QUuid(BLUETOOTH_SERVICE_UUID)));

    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;

    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(m_server->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    m_serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                               protocolDescriptorList);

    m_serviceInfo.registerService(localAdapter);



}

void ChatServerWindow::stopServer()
{
    m_serviceInfo.unregisterService();
    m_clientSockets.clear();
    m_server.clear();
}


void ChatServerWindow::showMessage(const QString & message, const QString & msg_sender,  int side)
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

void ChatServerWindow::sendMessage(const QString & message)
{
    if(message.isEmpty())
    {
        return;
    }

    QByteArray text = message.toUtf8() + '\n';
    foreach(auto socket, m_clientSockets)
    {
        socket->write(text);
    }
}


void ChatServerWindow::onClientConnected()
{
    QBluetoothSocket * socket = m_server->nextPendingConnection();
    if(!socket)
    {
        return;
    }

    connect(socket, &QBluetoothSocket::readyRead, this, &ChatServerWindow::onReadyRead);
    connect(socket, &QBluetoothSocket::disconnected, this, &ChatServerWindow::onClientDisconnected);

    m_clientSockets.append(QSharedPointer<QBluetoothSocket>(socket));

    showMessage(QString("%1 已连接").arg(socket->peerName()));

    ui->sendButton->setEnabled(true);
    ui->messageEdit->setEnabled(true);

}

void ChatServerWindow::onClientDisconnected()
{
    QBluetoothSocket * socket = qobject_cast<QBluetoothSocket*>(sender());
    if(!socket)
    {
        return;
    }

    showMessage(QString("%1 已断开").arg(socket->peerName()));

    foreach(auto savedSocket , m_clientSockets)
    {
        if(savedSocket.data() == socket)
        {
            m_clientSockets.removeOne(savedSocket);
            break;
        }
    }

    if(m_clientSockets.isEmpty())
    {
        showMessage("所有连接都已断开");
        ui->sendButton->setDisabled(true);
        ui->messageEdit->setDisabled(true);
    }

}

void ChatServerWindow::onReadyRead()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        emit messageReceived(
                    QString::fromUtf8(line.constData(), line.length()),
                    socket->peerName());
    }
}

void ChatServerWindow::showEvent(QShowEvent * )
{
    startServer();
}

void ChatServerWindow::onSendClicked()
{
    QString message = ui->messageEdit->text();
    if(message.isEmpty())
    {
        return;
    }

    ui->messageEdit->clear();

    showMessage(message, "Me", SIDE_LOCAL);

    sendMessage(message);
}

void ChatServerWindow::onMessageReceived(const QString & message, const QString & sender)
{
    showMessage(message, sender, SIDE_REMOTE);
}

