#ifndef HOMEFORM_H
#define HOMEFORM_H

#include "bluetooth.h"
#include "devices/treadmill.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QQmlApplicationEngine>
#include <QTimer>
#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#endif

class TemplateInfoSenderBuilder;
class ZwiftIncline;

class DataObject : public QObject {

    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(int gridId READ gridId NOTIFY gridIdChanged WRITE setGridId)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString secondLine READ secondLine WRITE setSecondLine NOTIFY secondLineChanged)
    Q_PROPERTY(int valueFontSize READ valueFontSize WRITE setValueFontSize NOTIFY valueFontSizeChanged)
    Q_PROPERTY(QString valueFontColor READ valueFontColor WRITE setValueFontColor NOTIFY valueFontColorChanged)
    Q_PROPERTY(int labelFontSize READ labelFontSize WRITE setLabelFontSize NOTIFY labelFontSizeChanged)
    Q_PROPERTY(bool writable READ writable NOTIFY writableChanged)
    Q_PROPERTY(bool visibleItem READ visibleItem NOTIFY visibleChanged)
    Q_PROPERTY(bool largeButton READ largeButton NOTIFY largeButtonChanged)
    Q_PROPERTY(QString largeButtonColor READ largeButtonColor NOTIFY largeButtonColorChanged)
    Q_PROPERTY(QString largeButtonLabel READ largeButtonLabel NOTIFY largeButtonLabelChanged)
    Q_PROPERTY(QString plusName READ plusName NOTIFY plusNameChanged)
    Q_PROPERTY(QString minusName READ minusName NOTIFY minusNameChanged)
    Q_PROPERTY(QString identificator READ identificator NOTIFY identificatorChanged)

  public:
    DataObject(const QString &name, const QString &icon, const QString &value, bool writable, const QString &id,
               int valueFontSize, int labelFontSize, const QString &valueFontColor = QStringLiteral("white"),
               const QString &secondLine = QLatin1String(""), const int gridId = 0, const bool largeButton = false,
               const QString largeButtonLabel = QLatin1String(""),
               const QString largeButtonColor = QStringLiteral("grey"));
    void setName(const QString &value);
    void setValue(const QString &value);
    void setSecondLine(const QString &value);
    void setValueFontSize(int value);
    void setValueFontColor(const QString &value);
    void setLabelFontSize(int value);
    void setVisible(bool visible);
    void setGridId(int id);
    void setLargeButtonColor(const QString &color);
    void setLargeButtonLabel(const QString &label);
    QString name() { return m_name; }
    QString icon() { return m_icon; }
    QString value() { return m_value; }
    QString secondLine() { return m_secondLine; }
    int gridId() { return m_gridId; }
    int valueFontSize() { return m_valueFontSize; }
    QString valueFontColor() { return m_valueFontColor; }
    int labelFontSize() { return m_labelFontSize; }
    bool writable() { return m_writable; }
    bool visibleItem() { return m_visible; }
    QString plusName() { return m_id + QStringLiteral("_plus"); }
    QString minusName() { return m_id + QStringLiteral("_minus"); }
    QString identificator() { return m_id; }
    bool largeButton() { return m_largeButton; }
    QString largeButtonLabel() { return m_largeButtonLabel; }
    QString largeButtonColor() { return m_largeButtonColor; }

    QString m_id;
    QString m_name;
    QString m_icon;
    QString m_value;
    QString m_secondLine = QLatin1String("");
    int m_valueFontSize;
    int m_gridId;
    QString m_valueFontColor = QStringLiteral("white");
    int m_labelFontSize;
    bool m_writable;
    bool m_visible = true;
    bool m_largeButton = false;
    QString m_largeButtonLabel = QLatin1String("");
    QString m_largeButtonColor = QStringLiteral("grey");

  signals:
    void valueChanged(QString value);
    void secondLineChanged(QString value);
    void valueFontSizeChanged(int value);
    void valueFontColorChanged(QString value);
    void labelFontSizeChanged(int value);
    void gridIdChanged(int value);
    void nameChanged(QString value);
    void iconChanged(QString value);
    void writableChanged(bool value);
    void visibleChanged(bool value);
    void plusNameChanged(QString value);
    void minusNameChanged(QString value);
    void identificatorChanged(QString value);
    void largeButtonChanged(bool value);
    void largeButtonLabelChanged(QString value);
    void largeButtonColorChanged(QString value);
};

