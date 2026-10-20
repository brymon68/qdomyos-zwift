#include "characteristicwriteprocessor2ad9.h"
#include "treadmill.h"
#include <QSettings>
#include <QtMath>

namespace {
constexpr quint8 FTMS_SET_TARGET_POWER = 0x05;
constexpr quint8 FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS = 0x11;
constexpr quint8 FTMS_RESPONSE_CODE = 0x80;
constexpr quint8 FTMS_SUCCESS = 0x01;
constexpr quint8 FTMS_NOT_SUPPORTED = 0x02;
}

CharacteristicWriteProcessor2AD9::CharacteristicWriteProcessor2AD9(bluetoothdevice *bike,
                                                                   CharacteristicNotifier2AD9 *notifier,
                                                                   QObject *parent)
    : CharacteristicWriteProcessor(bike, parent), notifier(notifier) {}

int CharacteristicWriteProcessor2AD9::writeProcess(quint16 uuid, const QByteArray &data, QByteArray &reply) {
    if (data.size()) {
        BLUETOOTH_TYPE dt = Bike->deviceType();
        if (dt == TREADMILL && data.at(0) == FTMS_SET_TARGET_POWER) {
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)FTMS_SET_TARGET_POWER);
            reply.append((quint8)FTMS_NOT_SUPPORTED);
        } else if (dt == TREADMILL) {
            // Preserve FTMS Start/Stop forwarding and manual speed/incline control.
            emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(), data);
            char a, b;
            if ((char)data.at(0) == 0x02) {
                // Set Target Speed
                a = data.at(1);
                b = data.at(2);

                uint16_t uspeed = a + (((uint16_t)b) << 8);
                double requestSpeed = (double)uspeed / 100.0;
                if (dt == TREADMILL) {
                    ((treadmill *)Bike)->changeSpeed(requestSpeed);
                }
                qDebug() << QStringLiteral("new requested speed ") + QString::number(requestSpeed);
            } else if ((char)data.at(0) == 0x03) // Set Target Inclination
            {
                a = data.at(1);
                b = data.at(2);

                int16_t sincline = a + (((int16_t)b) << 8);
                double requestIncline = (double)sincline / 10.0;

                if (dt == TREADMILL)
                    ((treadmill *)Bike)->changeInclination(requestIncline, requestIncline);
                qDebug() << "new requested incline " + QString::number(requestIncline);
            } else if ((char)data.at(0) == 0x07) // Start request
            {
                // Bike->start();
                qDebug() << QStringLiteral("request to start");
            } else if ((char)data.at(0) == 0x08) // Stop request
            {
                // Bike->stop();
                qDebug() << QStringLiteral("request to stop");
            } else if ((char)data.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) // simulation parameter
            {
                qDebug() << QStringLiteral("indoor bike simulation parameters");
                int16_t iresistance = (((uint8_t)data.at(3)) + (data.at(4) << 8));
                uint8_t crr = data.at(5);
                uint8_t cw = data.at(6);
                changeSlope(iresistance, crr, cw);
            }
            reply.append((quint8)FTMS_RESPONSE_CODE);
            reply.append((quint8)data.at(0));
            reply.append((quint8)FTMS_SUCCESS);
        }
        if (notifier) {
            notifier->answer = reply;
        }
        return CP_OK;
    } else
        return CP_INVALID;
}
