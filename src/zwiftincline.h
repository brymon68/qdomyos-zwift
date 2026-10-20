#ifndef ZWIFTINCLINE_H
#define ZWIFTINCLINE_H

#include "zwiftinclinegrade.h"
#include <QObject>
#include <QTimer>

class AuthToken;
class World;
class bluetooth;

class ZwiftIncline : public QObject {
    Q_OBJECT

  public:
    explicit ZwiftIncline(bluetooth *manager, QObject *parent = nullptr);
    ~ZwiftIncline() override;

  signals:
    void toastRequest(QString message);
    void zwiftLoginState(bool ok);

  private:
    void poll();
    bluetooth *bluetoothManager;
    AuthToken *authToken = nullptr;
    World *world = nullptr;
    int playerId = -1;
    int pollCounter = 5;
    bool loginToastShown = false;
    bool pollInProgress = false;
    QTimer timer;
    ZwiftInclineGrade terrain;
};

#endif // ZWIFTINCLINE_H
