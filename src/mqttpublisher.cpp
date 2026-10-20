#include "mqttpublisher.h"
#include "qzsettings.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>

MQTTPublisher::MQTTPublisher(const QString& host, quint16 port, QString username, QString password, bluetooth* manager, QObject *parent)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
    , m_device(nullptr)
{
    m_client = new QMqttClient(this);
    m_timer = new QTimer(this);
    m_manager = manager;
    m_username = username;
    m_password = password;
    m_deviceId = getDeviceId();

    // Setup timer for periodic publishing
    m_timer->setInterval(500); // 500ms interval
    connect(m_timer, &QTimer::timeout, this, &MQTTPublisher::publishWorkoutData);

    // Setup MQTT client connections
    connect(m_client, &QMqttClient::connected, this, &MQTTPublisher::onConnected);
    connect(m_client, &QMqttClient::disconnected, this, &MQTTPublisher::onDisconnected);
    connect(m_client, &QMqttClient::errorChanged, this, &MQTTPublisher::onError);
    connect(m_client, &QMqttClient::messageReceived, this, &MQTTPublisher::onMessageReceived);
    if (m_manager) {
        connect(m_manager, &bluetooth::bluetoothDeviceConnected, this, &MQTTPublisher::onBluetoothDeviceConnected);
        connect(m_manager, &bluetooth::bluetoothDeviceDisconnected, this, &MQTTPublisher::onBluetoothDeviceDisconnected);
        m_device = m_manager->device();
    }

    setupMQTTClient();
    start();
}

void MQTTPublisher::setupLastWillMessage() {
    QString statusTopic = getStatusTopic() + STATUS_TOPIC;

    // Set up Last Will and Testament message
    m_client->setWillTopic(statusTopic);
    m_client->setWillMessage(STATUS_OFFLINE.toUtf8());
    m_client->setWillQoS(1);
    m_client->setWillRetain(true);
}

void MQTTPublisher::publishOnlineStatus() {
    QString statusTopic = getStatusTopic() + STATUS_TOPIC;

    // Publish online status with retain flag
    m_client->publish(
        QMqttTopicName(statusTopic),
        STATUS_ONLINE.toUtf8(),
        1,  // QoS 1
        true  // retain flag
    );
}


MQTTPublisher::~MQTTPublisher() {
    stop();
}

void MQTTPublisher::setDevice(bluetoothdevice* device) {
    m_device = device;
}

void MQTTPublisher::onBluetoothDeviceConnected(bluetoothdevice *device) {
    m_device = device;
}

void MQTTPublisher::onBluetoothDeviceDisconnected() {
    m_device = nullptr;
}

QString MQTTPublisher::getDeviceId() const {
    QSettings settings;
    return settings.value(QZSettings::mqtt_deviceid, QZSettings::default_mqtt_deviceid).toString();
}

QString MQTTPublisher::getStatusTopic() const {
    return QString("QZ/%1/").arg(m_deviceId);
}

QString MQTTPublisher::getBaseTopic() const {
    return QString("QZ/%1/workout/").arg(m_deviceId);
}

QString MQTTPublisher::getControlTopic() const {
    return QString("QZ/%1/control/").arg(m_deviceId);
}

void MQTTPublisher::setupMQTTClient() {
    m_client->setHostname(m_host);
    m_client->setPort(m_port);
    if(m_username.length())
        m_client->setUsername(m_username);
    if(m_password.length())
        m_client->setPassword(m_password);

    setupLastWillMessage();
}

void MQTTPublisher::start() {
    connectToHost();
    m_timer->start();
}

void MQTTPublisher::stop() {
    m_timer->stop();
    if (m_client->state() == QMqttClient::Connected) {
        //m_client->disconnect();
    }
}

void MQTTPublisher::setHost(const QString& host) {
    m_host = host;
    m_client->setHostname(host);
}

void MQTTPublisher::setPort(quint16 port) {
    m_port = port;
    m_client->setPort(port);
}

bool MQTTPublisher::isConnected() const {
    return m_client->state() == QMqttClient::Connected;
}

void MQTTPublisher::connectToHost() {
    if (m_client->state() == QMqttClient::Disconnected) {
        m_client->connectToHost();
    }
}

void MQTTPublisher::onDisconnected() {
    qDebug() << "MQTT Client Disconnected";
}

void MQTTPublisher::onError(QMqttClient::ClientError error) {
    qDebug() << "MQTT Client Error:" << error;
}

