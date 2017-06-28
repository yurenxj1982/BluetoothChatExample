#ifndef NETWORKMESSAGEFORM_H
#define NETWORKMESSAGEFORM_H

#include <QWidget>

namespace Ui {
class NetworkMessageForm;
}

class NetworkMessageForm : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkMessageForm(QWidget *parent = 0);
    ~NetworkMessageForm();

public slots:
    void onShowMessage(const QString & message, const QString & msgSender, int msgType);

private:
    Ui::NetworkMessageForm *ui;
};

#endif // NETWORKMESSAGEFORM_H
