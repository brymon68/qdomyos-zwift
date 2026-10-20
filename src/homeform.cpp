#include "homeform.h"
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#include "ios/ios_liveactivity.h"
#endif
#include "localipaddress.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <jni.h>
#include <QAndroidJniObject>
#endif
#include "fitdatabaseprocessor.h"
#include "material.h"
#include "qfit.h"
#include "templateinfosenderbuilder.h"
#include "workoutmodel.h"
#include "zwiftworkout.h"

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QGeoCoordinate>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QProcess>
#include <QQmlContext>
#include <QQmlFile>

#include <QRandomGenerator>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QUuid>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <algorithm>
#include <chrono>

homeform *homeform::m_singleton = 0;
using namespace std::chrono_literals;

namespace {
QString sanitizeClipboardWorkoutName(const QString &input) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        trimmed = QStringLiteral("Clipboard_Workout");
    }
    QRegularExpression invalid(QStringLiteral("[^A-Za-z0-9_\\- ]"));
    trimmed.replace(invalid, QStringLiteral("_"));
    trimmed.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("_"));
    return trimmed;
}

QString uniqueClipboardWorkoutPath(const QString &extension, QString *displayName) {
    const QString trainingDir = homeform::getWritableAppDir() + QStringLiteral("training/");
    QDir dir(trainingDir);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    const QString baseName = sanitizeClipboardWorkoutName(
        QStringLiteral("Clipboard_Workout_%1")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))));
    QString candidate = baseName;
    int suffix = 2;
    while (QFile::exists(trainingDir + candidate + QStringLiteral(".") + extension)) {
        candidate = QStringLiteral("%1_%2").arg(baseName).arg(suffix++);
    }
    if (displayName) {
        *displayName = candidate + QStringLiteral(".") + extension;
    }
    return trainingDir + candidate + QStringLiteral(".") + extension;
}

QString firstXmlElementName(const QString &xml) {
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            return reader.name().toString();
        }
    }
    return QString();
}

bool writeClipboardWorkoutFile(const QString &path, const QString &content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    file.write(content.toUtf8());
    file.close();
    return true;
}

} // namespace

DataObject::DataObject(const QString &name, const QString &icon, const QString &value, bool writable, const QString &id,
                       int valueFontSize, int labelFontSize, const QString &valueFontColor, const QString &secondLine,
                       const int gridId, bool largeButton, QString largeButtonLabel, QString largeButtonColor) {
    m_name = name;
    m_icon = icon;
    m_value = value;
    m_secondLine = secondLine;
    m_writable = writable;
    m_id = id;
    m_valueFontSize = valueFontSize;
    m_valueFontColor = valueFontColor;
    m_labelFontSize = labelFontSize;
    m_gridId = gridId;
    m_largeButton = largeButton;
    m_largeButtonLabel = largeButtonLabel;
    m_largeButtonColor = largeButtonColor;

    emit plusNameChanged(plusName());   // NOTE: clazy-incorrecrt-emit
    emit minusNameChanged(minusName()); // NOTE: clazy-incorrecrt-emit
    emit identificatorChanged(identificator());
    emit largeButtonChanged(this->largeButton());
    emit largeButtonLabelChanged(this->largeButtonLabel());
    emit largeButtonColorChanged(this->largeButtonColor());
}

void DataObject::setName(const QString &v) {
    m_name = v;
    emit nameChanged(m_name);
}
void DataObject::setValue(const QString &v) {
    m_value = v;
    emit valueChanged(m_value);
}
void DataObject::setSecondLine(const QString &value) {
    m_secondLine = value;
    emit secondLineChanged(m_secondLine);
}
void DataObject::setValueFontSize(int value) {
    m_valueFontSize = value;
    emit valueFontSizeChanged(m_valueFontSize);
}
void DataObject::setValueFontColor(const QString &value) {
    m_valueFontColor = value;
    emit valueFontColorChanged(m_valueFontColor);
}
void DataObject::setLargeButtonColor(const QString &color) {
    m_largeButtonColor = color;
    emit largeButtonColorChanged(m_largeButtonColor);
}
void DataObject::setLargeButtonLabel(const QString &label) {
    m_largeButtonLabel = label;
    emit largeButtonLabelChanged(m_largeButtonLabel);
}
void DataObject::setLabelFontSize(int value) {
    m_labelFontSize = value;
    emit labelFontSizeChanged(m_labelFontSize);
}
void DataObject::setGridId(int id) {
    m_gridId = id;
    emit gridIdChanged(m_gridId);
}
void DataObject::setVisible(bool visible) {
    m_visible = visible;
    emit visibleChanged(m_visible);
}

