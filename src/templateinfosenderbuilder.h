#ifndef TEMPLATEINFOSENDERBUILDER_H
#define TEMPLATEINFOSENDERBUILDER_H

#include <QObject>
#include <QTimer>

class bluetoothdevice;
class WebServerInfoSender;
class QWebSocket;

class TemplateInfoSenderBuilder : public QObject {
    Q_OBJECT
  public:
    explicit TemplateInfoSenderBuilder(QObject *parent = nullptr);
    void start(bluetoothdevice *device);
    void stop();

  signals:
    void floatingClose();
    void speed_Plus();
    void speed_Minus();
    void inclination_Plus();
    void inclination_Minus();
    void Start();
    void Pause();
    void Stop();
    void autoResistance();

  private slots:
    void sendTelemetry();
    void onDataReceived(const QByteArray &data, QWebSocket *client);

  private:
    bluetoothdevice *device = nullptr;
    QTimer updateTimer;
    WebServerInfoSender *server = nullptr;
};

#endif
