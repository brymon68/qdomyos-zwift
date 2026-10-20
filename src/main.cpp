#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <QQmlContext>
#include <QLocale>
#include "logwriter.h"
#include "bluetooth.h"
#include "homeform.h"
#include "qfit.h"
#include <QDir>
#include <QGuiApplication>
#include <QOperatingSystemVersion>
#include <QQmlApplicationEngine>
#include <QSettings>
#ifdef X22I_TEST
#include "x22i_defaults.h"
#endif
#include <QStandardPaths>
#include <QList>
#ifdef CHARTJS
#include <QtWebView/QtWebView>
#endif

#include "mqttpublisher.h"
#include "androidstatusbar.h"
#include "fontmanager.h"
#include "filesearcher.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QtAndroid>
#endif

#ifdef Q_OS_MACOS
#include "macos/lockscreen.h"
#endif

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

#include "osc.h"


bool logs = true;
bool noHeartService = true;
QString logfilename = QStringLiteral("debug-") +
                      QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_hh-mm-ss")) +
                      QStringLiteral(".log");
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

// Global thread and writer instance
static QThread *logThread = nullptr;
static LogWriter *logWriter = nullptr;

void initializeLogThread() {
    if (!logThread) {
        logThread = new QThread();
        logWriter = new LogWriter();
        logWriter->moveToThread(logThread);
        logThread->start();
    }
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    QSettings settings;
    static bool logdebug = settings.value(QZSettings::log_debug, QZSettings::default_log_debug).toBool();
    if (!logdebug)
        return;

    // QByteArray localMsg = msg.toLocal8Bit(); // NOTE: clazy-unused-non-trivial-variable
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    QString txt = QDateTime::currentDateTime().toString() + QStringLiteral(" ") +
                  QString::number(QDateTime::currentMSecsSinceEpoch()) + QStringLiteral(" ");
    switch (type) {
    case QtInfoMsg:
        txt += QStringLiteral("Info: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtDebugMsg:
        txt += QStringLiteral("Debug: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtWarningMsg:
        txt += QStringLiteral("Warning: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtCriticalMsg:
        txt += QStringLiteral("Critical: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        break;
    case QtFatalMsg:
        txt += QStringLiteral("Fatal: %1 %2 %3\n").arg(file, function, msg); // NOTE: clazy-qstring-arg
        abort();
    }

    if (logs == true || logdebug == true) {

        QString path = homeform::getWritableAppDir();

        // Ensure thread is initialized
        initializeLogThread();

        // Write log in the worker thread
        QMetaObject::invokeMethod(logWriter, "writeLog",
                                 Qt::QueuedConnection,
                                 Q_ARG(QString, path + logfilename),
                                 Q_ARG(QString, txt));

    }
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}

int main(int argc, char *argv[]) {
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QScopedPointer<QApplication> app(new QApplication(argc, argv));

#ifdef CHARTJS
    QtWebView::initialize();
#endif

    app->setOrganizationName(QStringLiteral("Roberto Viola"));
    app->setOrganizationDomain(QStringLiteral("robertoviola.cloud"));
    app->setApplicationName(QStringLiteral("qDomyos-Zwift"));

    QSettings settings;
#ifdef X22I_TEST
    applyX22iDefaults(settings);
#endif

    // Older Android consoles may not support a CPU wake lock.
    if (QOperatingSystemVersion::current() < QOperatingSystemVersion(QOperatingSystemVersion::Android, 7) &&
        !settings.value(QZSettings::android_wakelock).isValid()) {
        settings.setValue(QZSettings::android_wakelock, false);
    }
    noHeartService = settings.value(QZSettings::bike_heartrate_service, !noHeartService).toBool();

    // Register custom meta types used in queued invocations
    qRegisterMetaType<SessionLine>("SessionLine");
    qRegisterMetaType<QList<SessionLine>>("QList<SessionLine>");
    qRegisterMetaType<BLUETOOTH_TYPE>("BLUETOOTH_TYPE");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<FIT_SPORT>("FIT_SPORT");

    qInstallMessageHandler(myMessageOutput);
    qDebug() << QStringLiteral("version ") << app->applicationVersion();
    settings.setValue(QZSettings::app_opening,
                      settings.value(QZSettings::app_opening, QZSettings::default_app_opening).toInt() + 1);

#if defined(Q_OS_ANDROID)
    auto result = QtAndroid::checkPermission(QString("android.permission.READ_EXTERNAL_STORAGE"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.READ_EXTERNAL_STORAGE"}));
        if (resultHash["android.permission.READ_EXTERNAL_STORAGE"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "READ_EXTERNAL_STORAGE denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.ACCESS_FINE_LOCATION"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.ACCESS_FINE_LOCATION"}));
        if (resultHash["android.permission.ACCESS_FINE_LOCATION"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "ACCESS_FINE_LOCATION denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH"}));
        if (resultHash["android.permission.BLUETOOTH"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_ADMIN"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_ADMIN"}));
        if (resultHash["android.permission.BLUETOOTH_ADMIN"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_ADMIN denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_SCAN"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_SCAN"}));
        if (resultHash["android.permission.BLUETOOTH_SCAN"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_SCAN denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_ADVERTISE"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_ADVERTISE"}));
        if (resultHash["android.permission.BLUETOOTH_ADVERTISE"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_ADVERTISE denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.BLUETOOTH_CONNECT"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.BLUETOOTH_CONNECT"}));
        if (resultHash["android.permission.BLUETOOTH_CONNECT"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "BLUETOOTH_CONNECT denied!";
    }

    result = QtAndroid::checkPermission(QString("android.permission.POST_NOTIFICATIONS"));
    if (result == QtAndroid::PermissionResult::Denied) {
        QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList({"android.permission.POST_NOTIFICATIONS"}));
        if (resultHash["android.permission.POST_NOTIFICATIONS"] == QtAndroid::PermissionResult::Denied)
            qDebug() << "POST_NOTIFICATIONS denied!";
    }

    // Create /sdcard/.wolfDev/keepme directory if it doesn't exist
    QDir wolfDevDir("/sdcard/.wolfDev/keepme");
    if (!wolfDevDir.exists()) {
        if (wolfDevDir.mkpath(".")) {
            qDebug() << "Created directory: /sdcard/.wolfDev/keepme";
        } else {
            qDebug() << "Failed to create directory: /sdcard/.wolfDev/keepme";
        }
    } else {
        qDebug() << "Directory already exists: /sdcard/.wolfDev/keepme";
    }
#endif

    bluetooth bl(logs, noHeartService);

    QString mqtt_host = settings.value(QZSettings::mqtt_host, QZSettings::default_mqtt_host).toString();
    int mqtt_port = settings.value(QZSettings::mqtt_port, QZSettings::default_mqtt_port).toInt();
    QString mqtt_username = settings.value(QZSettings::mqtt_username, QZSettings::default_mqtt_username).toString();
    QString mqtt_password = settings.value(QZSettings::mqtt_password, QZSettings::default_mqtt_password).toString();
    if(mqtt_host.length() > 0) {
        new MQTTPublisher(mqtt_host, mqtt_port, mqtt_username, mqtt_password, &bl, &bl);
    }

    QString OSC_ip = settings.value(QZSettings::OSC_ip, QZSettings::default_OSC_ip).toString();
    if(OSC_ip.length() > 0) {
        OSC* osc = new OSC(&bl);
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    lockscreen h;
    h.request();
#endif
#endif

    {
        AndroidStatusBar::registerQmlType();

#ifdef Q_OS_ANDROID
        FontManager fontManager;
        fontManager.initializeEmojiFont();
#endif

        QQmlApplicationEngine engine;
        const QUrl url(QStringLiteral("qrc:/main.qml"));
        QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreated, qobject_cast<QGuiApplication *>(app.data()),
            [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            },
            Qt::QueuedConnection);

#ifdef Q_OS_ANDROID
        engine.rootContext()->setContextProperty("OS_VERSION", QVariant("Android"));
#elif defined(Q_OS_IOS)
        engine.rootContext()->setContextProperty("OS_VERSION", QVariant("iOS"));
#else
        engine.rootContext()->setContextProperty("OS_VERSION", QVariant("Other"));
#endif
#ifdef CHARTJS
        engine.rootContext()->setContextProperty("CHARTJS", QVariant(true));
#else
        engine.rootContext()->setContextProperty("CHARTJS", QVariant(false));
#endif
#ifdef Q_OS_ANDROID
        engine.rootContext()->setContextProperty("fontManager", &fontManager);
#endif
        // Expose FileSearcher for fast recursive file searching
        FileSearcher fileSearcher;
        engine.rootContext()->setContextProperty("fileSearcher", &fileSearcher);

        engine.load(url);
        if (engine.rootObjects().isEmpty())
            return -1;
        homeform *h = new homeform(&engine, &bl);
        QObject::connect(app.data(), &QCoreApplication::aboutToQuit, h,
                         &homeform::aboutToQuit); // NOTE: clazy-unneeded-cast

        {
#ifdef Q_OS_ANDROID
            KeepAwakeHelper helper;
#elif defined Q_OS_MACOS
            lockScreen();
#elif defined Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen yc;
            yc.setTimerDisabled();
#endif
#endif
            // screen and CPU will stay awake during this section
            // lock will be released when helper object goes out of scope
            return app->exec();
        }
#ifdef Q_OS_MACOS
        unlockScreen();
#endif
    }
}