class homeform : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool labelHelp READ labelHelp NOTIFY changeLabelHelp)
    Q_PROPERTY(bool device READ getDevice NOTIFY changeOfdevice)
    Q_PROPERTY(int topBarHeight READ topBarHeight CONSTANT)
    Q_PROPERTY(QString info READ info NOTIFY infoChanged)
    Q_PROPERTY(QString signal READ signal NOTIFY signalChanged)
    Q_PROPERTY(QString startText READ startText NOTIFY startTextChanged)
    Q_PROPERTY(QString startIcon READ startIcon NOTIFY startIconChanged)
    Q_PROPERTY(QString startColor READ startColor NOTIFY startColorChanged)
    Q_PROPERTY(QString stopText READ stopText NOTIFY stopTextChanged)
    Q_PROPERTY(QString stopIcon READ stopIcon NOTIFY stopIconChanged)
    Q_PROPERTY(QString stopColor READ stopColor NOTIFY stopColorChanged)
    Q_PROPERTY(int zwiftLogin READ zwiftLogin NOTIFY zwiftLoginChanged)
    Q_PROPERTY(bool autoResistance READ autoResistance NOTIFY autoResistanceChanged WRITE setAutoResistance)
    Q_PROPERTY(bool stopRequested READ stopRequested NOTIFY stopRequestedChanged WRITE setStopRequestedChanged)
    Q_PROPERTY(bool startRequested READ startRequested NOTIFY startRequestedChanged WRITE setStartRequestedChanged)
    Q_PROPERTY(QString toastRequested READ toastRequested NOTIFY toastRequestedChanged WRITE setToastRequested)

  public:
    homeform(QQmlApplicationEngine *engine, bluetooth *bl);
    ~homeform() override;
    static homeform *singleton() { return m_singleton; }
    bluetooth *bluetoothManager;
    int topBarHeight() const { return 120; }
    QString info() const { return m_info; }
    QString signal();
    QString startText();
    QString startIcon();
    QString startColor();
    QString stopText();
    QString stopIcon();
    QString stopColor();
    bool labelHelp() const { return m_labelHelp; }
    int zwiftLogin() const { return m_zwiftLoginState; }
    bool autoResistance() const { return m_autoresistance; }
    void setAutoResistance(bool value) {
        m_autoresistance = value;
        emit autoResistanceChanged(value);
        if (bluetoothManager->device()) bluetoothManager->device()->setAutoResistance(value);
    }
    bool stopRequested() const { return m_stopRequested; }
    bool startRequested() const { return m_startRequested; }
    void setStopRequestedChanged(bool value) { m_stopRequested = value; emit stopRequestedChanged(value); }
    void setStartRequestedChanged(bool value) { m_startRequested = value; emit startRequestedChanged(value); }
    QString toastRequested() const { return m_toastRequested; }
    void setToastRequested(QString value) { m_toastRequested = value; emit toastRequestedChanged(value); }
    Q_INVOKABLE bool autoInclinationEnabled() {
        return bluetoothManager && bluetoothManager->device() && bluetoothManager->device()->VirtualDevice() &&
               static_cast<virtualtreadmill *>(bluetoothManager->device()->VirtualDevice())->autoInclinationEnabled();
    }
    Q_INVOKABLE void launchIFitApp();
    Q_INVOKABLE void sortTiles();
    static QString getWritableAppDir();
#ifdef Q_OS_ANDROID
    static QString getAndroidDataAppDir();
    QString getBluetoothName();
#endif
    Q_INVOKABLE bool handleKeyboardShortcut(const QString &sequence);
    Q_INVOKABLE void setNativeShortcutCaptureSuspended(bool suspended) { m_nativeShortcutCaptureSuspended = suspended; }

  public slots:
    void aboutToQuit();
    void restart();
    void Minus(const QString &name);
    void Plus(const QString &name);
    void Start();
    void Stop();
    void StopRequested();

  private slots:
    void StartRequested();
    void StartFromDevice();
    void PauseFromDevice();
    void StopFromDevice();
    void volumeDown();
    void volumeUp();
    void floatingOpen();
    void openFloatingWindowBrowser();
    void deviceConnected(QBluetoothDeviceInfo device);
    void bluetoothDeviceConnected(bluetoothdevice *device);
    void bluetoothDeviceDisconnected();
    void zwiftLoginState(bool ok);
    void onToastRequested(QString message) { setToastRequested(message); }
    void speedPlus();
    void speedMinus();
    void inclinationPlus();
    void inclinationMinus();
    void toggleAutoResistance() { setAutoResistance(!autoResistance()); }

  signals:
    void changeOfdevice();
    void changeLabelHelp(bool value);
    void signalChanged(QString value);
    void startTextChanged(QString value);
    void startIconChanged(QString value);
    void startColorChanged(QString value);
    void stopTextChanged(QString value);
    void stopIconChanged(QString value);
    void stopColorChanged(QString value);
    void infoChanged(QString value);
    void toastRequestedChanged(QString value);
    void autoResistanceChanged(bool value);
    void zwiftLoginChanged(int ok);
    void startRequestedChanged(bool value);
    void stopRequestedChanged(bool value);

  private:
    bool getDevice();
    void update();
    void Start_inner(bool send_event_to_device);
    static homeform *m_singleton;
    QQmlApplicationEngine *engine;
    TemplateInfoSenderBuilder *innerTemplateManager;
    ZwiftIncline *zwiftIncline;
    QList<QObject *> dataList;
    DataObject *speed;
    DataObject *inclination;
    DataObject *odometer;
    DataObject *pace;
    DataObject *heart;
    DataObject *elapsed;
    QTimer *timer;
    QString m_info = QStringLiteral("Connecting...");
    QString m_toastRequested;
    int m_zwiftLoginState = -1;
    bool m_labelHelp = true;
    bool paused = false;
    bool stopped = false;
    bool m_autoresistance = true;
    bool m_stopRequested = false;
    bool m_startRequested = false;
    bool m_nativeShortcutCaptureSuspended = false;
    bool floating_open = false;
};

#endif // HOMEFORM_H
