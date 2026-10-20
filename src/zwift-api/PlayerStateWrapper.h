#ifndef PLAYERSTATEWRAPPER_H
#define PLAYERSTATEWRAPPER_H

#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>

class ZwiftRequest: public QObject {
    Q_OBJECT

public:
    ZwiftRequest(const QString& getAccessToken) : getAccessToken(getAccessToken) {}

    QString json(const QString& url) {
        QNetworkRequest request(QUrl(BASE_URL + url));
        request.setRawHeader("Accept", "application/json");
        request.setRawHeader("Authorization", "Bearer " + getAccessToken.toUtf8());

        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error: " << reply->errorString();
            return "";
        }

        return reply->readAll();
    }

    QByteArray protobuf(const QString& url) {
        QNetworkRequest request(QUrl(BASE_URL + url));
        request.setRawHeader("Accept", "application/x-protobuf-lite");
        request.setRawHeader("Authorization", "Bearer " + getAccessToken.toUtf8());

        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error: " << reply->errorString();
            return QByteArray();
        }

        return reply->readAll();
    }

private:
    QNetworkAccessManager manager;
    const QString BASE_URL = "https://us-or-rly101.zwift.com";
    const QString getAccessToken;
};

class World {
public:    
    World(int worldId, const QString& getAccessToken) : worldId(worldId), request(getAccessToken) {}

    QString getPlayers() {
        return request.json("/relay/worlds/" + QString::number(worldId));
    }

    QByteArray playerStatus(int playerId) {
        QByteArray buffer = request.protobuf("/relay/worlds/" + QString::number(worldId) + "/players/" + QString::number(playerId));
        return buffer;
    }

    QString player_id() {
        return request.json("/api/profiles/me");
    }

private:
    int worldId;
    ZwiftRequest request;
};

#endif // PLAYERSTATEWRAPPER_H
