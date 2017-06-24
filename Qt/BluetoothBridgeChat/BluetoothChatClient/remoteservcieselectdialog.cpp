#include "remoteservcieselectdialog.h"
#include "ui_remoteservcieselectdialog.h"

#include <QBluetoothLocalDevice>
#include <QBluetoothLocalDevice>

RemoteServcieSelectDialog::RemoteServcieSelectDialog(const QUuid& serviceUuid, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteServcieSelectDialog)
{
    ui->setupUi(this);
    connect(ui->cancelButton, &QPushButton::clicked, this, &RemoteServcieSelectDialog::reject);
    connect(ui->connectButton, &QPushButton::clicked, this, &RemoteServcieSelectDialog::accept);
    connect(ui->scanButton, &QPushButton::clicked, this, &RemoteServcieSelectDialog::onStartScan);

    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &RemoteServcieSelectDialog::onServiceSelected );


    ui->connectButton->setDisabled(true);

    QList<QBluetoothHostInfo> allDevice = QBluetoothLocalDevice::allDevices();

    if(allDevice.isEmpty())
    {
        ui->notifyLabel->setText("本机蓝牙设备未打开， 请打开蓝牙设备重试");
        ui->scanButton->setDisabled(true);
        return;
    }

    foreach(auto device, allDevice)
    {
        QBluetoothServiceDiscoveryAgent * serviceDiscoveryAgent = new QBluetoothServiceDiscoveryAgent(device.address());

        connect(serviceDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this, & RemoteServcieSelectDialog::onServiceDiscovered);
        connect(serviceDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, & RemoteServcieSelectDialog::onScanFinished);
        if(!serviceUuid.isNull())
        {
            serviceDiscoveryAgent->setUuidFilter(serviceUuid);
        }

        m_discovertyAgents.append(QSharedPointer<QBluetoothServiceDiscoveryAgent>(serviceDiscoveryAgent));

    }
}

RemoteServcieSelectDialog::~RemoteServcieSelectDialog()
{
    delete ui;
}

void RemoteServcieSelectDialog::onStartScan()
{
    ui->scanButton->setDisabled(true);

    foreach(auto agent, m_discovertyAgents)
    {
        agent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
    }
}

void RemoteServcieSelectDialog::onScanFinished()
{
    foreach(auto agent, m_discovertyAgents)
    {
        if(agent->isActive())
        {
            return ;
        }
    }
    ui->scanButton->setEnabled(true);
}


void RemoteServcieSelectDialog::onServiceSelected()
{
    QListWidgetItem * item = ui->listWidget->currentItem();
    m_selectedServiceDescrption = item->text();
    QBluetoothServiceInfo info = selectedService();
    ui->notifyLabel->setText(QString("已选择%1(%2)").arg(info.serviceName()).arg(info.device().name()));
    ui->connectButton->setEnabled(true);
}

QBluetoothServiceInfo RemoteServcieSelectDialog::selectedService() const
{
    if(m_selectedServiceDescrption.isEmpty())
    {
        return QBluetoothServiceInfo();
    }
    else
    {
        return m_services.value(m_selectedServiceDescrption, QBluetoothServiceInfo());
    }

}

void RemoteServcieSelectDialog::onServiceDiscovered(const QBluetoothServiceInfo &info)
{
    QString label = QString("%1(%2)").arg(info.serviceName()).arg(info.device().name());

    QList<QListWidgetItem *> items = ui->listWidget->findItems(label, Qt::MatchExactly);
    if(items.empty())
    {
        QListWidgetItem * item = new QListWidgetItem(label);
        item->setTextColor(QColor(Qt::black));

        foreach(auto localDeviceInfo, QBluetoothLocalDevice::allDevices())
        {
            QBluetoothLocalDevice localDevice(localDeviceInfo.address());
            QBluetoothLocalDevice::Pairing paireingStatus = localDevice.pairingStatus(info.device().address());

            if(paireingStatus == QBluetoothLocalDevice::Paired || paireingStatus == QBluetoothLocalDevice::AuthorizedPaired)
            {
                item->setTextColor(QColor(Qt::green));
                break;
            }
        }

        ui->listWidget->addItem(item);

        m_services.insert(label, info);
    }
}
