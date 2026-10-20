#include "homeform.h"
#include "localipaddress.h"
#include "templateinfosenderbuilder.h"
#include "zwiftincline.h"
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QNetworkInterface>
#include <QOperatingSystemVersion>
#include <QProcess>
#include <QQmlContext>
#include <QStandardPaths>
#include <chrono>

homeform *homeform::m_singleton = nullptr;
using namespace std::chrono_literals;

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

homeform::homeform(QQmlApplicationEngine *engine, bluetooth *bl) : bluetoothManager(bl), engine(engine) {
    m_singleton = this;
    QSettings settings;
    const QString unit = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool()
                             ? QStringLiteral("mi") : QStringLiteral("km");
    const int labelFontSize = 16;
    const int valueElapsedFontSize = 36;
    innerTemplateManager = new TemplateInfoSenderBuilder(this);
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

    for (auto *tile : {speed, inclination, odometer, pace, heart, elapsed}) tile->setParent(this);
    connect(bl, &bluetooth::bluetoothDeviceConnected, this, &homeform::bluetoothDeviceConnected);
    connect(bl, &bluetooth::bluetoothDeviceDisconnected, this, &homeform::bluetoothDeviceDisconnected);
    connect(bl, &bluetooth::deviceConnected, this, &homeform::deviceConnected);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::floatingClose, this, &homeform::floatingOpen);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::autoResistance, this, &homeform::toggleAutoResistance);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::speed_Plus, this, &homeform::speedPlus);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::speed_Minus, this, &homeform::speedMinus);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::inclination_Plus, this, &homeform::inclinationPlus);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::inclination_Minus, this, &homeform::inclinationMinus);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::Start, this, &homeform::StartRequested);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::Pause, this, &homeform::Start);
    connect(innerTemplateManager, &TemplateInfoSenderBuilder::Stop, this, &homeform::StopRequested);
    engine->rootContext()->setContextProperty(QStringLiteral("rootItem"), this);
    zwiftIncline = new ZwiftIncline(bl, this);
    connect(zwiftIncline, &ZwiftIncline::toastRequest, this, &homeform::onToastRequested);
    connect(zwiftIncline, &ZwiftIncline::zwiftLoginState, this, &homeform::zwiftLoginState);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &homeform::update);
    timer->start(1s);
    QObject *root = engine->rootObjects().constFirst();
    QObject *home = root->findChild<QObject *>(QStringLiteral("home"));
    connect(home, SIGNAL(start_clicked()), this, SLOT(Start()));
    connect(home, SIGNAL(stop_clicked()), this, SLOT(Stop()));
    connect(home, SIGNAL(plus_clicked(QString)), this, SLOT(Plus(QString)));
    connect(home, SIGNAL(minus_clicked(QString)), this, SLOT(Minus(QString)));
    connect(root, SIGNAL(restart()), this, SLOT(restart()));
    connect(root, SIGNAL(floatingOpen()), this, SLOT(floatingOpen()));
    connect(root, SIGNAL(openFloatingWindowBrowser()), this, SLOT(openFloatingWindowBrowser()));
    connect(engine, &QQmlApplicationEngine::quit, &QGuiApplication::quit);
#ifdef Q_OS_ANDROID
    const QString bluetoothName = getBluetoothName();
    bool validName = bluetoothName.size() <= 9;
    for (QChar c : bluetoothName) {
        validName &= (c >= QLatin1Char('A') && c <= QLatin1Char('Z')) ||
                     (c >= QLatin1Char('a') && c <= QLatin1Char('z')) ||
                     (c >= QLatin1Char('0') && c <= QLatin1Char('9')) || c == QLatin1Char(' ');
    }
    if (!validName)
        setToastRequested("Bluetooth name too long, change it to a 4 letters one in the android settings and use only A-Z or 0-9 characters");
#endif
}

homeform::~homeform() { m_singleton = nullptr; }

void homeform::launchIFitApp() {
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("io/github/brymon68/x22i/AppLauncher", "launchIFitApp",
        "(Landroid/content/Context;)V", QtAndroid::androidContext().object());
#endif
}

