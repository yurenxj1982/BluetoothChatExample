#include "networkmessageservice.h"
#include "networkserver.h"
#include "networkclient.h"

NetworkMessageService::NetworkMessageService(QObject *parent) :
    QObject(parent)
{


    NetworkServer * server = new NetworkServer;

    connect(server, &NetworkServer::clientConnected, this, &NetworkMessageService::onClientConnected);
    connect(server, &NetworkServer::closed, this, &NetworkMessageService::onServerClosed);

    m_server = QSharedPointer<NetworkServer>(server);

}

NetworkMessageService::~NetworkMessageService()
{
    m_clients.clear();
    m_server.clear();
}

void NetworkMessageService::onSendMessage(const QString & message, const QString & toPeer)
{
    emit sendMessage(message, toPeer);
    emit showMessage(message, "Me", LocalMessage);
}

void NetworkMessageService::onClientConnected(QTcpSocket * socket)
{
    NetworkClient * client = new NetworkClient(socket);
    connect(client, &NetworkClient::receiveMessage, this, &NetworkMessageService::onReceiveMessage);
    connect(client, &NetworkClient::disconnected, this, &NetworkMessageService::onClientDisconnected);

    m_clients.insert(client->peerName(), QSharedPointer<NetworkClient>(client));

    emit showMessage(QString("%1已连接").arg(client->peerName()), "", ConnectedMessage);
}

void NetworkMessageService::onServerClosed()
{
    m_clients.clear();
    m_server.clear();
}

void NetworkMessageService::onClientDisconnected()
{
    NetworkClient * client = qobject_cast<NetworkClient *>(sender());

    foreach(auto savedClient, m_clients.values())
    {
        if(savedClient.data() == client)
        {
            emit showMessage(QString("%1已断开").arg(savedClient->peerName()), "", DisconnectedMessage);
            m_clients.remove(m_clients.key(savedClient));
            break;
        }
    }

    if(m_clients.isEmpty())
    {
            emit showMessage(QString("所有连接都已断开"), "", DisconnectedMessage);
    }

}

void NetworkMessageService::onReceiveMessage(const QString & message, const QString & peer)
{
    emit showMessage(message, peer, RemoteMessage);
}

