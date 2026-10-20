#include "webserverinfosender.h"
#include <QDebug>
#include <QSettings>
#include <QtWebSockets/QWebSocket>

WebServerInfoSender::WebServerInfoSender(QObject *parent) : QObject(parent) {
    httpServer = new QHttpServer(this);
    tcpServer = new QTcpServer(this);
    httpServer->route(QStringLiteral("/floating/<arg>"), [](const QUrl &url) {
        const QString file = url.path();
        const QStringList files = {
            QStringLiteral("floating.htm"), QStringLiteral("hfloating.htm"),
            QStringLiteral("jquery-3.6.0.min.js"), QStringLiteral("globals.js"),
            QStringLiteral("main_ws_manager.js"), QStringLiteral("radikalmedium.otf")
        };
        if (!files.contains(file))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        return QHttpServerResponse::fromFile(QStringLiteral(":/inner_templates/floating/") + file);
    });
    connect(httpServer, &QHttpServer::newWebSocketConnection, this, &WebServerInfoSender::onNewConnection);
    retryTimer.setSingleShot(true);
    retryTimer.setInterval(5000);
    connect(&retryTimer, &QTimer::timeout, this, &WebServerInfoSender::listen);
    listen();
}

void WebServerInfoSender::listen() {
    if (tcpServer->isListening())
        return;
    if (!tcpServer->listen(QHostAddress::Any, 0)) {
        qWarning() << "Floating telemetry server failed to listen" << tcpServer->errorString();
        retryTimer.start();
        return;
    }
    httpServer->bind(tcpServer);
    QSettings settings;
    settings.setValue(QStringLiteral("template_inner_QZWS_port"), tcpServer->serverPort());
    qDebug() << "WebServer listening on port" << tcpServer->serverPort();
}

void WebServerInfoSender::onNewConnection() {
    QWebSocket *client = httpServer->nextPendingWebSocketConnection();
    if (!client)
        return;
    if (client->requestUrl().path() != QStringLiteral("/floating-ws")) {
        client->close();
        client->deleteLater();
        return;
    }
    clients.append(client);
    connect(client, &QWebSocket::textMessageReceived, this, [this, client](const QString &data) {
        emit onDataReceived(data.toUtf8(), client);
    });
    connect(client, &QWebSocket::disconnected, this, [this, client]() {
        clients.removeAll(client);
        client->deleteLater();
    });
}

void WebServerInfoSender::send(const QByteArray &data) {
    const QString message = QString::fromUtf8(data);
    for (const auto &client : clients) {
        if (client)
            client->sendTextMessage(message);
    }
}
