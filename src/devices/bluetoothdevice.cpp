#include "devices/bluetoothdevice.h"

#include <QSettings>
#include <QTime>
#include <cmath>

bluetoothdevice::bluetoothdevice() {}

bluetoothdevice::~bluetoothdevice() {
    if(this->virtualDevice) {
        delete this->virtualDevice;
        this->virtualDevice = nullptr;
    }
}

BLUETOOTH_TYPE bluetoothdevice::deviceType() { return UNKNOWN; }
void bluetoothdevice::start() { requestStart = 1; lastStart = QDateTime::currentMSecsSinceEpoch(); }
void bluetoothdevice::stop(bool pause) {
    if (pause) {
        // Solo pause, NON stop
        requestPause = 1;
    } else {
        // Stop completo
        requestStop = 1;
    }
}
metric bluetoothdevice::currentHeart() { return Heart; }
metric bluetoothdevice::currentSpeed() { return Speed; }
metric bluetoothdevice::currentInclination() { return Inclination; }
QTime bluetoothdevice::movingTime() {
    int hours = (int)(moving.value() / 3600.0);
    return QTime(hours, (int)(moving.value() - ((double)hours * 3600.0)) / 60.0, ((uint32_t)moving.value()) % 60, 0);
}
QTime bluetoothdevice::elapsedTime() {
    int hours = (int)(elapsed.value() / 3600.0);
    return QTime(hours, (int)(elapsed.value() - ((double)hours * 3600.0)) / 60.0, ((uint32_t)elapsed.value()) % 60, 0);
}
metric bluetoothdevice::currentResistance() { return Resistance; }
metric bluetoothdevice::currentCadence() { return Cadence; }
double bluetoothdevice::currentCrankRevolutions() { return 0; }
uint16_t bluetoothdevice::lastCrankEventTime() { return 0; }

virtualdevice *bluetoothdevice::VirtualDevice() { return this->virtualDevice; }
void bluetoothdevice::changeResistance(resistance_t resistance) {}
void bluetoothdevice::changeInclination(double grade, double percentage) {}

void bluetoothdevice::offsetElapsedTime(int offset) { elapsed += offset; }

