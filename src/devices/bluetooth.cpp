#include "bluetooth.h"
#include "devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.h"

#include <QDebug>
#include <QTimer>
#include <cstdlib>

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras>
#endif

bluetooth::bluetooth(bool logs, bool noHeartService) : logs(logs), noHeartService(noHeartService) {
    // Let the UI connect its session and template handlers before creating the device.
    QTimer::singleShot(0, this, &bluetooth::start);
}

bluetooth::~bluetooth() {
    if (x22i) {
#ifdef Q_OS_ANDROID
        x22i->stopGrpcService();
#endif
        delete x22i;
    }
}

void bluetooth::start() {
    if (x22i)
        return;

    x22i = new nordictrackifitadbtreadmill(noHeartService);
    x22i->setParent(this);
    connect(x22i, &bluetoothdevice::connectedAndDiscovered, this, &bluetooth::connectedAndDiscovered);
    connect(x22i, &nordictrackifitadbtreadmill::debug, this, &bluetooth::debug);

    emit deviceConnected(QBluetoothDeviceInfo());
    emit searchingStop();
    emit bluetoothDeviceConnected(x22i);
}

bluetoothdevice *bluetooth::device() { return x22i; }

void bluetooth::restart() {
    if (x22i) {
        emit bluetoothDeviceDisconnected();
#ifdef Q_OS_ANDROID
        x22i->stopGrpcService();
#endif
        delete x22i;
        x22i = nullptr;
    }
    start();
}

void bluetooth::debug(const QString &message) {
    if (logs)
        qDebug() << message;
}

void bluetooth::connectedAndDiscovered() {
#ifdef Q_OS_ANDROID
    const QAndroidJniObject notification = QAndroidJniObject::fromString("QZ is running!");
    QAndroidJniObject::callStaticMethod<void>(
        "org/cagnulen/qdomyoszwift/NotificationClient", "notify", "(Landroid/content/Context;Ljava/lang/String;)V",
        QtAndroid::androidContext().object(), notification.object<jstring>());
#endif
}

bool bluetooth::handleSignal(int signal) {
    if (signal == SIGNALS::SIG_INT)
        std::exit(EXIT_SUCCESS);
    return false;
}
