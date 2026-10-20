#include "characteristicwriteprocessor.h"
#include <QSettings>

CharacteristicWriteProcessor::CharacteristicWriteProcessor(bluetoothdevice *bike, QObject *parent)
    : QObject(parent), Bike(bike) {}

void CharacteristicWriteProcessor::changeSlope(int16_t iresistance, uint8_t crr, uint8_t cw) {
    QSettings settings;
    bool zwift_negative_inclination_x2 =
        settings.value(QZSettings::zwift_negative_inclination_x2, QZSettings::default_zwift_negative_inclination_x2)
            .toBool();
    double offset =
        settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset).toDouble();
    double gain =
        settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain).toDouble();
    double min_inclination = settings.value(QZSettings::min_inclination, QZSettings::default_min_inclination).toDouble();

    double grade = ((iresistance / 100.0) * gain) + offset;
    double percentage = ((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * gain) + offset;
    if (zwift_negative_inclination_x2 && iresistance < 0) {
        grade = (((iresistance / 100.0) * 2.0) * gain) + offset;
        percentage = (((qTan(qDegreesToRadians(iresistance / 100.0)) * 100.0) * 2.0) * gain) + offset;
    }

    if(min_inclination > grade) {
        grade = min_inclination;
        qDebug() << "grade override due to min_inclination " << min_inclination;
    }

    Q_UNUSED(crr)
    Q_UNUSED(cw)
    emit changeInclination(grade, percentage);
    emit slopeChanged();
}
