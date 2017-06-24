#ifndef CHATSERVERWINDOW_H
#define CHATSERVERWINDOW_H

#include <QWidget>
#include <QBluetoothServer>
#include <QSharedPointer>

namespace Ui {
class ChatServerWindow;
}

class ChatServerWindow : public QWidget
{
    Q_OBJECT
public:
    enum MSG_SIDE {
        SIDE_INFO,
        SIDE_LOCAL,
        SIDE_REMOTE
    };

public:
    explicit ChatServerWindow(QWidget *parent = 0);
    ~ChatServerWindow();


    void startServer(const QBluetoothAddress & localAdapter = QBluetoothAddress());
    void stopServer();

public slots:
    void sendMessage(const QString & message);
    void showMessage(const QString & message, const QString & sender = "", int side = SIDE_INFO);

protected slots:
    void onClientConnected();
    void onClientDisconnected();
    void onReadyRead();

    void onSendClicked();
    void onMessageReceived(const QString & message, const QString & sender);

signals:
    void messageReceived(const QString & message, const QString & sender = "");

protected:
    void showEvent(QShowEvent * ) override;



private:
    Ui::ChatServerWindow *ui;

    QSharedPointer<QBluetoothServer> m_server;
    QBluetoothServiceInfo m_serviceInfo;
    QList<QSharedPointer<QBluetoothSocket>> m_clientSockets;
};

#endif // CHATSERVERWINDOW_H