bool MQTTPublisher::hasValueChanged(const QString& topic, const QVariant& newValue) {
    if (!m_lastPublishedValues.contains(topic)) {
        return true;
    }

    const QVariant& oldValue = m_lastPublishedValues[topic];

    // Handle floating point comparison with small epsilon
    if (newValue.userType() == QMetaType::Float || newValue.userType() == QMetaType::Double) {
        const double epsilon = 0.0001;
        return qAbs(newValue.toDouble() - oldValue.toDouble()) > epsilon;
    }

    return oldValue != newValue;
}

void MQTTPublisher::updateLastPublishedValue(const QString& topic, const QVariant& value) {
    m_lastPublishedValues[topic] = value;
}

void MQTTPublisher::publishToTopic(const QString& topic, const QVariant& value) {
    if (!isConnected()) return;

    QString fullTopic = getBaseTopic() + topic;

    // Only publish if value has changed
    if (!hasValueChanged(fullTopic, value)) {
        return;
    }

    QString payload;
    if (value.userType() == QMetaType::QJsonObject || value.userType() == QMetaType::QJsonArray) {
        payload = QJsonDocument::fromVariant(value).toJson(QJsonDocument::Compact);
    } else {
        switch (value.userType()) {
        case QMetaType::Bool:
            payload = value.toBool() ? "true" : "false";
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Long:
        case QMetaType::LongLong:
        case QMetaType::ULong:
        case QMetaType::ULongLong:
            payload = QString::number(value.toLongLong());
            break;
        case QMetaType::Float:
        case QMetaType::Double:
            payload = QString::number(value.toDouble(), 'f', 2);
            break;
        default:
            payload = value.toString();
            break;
        }
    }

    m_client->publish(QMqttTopicName(fullTopic), payload.toUtf8());
    updateLastPublishedValue(fullTopic, value);
}

void MQTTPublisher::onConnected() {
    qDebug() << "MQTT Client Connected";
    m_lastPublishedValues.clear();  // Reset stored values
    publishOnlineStatus();
    subscribeToControlTopics();

    // Delay discovery config to allow device initialization
    QTimer::singleShot(2000, this, &MQTTPublisher::publishDiscoveryConfig);
}

void MQTTPublisher::subscribeToControlTopics() {
    if(!isConnected()) return;

    QString controlTopic = getControlTopic() + "+";
    QString deviceControlTopic = getControlTopic() + "+/+";

    qDebug() << "Subscribing to control topics:" << controlTopic << deviceControlTopic;

    m_client->subscribe(QMqttTopicFilter(controlTopic), 1);
    m_client->subscribe(QMqttTopicFilter(deviceControlTopic), 1);
}

void MQTTPublisher::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic) {
    QString topicName = topic.name();
    QString controlPrefix = getControlTopic();

    if(!topicName.startsWith(controlPrefix)) {
        return;
    }

    QString command = topicName.mid(controlPrefix.length());
    QVariant value = QString::fromUtf8(message);

    qDebug() << "Received control command:" << command << "value:" << value.toString();

    handleControlCommand(command, value);
}

void MQTTPublisher::handleControlCommand(const QString& command, const QVariant& value) {
    if(!m_device) return;

    QStringList parts = command.split('/');
    if(parts.isEmpty()) return;

    // Handle treadmill speed and inclination commands.
    if(parts.size() == 2) {
        processDeviceCommand(parts[0], parts[1], value);
        return;
    }

    // Handle general commands
    QString mainCommand = parts[0];

    if(mainCommand == "start") {
        m_device->start();
    } else if(mainCommand == "stop") {
        m_device->stop(false);
    } else if(mainCommand == "pause") {
        m_device->setPaused(value.toBool());
    } else if(mainCommand == "inclination") {
        m_device->changeInclination(value.toDouble(), value.toDouble());
    }
}

void MQTTPublisher::processDeviceCommand(const QString& deviceType, const QString& command, const QVariant& value) {
    if(!m_device) return;

    if(deviceType == "treadmill" && m_device->deviceType() == TREADMILL) {
        treadmill* treadDevice = static_cast<treadmill*>(m_device.data());

        if(command == "speed") {
            treadDevice->changeSpeed(value.toDouble());
        } else if(command == "inclination") {
            treadDevice->changeInclination(value.toDouble(), value.toDouble());
        }

    }
}