homeform::homeform(QQmlApplicationEngine *engine, bluetooth *bl) {
    m_singleton = this;
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    QString unit = QStringLiteral("km");
    QString meters = QStringLiteral("m");
    QString weightLossUnit = QStringLiteral("Kg");
    QString cm = QStringLiteral("cm");
    if (miles) {
        unit = QStringLiteral("mi");
        weightLossUnit = QStringLiteral("Oz");
        meters = QStringLiteral("ft");
        cm = QStringLiteral("in");
    }

#ifdef Q_OS_ANDROID
    const bool nordictrack = true; // to replace
    if(!nordictrack) {
        m_locationServices = QAndroidJniObject::callStaticMethod<jboolean>("org/cagnulen/qdomyoszwift/LocationHelper", "start",
                                                "(Landroid/content/Context;)Z", QtAndroid::androidContext().object());
        if(m_locationServices) {
            QSettings settings;
            // so if someone pressed the skip message but now he forgot to enable GPS it will prompt out
            settings.setValue(QZSettings::skipLocationServicesDialog, QZSettings::default_skipLocationServicesDialog);
        }
    }
#endif

#ifdef Q_OS_IOS
    const int labelFontSize = 14;
    const int valueElapsedFontSize = 36;
    const int valueTimeFontSize = 26;
#elif defined Q_OS_ANDROID
    const int labelFontSize = 16;
    const int valueElapsedFontSize = 36;
    const int valueTimeFontSize = 26;
#else
    const int labelFontSize = 10;
    const int valueElapsedFontSize = 30;
    const int valueTimeFontSize = 22;
#endif

    QString innerId = QStringLiteral("inner");
    QString sKey = QStringLiteral("template_") + innerId + QStringLiteral("_" TEMPLATE_PRIVATE_WEBSERVER_ID "_");

    QString path = homeform::getWritableAppDir() + QStringLiteral("QZTemplates");
    this->userTemplateManager = TemplateInfoSenderBuilder::getInstance(
        QStringLiteral("user"), QStringList({path, QStringLiteral(":/templates/")}), this);

    settings.setValue(sKey + QStringLiteral("enabled"), true);
    settings.setValue(sKey + QStringLiteral("type"), TEMPLATE_TYPE_WEBSERVER);
    settings.setValue(sKey + QStringLiteral("port"), 0);
    this->innerTemplateManager =
        TemplateInfoSenderBuilder::getInstance(innerId, QStringList({QStringLiteral(":/inner_templates/")}), this);

    speed = new DataObject(tr("Speed (%1/h)").arg(unit),
                           QStringLiteral("icons/icons/speed.png"), QStringLiteral("0.0"), true,
                           QStringLiteral("speed"), 48, labelFontSize);
    inclination = new DataObject(tr("Inclination (%)"), QStringLiteral("icons/icons/inclination.png"),
                                 QStringLiteral("0.0"), true, QStringLiteral("inclination"), 48, labelFontSize);
    odometer = new DataObject(tr("Odometer (%1)").arg(unit),
                              QStringLiteral("icons/icons/odometer.png"), QStringLiteral("0.0"), false,
                              QStringLiteral("odometer"), 48, labelFontSize);
    pace =
        new DataObject(tr("Pace (m/%1)").arg(unit), QStringLiteral("icons/icons/pace.png"),
                       QStringLiteral("0:00"), false, QStringLiteral("pace"), 48, labelFontSize);

    heart = new DataObject(tr("Heart (bpm)"), QStringLiteral("icons/icons/heart_red.png"),
                           QStringLiteral("0"), false, QStringLiteral("heart"), 48, labelFontSize);
    elapsed =
        new DataObject(tr("Elapsed"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0:00:00"),
                       false, QStringLiteral("elapsed"), valueElapsedFontSize, labelFontSize);

    activityDescription = QLatin1String("");
    loadedWorkoutName = QLatin1String("");
    movieFileName = QUrl("");

    this->bluetoothManager = bl;
    this->engine = engine;
    connect(bluetoothManager, &bluetooth::bluetoothDeviceConnected, this, &homeform::bluetoothDeviceConnected);
    connect(bluetoothManager, &bluetooth::bluetoothDeviceDisconnected, this, &homeform::bluetoothDeviceDisconnected);
    connect(bluetoothManager, &bluetooth::deviceConnected, this, &homeform::deviceConnected);
    connect(bluetoothManager, &bluetooth::deviceConnected, this, &homeform::trainProgramSignals);
    connect(this, &homeform::workoutNameChanged, this->userTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutNameChanged);
    connect(this, &homeform::workoutStartDateChanged, this->userTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutStartDate);
    connect(this, &homeform::workoutEventStateChanged, this->userTemplateManager,
            &TemplateInfoSenderBuilder::workoutEventStateChanged);
    connect(this->userTemplateManager, &TemplateInfoSenderBuilder::activityDescriptionChanged, this,
            &homeform::setActivityDescription);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::chartSaved, this, &homeform::chartSaved);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::lap, this, &homeform::Lap);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::floatingClose, this, &homeform::floatingOpen);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::autoResistance, this,
            &homeform::toggleAutoResistance);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::speed_Plus, this, &homeform::speedPlus);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::speed_Minus, this, &homeform::speedMinus);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::inclination_Plus, this, &homeform::inclinationPlus);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::inclination_Minus, this, &homeform::inclinationMinus);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::Start, this, &homeform::StartRequested);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::Pause, this, &homeform::Start);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::Stop, this, &homeform::StopRequested);
    connect(this, &homeform::workoutNameChanged, this->innerTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutNameChanged);
    connect(this, &homeform::workoutStartDateChanged, this->innerTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutStartDate);
    connect(this, &homeform::workoutEventStateChanged, this->innerTemplateManager,
            &TemplateInfoSenderBuilder::workoutEventStateChanged);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::activityDescriptionChanged, this,
            &homeform::setActivityDescription);
    engine->rootContext()->setContextProperty(QStringLiteral("rootItem"), (QObject *)this);

    this->trainProgram = new trainprogram(QList<trainrow>(), bl);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &homeform::update);
    timer->start(1s);

    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &homeform::backup);
    backupTimer->start(1min);

    if (settings.value(QZSettings::trainprogram_clipboard_workout_enabled,
                       QZSettings::default_trainprogram_clipboard_workout_enabled)
            .toBool()) {
        clipboardWorkoutTimer = new QTimer(this);
        connect(clipboardWorkoutTimer, &QTimer::timeout, this, &homeform::checkClipboardForWorkout);
        clipboardWorkoutTimer->start(5s);
    }

    // Initialize FIT backup thread
    fitBackupThread = new QThread(this);
    fitBackupWriter = new FitBackupWriter();
    fitBackupWriter->moveToThread(fitBackupThread);
    fitBackupThread->start();

    QObject *rootObject = engine->rootObjects().constFirst();
    QObject *home = rootObject->findChild<QObject *>(QStringLiteral("home"));
    QObject *stack = rootObject;
    engine->rootContext()->setContextProperty("pathController", &pathController);
    QObject::connect(home, SIGNAL(start_clicked()), this, SLOT(Start()));
    QObject::connect(home, SIGNAL(stop_clicked()), this, SLOT(Stop()));
    QObject::connect(stack, SIGNAL(trainprogram_open_clicked(QUrl)), this, SLOT(trainprogram_open_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(trainprogram_open_other_folder(QUrl)), this, SLOT(trainprogram_open_other_folder(QUrl)));
    QObject::connect(stack, SIGNAL(gpx_open_other_folder(QUrl)), this, SLOT(gpx_open_other_folder(QUrl)));
    QObject::connect(stack, SIGNAL(trainprogram_preview(QUrl)), this, SLOT(trainprogram_preview(QUrl)));
    QObject::connect(stack, SIGNAL(gpxpreview_open_clicked(QUrl)), this, SLOT(gpxpreview_open_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(fitfile_preview_clicked(QUrl)), this, SLOT(fitfile_preview_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(trainprogram_zwo_loaded(QString)), this, SLOT(trainprogram_zwo_loaded(QString)));
    QObject::connect(stack, SIGNAL(trainprogram_autostart_requested()), this, SLOT(trainprogram_autostart_requested()));
    QObject::connect(stack, SIGNAL(gpx_open_clicked(QUrl)), this, SLOT(gpx_open_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(gpx_save_clicked()), this, SLOT(gpx_save_clicked()));
    QObject::connect(stack, SIGNAL(fit_save_clicked()), this, SLOT(fit_save_clicked()));
    QObject::connect(home, SIGNAL(lap_clicked()), this, SLOT(Lap()));
    QObject::connect(stack, SIGNAL(restart()), this, SLOT(restart()));

    QObject::connect(stack, SIGNAL(volumeUp()), this, SLOT(volumeUp()));
    QObject::connect(stack, SIGNAL(volumeDown()), this, SLOT(volumeDown()));
    QObject::connect(stack, SIGNAL(floatingOpen()), this, SLOT(floatingOpen()));
    QObject::connect(stack, SIGNAL(openFloatingWindowBrowser()), this, SLOT(openFloatingWindowBrowser()));

    qDebug() << "homeform constructor events linked";

#if !defined(Q_OS_IOS)
    QObject::connect(engine, &QQmlApplicationEngine::quit, &QGuiApplication::quit);
#endif

    emit stopIconChanged(stopIcon());     // NOTE: clazy-incorrecrt-emit
    emit stopTextChanged(stopText());     // NOTE: clazy-incorrecrt-emit
    emit startIconChanged(startIcon());   // NOTE: clazy-incorrecrt-emit
    emit startTextChanged(startText());   // NOTE: clazy-incorrecrt-emit
    emit startColorChanged(startColor()); // NOTE: clazy-incorrecrt-emit
    emit stopColorChanged(stopColor());   // NOTE: clazy-incorrecrt-emit

    emit tile_orderChanged(tile_order()); // NOTE: clazy-incorrecrt-emit

    QDir().mkdir(getWritableAppDir() + "training/");

    QDirIterator itGpx(":/gpx/");
    QDir().mkdir(getWritableAppDir() + "gpx/");
    while (itGpx.hasNext()) {
        qDebug() << itGpx.next() << itGpx.fileName();
        QString targetPath = getWritableAppDir() + "gpx/" + itGpx.fileName();
        QString markerPath = getWritableAppDir() + "gpx/.deleted_" + itGpx.fileName();
        // Only copy if file doesn't exist AND no deletion marker exists
        if (!QFile(targetPath).exists() && !QFile(markerPath).exists()) {
            QFile::copy(":/gpx/" + itGpx.fileName(), targetPath);
        }
    }

    QDirIterator itFit(getWritableAppDir(), QStringList() << "*.fit", QDir::Files);
    qDebug() << itFit.path();
    QDir().mkdir(getWritableAppDir() + "fit");
    while (itFit.hasNext()) {
        qDebug() << itFit.filePath() << itFit.fileName() << itFit.filePath().replace(itFit.path(), "");
        if (!QFile(getWritableAppDir() + "fit/" + itFit.next().replace(itFit.path(), "")).exists() && !itFit.fileName().contains("backup")) {
            if(QFile::copy(itFit.filePath(), getWritableAppDir() + "fit/" + itFit.filePath().replace(itFit.path(), "")))
                QFile::remove(itFit.filePath());
        }
    }

#ifdef Q_OS_ANDROID

    QString bluetoothName = getBluetoothName();
    qDebug() << "getBluetoothName()" << bluetoothName;

    QRegularExpression regex("^[A-Za-z0-9 ]+$");
    if(bluetoothName.length() > 9 || !regex.match(bluetoothName).hasMatch()) {
        setToastRequested("Bluetooth name too long, change it to a 4 letters one in the android settings and use only A-Z or 0-9 characters");
    }

    // Android 14 restrics access to /Android/data folder
    bool android_documents_folder = settings.value(QZSettings::android_documents_folder, QZSettings::default_android_documents_folder).toBool();
    if (android_documents_folder || QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 14)) {
        QDirIterator itAndroid(getAndroidDataAppDir(), QDirIterator::Subdirectories);
        QDir().mkdir(getWritableAppDir());
        while (itAndroid.hasNext()) {
            qDebug() << itAndroid.filePath() << itAndroid.fileName() << itAndroid.filePath().replace(itAndroid.path(), "");
            if (!QFile(getWritableAppDir() + itAndroid.next().replace(itAndroid.path(), "")).exists()) {
                if(QFile::copy(itAndroid.filePath(), getWritableAppDir() + itAndroid.filePath().replace(itAndroid.path(), "")))
                       QFile::remove(itAndroid.filePath());
            }
        }
    }
#endif

    fitProcessor = new FitDatabaseProcessor(getWritableAppDir() + "ddb.sqlite");
    connect(fitProcessor, &FitDatabaseProcessor::fileProcessed,
            this, [](const QString& filename) {
                qDebug() << "FitDatabaseProcessor Processing:" << filename;
            });
    connect(fitProcessor, &FitDatabaseProcessor::progress,
            this, [](int processed, int total) {
                qDebug() << "FitDatabaseProcessor Progress:" << processed << "/" << total;
            });
    connect(fitProcessor, &FitDatabaseProcessor::error,
            this, [](const QString& error) {
                qDebug() << "FitDatabaseProcessor Error:" << error;
            });
    workoutModel = new WorkoutModel(getWritableAppDir() + "ddb.sqlite");
    engine->rootContext()->setContextProperty("workoutModel", workoutModel);

    connect(fitProcessor, &FitDatabaseProcessor::processingStopped,
            this, [this]() {
                qDebug() << "FitDatabaseProcessor Processing stopped - refreshing workout model";
                workoutModel->setDatabaseProcessing(false);
                workoutModel->refresh();
            });
    fitProcessor->processDirectory(getWritableAppDir() + "fit");

    m_speech.setLocale(QLocale::English);

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QBluetoothDeviceInfo b;
    deviceConnected(b);
#endif

    if (QSslSocket::supportsSsl()) {
        qDebug() << "SSL supported";
    } else {
        qDebug() << "SSL non supported";
    }

#ifdef Q_OS_ANDROID
    QAndroidJniObject javaPath = QAndroidJniObject::fromString(getWritableAppDir());
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Shortcuts", "createShortcutsForFiles",
                                                "(Ljava/lang/String;Landroid/content/Context;)V", javaPath.object<jstring>(), QtAndroid::androidContext().object());

#endif

}

void homeform::setActivityDescription(QString desc) { activityDescription = desc; }

void homeform::chartSaved(QString fileName) {
    if (!stopped)
        return;
    chartImagesFilenames.append(fileName);
    if (chartImagesFilenames.length() >= 3) {
        qDebug() << "removing chart images";
        for (const QString &f : qAsConst(chartImagesFilenames)) {
            QFile::remove(f);
        }
        chartImagesFilenames.clear();
    }
}

void homeform::volumeUp() { Plus(QStringLiteral("speed")); }

void homeform::volumeDown() { Minus(QStringLiteral("speed")); }

void homeform::floatingOpen() {
#ifdef Q_OS_ANDROID
    if (!floating_open) {

        QSettings settings;
        // Get the floating window type setting (0 = classic, 1 = horizontal)
        int floatingWindowType = settings.value(QZSettings::floatingwindow_type, QZSettings::default_floatingwindow_type).toInt();

        // Determine which HTML file to use based on the setting
        QString htmlFile = (floatingWindowType == 0) ? "floating.htm" : "hfloating.htm";

        QAndroidJniObject javaHtmlFile = QAndroidJniObject::fromString(htmlFile);

        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/FloatingHandler", "show", "(Landroid/content/Context;IIIILjava/lang/String;)V",
            QtAndroid::androidContext().object(),
            settings.value("template_inner_QZWS_port", 6666).toInt(),
            settings.value(QZSettings::floating_width, QZSettings::default_floating_width).toInt(),
            settings.value(QZSettings::floating_height, QZSettings::default_floating_height).toInt(),
            settings.value(QZSettings::floating_transparency, QZSettings::default_floating_transparency).toInt(),
            javaHtmlFile.object<jstring>());
    } else {
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/FloatingHandler", "hide", "()V");
    }
    floating_open = !floating_open;
#endif
}

