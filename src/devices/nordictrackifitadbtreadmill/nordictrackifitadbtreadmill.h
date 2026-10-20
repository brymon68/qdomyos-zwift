#ifndef NORDICTRACKIFITADBTREADMILL_H
#define NORDICTRACKIFITADBTREADMILL_H

#include "treadmill.h"
#include <QDateTime>
#include <QTimer>

class nordictrackifitadbtreadmill : public treadmill {
    Q_OBJECT
  public:
    enum RequestOrigin {
        ORIGIN_USER,     // User initiated action
        ORIGIN_GRPC,     // Action from gRPC/iFit app
        ORIGIN_INTERNAL  // Internal app logic
    };

    explicit nordictrackifitadbtreadmill(bool noHeartService = false);
    bool connected() override;
    bool canStartStop() override;
    double minStepSpeed() override { return 0.1; }

  private:
    void forceIncline(double incline);
    void forceSpeed(double speed);

    // gRPC integration methods
    void initializeGrpcService();
    void startGrpcMetricsUpdates();
    void stopGrpcMetricsUpdates();
    double getGrpcSpeed();
    double getGrpcIncline();
    double getGrpcCadence();
    double getGrpcHeartRate();
    void setGrpcSpeed(double speed);
    void setGrpcIncline(double incline);
    void startGrpcWorkoutStateMonitoring();
    int getGrpcWorkoutState();

    QTimer *refresh;

    bool initRequest = false;

    bool grpcInitialized = false;
    int previousWorkoutState = 1; // WORKOUT_STATE_IDLE

    QDateTime lastTimeDataReceived;
    bool firstDataReceived = true;

    // Track origin of stop/pause/start requests to prevent echoing gRPC events
    RequestOrigin requestStartOrigin = ORIGIN_USER;
    RequestOrigin requestStopOrigin = ORIGIN_USER;
    RequestOrigin requestPauseOrigin = ORIGIN_USER;

  signals:
    void disconnected();
    void debug(QString string);

  private slots:

    void changeInclinationRequested(double grade, double percentage);

    void update();
    
  public slots:
    void stopGrpcService();
};

#endif // NORDICTRACKIFITADBTREADMILL_H