QJsonObject MQTTPublisher::getDeviceInfo() const {
    QJsonObject device;

    if (m_device) {
        device["identifiers"] = QJsonArray{m_device->bluetoothDevice.address().toString()};
        device["name"] = QString("QZ Fitness Device (%1)").arg(m_deviceId);
        device["model"] = m_device->bluetoothDevice.name();
        device["manufacturer"] = "QDomyos-Zwift";
        device["sw_version"] = QCoreApplication::applicationVersion();
    } else {
        device["identifiers"] = QJsonArray{m_deviceId};
        device["name"] = QString("QZ Fitness Device (%1)").arg(m_deviceId);
        device["manufacturer"] = "QDomyos-Zwift";
        device["sw_version"] = QCoreApplication::applicationVersion();
    }

    return device;
}

QString MQTTPublisher::getDiscoveryTopic(const QString& component, const QString& objectId) const {
    return QString("homeassistant/%1/qz_%2_%3/config").arg(component, m_deviceId, objectId);
}

void MQTTPublisher::publishSensorDiscovery(const QString& sensorType, const QString& name, const QString& stateTopic, const QString& unit, const QString& deviceClass, const QString& icon) {
    if (!isConnected()) return;

    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_deviceId, sensorType);
    config["state_topic"] = stateTopic;
    config["device"] = getDeviceInfo();

    if (!unit.isEmpty()) config["unit_of_measurement"] = unit;
    if (!deviceClass.isEmpty()) config["device_class"] = deviceClass;
    if (!icon.isEmpty()) config["icon"] = icon;

    QString topic = getDiscoveryTopic("sensor", sensorType);
    QJsonDocument doc(config);

    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::publishBinarySensorDiscovery(const QString& sensorType, const QString& name, const QString& stateTopic, const QString& deviceClass, const QString& icon) {
    if (!isConnected()) return;

    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_deviceId, sensorType);
    config["state_topic"] = stateTopic;
    config["device"] = getDeviceInfo();
    config["payload_on"] = "true";
    config["payload_off"] = "false";

    if (!deviceClass.isEmpty()) config["device_class"] = deviceClass;
    if (!icon.isEmpty()) config["icon"] = icon;

    QString topic = getDiscoveryTopic("binary_sensor", sensorType);
    QJsonDocument doc(config);

    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::publishNumberDiscovery(const QString& entityType, const QString& name, const QString& stateTopic, const QString& commandTopic, double min, double max, double step, const QString& unit, const QString& icon) {
    if (!isConnected()) return;

    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_deviceId, entityType);
    config["state_topic"] = stateTopic;
    config["command_topic"] = commandTopic;
    config["device"] = getDeviceInfo();
    config["min"] = min;
    config["max"] = max;
    config["step"] = step;
    config["mode"] = "box";

    if (!unit.isEmpty()) config["unit_of_measurement"] = unit;
    if (!icon.isEmpty()) config["icon"] = icon;

    QString topic = getDiscoveryTopic("number", entityType);
    QJsonDocument doc(config);

    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::publishSwitchDiscovery(const QString& entityType, const QString& name, const QString& stateTopic, const QString& commandTopic, const QString& icon) {
    if (!isConnected()) return;

    QJsonObject config;
    config["name"] = name;
    config["unique_id"] = QString("qz_%1_%2").arg(m_deviceId, entityType);
    config["state_topic"] = stateTopic;
    config["command_topic"] = commandTopic;
    config["device"] = getDeviceInfo();
    config["payload_on"] = "true";
    config["payload_off"] = "false";

    if (!icon.isEmpty()) config["icon"] = icon;

    QString topic = getDiscoveryTopic("switch", entityType);
    QJsonDocument doc(config);

    m_client->publish(QMqttTopicName(topic), doc.toJson(QJsonDocument::Compact), 1, true);
}

void MQTTPublisher::removeDiscoveryConfig() {
    if (!isConnected()) return;

    // Remove all discovery configs by publishing empty messages
    QStringList components = {"sensor", "binary_sensor", "number", "switch", "button"};
    QStringList entities = {
        "speed_current", "speed_avg", "distance", "elapsed_time", "elapsed_total_seconds", "heart_current", "heart_avg",
        "connected", "paused", "resistance", "cadence", "inclination",
        "start", "stop", "pause"
    };

    for (const QString& component : components) {
        for (const QString& entity : entities) {
            QString topic = getDiscoveryTopic(component, entity);
            m_client->publish(QMqttTopicName(topic), QByteArray(), 1, true);
        }
    }
}

