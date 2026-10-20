#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QBluetoothDeviceInfo>
#include <QObject>

#include "devices/bluetoothdevice.h"
#include "signalhandler.h"

class nordictrackifitadbtreadmill;

class bluetooth : public QObject, public SignalHandler {
    Q_OBJECT

  public:
    explicit bluetooth(bool logs, bool noHeartService = false);
    ~bluetooth() override;
    bluetoothdevice *device();

  signals:
    void deviceConnected(QBluetoothDeviceInfo device);
    void searchingStop();
    void bluetoothDeviceConnected(bluetoothdevice *device);
    void bluetoothDeviceDisconnected();

  public slots:
    void restart();
    void debug(const QString &message);

  private:
    void start();
    void connectedAndDiscovered();
    bool handleSignal(int signal) override;

    nordictrackifitadbtreadmill *x22i = nullptr;
    bool logs;
    bool noHeartService;
};

#endif // BLUETOOTH_H
