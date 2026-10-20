#ifndef WEBSERVERINFOSENDER_H
#define WEBSERVERINFOSENDER_H

#include <QHttpServer>
#include <QObject>
#include <QPointer>
#include <QTcpServer>
#include <QTimer>

class QWebSocket;

class WebServerInfoSender : public QObject {
    Q_OBJECT
  public:
    explicit WebServerInfoSender(QObject *parent = nullptr);
    void send(const QByteArray &data);

  signals:
    void onDataReceived(const QByteArray &data, QWebSocket *client);

  private slots:
    void listen();
    void onNewConnection();

  private:
    QHttpServer *httpServer;
    QTcpServer *tcpServer;
    QTimer retryTimer;
    QList<QPointer<QWebSocket>> clients;
};

#endif