void homeform::openFloatingWindowBrowser() {
    QSettings settings;
    QHostAddress a;
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces()) {
        // Return only the first non-loopback MAC Address
        QString addr = netInterface.hardwareAddress();
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack) && !addr.isEmpty()) {
            const auto entries = netInterface.addressEntries();
            for (const QNetworkAddressEntry &newEntry : entries) {
                qDebug() << newEntry.ip().toIPv4Address();
                if (!newEntry.ip().isLoopback()) {
                    a = newEntry.ip();
                    break;
                }
            }
        }
    }
    QString url = "http://" + localipaddress::getIP(a).toString() + ":" +
                  QString::number(settings.value("template_inner_QZWS_port", 6666).toInt()) + "/floating/floating.htm";
    QDesktopServices::openUrl(url);
}

void homeform::zwiftLoginState(bool ok) {

    m_zwiftLoginState = (ok ? 1 : 0);
    emit zwiftLoginChanged(m_zwiftLoginState);
    if (!ok) {
        setToastRequested("Zwift Login Error!");
    }
}

QString homeform::getWritableAppDir() {
    QString path = QLatin1String("");
#if defined(Q_OS_ANDROID)
    QSettings settings;
    bool android_documents_folder = settings.value(QZSettings::android_documents_folder, QZSettings::default_android_documents_folder).toBool();
    if (android_documents_folder || QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 14)) {
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/QZ/";
        QDir().mkdir(path);
        // Create .nomedia file to prevent gallery indexing
        QFile nomediaFile(path + ".nomedia");
        if (!nomediaFile.exists()) {
            nomediaFile.open(QIODevice::WriteOnly);
            nomediaFile.close();
        }
    } else {
        path = getAndroidDataAppDir() + "/";
    }
#elif defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#elif defined(Q_OS_WINDOWS)
    path = QDir::currentPath() + "/";
#endif
    return path;
}

void homeform::backup() {

    static uint8_t index = 0;
    qDebug() << QStringLiteral("scheduling fit file backup...");

    QString path = getWritableAppDir();
    bluetoothdevice *dev = bluetoothManager->device();
    if (dev) {

        QString filename = path + QString::number(index) + backupFitFileName;

        // Use thread to write FIT backup file
        QMetaObject::invokeMethod(fitBackupWriter, "writeFitBackup",
                                 Qt::QueuedConnection,
                                 Q_ARG(QString, filename),
                                 Q_ARG(QList<SessionLine>, Session),
                                 Q_ARG(BLUETOOTH_TYPE, dev->deviceType()),
                                 Q_ARG(uint32_t, QFIT_PROCESS_NONE),
                                 Q_ARG(FIT_SPORT, FIT_SPORT_INVALID),
                                 Q_ARG(QString, workoutName()),
                                 Q_ARG(QString, dev->bluetoothDevice.name()));

        index++;
        if (index > 1) {
            index = 0;
        }
    }
}

QString homeform::stopColor() { return QStringLiteral("#00000000"); }

QString homeform::startColor() {
    static uint8_t startColorToggle = 0;
    if (paused || stopped) {
        if (startColorToggle) {

            startColorToggle = 0;
            return QStringLiteral("red");
        } else {

            startColorToggle = 1;
            return QStringLiteral("#00000000");
        }
    }
    return QStringLiteral("#00000000");
}

bool homeform::hasConnectedDevice() const {
    return bluetoothManager && bluetoothManager->device();
}

homeform::~homeform() {
    // Cleanup FIT backup thread
    if (fitBackupThread && fitBackupThread->isRunning()) {
        fitBackupThread->quit();
        fitBackupThread->wait();
    }
    if (fitBackupWriter) {
        delete fitBackupWriter;
    }

    gpx_save_clicked();
    fit_save_clicked();
}

void homeform::aboutToQuit() {
    qDebug() << "homeform::aboutToQuit()";

#ifdef Q_OS_ANDROID
    // closing floating window
    if (floating_open)
        floatingOpen();
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/NotificationClient", "hide", "()V");
#endif

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    // End iOS Live Activity
    ios_liveactivity::endLiveActivity();
#endif
#endif

    QSettings settings;
    if (settings.value(QZSettings::fit_file_saved_on_quit, QZSettings::default_fit_file_saved_on_quit).toBool()) {
        qDebug() << "fit_file_saved_on_quit true";
        fit_save_clicked();
    }

    if (bluetoothManager->device())
        bluetoothManager->device()->disconnectBluetooth();
}

void homeform::trainProgramSignals() {
    if (bluetoothManager->device()) {
        disconnect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start);
        disconnect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop);
        disconnect(trainProgram, &trainprogram::stop, this, &homeform::StopFromTrainProgram);
        disconnect(trainProgram, &trainprogram::lap, this, &homeform::Lap);
        disconnect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeed);
        disconnect(trainProgram, &trainprogram::changeSpeed, this,
                   &homeform::onTrainingProgramSpeedChanged);
        disconnect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeInclination);
        disconnect(trainProgram, &trainprogram::changeNextInclination300Meters, bluetoothManager->device(),
                   &bluetoothdevice::changeNextInclination300Meters);
        disconnect(trainProgram, &trainprogram::changeSpeedAndInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeedAndInclination);

        disconnect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                   &trainprogram::onTapeStarted);
        disconnect(((treadmill *)bluetoothManager->device()), &treadmill::buttonHWStart, this,
                   &homeform::StartFromDevice);
        disconnect(((treadmill *)bluetoothManager->device()), &treadmill::buttonHWPause, this,
                   &homeform::PauseFromDevice);
        disconnect(((treadmill *)bluetoothManager->device()), &treadmill::buttonHWStop, this,
                   &homeform::StopFromDevice);
        disconnect(trainProgram, &trainprogram::changeGeoPosition, bluetoothManager->device(),
                   &bluetoothdevice::changeGeoPosition);
        disconnect(this, &homeform::workoutEventStateChanged, bluetoothManager->device(),
                   &bluetoothdevice::workoutEventStateChanged);
        disconnect(trainProgram, &trainprogram::changeTimestamp, this, &homeform::changeTimestamp);
        disconnect(trainProgram, &trainprogram::toastRequest, this, &homeform::onToastRequested);
        disconnect(trainProgram, &trainprogram::intervalTransitionApplied, this,
                   &homeform::onTrainingProgramIntervalTransition);
        disconnect(trainProgram, &trainprogram::zwiftLoginState, this, &homeform::zwiftLoginState);

        connect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start);
        connect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop);
        connect(trainProgram, &trainprogram::stop, this, &homeform::StopFromTrainProgram);
        connect(trainProgram, &trainprogram::lap, this, &homeform::Lap);
        connect(trainProgram, &trainprogram::toastRequest, this, &homeform::onToastRequested);
        connect(trainProgram, &trainprogram::intervalTransitionApplied, this,
                &homeform::onTrainingProgramIntervalTransition);
        // Connect training program speed changes to reset HR PID timer
        connect(trainProgram, &trainprogram::changeSpeed, this,
                &homeform::onTrainingProgramSpeedChanged);
        {
            connect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                    &treadmill::changeSpeed);
            connect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                    &treadmill::changeInclination);
            connect(trainProgram, &trainprogram::changeSpeedAndInclination, ((treadmill *)bluetoothManager->device()),
                    &treadmill::changeSpeedAndInclination);

            connect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                    &trainprogram::onTapeStarted);
            connect(((treadmill *)bluetoothManager->device()), &treadmill::buttonHWStart, this,
                    &homeform::StartFromDevice);
            connect(((treadmill *)bluetoothManager->device()), &treadmill::buttonHWPause, this,
                    &homeform::PauseFromDevice);
            connect(((treadmill *)bluetoothManager->device()), &treadmill::buttonHWStop, this,
                    &homeform::StopFromDevice);

        }
        connect(trainProgram, &trainprogram::changeNextInclination300Meters, bluetoothManager->device(),
                &bluetoothdevice::changeNextInclination300Meters);
        connect(trainProgram, &trainprogram::changeGeoPosition, bluetoothManager->device(),
                &bluetoothdevice::changeGeoPosition);
        connect(trainProgram, &trainprogram::changeTimestamp, this, &homeform::changeTimestamp);
        connect(this, &homeform::workoutEventStateChanged, bluetoothManager->device(),
                &bluetoothdevice::workoutEventStateChanged);
        connect(trainProgram, &trainprogram::zwiftLoginState, this, &homeform::zwiftLoginState);

        if (trainProgram) {
            bool treadmillMode = bluetoothManager->device()->deviceType() == TREADMILL &&
                                  (trainProgram->speedInclinationTargetWorkout() || trainProgram->chartTargetWorkout());
            setChartTreadmillMode(treadmillMode);
            bool chartWorkout = trainProgram->chartTargetWorkout() || trainProgram->speedInclinationTargetWorkout();
            setChartIconVisible(chartWorkout);
            if (chartFooterVisible()) {
                if (chartWorkout) {
                    // reloading
                    setChartFooterVisible(false);
                    setChartFooterVisible(true);
                } else {
                    setChartFooterVisible(false);
                }
            }
        }

        qDebug() << QStringLiteral("trainProgram associated to a device");
    } else {
        qDebug() << QStringLiteral("trainProgram NOT associated to a device");
    }
}

void homeform::onToastRequested(QString message) {
    QSettings settings;
    setToastRequested(message);

    // Use TTS if enabled
    if (settings.value(QZSettings::tts_enabled, QZSettings::default_tts_enabled).toBool()) {
        m_speech.say(message);
    }
}

void homeform::onTrainingProgramIntervalTransition() {
    QSettings settings;
    if (settings.value(QZSettings::trainprogram_sound_on_segment,
                       QZSettings::default_trainprogram_sound_on_segment)
            .toBool()) {
        emit trainingProgramIntervalSoundRequested();
    }
}