void MQTTPublisher::publishWorkoutData() {
    if (m_manager) {
        bluetoothdevice *currentDevice = m_manager->device();
        if (currentDevice != m_device.data()) {
            m_device = currentDevice;
        }
    }

    if (!isConnected() || !m_device) return;

    // Device Information
    publishToTopic("device/id", QVariant(m_device->bluetoothDevice.address().toString()));
    publishToTopic("device/name", QVariant(m_device->bluetoothDevice.name()));
    publishToTopic("device/rssi", m_device->bluetoothDevice.rssi());
    publishToTopic("device/type", static_cast<int>(m_device->deviceType()));
    publishToTopic("device/connected", m_device->connected());
    publishToTopic("device/paused", m_device->isPaused());

    // Time Metrics
    QTime elapsedTime = m_device->elapsedTime();
    publishToTopic("elapsed/seconds", elapsedTime.second());
    publishToTopic("elapsed/minutes", elapsedTime.minute());
    publishToTopic("elapsed/hours", elapsedTime.hour());
    publishToTopic("elapsed/total_seconds", QTime(0, 0).secsTo(elapsedTime));

    QTime lapTime = m_device->lapElapsedTime();
    publishToTopic("lap/elapsed/seconds", lapTime.second());
    publishToTopic("lap/elapsed/minutes", lapTime.minute());
    publishToTopic("lap/elapsed/hours", lapTime.hour());

    // Current Pace
    QTime currentPace = m_device->currentPace();
    publishToTopic("pace/current/seconds", currentPace.second());
    publishToTopic("pace/current/minutes", currentPace.minute());
    publishToTopic("pace/current/hours", currentPace.hour());

    // Average Pace
    QTime avgPace = m_device->averagePace();
    publishToTopic("pace/avg/seconds", avgPace.second());
    publishToTopic("pace/avg/minutes", avgPace.minute());
    publishToTopic("pace/avg/hours", avgPace.hour());

    // Moving Time
    QTime movingTime = m_device->movingTime();
    publishToTopic("moving/seconds", movingTime.second());
    publishToTopic("moving/minutes", movingTime.minute());
    publishToTopic("moving/hours", movingTime.hour());

    // Basic Metrics
    publishToTopic("speed/current", m_device->currentSpeed().value());
    publishToTopic("speed/avg", m_device->currentSpeed().average());
    publishToTopic("speed/lap_avg", m_device->currentSpeed().lapAverage());
    publishToTopic("speed/lap_max", m_device->currentSpeed().lapMax());

    publishToTopic("distance", m_device->odometer());
    publishToTopic("elevation", m_device->elevationGain().value());
    publishToTopic("difficult", m_device->difficult());

    // Heart Rate
    metric heart = m_device->currentHeart();
    publishToTopic("heart/current", heart.value());
    publishToTopic("heart/avg", heart.average());
    publishToTopic("heart/lap_avg", heart.lapAverage());
    publishToTopic("heart/max", heart.max());
    publishToTopic("heart/lap_max", heart.lapMax());

    // Location Data
    QGeoCoordinate coord = m_device->currentCordinate();
    if (coord.isValid()) {
        publishToTopic("location/latitude", coord.latitude());
        publishToTopic("location/longitude", coord.longitude());
        publishToTopic("location/altitude", coord.altitude());
    }

    // Treadmill telemetry
    treadmill* treadDevice = static_cast<treadmill*>(m_device.data());
    publishToTopic("treadmill/target_speed", treadDevice->lastRequestedSpeed().value());
    publishToTopic("treadmill/target_inclination", treadDevice->lastRequestedInclination().value());

    metric inclination = treadDevice->currentInclination();
    publishToTopic("treadmill/inclination/current", inclination.value());
    publishToTopic("treadmill/inclination/avg", inclination.average());
    publishToTopic("treadmill/inclination/lap_avg", inclination.lapAverage());
    publishToTopic("treadmill/inclination/lap_max", inclination.lapMax());

    metric cadence = m_device->currentCadence();
    publishToTopic("treadmill/cadence/current", cadence.value());
    publishToTopic("treadmill/cadence/avg", cadence.average());
    publishToTopic("treadmill/cadence/lap_avg", cadence.lapAverage());
    publishToTopic("treadmill/cadence/lap_max", cadence.lapMax());

    publishToTopic("treadmill/stride_length", treadDevice->currentStrideLength().value());
    publishToTopic("treadmill/ground_contact", treadDevice->currentGroundContact().value());
    publishToTopic("treadmill/vertical_oscillation", treadDevice->currentVerticalOscillation().value());
}