void homeform::aboutToQuit() {
#ifdef Q_OS_ANDROID
    if (floating_open) floatingOpen();
    QAndroidJniObject::callStaticMethod<void>("io/github/brymon68/x22i/NotificationClient", "hide", "()V");
#endif
    if (bluetoothManager->device()) bluetoothManager->device()->disconnectBluetooth();
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
            "io/github/brymon68/x22i/FloatingHandler", "show", "(Landroid/content/Context;IIIILjava/lang/String;)V",
            QtAndroid::androidContext().object(),
            settings.value("template_inner_QZWS_port", 6666).toInt(),
            settings.value(QZSettings::floating_width, QZSettings::default_floating_width).toInt(),
            settings.value(QZSettings::floating_height, QZSettings::default_floating_height).toInt(),
            settings.value(QZSettings::floating_transparency, QZSettings::default_floating_transparency).toInt(),
            javaHtmlFile.object<jstring>());
    } else {
        QAndroidJniObject::callStaticMethod<void>("io/github/brymon68/x22i/FloatingHandler", "hide", "()V");
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
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/X22i/";
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
#endif
    return path;
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

void homeform::deviceConnected(QBluetoothDeviceInfo) {
    auto *device = static_cast<treadmill *>(bluetoothManager->device());
    if (!device) return;
    connect(device, &treadmill::buttonHWStart, this, &homeform::StartFromDevice, Qt::UniqueConnection);
    connect(device, &treadmill::buttonHWPause, this, &homeform::PauseFromDevice, Qt::UniqueConnection);
    connect(device, &treadmill::buttonHWStop, this, &homeform::StopFromDevice, Qt::UniqueConnection);
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    m_info = QStringLiteral("X22i connected");
    emit infoChanged(m_info);
    m_labelHelp = false;
    emit changeLabelHelp(false);
    QSettings settings;
    if (settings.value(QZSettings::pause_on_start_treadmill, QZSettings::default_pause_on_start_treadmill).toBool()) {
        Start_inner(false); // Initialize the paused UI without sending a belt-start command.
        stopped = true;
    }
    sortTiles();
    if (settings.value(QZSettings::floating_startup, QZSettings::default_floating_startup).toBool()) floatingOpen();
}

void homeform::bluetoothDeviceConnected(bluetoothdevice *device) { innerTemplateManager->start(device); }
void homeform::bluetoothDeviceDisconnected() { innerTemplateManager->stop(); }

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
    } else if (matches(QZSettings::shortcut_start_stop, QZSettings::default_shortcut_start_stop)) {
        StartRequested();
    } else if (matches(QZSettings::shortcut_stop, QZSettings::default_shortcut_stop)) {
        StopRequested();
    } else {
        return false;
    }

    return true;
}

void homeform::Start() { Start_inner(true); }

void homeform::Start_inner(bool send_event_to_device) {
    if (!paused && !stopped) {
        paused = true;
        if (bluetoothManager->device() && send_event_to_device) bluetoothManager->device()->stop(true);
    } else {
        if (bluetoothManager->device() && send_event_to_device) bluetoothManager->device()->start();
        if (stopped && bluetoothManager->device()) bluetoothManager->device()->clearStats();
        paused = false;
        stopped = false;
    }
    emit stopIconChanged(stopIcon());
    emit stopTextChanged(stopText());
    emit stopColorChanged(stopColor());
    emit startIconChanged(startIcon());
    emit startTextChanged(startText());
    emit startColorChanged(startColor());
    if (bluetoothManager->device()) bluetoothManager->device()->setPaused(paused | stopped);
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

void homeform::Stop() {
    m_startRequested = false;
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

    if (bluetoothManager->device()) bluetoothManager->device()->stop(false);
    paused = false;
    stopped = true;
    if (bluetoothManager->device()) bluetoothManager->device()->setPaused(true);
    emit stopIconChanged(stopIcon());
    emit stopTextChanged(stopText());
    emit stopColorChanged(stopColor());
    emit startIconChanged(startIcon());
    emit startTextChanged(startText());
    emit startColorChanged(startColor());
}

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
    }
    emit changeOfdevice();
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

#ifdef Q_OS_ANDROID
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
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