void homeform::onTrainingProgramSpeedChanged(double speed) {
    // Record the timestamp when the training program changed speed
    // This is used by the HR PID controller to avoid race conditions
    lastTrainingProgramSpeedChange = QDateTime::currentDateTime();
}

QStringList homeform::tile_order() {

    QStringList r;
    r.reserve(100);
    for (int i = 0; i < 100; i++) {
        r.append(QString::number(i));
    }
    return r;
}

// these events are coming from the SS2K, so when the auto resistance is off, this event shouldn't be processed

void homeform::speedPlus() {
    Plus(QStringLiteral("speed"));
}

void homeform::speedMinus() {
    Minus(QStringLiteral("speed"));
}

void homeform::inclinationPlus() {
    Plus(QStringLiteral("inclination"));
}

void homeform::inclinationMinus() {
    Minus(QStringLiteral("inclination"));
}

void homeform::sortTiles() {
    if (!bluetoothManager || !bluetoothManager->device())
        return;

    dataList.clear();
    const QList<DataObject *> tiles = {speed, inclination, pace, odometer, elapsed, heart};
    for (int i = 0; i < tiles.size(); ++i) {
        tiles.at(i)->setGridId(i);
        dataList.append(tiles.at(i));
    }
    engine->rootContext()->setContextProperty(QStringLiteral("appModel"), QVariant::fromValue(dataList));
}

DataObject *homeform::tileFromName(QString name) {
    foreach (QObject *d, dataList) {
        if (!((DataObject *)d)->name().compare(name)) {
            return (DataObject *)d;
        }
    }
    return nullptr;
}

void homeform::moveTile(QString name, int newIndex, int oldIndex) {
    QSettings settings;
    DataObject *current = tileFromName(name);
    if (current) {
        qDebug() << "moveTile" << name << newIndex << oldIndex;

        foreach (QString s, settings.allKeys()) {
            if (s.contains(QStringLiteral("tile_")) && s.contains(QStringLiteral("_order"))) {

                qDebug() << s << settings.value(s);
            }
        }

        int i = 0;
        foreach (QObject *d, dataList) {
            if (i == newIndex) {
                settings.setValue("tile_" + current->m_id.toLower() + "_order", i);
                i++;
            }
            QString n = ((DataObject *)d)->m_id;
            if (((DataObject *)d)->name().compare(name)) {
                settings.setValue("tile_" + n.toLower() + "_order", i);
                i++;
            }
        }

        foreach (QString s, settings.allKeys()) {
            if (s.contains(QStringLiteral("tile_")) && s.contains(QStringLiteral("_order"))) {

                qDebug() << s << settings.value(s);
            }
        }

        // sortTiles();
        // dataList.move(oldIndex, newIndex);
        // very dirty, but i needed a way to synchronize QML with C++
        QTimer::singleShot(100, this, &homeform::sortTilesTimeout);
    }
}

void homeform::sortTilesTimeout() { sortTiles(); }

void homeform::deviceConnected(QBluetoothDeviceInfo b) {

    qDebug() << "deviceConnected" << bluetoothManager << engine;
    if (bluetoothManager)
        qDebug() << bluetoothManager->device();

    if (bluetoothManager->device() == nullptr)
        return;

    // if the device reconnects in the same session, the tiles shouldn't be created again
    static bool first = false;
    if (first) {
        return;
    }
    first = true;

    m_info = QStringLiteral("X22i connected");
    emit infoChanged(m_info);

    m_labelHelp = false;
    emit changeLabelHelp(m_labelHelp);

    QSettings settings;

    if (settings.value(QZSettings::pause_on_start_treadmill, QZSettings::default_pause_on_start_treadmill)
                   .toBool() &&
               bluetoothManager->device()->deviceType() == TREADMILL) {
        Start_inner(false); // because if you sent the start command to a treadmill it could start the tape
        stopped = true; // when you will press start while you did some kms in pause mode from the beginning, the stats must be resetted
    }

    sortTiles();

    QObject *rootObject = engine->rootObjects().constFirst();
    QObject *home = rootObject->findChild<QObject *>(QStringLiteral("home"));
    QObject::connect(home, SIGNAL(plus_clicked(QString)), this, SLOT(Plus(QString)));
    QObject::connect(home, SIGNAL(minus_clicked(QString)), this, SLOT(Minus(QString)));

    emit workoutNameChanged(workoutName());

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::floating_startup, QZSettings::default_floating_startup).toBool()) {
        floatingOpen();
    }

#endif

}

bool homeform::handleKeyboardShortcut(const QString &sequence) {
    const QString normalizedSequence = sequence.trimmed().toUpper();
    if (normalizedSequence.isEmpty() || m_nativeShortcutCaptureSuspended) {
        return false;
    }

    QSettings settings;
    if (!settings.value(QZSettings::shortcuts_enabled, QZSettings::default_shortcuts_enabled).toBool()) {
        return false;
    }

    const auto matches = [&settings, &normalizedSequence](const QString &settingName,
                                                           const QString &defaultValue) {
        return normalizedSequence.compare(settings.value(settingName, defaultValue).toString().trimmed(),
                                          Qt::CaseInsensitive) == 0;
    };

    if (matches(QZSettings::shortcut_speed_plus, QZSettings::default_shortcut_speed_plus)) {
        Plus(QStringLiteral("speed"));
    } else if (matches(QZSettings::shortcut_speed_minus, QZSettings::default_shortcut_speed_minus)) {
        Minus(QStringLiteral("speed"));
    } else if (matches(QZSettings::shortcut_inclination_plus, QZSettings::default_shortcut_inclination_plus)) {
        Plus(QStringLiteral("inclination"));
    } else if (matches(QZSettings::shortcut_inclination_minus, QZSettings::default_shortcut_inclination_minus)) {
        Minus(QStringLiteral("inclination"));
    } else if (matches(QZSettings::shortcut_lap, QZSettings::default_shortcut_lap)) {
        Lap();
    } else if (matches(QZSettings::shortcut_start_stop, QZSettings::default_shortcut_start_stop)) {
        StartRequested();
    } else if (matches(QZSettings::shortcut_stop, QZSettings::default_shortcut_stop)) {
        StopRequested();
    } else {
        return false;
    }

    return true;
}

void homeform::Plus(const QString &name) {
    if (!bluetoothManager || !bluetoothManager->device())
        return;

    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    if (name == QStringLiteral("speed") || name == QStringLiteral("speed_plus")) {
        if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == TREADMILL) {
            // round up to the next .5 increment (.0 or .5)
            double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
            double requestedspeed = ((treadmill *)bluetoothManager->device())->requestedSpeed();
            double targetspeed = ((treadmill *)bluetoothManager->device())->currentTargetSpeed();
            qDebug() << QStringLiteral("Current Speed") << speed << QStringLiteral("Current Requested Speed")
                     << requestedspeed << QStringLiteral("Current Target Speed") << targetspeed;
            if (targetspeed != -1)
                speed = targetspeed;
            if (requestedspeed != -1)
                speed = requestedspeed;
            double minStepSpeed = ((treadmill *)bluetoothManager->device())->minStepSpeed();
            double step =
                settings.value(QZSettings::treadmill_step_speed, QZSettings::default_treadmill_step_speed).toDouble();
            if (!miles)
                step = ((double)qRound(step * 10.0)) / 10.0;
            if (step > minStepSpeed)
                minStepSpeed = step;
            int rest = 0;
            if (!miles)
                rest = (minStepSpeed * 10.0) - (((int)(speed * 10.0)) % (uint8_t)(minStepSpeed * 10.0));
            if (rest == 5 || rest == 0)
                speed = speed + minStepSpeed;
            else
                speed = speed + (((double)rest) / 10.0);

            ((treadmill *)bluetoothManager->device())->changeSpeed(speed);
        }
    } else if (name == QStringLiteral("inclination") || name == QStringLiteral("inclination_plus")) {
        if (bluetoothManager->device()) {
            {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                if (step < ((treadmill *)bluetoothManager->device())->minStepInclination())
                    step = ((treadmill *)bluetoothManager->device())->minStepInclination();
                double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() + step;
                ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
            }
        }
    }
}

void homeform::bluetoothDeviceConnected(bluetoothdevice *b) {
    this->innerTemplateManager->start(b);
    this->userTemplateManager->start(b);

}

void homeform::bluetoothDeviceDisconnected() {
    this->innerTemplateManager->stop();
    this->userTemplateManager->stop();
}

void homeform::Minus(const QString &name) {
    if (!bluetoothManager || !bluetoothManager->device())
        return;

    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    if (name == QStringLiteral("speed") || name == QStringLiteral("speed_minus")) {
        if (bluetoothManager->device()) {
            {
                // round up to the next .5 increment (.0 or .5)
                double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                double requestedspeed = ((treadmill *)bluetoothManager->device())->requestedSpeed();
                double targetspeed = ((treadmill *)bluetoothManager->device())->currentTargetSpeed();
                qDebug() << QStringLiteral("Current Speed") << speed << QStringLiteral("Current Requested Speed")
                         << requestedspeed << QStringLiteral("Current Target Speed") << targetspeed;
                if (targetspeed != -1)
                    speed = targetspeed;
                if (requestedspeed != -1 && requestedspeed < speed)
                    speed = requestedspeed;
                double minStepSpeed = ((treadmill *)bluetoothManager->device())->minStepSpeed();
                double step = settings.value(QZSettings::treadmill_step_speed, QZSettings::default_treadmill_step_speed)
                                  .toDouble();
                if (!miles)
                    step = ((double)qRound(step * 10.0)) / 10.0;
                if (step > minStepSpeed)
                    minStepSpeed = step;
                int rest = 0;
                if (!miles)
                    rest = (minStepSpeed * 10.0) - (((int)(speed * 10.0)) % (uint8_t)(minStepSpeed * 10.0));
                if (rest == 5 || rest == 0)
                    speed = speed - minStepSpeed;
                else
                    speed = speed - (((double)rest) / 10.0);
                ((treadmill *)bluetoothManager->device())->changeSpeed(speed);
            }
        }
    } else if (name == QStringLiteral("inclination") || name == QStringLiteral("inclination_minus")) {
        if (bluetoothManager->device()) {
            {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                if (step < ((treadmill *)bluetoothManager->device())->minStepInclination())
                    step = ((treadmill *)bluetoothManager->device())->minStepInclination();
                double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() - step;
                ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
            }
        }
    }
}

