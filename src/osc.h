#ifndef OSC_H
#define OSC_H

#include <QObject>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QHash>
#include <QVariant>
#include "bluetoothdevice.h"
#include "devices/treadmill.h"
#include "homeform.h"
#include "bluetooth.h"
#include <oscpp/client.hpp>

class OSC : public QObject
{
    Q_OBJECT
  public:
    explicit OSC(bluetooth* manager, QObject *parent = nullptr);

  private:
    QTimer* m_timer;
    bluetooth* bluetoothManager;

    size_t OSC_makePacket(void* buffer, size_t size);
    QUdpSocket* OSC_sendSocket = new QUdpSocket(this);

  private slots:
    void publishWorkoutData();

  signals:

};

#endif // OSC_H
