#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class NetworkServer : public QObject
{
    Q_OBJECT

public:
    NetworkServer(QObject * parent = nullptr);

    ~NetworkServer();

signals:
    void clientConnected(QTcpSocket * socket);
    void closed();

protected slots:
    void onNewConnection();
    void onServerClosed();


private:
    QTcpServer * m_server;
};

#endif // NETWORKSERVER_H