void homeform::Start() { Start_inner(true); }

void homeform::Start_inner(bool send_event_to_device) {
    QSettings settings;
    qDebug() << QStringLiteral("Start pressed - paused") << paused << QStringLiteral("stopped") << stopped;

    if (settings.value(QZSettings::tts_enabled, QZSettings::default_tts_enabled).toBool())
        m_speech.say("Start pressed");

    if (!paused && !stopped) {
        paused = true;
        if (bluetoothManager->device() && send_event_to_device) {
            bluetoothManager->device()->stop(paused);
        }
        emit workoutEventStateChanged(bluetoothdevice::PAUSED);
        // Pause Video if running and visible
        if ((trainProgram) && (videoVisible() == true)) {
            QObject *rootObject = engine->rootObjects().constFirst();
            auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
            auto videoPlaybackHalfPlayer = qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
            videoPlaybackHalfPlayer->pause();
        }
    } else {

        if (bluetoothManager->device() && send_event_to_device) {
            bluetoothManager->device()->start();
        }

        if (stopped) {
            trainProgram->restart();
            if (bluetoothManager->device()) {

                bluetoothManager->device()->clearStats();
            }
            Session.clear();
            chartImagesFilenames.clear();

#ifdef Q_OS_IOS
            this->innerTemplateManager->start(bluetoothManager->device());
#endif

            movieFileName = QLatin1String("");
            loadedWorkoutName = QLatin1String("");
            emit workoutNameChanged(workoutName());
            emit workoutEventStateChanged(bluetoothdevice::STARTED);
        } else {
            // if loading a training program (gpx or xml) directly from the startup of QZ, there is no way to start
            // the program otherwise
            if (!trainProgram->isStarted()) {
                qDebug() << QStringLiteral("starting training program from a resume");
                trainProgram->restart();
            }
            emit workoutEventStateChanged(bluetoothdevice::RESUMED);
            // Resume Video if visible
            if ((trainProgram) && (videoVisible() == true)) {
                QObject *rootObject = engine->rootObjects().constFirst();
                auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
                auto videoPlaybackHalfPlayer =
                    qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
                videoPlaybackHalfPlayer->play();
            }
        }

        paused = false;
        stopped = false;
    }

    emit stopIconChanged(stopIcon());
    emit stopTextChanged(stopText());
    emit stopColorChanged(stopColor());
    emit startIconChanged(startIcon());
    emit startTextChanged(startText());
    emit startColorChanged(startColor());

    if (bluetoothManager->device()) {
        bluetoothManager->device()->setPaused(paused | stopped);
    }
}

void homeform::StartFromDevice() {
    qDebug() << QStringLiteral("Physical start button pressed on device");
    Start_inner(false);  // false = don't send command back to device (it already started)
}

void homeform::PauseFromDevice() {
    qDebug() << QStringLiteral("Physical pause button pressed on device");
    Start_inner(false);  // false = don't send command back to device
}

void homeform::StopFromDevice() {
    qDebug() << QStringLiteral("Physical stop button pressed on device - stopping app");
    Stop();
}

void homeform::StartRequested() {
    Start();
    m_stopRequested = false;
    m_startRequested = true;
    emit stopRequestedChanged(m_stopRequested);
    emit startRequestedChanged(m_startRequested);
}

void homeform::StopRequested() {
    m_startRequested = false;
    m_stopRequested = true;
    emit startRequestedChanged(m_startRequested);
    emit stopRequestedChanged(m_stopRequested);
}

void homeform::StopFromTrainProgram(bool paused) {
    Stop();
}

void homeform::Stop() {
    QSettings settings;

    m_startRequested = false;

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    // End iOS Live Activity when workout stops
    ios_liveactivity::endLiveActivity();
#endif
#endif

    qDebug() << QStringLiteral("Stop pressed - paused") << paused << QStringLiteral("stopped") << stopped;

    if (stopped) {
        qDebug() << QStringLiteral("Stop pressed - already pressed, ignoring...");
        return;
    }

    if (bluetoothManager->device()) {
        {
            QTime zero(0, 0, 0, 0);
            if (bluetoothManager->device()->currentSpeed().value() == 0.0 &&
                zero.secsTo(bluetoothManager->device()->elapsedTime()) == 0) {
                qDebug() << QStringLiteral("Stop pressed - nothing to do. Elapsed time is 0 and current speed is 0");
                return;
            }
        }
    }

#ifdef Q_OS_IOS
    this->innerTemplateManager->reinit();
#endif

    if (settings.value(QZSettings::tts_enabled, QZSettings::default_tts_enabled).toBool())
        m_speech.say("Stop pressed");

    if (bluetoothManager->device()) {
        bluetoothManager->device()->stop(false);
    }

    paused = false;
    stopped = true;

    emit workoutEventStateChanged(bluetoothdevice::STOPPED);

    saveSessionAsTrainingProgram();
    fit_save_clicked();

    if (bluetoothManager->device()) {
        bluetoothManager->device()->setPaused(paused | stopped);
    }

    emit stopIconChanged(stopIcon());
    emit stopTextChanged(stopText());
    emit stopColorChanged(stopColor());
    emit startIconChanged(startIcon());
    emit startTextChanged(startText());
    emit startColorChanged(startColor());

    // clearing the label on top because if it was running a training program, with stop the program will be terminated
    m_info = workoutName();
    emit infoChanged(m_info);

    if (trainProgram) {
        trainProgram->clearRows();
    }

    if (!m_activeClipboardWorkoutFile.isEmpty() &&
        settings.value(QZSettings::trainprogram_clipboard_workout_enabled,
                       QZSettings::default_trainprogram_clipboard_workout_enabled).toBool()) {
        setClipboardWorkoutDeletePromptRequested(true);
    }
}

void homeform::Lap() {
    qDebug() << QStringLiteral("lap pressed");
    if (bluetoothManager) {
        if (bluetoothManager->device()) {

            bluetoothManager->device()->setLap();
            lapTrigger = true;
            if (trainProgram) {
                trainProgram->advanceLapButtonStep();
            }
        }
    }
}

bool homeform::labelHelp() { return m_labelHelp; }

QString homeform::stopText() { return tr("Stop"); }

QString homeform::stopIcon() { return QStringLiteral("icons/icons/stop.png"); }

QString homeform::startText() { return paused || stopped ? tr("Start") : tr("Pause"); }

QString homeform::startIcon() {
    return paused || stopped ? QStringLiteral("icons/icons/start.png") : QStringLiteral("icons/icons/pause.png");
}

QString homeform::signal() {
    if (!bluetoothManager) {
        return QStringLiteral("icons/icons/signal-1.png");
    }

    if (!bluetoothManager->device()) {
        return QStringLiteral("icons/icons/signal-1.png");
    }

    int16_t rssi = bluetoothManager->device()->bluetoothDevice.rssi();
    if (rssi > -40) {
        return QStringLiteral("icons/icons/signal-3.png");
    } else if (rssi > -60) {
        return QStringLiteral("icons/icons/signal-2.png");
    }

    return QStringLiteral("icons/icons/signal-1.png");
}

