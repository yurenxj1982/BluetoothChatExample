#ifndef NETWORKMESSAGESERVICE_H
#define NETWORKMESSAGESERVICE_H

#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>

class NetworkServer;
class NetworkClient;

class NetworkMessageService : public QObject
{
    Q_OBJECT
public:
    enum MessageType {
        ConnectedMessage,
        DisconnectedMessage,
        LocalMessage,
        RemoteMessage,
    };

public:
    explicit NetworkMessageService(QObject *parent = nullptr);
    virtual ~NetworkMessageService();

signals:
    void showMessage(const QString & message, const QString & sender, int type);

    void sendMessage(const QString & message, const QString & peer);

public slots:
    void onSendMessage(const QString & message, const QString & toPeer);

protected slots:
    void onClientConnected(QTcpSocket * socket);
    void onServerClosed();


    void onClientDisconnected();
    void onReceiveMessage(const QString & message, const QString & peer);


private:
    QSharedPointer<NetworkServer> m_server;
    QMap<QString, QSharedPointer<NetworkClient>> m_clients;


};

#endif // NETWORKMESSAGESERVICE_H
