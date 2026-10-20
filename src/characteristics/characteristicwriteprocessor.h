#ifndef CHARACTERISTICWRITEPROCESSOR_H
#define CHARACTERISTICWRITEPROCESSOR_H

#include "devices/bluetoothdevice.h"
#include <QObject>
#include <QSettings>
#include <QtMath>

#define CP_INVALID -1
#define CP_OK 0

class CharacteristicWriteProcessor : public QObject {
    Q_OBJECT
  public:
    bluetoothdevice *Bike = nullptr;

    explicit CharacteristicWriteProcessor(bluetoothdevice *bike, QObject *parent = nullptr);
    virtual int writeProcess(quint16 uuid, const QByteArray &data, QByteArray &out) = 0;
    virtual void changeSlope(int16_t iresistance, uint8_t crr, uint8_t cw);
  signals:
    void changeInclination(double grade, double percentage);
    void slopeChanged();
};

#endif // CHARACTERISTICWRITEPROCESSOR_H