void MQTTPublisher::publishDiscoveryConfig() {
    if (!isConnected()) return;

    qDebug() << "Publishing Home Assistant discovery configuration";

    QString baseTopic = getBaseTopic();
    QString controlTopic = getControlTopic();

    // Common sensors for all devices
    publishSensorDiscovery("speed_current", "Speed", baseTopic + "speed/current", "km/h", "", "mdi:speedometer");
    publishSensorDiscovery("speed_avg", "Average Speed", baseTopic + "speed/avg", "km/h", "", "mdi:speedometer");
    publishSensorDiscovery("distance", "Distance", baseTopic + "distance", "km", "distance", "mdi:map-marker-distance");
    publishSensorDiscovery("elapsed_time", "Elapsed Time", baseTopic + "elapsed/minutes", "min", "duration", "mdi:timer");
    publishSensorDiscovery("elapsed_total_seconds", "Elapsed Total Seconds", baseTopic + "elapsed/total_seconds", "s", "duration", "mdi:timer");
    publishSensorDiscovery("heart_current", "Heart Rate", baseTopic + "heart/current", "bpm", "", "mdi:heart-pulse");
    publishSensorDiscovery("heart_avg", "Average Heart Rate", baseTopic + "heart/avg", "bpm", "", "mdi:heart-pulse");
    publishSensorDiscovery("elevation", "Elevation Gain", baseTopic + "elevation", "m", "", "mdi:elevation-rise");

    // Binary sensors
    QString statusBaseTopic = QString("QZ/%1/").arg(m_deviceId);
    publishBinarySensorDiscovery("connected", "Device Connected", statusBaseTopic + "device/connected", "connectivity", "mdi:bluetooth");
    publishBinarySensorDiscovery("paused", "Workout Paused", statusBaseTopic + "device/paused", "", "mdi:pause");

    // Treadmill controls
    if (m_device) {
        // Treadmill-specific sensors
        publishSensorDiscovery("treadmill_inclination", "Inclination", baseTopic + "treadmill/inclination/current", "%", "", "mdi:angle-acute");
        publishSensorDiscovery("treadmill_cadence", "Cadence", baseTopic + "treadmill/cadence/current", "spm", "", "mdi:run");
        publishSensorDiscovery("stride_length", "Stride Length", baseTopic + "treadmill/stride_length", "cm", "", "mdi:ruler");

        // Treadmill controls
        publishNumberDiscovery("treadmill_speed", "Treadmill Speed", baseTopic + "treadmill/target_speed", controlTopic + "treadmill/speed", 0, 25, 0.1, "km/h", "mdi:speedometer");
        publishNumberDiscovery("treadmill_inclination", "Treadmill Inclination", baseTopic + "treadmill/inclination/current", controlTopic + "treadmill/inclination", -10, 40, 0.5, "%", "mdi:angle-acute");
    }

    // Common control entities

    // Control buttons
    QJsonObject startConfig;
    startConfig["name"] = "Start Workout";
    startConfig["unique_id"] = QString("qz_%1_start").arg(m_deviceId);
    startConfig["command_topic"] = controlTopic + "start";
    startConfig["device"] = getDeviceInfo();
    startConfig["icon"] = "mdi:play";
    QString startTopic = getDiscoveryTopic("button", "start");
    m_client->publish(QMqttTopicName(startTopic), QJsonDocument(startConfig).toJson(QJsonDocument::Compact), 1, true);

    QJsonObject stopConfig;
    stopConfig["name"] = "Stop Workout";
    stopConfig["unique_id"] = QString("qz_%1_stop").arg(m_deviceId);
    stopConfig["command_topic"] = controlTopic + "stop";
    stopConfig["device"] = getDeviceInfo();
    stopConfig["icon"] = "mdi:stop";
    QString stopTopic = getDiscoveryTopic("button", "stop");
    m_client->publish(QMqttTopicName(stopTopic), QJsonDocument(stopConfig).toJson(QJsonDocument::Compact), 1, true);

    // Pause switch
    publishSwitchDiscovery("pause", "Pause Workout", statusBaseTopic + "device/paused", controlTopic + "pause", "mdi:pause");

    qDebug() << "Home Assistant discovery configuration published";
}