void homeform::update() {
    QSettings settings;
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 msecsElapsed = lastUpdateCall.msecsTo(now);
    if (qAbs(currentUpdateJitter) > 5000)
        currentUpdateJitter = 0;
    currentUpdateJitter += msecsElapsed - 1000;
    lastUpdateCall = now;

    emit stopIconChanged(stopIcon());
    emit stopTextChanged(stopText());
    emit startIconChanged(startIcon());
    emit startTextChanged(startText());
    emit startColorChanged(startColor());
    emit stopColorChanged(stopColor());

    auto *device = static_cast<treadmill *>(bluetoothManager->device());
    if (device) {
        const bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
        const double unitConversion = miles ? 0.621371 : 1.0;
        const double inclination = device->currentInclination().value();
        speed->setValue(QString::number(device->currentSpeed().value() * unitConversion, 'f', 1));
        speed->setSecondLine(QStringLiteral("AVG: ") + QString::number(device->currentSpeed().average() * unitConversion, 'f', 1) +
                             QStringLiteral(" MAX: ") + QString::number(device->currentSpeed().max() * unitConversion, 'f', 1));
        this->inclination->setValue(QString::number(inclination, 'f', 1));
        this->inclination->setSecondLine(QStringLiteral("AVG: ") + QString::number(device->currentInclination().average(), 'f', 1) +
                                         QStringLiteral(" MAX: ") + QString::number(device->currentInclination().max(), 'f', 1));
        this->pace->setValue(device->currentSpeed().value() > 2 ? device->currentPace().toString(QStringLiteral("m:ss")) : QStringLiteral("N/A"));
        this->pace->setSecondLine(QStringLiteral("AVG: ") + device->averagePace().toString(QStringLiteral("m:ss")));
        odometer->setValue(QString::number(device->odometer() * unitConversion, 'f', 2));
        elapsed->setValue(device->elapsedTime().toString(QStringLiteral("h:mm:ss")));
        heart->setValue(QString::number(device->currentHeart().value(), 'f', 0));
        heart->setSecondLine(QStringLiteral("AVG: ") + QString::number(device->currentHeart().average(), 'f', 0));
        emit currentSpeedChanged(currentSpeed());

        if (!paused && !stopped) {
            const uint8_t cadence = device->currentCadence().value();
            const double paceSeconds = QTime(0, 0).secsTo(device->currentPace());
            const double pace = device->currentSpeed().value() > 0 && paceSeconds > 0 ? 10000.0 / paceSeconds : 0;
            const uint32_t totalStrokes = 0;
            const double avgStrokesRate = 0, maxStrokesRate = 0, avgStrokesLength = 0;
            const double strideLength = device->currentStrideLength().value();
            const double groundContact = device->currentGroundContact().value();
            const double verticalOscillation = device->currentVerticalOscillation().value();
            const double stepCount = device->currentStepCount().value();
            bool treadmill_direct_distance = settings.value(QZSettings::treadmill_direct_distance, QZSettings::default_treadmill_direct_distance).toBool();
            double distance1s = 0;
            if (treadmill_direct_distance) {
                distance1s = bluetoothManager->device()->odometer();
            } else {
                if(bluetoothManager->device()->currentSpeed().value() > 0 && !isinf(bluetoothManager->device()->currentSpeed().value()))
                    bluetoothManager->device()->addCurrentDistance1s((bluetoothManager->device()->currentSpeed().value() / 3600.0));
                distance1s = bluetoothManager->device()->currentDistance1s().value();
            }

            qDebug() << "Current Distance 1s:" << distance1s << bluetoothManager->device()->currentSpeed().value();

            // Calculate current elapsed time in seconds
            uint32_t currentElapsedSeconds = bluetoothManager->device()->elapsedTime().second() +
                (bluetoothManager->device()->elapsedTime().minute() * 60) +
                (bluetoothManager->device()->elapsedTime().hour() * 3600);

            if (Session.empty()) {
                currentUpdateJitter = 0;
            }

            // Check for timer jitter gaps and fill missing SessionLine records (same logic as trainprogram)
            if (!Session.empty() && qAbs(currentUpdateJitter) > 1000) {
                if (currentUpdateJitter > 1000) {
                    // We are late... fill the missing seconds with SessionLine records
                    int missedSeconds = currentUpdateJitter / 1000;
                    qDebug() << "Timer jitter detected: filling" << missedSeconds << "missing SessionLine records";

                    // Create SessionLine records for each missed second using current device values
                    uint32_t lastRecordedTime = Session.last().elapsedTime;
                    for (int i = 1; i <= missedSeconds; i++) {
                        SessionLine gapFill(
                            bluetoothManager->device()->currentSpeed().value(), inclination, distance1s,
                            0, 0, (uint8_t)bluetoothManager->device()->currentHeart().value(),
                            pace, cadence, 0,
                            bluetoothManager->device()->elevationGain().value(),
                            bluetoothManager->device()->negativeElevationGain().value(),
                            lastRecordedTime + i,  // Fill each missing second
                            lapTrigger, totalStrokes, avgStrokesRate, maxStrokesRate, avgStrokesLength,
                            bluetoothManager->device()->currentCordinate(), strideLength, groundContact, verticalOscillation, stepCount,
                            0, 0,
                            device->lastRequestedInclination().value(), device->lastRequestedSpeed().value(),
                            bluetoothManager->device()->CoreBodyTemperature.value(), bluetoothManager->device()->SkinTemperature.value(), bluetoothManager->device()->HeatStrainIndex.value(),
                            0.0, QList<double>());

                        Session.append(gapFill);
                        qDebug() << "Added gap-filling SessionLine for elapsed time:" << (lastRecordedTime + i);
                    }

                    // Adjust jitter counter (same as trainprogram)
                    currentUpdateJitter -= (missedSeconds * 1000);
                } else if (currentUpdateJitter < -1000) {
                    // We are early (negative jitter)... remove excess SessionLine records
                    int excessSeconds = (-currentUpdateJitter) / 1000;
                    qDebug() << "Negative timer jitter detected: removing" << excessSeconds << "excess SessionLine records";

                    // Remove excess SessionLine records from the end
                    for (int i = 0; i < excessSeconds && !Session.empty(); i++) {
                        Session.removeLast();
                        qDebug() << "Removed excess SessionLine record";
                    }

                    // Adjust jitter counter (same as trainprogram)
                    currentUpdateJitter += (excessSeconds * 1000);
                }
            }

            SessionLine s(
                bluetoothManager->device()->currentSpeed().value(), inclination, distance1s,
                0, 0, (uint8_t)bluetoothManager->device()->currentHeart().value(),
                pace, cadence, 0,
                bluetoothManager->device()->elevationGain().value(),
                bluetoothManager->device()->negativeElevationGain().value(),
                currentElapsedSeconds,

                lapTrigger, totalStrokes, avgStrokesRate, maxStrokesRate, avgStrokesLength,
                bluetoothManager->device()->currentCordinate(), strideLength, groundContact, verticalOscillation, stepCount,
                0, 0,
                device->lastRequestedInclination().value(), device->lastRequestedSpeed().value(),
                bluetoothManager->device()->CoreBodyTemperature.value(), bluetoothManager->device()->SkinTemperature.value(), bluetoothManager->device()->HeatStrainIndex.value(),
                bluetoothManager->device()->currentHRV().value(),
                bluetoothManager->device()->getRRIntervalsAndClear());

            Session.append(s);

            if (lapTrigger) {
                lapTrigger = false;
            }

        }
        emit workoutStartDateChanged(workoutStartDate());
    }
    emit changeOfdevice();
    emit changeOflap();
}

bool homeform::getDevice() {

    static bool toggle = false;
    if (!this->bluetoothManager->device()) {

        // toggling the bluetooth icon
        toggle = !toggle;
        return toggle;
    }
    return this->bluetoothManager->device()->connected();
}

bool homeform::getLap() {
    if (!this->bluetoothManager->device()) {

        return false;
    }
    return true;
}

QString homeform::getFileNameFromContentUri(const QString &uriString) {
    qDebug() << "getFileNameFromContentUri" << uriString;
    if(!uriString.startsWith("content")) {
        return uriString;
    }
#ifdef Q_OS_ANDROID

    QAndroidJniObject jUriString = QAndroidJniObject::fromString(uriString);
    QAndroidJniObject jUri = QAndroidJniObject::callStaticObjectMethod("android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;", jUriString.object<jstring>());
    QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/ContentHelper",
        "getFileName",
        "(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;",
        QtAndroid::androidContext().object(),
        jUri.object());
    return result.toString();
#else
    return uriString;
#endif
}

QString homeform::copyAndroidContentsURI(QUrl file, QString subfolder) {
#ifdef Q_OS_ANDROID
    QString fileNameLocal = "";
    qDebug() << "Android Version:" << QOperatingSystemVersion::current();
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 13))
        fileNameLocal = getFileNameFromContentUri(file.toString());
    if(fileNameLocal.contains(getWritableAppDir() + subfolder + "/")) {
        qDebug() << "no need to copy file, the file is already in QZ subfolder" << file << subfolder;
        return file.toString();
    }

    QString filename = "";
    QFile fileFile(QQmlFile::urlToLocalFileOrQrc(file));
    // android <14 fallback
    if(fileNameLocal.length() == 0) {
        qDebug() << "android <14 fallback" << fileNameLocal << filename << file.fileName();
        filename = file.fileName();
    } else {
        QFileInfo f(fileNameLocal);
        filename = f.fileName();
    }
    QString dest = getWritableAppDir() + subfolder + "/" + filename;
    qDebug() << file.fileName() << fileNameLocal << filename;
    QFile::remove(dest);
    bool copy = fileFile.copy(dest);
    qDebug() << "copy" << dest << copy << fileFile.exists() << fileFile.isReadable();
    return dest;
#endif
    return file.toString();
}

void homeform::trainprogram_open_other_folder(const QUrl &fileName) {
    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    copyAndroidContentsURI(fileName, "training");
}

void homeform::gpx_open_other_folder(const QUrl &fileName) {
    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    copyAndroidContentsURI(fileName, "gpx");
}

bool homeform::startTrainingProgramFromFile(const QString &filePath) {
    if (filePath.isEmpty()) {
        return false;
    }
    QUrl url(filePath);
    if (!url.isValid() || (!url.isLocalFile() && url.scheme().isEmpty())) {
        url = QUrl::fromLocalFile(filePath);
    }
    QString localPath = QQmlFile::urlToLocalFileOrQrc(url);
    if (localPath.isEmpty() || !QFile::exists(localPath)) {
        return false;
    }
    trainprogram_open_clicked(QUrl::fromLocalFile(localPath));
    return true;
}

