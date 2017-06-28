#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>

class NetworkClient: public QObject
{
    Q_OBJECT
public:
    NetworkClient(QTcpSocket * socket, QObject * parent = nullptr);
    virtual ~NetworkClient();

    QString peerName() const;

signals:
    void receiveMessage(const QString & message, const QString & peerName);
    void disconnected();

public slots:
    void onSendMessage(const QString & message, const QString & peer);

protected slots:
    void onSocketReadyRead();
    void onTextMessageReceived(const QString & message);
    void onTextFrameReceived(const QString &frame, bool isLastFrame);

    void onHandleMessage(const QString & message);

public:
    void handleMessage(const QString & message);
private:
    QString m_peerName;
    QTcpSocket * m_socket;
};

#endif // NETWORKCLIENT_H
