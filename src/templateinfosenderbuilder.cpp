#include "templateinfosenderbuilder.h"
#include "devices/treadmill.h"
#include "homeform.h"
#include "qzsettings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QtWebSockets/QWebSocket>
#ifdef Q_HTTPSERVER
#include "webserverinfosender.h"
#endif

TemplateInfoSenderBuilder::TemplateInfoSenderBuilder(QObject *parent) : QObject(parent) {
    connect(&updateTimer, &QTimer::timeout, this, &TemplateInfoSenderBuilder::sendTelemetry);
    updateTimer.setInterval(1000);
#ifdef Q_HTTPSERVER
    server = new WebServerInfoSender(this);
    connect(server, &WebServerInfoSender::onDataReceived, this, &TemplateInfoSenderBuilder::onDataReceived);
#endif
}

void TemplateInfoSenderBuilder::start(bluetoothdevice *dev) {
    device = dev;
    updateTimer.start();
}

void TemplateInfoSenderBuilder::stop() {
    updateTimer.stop();
    device = nullptr;
}

void TemplateInfoSenderBuilder::sendTelemetry() {
#ifdef Q_HTTPSERVER
    if (!server || !device || device->deviceType() != TREADMILL)
        return;
    auto *tread = static_cast<treadmill *>(device);
    const QTime pace = device->currentPace();
    const QTime elapsed = device->elapsedTime();
    QJsonObject values;
    values.insert(QStringLiteral("speed"), device->currentSpeed().value());
    values.insert(QStringLiteral("inclination"), tread->currentInclination().value());
    values.insert(QStringLiteral("distance"), device->odometer());
    values.insert(QStringLiteral("heart"), device->currentHeart().value());
    values.insert(QStringLiteral("pace_m"), pace.minute());
    values.insert(QStringLiteral("pace_s"), pace.second());
    values.insert(QStringLiteral("elapsed_h"), elapsed.hour());
    values.insert(QStringLiteral("elapsed_m"), elapsed.minute());
    values.insert(QStringLiteral("elapsed_s"), elapsed.second());
    values.insert(QStringLiteral("devicePaused"), device->isPaused());
    values.insert(QStringLiteral("autoresistance"), homeform::singleton() && homeform::singleton()->autoResistance());
    QJsonObject message;
    message.insert(QStringLiteral("msg"), QStringLiteral("telemetry"));
    message.insert(QStringLiteral("content"), values);
    server->send(QJsonDocument(message).toJson(QJsonDocument::Compact));
#endif
}

void TemplateInfoSenderBuilder::onDataReceived(const QByteArray &data, QWebSocket *client) {
    const QJsonDocument document = QJsonDocument::fromJson(data);
    if (!document.isObject() || !client)
        return;
    const QString command = document.object().value(QStringLiteral("msg")).toString();
    QJsonObject content;
    if (command == QStringLiteral("display_settings")) {
        QSettings settings;
        content.insert(QStringLiteral("miles_unit"),
                       settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool());
    } else if (command == QStringLiteral("speed_plus")) {
        emit speed_Plus();
    } else if (command == QStringLiteral("speed_minus")) {
        emit speed_Minus();
    } else if (command == QStringLiteral("inclination_plus")) {
        emit inclination_Plus();
    } else if (command == QStringLiteral("inclination_minus")) {
        emit inclination_Minus();
    } else if (command == QStringLiteral("start")) {
        emit Start();
    } else if (command == QStringLiteral("pause")) {
        emit Pause();
    } else if (command == QStringLiteral("stop")) {
        emit Stop();
    } else if (command == QStringLiteral("autoresistance")) {
        emit autoResistance();
    } else if (command == QStringLiteral("floating_close")) {
        emit floatingClose();
    } else {
        return;
    }
    QJsonObject reply;
    reply.insert(QStringLiteral("msg"), QStringLiteral("R_") + command);
    reply.insert(QStringLiteral("content"), content);
    client->sendTextMessage(QString::fromUtf8(QJsonDocument(reply).toJson(QJsonDocument::Compact)));
}