bool homeform::deleteTrainingProgramFile(const QString &fileUrl) {
    if (fileUrl.isEmpty()) {
        return false;
    }

    QUrl url(fileUrl);
    if (!url.isValid() || (!url.isLocalFile() && url.scheme().isEmpty())) {
        url = QUrl::fromLocalFile(fileUrl);
    }

    const QString localPath = QQmlFile::urlToLocalFileOrQrc(url);
    QFileInfo fileInfo(localPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    const QString trainingRoot = QDir(homeform::getWritableAppDir() + QStringLiteral("training")).canonicalPath();
    const QString canonicalFilePath = fileInfo.canonicalFilePath();
    if (trainingRoot.isEmpty() || canonicalFilePath.isEmpty() ||
        !canonicalFilePath.startsWith(trainingRoot + QStringLiteral("/"), Qt::CaseInsensitive)) {
        qDebug() << "deleteTrainingProgramFile: refusing to delete outside training folder" << localPath;
        return false;
    }

    if (!QFile::remove(canonicalFilePath)) {
        return false;
    }

    QFile markerFile(fileInfo.absolutePath() + QStringLiteral("/.deleted_") + fileInfo.fileName());
    if (markerFile.open(QIODevice::WriteOnly)) {
        markerFile.write("This file was intentionally deleted by the user");
        markerFile.close();
    }
    return true;
}

void homeform::trainprogram_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("trainprogram_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));

    if (!file.fileName().isEmpty()) {
        {
            if (previewTrainProgram) {
                delete previewTrainProgram;
                previewTrainProgram = 0;
            }
            if (trainProgram) {
                delete trainProgram;
            }

            trainProgram = trainprogram::load(file.fileName(), bluetoothManager, file.fileName().right(3).toUpper());

            QString movieName = file.fileName().left(file.fileName().length() - 3) + "mp4";
            if (QFile::exists(movieName)) {
                qDebug() << movieName << QStringLiteral("exist!");
                movieFileName = QUrl::fromLocalFile(movieName);
                emit videoPathChanged(movieFileName);
                setVideoIconVisible(true);
                setVideoRate(1);
                trainingProgram()->setVideoAvailable(true);
            } else {
                qDebug() << movieName << QStringLiteral("doesn't exist!");
                movieFileName = "";
                setVideoIconVisible(false);
                trainingProgram()->setVideoAvailable(false);
            }

            loadedWorkoutName = QFileInfo(file.fileName()).baseName();
            emit workoutNameChanged(workoutName());

            QSettings settings;
            m_info = workoutName();
            emit infoChanged(m_info);

        }

        trainProgramSignals();
    }
}

void homeform::trainprogram_autostart_requested() {
    qDebug() << QStringLiteral("trainprogram_autostart_requested");

    bluetoothdevice *dev = nullptr;
    if (bluetoothManager) {
        dev = bluetoothManager->device();
    }

    if (dev && !dev->isPaused()) {
        // Device is running, call Start() twice (pause then start)
        QMetaObject::invokeMethod(this, "Start", Qt::QueuedConnection);
        QThread::msleep(200);
        QMetaObject::invokeMethod(this, "Start", Qt::QueuedConnection);
    } else {
        // Device is paused/stopped, call Start() once
        QMetaObject::invokeMethod(this, "Start", Qt::QueuedConnection);
    }
}

void homeform::checkClipboardForWorkout() {
    QClipboard *clipboard = QApplication::clipboard();
    const QString clipboardText = clipboard ? clipboard->text().trimmed() : QString();
    const QByteArray currentHash = QCryptographicHash::hash(clipboardText.toUtf8(), QCryptographicHash::Sha1);

    if (currentHash == m_lastClipboardWorkoutHash) {
        return;
    }

    m_lastClipboardWorkoutHash = currentHash;
    m_clipboardWorkoutPromptFile.clear();
    m_clipboardWorkoutPromptName.clear();
    emit clipboardWorkoutPromptNameChanged(m_clipboardWorkoutPromptName);
    setClipboardWorkoutPromptRequested(false);

    if (clipboardText.isEmpty()) {
        return;
    }

    const QString rootName = firstXmlElementName(clipboardText);
    if (rootName.isEmpty()) {
        return;
    }

    QString displayName;
    QString filePath;
    QList<trainrow> rows;
    const bool looksLikeZwo = rootName.compare(QStringLiteral("workout_file"), Qt::CaseInsensitive) == 0 ||
                              rootName.compare(QStringLiteral("Workout"), Qt::CaseInsensitive) == 0;

    if (looksLikeZwo) {
        QString description;
        QString tags;
        rows = zwiftworkout::load(clipboardText.toUtf8(), &description, &tags);
        if (rows.isEmpty()) {
            return;
        }
        filePath = uniqueClipboardWorkoutPath(QStringLiteral("zwo"), &displayName);
        if (!writeClipboardWorkoutFile(filePath, clipboardText)) {
            return;
        }
    } else {
        filePath = uniqueClipboardWorkoutPath(QStringLiteral("xml"), &displayName);
        if (!writeClipboardWorkoutFile(filePath, clipboardText)) {
            return;
        }

        BLUETOOTH_TYPE dtype = BLUETOOTH_TYPE::BIKE;
        if (bluetoothManager && bluetoothManager->device()) {
            dtype = bluetoothManager->device()->deviceType();
        }
        rows = trainprogram::loadXML(filePath, dtype);
        if (rows.isEmpty()) {
            QFile::remove(filePath);
            return;
        }
    }

    m_clipboardWorkoutPromptFile = filePath;
    m_clipboardWorkoutPromptName = displayName;
    emit clipboardWorkoutPromptNameChanged(m_clipboardWorkoutPromptName);
    setClipboardWorkoutPromptRequested(true);
}

void homeform::clipboard_accept_workout_prompt() {
    m_activeClipboardWorkoutFile = m_clipboardWorkoutPromptFile;
    clipboard_dismiss_workout_prompt();
}

void homeform::clipboard_dismiss_workout_prompt() {
    m_clipboardWorkoutPromptFile.clear();
    m_clipboardWorkoutPromptName.clear();
    emit clipboardWorkoutPromptNameChanged(m_clipboardWorkoutPromptName);
    setClipboardWorkoutPromptRequested(false);
}

void homeform::clipboard_delete_finished_workout() {
    if (!m_activeClipboardWorkoutFile.isEmpty()) {
        QFile::remove(m_activeClipboardWorkoutFile);
        m_activeClipboardWorkoutFile.clear();
    }
    setClipboardWorkoutDeletePromptRequested(false);
}

void homeform::clipboard_keep_finished_workout() {
    m_activeClipboardWorkoutFile.clear();
    setClipboardWorkoutDeletePromptRequested(false);
}

void homeform::trainprogram_preview(const QUrl &fileName) {
    qDebug() << QStringLiteral("trainprogram_preview") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    QString fileNameLocal = getFileNameFromContentUri(file.fileName());
    qDebug() << fileNameLocal;
    if (!fileNameLocal.isEmpty()) {
        {
            if (previewTrainProgram) {
                delete previewTrainProgram;
                previewTrainProgram = 0;
            }
            previewTrainProgram = trainprogram::load(file.fileName(), bluetoothManager, fileNameLocal.right(3).toUpper());
            emit previewWorkoutPointsChanged(preview_workout_points());
            emit previewWorkoutDescriptionChanged(previewWorkoutDescription());
            emit previewWorkoutTagsChanged(previewWorkoutTags());
        }
    }
}

void homeform::trainprogram_zwo_loaded(const QString &s) {
    qDebug() << QStringLiteral("trainprogram_zwo_loaded") << s;
    trainProgram = new trainprogram(zwiftworkout::loadJSON(s), bluetoothManager);
    if (trainProgram) {
        QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("name"))) {
                emit workoutNameChanged(workoutName());

                QSettings settings;

                m_info = workoutName();
                emit infoChanged(m_info);
            }
        }
    }
    trainProgramSignals();
}

void homeform::gpx_save_clicked() {

    QString path = getWritableAppDir();

    if (bluetoothManager->device()) {
        gpx::save(path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                      QStringLiteral(".gpx"),
                  Session, bluetoothManager->device()->deviceType());
    }
}

void homeform::saveSessionAsTrainingProgram() {
    if (Session.isEmpty()) {
        return;
    }

    QString path = getWritableAppDir();
    bluetoothdevice *dev = bluetoothManager->device();
    if (!dev) {
        return;
    }

    // Determine subdirectory based on device type
    QString subdir;
    {
        subdir = "run/";
    }

    // Create the subdirectory if it doesn't exist
    QDir dir(path + subdir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filename = path + subdir +
                      QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                      QStringLiteral("_session.xml");

    // Convert Session data to trainrow format
    QList<trainrow> rows;
    for (int i = 0; i < Session.size(); i++) {
        const SessionLine &sessionLine = Session[i];
        trainrow row;

        // Set duration to 1 second since we collect data every second
        row.duration = QTime(0, 0, 1);

        // Set target values based on device type
        {
            if (sessionLine.target_speed > 0) {
                row.speed = sessionLine.target_speed;
            }
            if (sessionLine.target_inclination >= -50) {
                row.inclination = sessionLine.target_inclination;
            }
        }

        rows.append(row);
    }

    // Save the XML file
    if (trainprogram::saveXML(filename, rows, dev ? dev->deviceType() : UNKNOWN)) {
        lastTrainProgramFileSaved = filename;
        qDebug() << "Session saved as training program:" << filename;
    }
}

void homeform::fit_save_clicked() {
    bluetoothdevice *dev = bluetoothManager->device();
    if (!dev)
        return;

    const QString filename = getWritableAppDir() + QStringLiteral("fit/") +
        QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
        QStringLiteral(".fit");
    qfit::save(filename, Session, dev->deviceType(), QFIT_PROCESS_NONE, FIT_SPORT_INVALID,
               workoutName(), dev->bluetoothDevice.name(), QStringLiteral("QZ"), QString(), QString(),
               lastTrainProgramFileSaved);
    lastFitFileSaved = filename;

    if (fitProcessor && workoutModel) {
        fitProcessor->processFile(filename);
        workoutModel->refresh();
    }
}

void homeform::gpx_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("gpx_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));

    loadedWorkoutName = QFileInfo(file.fileName()).baseName();
    if (!file.fileName().isEmpty()) {
        {
            if (trainProgram) {

                delete trainProgram;
            }

            // KML to GPX https://www.gpsvisualizer.com/elevation
            gpx g;
            QList<trainrow> list;
            auto g_list = g.open(file.fileName(), bluetoothManager->device() ? bluetoothManager->device()->deviceType() : BIKE);
            if (bluetoothManager->device())
                bluetoothManager->device()->setGPXFile(file.fileName());
            gpx_altitude_point_for_treadmill last;
            quint32 i = 0;
            list.reserve(g_list.size() + 1);
            for (const auto &p : g_list) {
                trainrow r;
                if (p.speed > 0 && i > 0) {
                    QGeoCoordinate p1(last.latitude, last.longitude);
                    QGeoCoordinate p2(p.latitude, p.longitude, p.elevation);
                    r.azimuth = p1.azimuthTo(p2);
                    r.speed = p.speed;
                    r.distance = p.distance;
                    r.duration = QTime(0, 0, 0, 0);
                    r.duration = r.duration.addSecs(p.seconds);
                    r.forcespeed = true;

                    r.altitude = last.elevation;
                    r.inclination = p.inclination;
                    r.latitude = last.latitude;
                    r.longitude = last.longitude;
                    r.gpxElapsed = QTime(0, 0, 0).addSecs(p.seconds);

                    list.append(r);

                } else {
                    if (i > 0) {
                        QGeoCoordinate p1(last.latitude, last.longitude);
                        QGeoCoordinate p2(p.latitude, p.longitude, p.elevation);
                        r.azimuth = p1.azimuthTo(p2);
                        r.distance = p.distance;
                        r.altitude = last.elevation;
                        r.inclination = p.inclination;
                        r.latitude = last.latitude;
                        r.longitude = last.longitude;
                        r.gpxElapsed = QTime(0, 0, 0).addSecs(p.seconds);

                        list.append(r);
                    }
                }

                last = p;
                i++;
            }
            setMapsVisible(true);
            if (g.getVideoURL().isEmpty() == false) {
                movieFileName = QUrl(g.getVideoURL());
                emit videoPathChanged(movieFileName);
                setVideoIconVisible(true);
            } else if (QFile::exists(file.fileName().replace(".gpx", ".mp4"))) {
                movieFileName = QUrl::fromLocalFile(file.fileName().replace(".gpx", ".mp4"));
                emit videoPathChanged(movieFileName);
                setVideoIconVisible(true);
            }
            trainProgram = new trainprogram(list, bluetoothManager, nullptr, nullptr, videoIconVisible());
        }

        trainProgramSignals();
    }
}

void homeform::fitfile_preview_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("fitfile_preview_clicked called with URL:") << fileName;
    qDebug() << QStringLiteral("URL toString:") << fileName.toString();
    qDebug() << QStringLiteral("URL toLocalFile:") << fileName.toLocalFile();

    // Use the full file path directly instead of reconstructing it
    QString filePath = fileName.toLocalFile();
    QFile file(filePath);
    qDebug() << "Opening FIT file:" << filePath;

    if (file.exists()) {
        QList<SessionLine> a;
        FIT_SPORT sport;
        QString workoutName;
        qfit::open(filePath, &a, &sport, &workoutName);
        qDebug() << "FIT file read:" << a.size() << "records, sport:" << sport << "workoutName:" << workoutName;
        if (!a.isEmpty()) {
            this->innerTemplateManager->previewSessionOnChart(&a, sport, workoutName);
            emit previewFitFile(filePath, QTime(0,0,0,0).addSecs(a.last().elapsedTime).toString(), workoutName);
        } else {
            qDebug() << "No data read from FIT file";
        }
    } else {
        qDebug() << "FIT file does not exist:" << filePath;
    }
}

