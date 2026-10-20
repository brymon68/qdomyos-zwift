#include "osc.h"

OSC::OSC(bluetooth* manager, QObject *parent)
    : QObject{parent}
{
    bluetoothManager = manager;
    // Setup timer for periodic publishing
    m_timer = new QTimer();
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &OSC::publishWorkoutData);

    m_timer->start();
}

void OSC::publishWorkoutData() {
    if(!bluetoothManager->device()) return;
    QSettings settings;
    QString OSC_ip = settings.value(QZSettings::OSC_ip, QZSettings::default_OSC_ip).toString();
    int OSC_port = settings.value(QZSettings::OSC_port, QZSettings::default_OSC_port).toInt();
    char osc_buffer[3000];
    int osc_len = OSC_makePacket(osc_buffer, sizeof(osc_buffer));
    int osc_ret_len = OSC_sendSocket->writeDatagram(osc_buffer, osc_len, QHostAddress(OSC_ip), OSC_port);
    qDebug() << "OSC >> " << osc_ret_len << QByteArray::fromRawData(osc_buffer, osc_len).toHex(' ');
}

size_t OSC::OSC_makePacket(void* buffer, size_t size)
{
    // Construct a packet
    OSCPP::Client::Packet packet(buffer, size);
    packet
           // Open a bundle with a timetag
        .openBundle(1234ULL)
                             // Add a message with two arguments and an array with 6 elements;
                             // for efficiency this needs to be known in advance.

        .openMessage("/QZ/Heart", 1)
        .int32(bluetoothManager->device()->currentHeart().value())
        .closeMessage()

        .openMessage("/QZ/Speed", 1)
        .float32(bluetoothManager->device()->currentSpeed().value())
        .closeMessage()

        .openMessage("/QZ/Pace", 1)
        .string(bluetoothManager->device()->currentPace().toString(QStringLiteral("m:ss")).toLatin1())
        .closeMessage()

        .openMessage("/QZ/Inclination", 1)
        .float32(bluetoothManager->device()->currentInclination().value())
        .closeMessage()

        .openMessage("/QZ/AveragePace", 1)
        .string(bluetoothManager->device()->averagePace().toString(QStringLiteral("m:ss")).toLatin1())
        .closeMessage()

        .openMessage("/QZ/MaxPace", 1)
        .string(bluetoothManager->device()->maxPace().toString(QStringLiteral("m:ss")).toLatin1())
        .closeMessage()

        .openMessage("/QZ/Odometer", 1)
        .float32(bluetoothManager->device()->odometer())
        .closeMessage()

        .openMessage("/QZ/OdometerFromStartup", 1)
        .float32(bluetoothManager->device()->odometerFromStartup())
        .closeMessage()

        .openMessage("/QZ/Distance", 1)
        .float32(bluetoothManager->device()->currentDistance().value())
        .closeMessage()

        .openMessage("/QZ/Distance1s", 1)
        .float32(bluetoothManager->device()->currentDistance1s().value())
        .closeMessage()

        .openMessage("/QZ/ElapsedTime", 1)
        .string(bluetoothManager->device()->elapsedTime().toString(QStringLiteral("m:ss")).toLatin1())
        .closeMessage()

        .openMessage("/QZ/MovingTime", 1)
        .string(bluetoothManager->device()->movingTime().toString(QStringLiteral("m:ss")).toLatin1())
        .closeMessage()

        .openMessage("/QZ/LapElapsedTime", 1)
        .string(bluetoothManager->device()->lapElapsedTime().toString(QStringLiteral("m:ss")).toLatin1())
        .closeMessage()

        .openMessage("/QZ/Connected", 1)
        .int32(bluetoothManager->device()->connected())
        .closeMessage()

        .openMessage("/QZ/Cadence", 1)
        .float32(bluetoothManager->device()->currentCadence().value())
        .closeMessage()

        .openMessage("/QZ/Coordinate", 2)
        .float32(bluetoothManager->device()->currentCordinate().latitude())
        .float32(bluetoothManager->device()->currentCordinate().longitude())
        .closeMessage()

        .openMessage("/QZ/Azimuth", 1)
        .float32(bluetoothManager->device()->currentAzimuth())
        .closeMessage()

        .openMessage("/QZ/AverageAzimuthNext300m", 1)
        .float32(bluetoothManager->device()->averageAzimuthNext300m())
        .closeMessage()

        .openMessage("/QZ/ElevationGain", 1)
        .float32(bluetoothManager->device()->elevationGain().value())
        .closeMessage()

        .openMessage("/QZ/Paused", 1)
        .int32(bluetoothManager->device()->isPaused())
        .closeMessage()

        .openMessage("/QZ/AutoResistance", 1)
        .int32(bluetoothManager->device()->autoResistance())
        .closeMessage()

        .openMessage("/QZ/Difficulty", 1)
        .float32(bluetoothManager->device()->difficult())
        .closeMessage()

        .openMessage("/QZ/InclinationDifficulty", 1)
        .float32(bluetoothManager->device()->inclinationDifficult())
        .closeMessage()

        .openMessage("/QZ/DifficultyOffset", 1)
        .float32(bluetoothManager->device()->difficultOffset())
        .closeMessage()

        .openMessage("/QZ/InclinationDifficultyOffset", 1)
        .float32(bluetoothManager->device()->inclinationDifficultOffset())
        .closeMessage()

        .openMessage("/QZ/DeviceType", 1)
        .int32(bluetoothManager->device()->deviceType())
        .closeMessage()

        .closeBundle();
    return packet.size();
}
