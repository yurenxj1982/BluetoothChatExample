#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    enum MSG_SIDE {
        SIDE_INFO,
        SIDE_LOCAL,
        SIDE_REMOTE,
    };

public:
    explicit ChatWindow(QWidget *parent = 0);
    ~ChatWindow();

signals:
    void messageReceived(const QString & message, const QString & sender = "", int  side = SIDE_INFO);

protected slots:
    void onStartSelectServcie();
    void onConnectToService();
    void onDisconnectToService();

    void onReadyRead();
    void onServiceConnected();
    void onServiceDisconnected();
    void onSendMessage();

    void appendMessage(const QString & message, const QString & sender = "", int side = SIDE_INFO);

private:
    Ui::ChatWindow *ui;

    QBluetoothServiceInfo m_currentSelectService;
    QBluetoothSocket * m_socket;
};

#endif // CHATWINDOW_H