void homeform::gpxpreview_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("gpxpreview_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();

    if (!file.fileName().isEmpty()) {
        gpx g;
        // Force no loop for preview to show actual GPX distance
        auto g_list = g.open(file.fileName(), bluetoothManager->device() ? bluetoothManager->device()->deviceType() : BIKE, true);
        gpx_preview.clearPath();
        for (const auto &p : g_list) {
            gpx_preview.addCoordinate(QGeoCoordinate(p.latitude, p.longitude, p.elevation));
        }
        // Set distance BEFORE setGeoPath to ensure QML onGeopathChanged has correct value
        pathController.setDistance(g.getTotalDistance());
        pathController.setGeoPath(gpx_preview);
        pathController.setCenter(gpx_preview.center());
    }
}

QStringList homeform::metrics() { return bluetoothdevice::metrics(); }

bool homeform::generalPopupVisible() { return m_generalPopupVisible; }

void homeform::setGeneralPopupVisible(bool value) {

    m_generalPopupVisible = value;
    emit generalPopupVisibleChanged(m_generalPopupVisible);
}

bool homeform::mapsVisible() { return m_MapsVisible; }

void homeform::setMapsVisible(bool value) {

    m_MapsVisible = value;
    emit mapsVisibleChanged(m_MapsVisible);
}

bool homeform::videoIconVisible() { return m_VideoIconVisible; }

void homeform::setVideoIconVisible(bool value) {

    m_VideoIconVisible = value;
    emit videoIconVisibleChanged(m_VideoIconVisible);
}

bool homeform::chartIconVisible() { return m_ChartIconVisible; }

void homeform::setChartIconVisible(bool value) {

    m_ChartIconVisible = value;
    emit chartIconVisibleChanged(m_ChartIconVisible);
}

int homeform::videoPosition() { return m_VideoPosition; }

void homeform::setVideoPosition(int value) {

    m_VideoPosition = value;
    emit videoPositionChanged(m_VideoPosition);
}

double homeform::videoRate() { return m_VideoRate; }

void homeform::setVideoRate(double value) {

    m_VideoRate = value;
    emit videoRateChanged(m_VideoRate);
}

#if defined(Q_OS_ANDROID)

QString homeform::getBluetoothName()
{
    QAndroidJniObject bluetoothAdapter = QAndroidJniObject::callStaticObjectMethod(
        "android/bluetooth/BluetoothAdapter",
        "getDefaultAdapter",
        "()Landroid/bluetooth/BluetoothAdapter;");

    if (bluetoothAdapter.isValid()) {
        QAndroidJniObject name = bluetoothAdapter.callObjectMethod(
            "getName",
            "()Ljava/lang/String;");

        if (name.isValid()) {
            return name.toString();
        }
    }

    return QString();
}

QString homeform::getAndroidDataAppDir() {
    static QString path = "";

    if (path.length()) {
        return path;
    }

    QAndroidJniObject filesArr = QtAndroid::androidActivity().callObjectMethod(
        "getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;", nullptr);
    jobjectArray dataArray = filesArr.object<jobjectArray>();
    QString out;
    if (dataArray) {
        QAndroidJniEnvironment env;
        jsize dataSize = env->GetArrayLength(dataArray);
        if (dataSize) {
            QAndroidJniObject mediaPath;
            QAndroidJniObject file;
            for (int i = 0; i < dataSize; i++) {
                file = env->GetObjectArrayElement(dataArray, i);
                if (!file.isValid())
                    continue;
                // isExternalStorageRemovable throws IllegalArgumentException on Waydroid/emulators
                // where vold can't resolve the storage volume — clear any pending exception.
                jboolean val = QAndroidJniObject::callStaticMethod<jboolean>(
                    "android/os/Environment", "isExternalStorageRemovable", "(Ljava/io/File;)Z", file.object());
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                    val = JNI_FALSE;
                }
                mediaPath = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
                out = mediaPath.toString();
                if (!val)
                    break;
            }
        }
    }
    // Fallback to internal storage when external storage is unavailable (e.g. Waydroid)
    if (out.isEmpty()) {
        QAndroidJniObject internalDir = QtAndroid::androidActivity().callObjectMethod(
            "getFilesDir", "()Ljava/io/File;");
        if (internalDir.isValid()) {
            QAndroidJniObject internalPath = internalDir.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
            out = internalPath.toString();
        }
    }
    path = out;
    return out;
}
#endif

void homeform::restart() {
    qApp->quit();
#if !defined(Q_OS_DARWIN) && !defined(Q_OS_IOS) && !defined(Q_OS_WINRT)
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
#endif
}

void homeform::clearFiles() {
    QString path = homeform::getWritableAppDir();
    QDir dir(path);
    QFileInfoList list = dir.entryInfoList(QDir::Files);
    foreach (QFileInfo f, list) {
        if (!f.suffix().toLower().compare("log") || !f.suffix().toLower().compare("jpg") ||
            !f.suffix().toLower().compare("fit") || !f.suffix().toLower().compare("png")) {
            QFile::remove(f.filePath());
        }
    }
}

int homeform::preview_workout_points() {
    if (previewTrainProgram) {
        QTime d = previewTrainProgram->duration();
        return (d.hour() * 3600) + (d.minute() * 60) + d.second();
    }
    return 0;
}

void homeform::changeTimestamp(QTime source, QTime actual) {
    QSettings settings;
    // only needed if a gpx is loaded and the video is visible, otherwise do nothing.
    if ((trainProgram) && (videoVisible() == true)) {
        QObject *rootObject = engine->rootObjects().constFirst();
        auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
        auto videoPlaybackHalfPlayer = qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
        double videoTimeStampSeconds = (double)videoPlaybackHalfPlayer->position() / 1000.0;
        // Check for time differences between Video and gpx Data
        if (videoTimeStampSeconds != 0.0) {
            double videoLengthSeconds = ((double)(videoPlaybackHalfPlayer->duration() / 1000.0));
            double trainProgramLengthSeconds = ((double)(trainProgram->TotalGPXSecs()));
            int recordingFactor = 1;

            // if Video is > 60 secs Shorter it will be a speed adjusted one
            if ((trainProgramLengthSeconds - videoLengthSeconds) >= 60.0) {
                double recfac = ((trainProgramLengthSeconds / videoLengthSeconds) + 0.5);
                recordingFactor = ((int)(recfac));
                qDebug() << "Video Recording Factor" << recordingFactor << trainProgramLengthSeconds
                         << videoLengthSeconds << (videoLengthSeconds * ((double)(recordingFactor)))
                         << videoTimeStampSeconds << (videoTimeStampSeconds * ((double)(recordingFactor)));
                videoLengthSeconds = (videoLengthSeconds * ((double)(recordingFactor)));
                videoTimeStampSeconds = (videoTimeStampSeconds * ((double)(recordingFactor)));
            }

            // check if there is a difference >= 1 second
            if ((fabs(videoLengthSeconds - trainProgramLengthSeconds)) >= 1.0) {
                // correct Video TimeStamp by difference
                videoTimeStampSeconds = (videoTimeStampSeconds - videoLengthSeconds + trainProgramLengthSeconds);
            }

            qDebug() << videoTimeStampSeconds;
            // Video was just displayed, set the start Position
            if (videoMustBeReset) {
                double videoStartPos =
                    ((double)(QTime(0, 0, 0).secsTo(source)) + videoLengthSeconds - trainProgramLengthSeconds);
                // if videoStartPos is negativ the Video is shorter then the GPX. Wait for the gpx to reach a point
                // where the Video can be played
                if (videoStartPos >= 0.0) {
                    videoTimeStampSeconds = (videoStartPos - videoLengthSeconds + trainProgramLengthSeconds);
                    videoStartPos = videoStartPos / ((double)(recordingFactor));
                    qDebug() << "SetVideoStartPosition" << (videoStartPos * 1000.0);
                    videoPlaybackHalfPlayer->setPosition(videoStartPos * 1000.0);
                    videoMustBeReset = false;
                }
            }
            // Video is started now, calculate and set the Rate
            if (!videoMustBeReset) {
                // calculate and set the new Video Rate
                double rate = trainProgram->TimeRateFromGPX(
                    ((double)QTime(0, 0, 0).msecsTo(source)) / 1000.0, videoTimeStampSeconds,
                    bluetoothManager->device()->currentSpeed().average5s(), recordingFactor);
                rate = rate / ((double)(recordingFactor));
                setVideoRate(rate);
            } else {
                qDebug() << "videoMustBeReset = True";
            }
        } else {
            qDebug() << "videoTimeStampSeconds = 0";
        }
    }

    if (!videoVisible()) {
        // set the maximum Speed that the player can reached based on the Video speed.
        // When Video is not displayed (or not displayed any longer) remove the Limit

        // Prepare for a possible Video play. Set the Start Position to 1 and a Rate so low that only a few frames
        // are played
        setVideoPosition(1);
        setVideoRate(0.01);
        videoMustBeReset = true;
    }
}

void homeform::videoSeekPosition(int ms) {
    QObject *rootObject = engine->rootObjects().constFirst();
    auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
    auto videoPlaybackHalfPlayer = qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
    videoPlaybackHalfPlayer->setPosition(ms);
}
