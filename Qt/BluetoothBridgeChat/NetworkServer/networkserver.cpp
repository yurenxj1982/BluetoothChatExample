#include "networkserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>

NetworkServer::NetworkServer(QObject * parent) :
    QObject (parent),
    m_server(new QTcpServer(
                 /*
                 QString("BluetoothChatNetworkServer"),
                 QTcpServer::NonSecureMode,
                 */
                 this))
{

    quint16 port = quint16(NETWORK_SERVER_PORT);
    if(m_server->listen(QHostAddress::Any, port )) {
        qDebug() << "BluetoothChatNetworkServer listening on port" << port;

        connect(m_server, &QTcpServer::newConnection,
                this, &NetworkServer::onNewConnection);
    }

}

NetworkServer::~NetworkServer()
{
    m_server->close();
}


void NetworkServer::onNewConnection()
{
    QTcpSocket *pSocket = m_server->nextPendingConnection();
    emit clientConnected(pSocket);

}

void NetworkServer::onServerClosed()
{
    qDebug() << "BluetoothChatNetworkServer closed";
    emit closed();
}