QTime bluetoothdevice::currentPace() {
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (Speed.value() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.value() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

QTime bluetoothdevice::averagePace() {

    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (Speed.average() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.average() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

QTime bluetoothdevice::maxPace() {

    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (Speed.max() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.max() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

double bluetoothdevice::odometerFromStartup() { return Distance.valueRaw(); }
double bluetoothdevice::odometer() { return Distance.value(); }
bool bluetoothdevice::connected() { return false; }
metric bluetoothdevice::elevationGain() { return elevationAcc; }
metric bluetoothdevice::negativeElevationGain() { return negativeElevationAcc; }
void bluetoothdevice::heartRate(uint8_t heart) {
    Heart.setValue(heart);
}
void bluetoothdevice::coreBodyTemperature(double coreBodyTemperature) { CoreBodyTemperature.setValue(coreBodyTemperature); }
void bluetoothdevice::skinTemperature(double skinTemperature) { SkinTemperature.setValue(skinTemperature); }
void bluetoothdevice::heatStrainIndex(double heatStrainIndex) { HeatStrainIndex.setValue(heatStrainIndex); }
void bluetoothdevice::rrIntervalReceived(double rrInterval) {
    // RR-interval is in milliseconds
    // Add to buffer for RMSSD calculation (keep max 30 samples for real-time HRV display)
    // Using 30 samples (~20-30 seconds of data) gives more responsive and accurate HRV
    // than using longer windows which can include heart rate transitions
    rrIntervals.append(rrInterval);
    while (rrIntervals.size() > 30) {
        rrIntervals.removeFirst();
    }

    // Calculate RMSSD when we have at least 5 RR-intervals
    if (rrIntervals.size() >= 5) {
        double sumSquaredDiff = 0.0;
        int count = 0;
        for (int i = 1; i < rrIntervals.size(); i++) {
            double diff = rrIntervals.at(i) - rrIntervals.at(i - 1);
            sumSquaredDiff += diff * diff;
            count++;
        }
        if (count > 0) {
            double rmssd = sqrt(sumSquaredDiff / count);
            HRV.setValue(rmssd);
            qDebug() << "HRV (RMSSD):" << rmssd << "ms from" << rrIntervals.size() << "RR-intervals";
        }
    }
}
void bluetoothdevice::disconnectBluetooth() {
    if (m_control) {
        m_control->disconnectFromDevice();
    }
}
void bluetoothdevice::setDifficult(double d) { m_difficult = d; }
double bluetoothdevice::difficult() { return m_difficult; }
void bluetoothdevice::setInclinationDifficult(double d) { m_inclination_difficult = d; }
double bluetoothdevice::inclinationDifficult() { return m_inclination_difficult; }
void bluetoothdevice::setDifficultOffset(double d) { m_difficult_offset = d; }
double bluetoothdevice::difficultOffset() { return m_difficult_offset; }
void bluetoothdevice::setInclinationDifficultOffset(double d) { m_inclination_difficult_offset = d; }
double bluetoothdevice::inclinationDifficultOffset() { return m_inclination_difficult_offset; }
void bluetoothdevice::cadenceSensor(uint8_t cadence) { Q_UNUSED(cadence) }
void bluetoothdevice::speedSensor(double speed) { Q_UNUSED(speed) }
void bluetoothdevice::inclinationSensor(double grade, double inclination) { Q_UNUSED(grade); Q_UNUSED(inclination) }
void bluetoothdevice::instantaneousStrideLengthSensor(double length) { Q_UNUSED(length); }
void bluetoothdevice::groundContactSensor(double groundContact) { Q_UNUSED(groundContact); }
void bluetoothdevice::verticalOscillationSensor(double verticalOscillation) { Q_UNUSED(verticalOscillation); }

bool bluetoothdevice::hasVirtualDevice() { return this->virtualDevice!=nullptr; }

void bluetoothdevice::setVirtualDevice(virtualdevice *virtualDevice, VIRTUAL_DEVICE_MODE mode) {

    if(mode!=VIRTUAL_DEVICE_MODE::NONE && !virtualDevice)
        throw "Virtual device mode should be NONE when no virtual device is specified.";

    if(this->virtualDevice)
        delete this->virtualDevice;
    this->virtualDevice = virtualDevice;
    this->virtualDeviceMode=mode;
}

void bluetoothdevice::clearStats() {

    elapsed.clear(true);
    moving.clear(true);
    Speed.clear(false);
    Distance.clear(true);
    Heart.clear(false);
    elevationAcc = 0;
    negativeElevationAcc = 0;
    Cadence.clear(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].clear(false);
    }
    for(int i=0; i<maxHeatZone(); i++) {
        heatZonesSeconds[i].clear(false);
    }    
}

void bluetoothdevice::setPaused(bool p) {

    paused = p;
    moving.setPaused(p);
    elapsed.setPaused(p);
    Speed.setPaused(p);
    Distance.setPaused(p);
    Heart.setPaused(p);
    Cadence.setPaused(p);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setPaused(p);
    }
    for(int i=0; i<maxHeatZone(); i++) {
        heatZonesSeconds[i].setPaused(p);
    }    
}

QStringList bluetoothdevice::metrics() {
    return {QStringLiteral("Speed"), QStringLiteral("Inclination"), QStringLiteral("Pace"),
            QStringLiteral("Odometer"), QStringLiteral("Elapsed"), QStringLiteral("Heart Rate")};
}

resistance_t bluetoothdevice::maxResistance() { return 100; }

void bluetoothdevice::setInclination(double inclination) { Inclination = inclination; }
void bluetoothdevice::setHeartZone(double hz) {
    HeartZone = hz;
    if(isPaused() == false && currentHeart().value() > 0) {
        hz = hz - 1;
        if(hz >= maxHeartZone() ) {
            hrZonesSeconds[maxHeartZone() - 1].setValue(hrZonesSeconds[maxHeartZone() - 1].value() + 1);
        } else if(hz <= 0) {
            hrZonesSeconds[0].setValue(hrZonesSeconds[0].value() + 1);
        } else {
            hrZonesSeconds[(int)hz].setValue(hrZonesSeconds[(int)hz].value() + 1);
        }
    }
}

void bluetoothdevice::setHeatZone(double heatStrainIndex) {
    // Determine heat zone based on Heat Strain Index values
    uint8_t zone;
    if (heatStrainIndex >= 0 && heatStrainIndex <= 1.99) {
        zone = 1;
    } else if (heatStrainIndex < 3.0) {
        zone = 2;
    } else if (heatStrainIndex < 7.0) {
        zone = 3;
    } else {
        zone = 4;
    }
    
    HeatZone = zone;
    if(isPaused() == false && heatStrainIndex > 0) {
        // Convert to 0-based index for array access
        uint8_t zoneIndex = zone - 1;
        if(zoneIndex < maxHeatZone()) {
            heatZonesSeconds[zoneIndex].setValue(heatZonesSeconds[zoneIndex].value() + 1);
        }
    }
}

uint32_t bluetoothdevice::secondsForHeartZone(uint8_t zone) {
    if(zone < maxHeartZone()) {
       return hrZonesSeconds[zone].value();
    }
    return 0;
}

uint32_t bluetoothdevice::secondsForHeatZone(uint8_t zone) {
    if(zone < maxHeatZone()) {
       return heatZonesSeconds[zone].value();
    }
    return 0;
}
