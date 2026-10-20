#include "characteristicnotifier2ad2.h"
#include "devices/treadmill.h"

CharacteristicNotifier2AD2::CharacteristicNotifier2AD2(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2ad2, parent), Bike(Bike) {}

int CharacteristicNotifier2AD2::notify(QByteArray &value) {
    if (Bike->deviceType() != TREADMILL)
        return CN_INVALID;

    const uint16_t speed = qRound(Bike->currentSpeed().value() * 100);
    const uint16_t cadence = Bike->currentCadence().value() * 2;
    value.append((char)0x24); // speed, instantaneous cadence, resistance
    value.append((char)0x02); // heart rate
    value.append((char)(speed & 0xFF));
    value.append((char)((speed >> 8) & 0xFF));
    value.append((char)(cadence & 0xFF));
    value.append((char)((cadence >> 8) & 0xFF));
    value.append((char)0); // resistance
    value.append((char)0);
    value.append((char)Bike->currentHeart().value());
    value.append((char)0); // Bkool HRM compatibility
    return CN_OK;
}
