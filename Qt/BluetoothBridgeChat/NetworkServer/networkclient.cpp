#include "networkclient.h"
#include <QString>
#include <QtConcurrent>
#include <QHostAddress>

NetworkClient::NetworkClient(QTcpSocket * socket, QObject * parent):
    QObject (parent),
    m_socket(socket)
{
    if(m_socket)
    {
        m_peerName = m_socket->peerName();

        if(m_peerName.isEmpty())
        {
            m_peerName = QString("Client %1:%2").arg(m_socket->peerAddress().toString()).arg(m_socket->peerPort());
        }

        qDebug() << QString("%1 connected").arg(m_peerName);

        /*
        connect(m_socket, &QTcpSocket::textMessageReceived,
                this, &NetworkClient::onTextMessageReceived);
        connect(m_socket, &QTcpSocket::textFrameReceived,
                this, &NetworkClient::onTextFrameReceived);
        */
        connect(m_socket, &QTcpSocket::readyRead,
                this, &NetworkClient::onSocketReadyRead);
        connect(m_socket, &QTcpSocket::disconnected,
                this, &NetworkClient::disconnected);
        connect(this, &NetworkClient::destroyed,
                m_socket, &QTcpSocket::deleteLater);
    }

}

QString NetworkClient::peerName() const
{
    return m_peerName;

}

void NetworkClient::onSocketReadyRead()
{
    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine().trimmed();
        onTextMessageReceived(QString::fromUtf8(line.constData(), line.length()));
    }

}

NetworkClient::~NetworkClient()
{
    m_socket->close();
    m_socket = nullptr;
}


void NetworkClient::onSendMessage(const QString & message, const QString & peer)
{
    if(m_socket)
    {
        if(peer.isEmpty() || peer == peerName())
        {
            m_socket->write(message.toUtf8());
            //m_socket->sendTextMessage(message);
        }
    }
}

void NetworkClient::onTextMessageReceived(const QString & message)
{
    onHandleMessage(message);
}

void NetworkClient::onTextFrameReceived(const QString & message, bool isLastFrame)
{
    static QString buf;

    buf.append(message);

    if(isLastFrame)
    {
        onHandleMessage(message);
        buf.clear();
    }
}

void NetworkClient::onHandleMessage(const QString & message)
{
    QtConcurrent::run(this, &NetworkClient::handleMessage, message);

}

void NetworkClient::handleMessage(const QString & message)
{
    emit receiveMessage(message, peerName());
}
