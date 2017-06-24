#ifndef REMOTESERVCIESELECTDIALOG_H
#define REMOTESERVCIESELECTDIALOG_H

#include <QDialog>
#include <QBluetoothServiceDiscoveryAgent>
#include <QSharedPointer>


namespace Ui {
class RemoteServcieSelectDialog;
}

class RemoteServcieSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteServcieSelectDialog(const QUuid &serviceUuid = QUuid(),  QWidget *parent = 0);
    ~RemoteServcieSelectDialog();

    QBluetoothServiceInfo selectedService() const;

protected slots:
    void onStartScan();

    void onScanFinished();

    void onServiceDiscovered(const QBluetoothServiceInfo &info);

    void onServiceSelected();

private:
    Ui::RemoteServcieSelectDialog *ui;

    QMap<QString, QBluetoothServiceInfo> m_services;
    QList<QSharedPointer<QBluetoothServiceDiscoveryAgent>> m_discovertyAgents;
    QString m_selectedServiceDescrption;
};

#endif // REMOTESERVCIESELECTDIALOG_H
