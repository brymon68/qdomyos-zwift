#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <stdexcept>

// Exercise the production encoders and control-point handler without a Bluetooth
// adapter or the complete device graph. Only the device/platform interfaces are
// doubled; packet construction and command dispatch come from the .cpp files below.
#define CHARACTERISTICNOTIFIER2AD2_H
#define CHARACTERISTICNOTIFIES2ACC_H
#define CHARACTERISTICWRITEPROCESSOR2AD9_H
#define TREADMILL_H

constexpr int CN_OK = 0, CN_INVALID = -1, CP_OK = 0, CP_INVALID = -1;
using resistance_t = double;
enum BLUETOOTH_TYPE { BIKE, TREADMILL, ELLIPTICAL, ROWING };
struct metric {
    double reading;
    double value() const { return reading; }
};
struct bluetoothdevice {
    int powerReads = 0;
    BLUETOOTH_TYPE deviceType() const { return TREADMILL; }
    metric currentSpeed() const { return {12.34}; }
    metric currentCadence() const { return {90}; }
    metric currentResistance() const { return {0}; }
    metric currentHeart() const { return {137}; }
    double wattsMetricforUI() { ++powerReads; return 250; }
    void changeResistance(double) {}
};
struct treadmill : bluetoothdevice {
    double targetSpeed = -1, targetIncline = -1;
    void changeSpeed(double speed) { targetSpeed = speed; }
    void changeInclination(double grade, double) { targetIncline = grade; }
};
struct QLowEnergyCharacteristic {};
struct CharacteristicNotifier {
    CharacteristicNotifier(int, QObject *) {}
};
struct CharacteristicNotifier2AD2 : CharacteristicNotifier {
    bluetoothdevice *Bike;
    CharacteristicNotifier2AD2(bluetoothdevice *, QObject * = nullptr);
    int notify(QByteArray &);
};
struct CharacteristicNotifier2ACC : CharacteristicNotifier {
    bluetoothdevice *Bike;
    CharacteristicNotifier2ACC(bluetoothdevice *, QObject * = nullptr);
    int notify(QByteArray &);
};
struct CharacteristicNotifier2AD9 { QByteArray answer; };
struct CharacteristicWriteProcessor {
    bluetoothdevice *Bike;
    int powerChanges = 0, slopeChanges = 0;
    int16_t lastSlope = 0;
    CharacteristicWriteProcessor(bluetoothdevice *device, QObject *) : Bike(device) {}
    void changePower(uint16_t) { ++powerChanges; }
    void changeSlope(int16_t slope, uint8_t, uint8_t) { ++slopeChanges; lastSlope = slope; }
};
struct CharacteristicWriteProcessor2AD9 : CharacteristicWriteProcessor {
    CharacteristicNotifier2AD9 *notifier;
    int forwards = 0;
    CharacteristicWriteProcessor2AD9(bluetoothdevice *, CharacteristicNotifier2AD9 *, QObject * = nullptr);
    int writeProcess(quint16, const QByteArray &, QByteArray &);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &, const QByteArray &) { ++forwards; }
};

#include "../../src/characteristics/characteristicnotifier2ad2.cpp"
#include "../../src/characteristics/characteristicnotifier2acc.cpp"
#include "../../src/characteristics/characteristicwriteprocessor2ad9.cpp"

static void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

int main(int argc, char **argv) {
    QCoreApplication application(argc, argv);
    QCoreApplication::setOrganizationName("X22iProtocolTest");
    QCoreApplication::setApplicationName("Protocol");
    QTemporaryDir settingsDirectory;
    require(settingsDirectory.isValid(), "Cannot isolate protocol test settings");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());

    treadmill device;
    CharacteristicNotifier2AD2 dataNotifier(&device);
    QByteArray packet;
    require(dataNotifier.notify(packet) == CN_OK, "Treadmill packet was not generated");
    require(packet == QByteArray::fromHex("2402d204b40000008900"),
            "Treadmill packet must contain speed, cadence, resistance and heart rate, with no power field");
    require(device.powerReads == 0, "Treadmill packet read a power measurement");

    CharacteristicNotifier2ACC featureNotifier(&device);
    QByteArray features;
    require(featureNotifier.notify(features) == CN_OK && features.size() == 8, "Invalid feature packet");
    require((features.at(1) & 0x40) == 0 && (features.at(4) & 0x08) == 0,
            "Treadmill must not advertise power measurement or a power target");
    require((features.at(4) & 0x03) == 0x03, "Speed and incline targets must remain supported");

    CharacteristicNotifier2AD9 responseNotifier;
    CharacteristicWriteProcessor2AD9 control(&device, &responseNotifier);
    for (const char *request : {"05fa00", "05"}) {
        QByteArray response;
        require(control.writeProcess(0x2ad9, QByteArray::fromHex(request), response) == CP_OK,
                "Unsupported power request did not return a protocol response");
        require(response == QByteArray::fromHex("800502") && responseNotifier.answer == response,
                "Power request must return opcode-not-supported");
        require(control.powerChanges == 0 && control.forwards == 0 &&
                    device.targetSpeed == -1 && device.targetIncline == -1,
                "Power request triggered a device action");
    }

    const char *motionRequests[] = {"027003", "033200", "110000f4012833", "1100000cfe2833"};
    for (const char *request : motionRequests) {
        QByteArray response;
        const QByteArray command = QByteArray::fromHex(request);
        require(control.writeProcess(0x2ad9, command, response) == CP_OK &&
                    response == QByteArray::fromHex("80") + command.left(1) + QByteArray::fromHex("01"),
                "Motion command must remain accepted");
        if (command.at(0) == 0x11)
            require(control.lastSlope == (control.slopeChanges == 1 ? 500 : -500),
                    "Terrain grade changed while removing power support");
    }
    require(device.targetSpeed == 8.8 && device.targetIncline == 5.0 &&
                control.slopeChanges == 2 && control.forwards == 4 && control.powerChanges == 0,
            "Speed, incline and terrain commands were not dispatched correctly");
}
