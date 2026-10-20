#include "zwiftincline.h"
#include "devices/bluetooth.h"
#include "qzsettings.h"
#include "zwift-api/PlayerStateWrapper.h"
#include "zwift-api/zwift_client_auth.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopedValueRollback>
#include <QSettings>

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#endif

ZwiftIncline::ZwiftIncline(bluetooth *manager, QObject *parent)
    : QObject(parent), bluetoothManager(manager) {
    QSettings settings;
    const QString username =
        settings.value(QZSettings::zwift_username, QZSettings::default_zwift_username).toString();
    if (!username.isEmpty()) {
        authToken = new AuthToken(
            username, settings.value(QZSettings::zwift_password, QZSettings::default_zwift_password).toString(), this);
        connect(authToken, &AuthToken::tokenReceived, this, [this](bool, const QString &message) {
            if (!loginToastShown) {
                emit toastRequest(message);
                loginToastShown = true;
            }
        });
        authToken->getAccessToken();
    }
    connect(&timer, &QTimer::timeout, this, &ZwiftIncline::poll);
    timer.start(1000);
}

ZwiftIncline::~ZwiftIncline() { delete world; }

void ZwiftIncline::poll() {
    // World requests run a nested event loop; do not start a second poll inside it.
    if (pollInProgress)
        return;
    QScopedValueRollback<bool> polling(pollInProgress, true);
    QSettings settings;
    auto *device = bluetoothManager->device();
    if (!device || device->deviceType() != TREADMILL || !authToken || authToken->access_token.isEmpty() ||
        settings.value(QZSettings::zwift_username, QZSettings::default_zwift_username).toString().isEmpty())
        return;

    if (!world) {
        world = new World(1, authToken->getAccessToken());
        qDebug() << "creating zwift api world";
        return;
    }
    if (playerId == -1) {
        const QString response = world->player_id();
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(response.toLocal8Bit(), &parseError);
        const QJsonObject player = document.object();
        qDebug() << "zwift api player" << player;
        playerId = player[QStringLiteral("id")].toInt();
        emit zwiftLoginState(true);
        return;
    }

    const int interval = qMax(5, settings.value(QZSettings::zwift_api_poll, QZSettings::default_zwift_api_poll).toInt());
    if (pollCounter++ < interval - 1)
        return;
    pollCounter = 0;
    const QByteArray payload = world->playerStatus(playerId);
    if (payload.isEmpty())
        return;

#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    jbyteArray bytes = env->NewByteArray(payload.size());
    if (!bytes)
        return;
    env->SetByteArrayRegion(bytes, 0, payload.size(), reinterpret_cast<const jbyte *>(payload.constData()));
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/ZwiftAPI", "zwift_api_decodemessage_player", "([B)V", bytes);
    env->DeleteLocalRef(bytes);
    const float altitude =
        QAndroidJniObject::callStaticMethod<float>("org/cagnulen/qdomyoszwift/ZwiftAPI", "getAltitude", "()F");
    const float distance =
        QAndroidJniObject::callStaticMethod<float>("org/cagnulen/qdomyoszwift/ZwiftAPI", "getDistance", "()F");

    const bool enabled =
        settings.value(QZSettings::zwift_api_autoinclination, QZSettings::default_zwift_api_autoinclination).toBool();
    const double gain =
        settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain).toDouble();
    const double offset =
        settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset).toDouble();
    const bool doubleNegative =
        settings.value(QZSettings::zwift_negative_inclination_x2, QZSettings::default_zwift_negative_inclination_x2).toBool();
    const auto grade = terrain.update(distance, altitude, enabled, gain, offset, doubleNegative);
    if (grade) {
        // The treadmill applies its existing auto-incline toggle, calibration and clamps.
        qDebug() << "zwift api incline" << *grade;
        if (bluetoothManager->device())
            bluetoothManager->device()->changeInclination(*grade, *grade);
    }
#endif
}
