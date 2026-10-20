#include "nordictrackifitadbtreadmill.h"
#include "homeform.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QSettings>
#include <chrono>

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

using namespace std::chrono_literals;

nordictrackifitadbtreadmill::nordictrackifitadbtreadmill(bool noHeartService) {
    lastTimeDataReceived = QDateTime::currentDateTime();
    QSettings settings;
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &nordictrackifitadbtreadmill::update);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this,
            &nordictrackifitadbtreadmill::stopGrpcService);
    refresh->start(200ms);

    // Initialize gRPC service on Android
#ifdef Q_OS_ANDROID
    emit debug(QString("Initializing gRPC service..."));
    initializeGrpcService();
    emit debug(QString("After initializeGrpcService(), grpcInitialized=%1").arg(grpcInitialized));
    if (grpcInitialized) {
        emit debug(QString("Starting gRPC metrics updates and workout state monitoring..."));
        startGrpcMetricsUpdates();
        startGrpcWorkoutStateMonitoring();
    } else {
        emit debug(QString("gRPC not initialized, skipping metrics and workout state monitoring"));
    }
#endif

    initRequest = true;

    if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
        debug("creating virtual treadmill interface...");
        auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
        connect(virtualTreadmill, &virtualtreadmill::debug, this, &nordictrackifitadbtreadmill::debug);
        connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                &nordictrackifitadbtreadmill::changeInclinationRequested);
        setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
    }
}

void nordictrackifitadbtreadmill::forceIncline(double incline) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        setGrpcIncline(incline);
    }
#endif
}

void nordictrackifitadbtreadmill::forceSpeed(double speed) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        setGrpcSpeed(speed);
    }
#endif
}

void nordictrackifitadbtreadmill::update() {

    QSettings settings;
    update_metrics();

    if (initRequest) {
        initRequest = false;
        emit connectedAndDiscovered();
    }

    if (requestStart != -1) {
        emit debug(QStringLiteral("starting..."));

#ifdef Q_OS_ANDROID
        // Start or Resume workout via gRPC WorkoutService (only if not originated from gRPC)
        if (grpcInitialized && requestStartOrigin != ORIGIN_GRPC) {
            if (isPaused()) {
                // Resume from pause
                QAndroidJniObject::callStaticMethod<void>(
                    "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                    "resumeWorkout",
                    "()V"
                );
                emit debug("Resumed workout via gRPC WorkoutService");
            } else {
                // Start new workout
                QAndroidJniObject::callStaticMethod<void>(
                    "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                    "startWorkout",
                    "()V"
                );
                emit debug("Started workout via gRPC WorkoutService");
            }
        } else if (requestStartOrigin == ORIGIN_GRPC) {
            emit debug("Skipping gRPC start/resumeWorkout (originated from gRPC event)");
        }
#endif

        requestStart = -1;
        requestStartOrigin = ORIGIN_USER; // Reset to default
        emit tapeStarted();
    }
    if (requestStop != -1) {
        emit debug(QStringLiteral("stopping..."));

#ifdef Q_OS_ANDROID
        // Stop workout via gRPC WorkoutService (only if not originated from gRPC)
        if (grpcInitialized && requestStopOrigin != ORIGIN_GRPC) {
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "stopWorkout",
                "()V"
            );
            emit debug("Stopped workout via gRPC WorkoutService");
        } else if (requestStopOrigin == ORIGIN_GRPC) {
            emit debug("Skipping gRPC stopWorkout (originated from gRPC event)");
        }
#endif

        requestStop = -1;
        requestStopOrigin = ORIGIN_USER; // Reset to default
    }

    if (requestPause != -1) {
        emit debug(QStringLiteral("pausing..."));

#ifdef Q_OS_ANDROID
        // Pause workout via gRPC WorkoutService (only if not originated from gRPC)
        if (grpcInitialized && requestPauseOrigin != ORIGIN_GRPC) {
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "pauseWorkout",
                "()V"
            );
            emit debug("Paused workout via gRPC WorkoutService");
        } else if (requestPauseOrigin == ORIGIN_GRPC) {
            emit debug("Skipping gRPC pauseWorkout (originated from gRPC event)");
        }
#endif

        requestPause = -1;
        requestPauseOrigin = ORIGIN_USER; // Reset to default
    }

#ifdef Q_OS_ANDROID
    // Read the local iFit service metrics.
    if (grpcInitialized) {
        double currentSpeed = getGrpcSpeed();
        double currentIncline = getGrpcIncline();
        double currentCadence = getGrpcCadence();
        
        // Update the metrics if they've changed
        if (currentSpeed != Speed.value()) {
            Speed = currentSpeed;
            emit debug(QString("gRPC Speed: %1").arg(currentSpeed));
        }
        
        if (currentIncline != Inclination.value()) {
            Inclination = currentIncline;
            emit debug(QString("gRPC Incline: %1").arg(currentIncline));
        }
        
        if (currentCadence != Cadence.value() && currentCadence > 0) {
            Cadence = currentCadence;
            emit debug(QString("gRPC Cadence: %1").arg(currentCadence));
        }
        

        if (!firstDataReceived) {
            Distance += ((Speed.value() / (double)3600.0) /
                        ((double)1000.0 / (double)(lastTimeDataReceived.msecsTo(QDateTime::currentDateTime()))));
        }
        lastTimeDataReceived = QDateTime::currentDateTime();
        firstDataReceived = false;

        {
            double currentHeartRate = getGrpcHeartRate();
            if (currentHeartRate > 0 && currentHeartRate != Heart.value()) {
                Heart = currentHeartRate;
                emit debug(QString("gRPC Heart Rate: %1").arg(currentHeartRate));
            }
        }

        if (requestInclination != -100) {
            setGrpcIncline(requestInclination);
            requestInclination = -100;
        }

        if (requestSpeed != -1) {
            setGrpcSpeed(requestSpeed);
            requestSpeed = -1;
        }

        // Monitor workout state changes from iFit app
        // Get next state transition from queue (returns null if queue empty)
        QAndroidJniEnvironment env;
        QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getNextWorkoutStateChange",
            "()[I"
        );

        if (result.isValid() && result.object() != nullptr) {
            jintArray javaArray = static_cast<jintArray>(result.object());
            jint* states = env->GetIntArrayElements(javaArray, nullptr);

            if (states != nullptr) {
                int previousState = states[0];
                int currentWorkoutState = states[1];
                env->ReleaseIntArrayElements(javaArray, states, JNI_ABORT);

                emit debug(QString("Workout state changed: %1 -> %2").arg(previousState).arg(currentWorkoutState));

                // WORKOUT_STATE_IDLE = 1
                // WORKOUT_STATE_RUNNING = 3
                // WORKOUT_STATE_PAUSED = 4
                // WORKOUT_STATE_RESULTS = 5

                // If workout started (IDLE/PAUSED -> RUNNING)
                if (currentWorkoutState == 3 && (previousState == 1 || previousState == 4)) {
                    emit debug("Workout started in iFit app - auto-starting QZ recording");
                    if (homeform::singleton()) {
                        const bool qzPausedOrStopped = isPaused();
                        // If starting from IDLE and QZ is on complete screen (stopped), close it first
                        if (previousState == 1 && qzPausedOrStopped) {
                            emit debug("Closing complete screen before starting new workout");
                            emit homeform::singleton()->closeCompleteScreenRequested();
                        }

                        // Only call Start() if QZ is not already running (to avoid toggling pause)
                        if (qzPausedOrStopped) {
                            requestStartOrigin = ORIGIN_GRPC;
                            requestStopOrigin = ORIGIN_GRPC;
                            requestPauseOrigin = ORIGIN_GRPC;
                            homeform::singleton()->Start();
                        } else {
                            emit debug("QZ already running, not calling Start() again");
                        }
                    }
                }
                // If workout stopped/completed (any state -> RESULTS or PAUSED/RUNNING/RESULTS -> IDLE)
                else if (currentWorkoutState == 5 ||
                         (currentWorkoutState == 1 && (previousState == 3 || previousState == 4 || previousState == 5))) {
                    emit debug("Workout completed in iFit app - showing workout complete screen");
                    if (homeform::singleton()) {
                        requestStopOrigin = ORIGIN_GRPC;
                        homeform::singleton()->StopRequested();
                    }
                }
                // If workout paused (RUNNING -> PAUSED)
                else if (currentWorkoutState == 4 && previousState == 3) {
                    emit debug("Workout paused in iFit app - auto-pausing QZ recording");
                    if (homeform::singleton()) {
                        requestStopOrigin = ORIGIN_GRPC;
                        requestPauseOrigin = ORIGIN_GRPC;
                        homeform::singleton()->Start();
                    }
                }

                previousWorkoutState = currentWorkoutState;
            }
        }
    }
#endif
}

void nordictrackifitadbtreadmill::changeInclinationRequested(double grade, double percentage) {
    // these treadmills support negative inclination
    /*if (percentage < 0)
        percentage = 0;*/
    changeInclination(grade, percentage);
}

bool nordictrackifitadbtreadmill::connected() { return true; }

bool nordictrackifitadbtreadmill::canStartStop() { return grpcInitialized; }

void nordictrackifitadbtreadmill::stopGrpcService() {
    stopGrpcMetricsUpdates();
}

// gRPC integration methods implementation
void nordictrackifitadbtreadmill::initializeGrpcService() {
#ifdef Q_OS_ANDROID
    if (!grpcInitialized) {
        try {
            // iFIT's TLS certificate requires the localhost hostname, not its numeric address.
            const QString host = QStringLiteral("localhost");
            
            // Set Android context first
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "setContext",
                "(Landroid/content/Context;)V",
                QtAndroid::androidContext().object()
            );
            
            // Now initialize the service with the local hostname.
            QAndroidJniObject hostObj = QAndroidJniObject::fromString(host);
            QAndroidJniObject::callStaticMethod<void>(
                "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
                "initialize",
                "(Ljava/lang/String;)V",
                hostObj.object<jstring>()
            );
            grpcInitialized = true;
            emit debug("gRPC service initialized successfully with host: " + host);
        } catch (...) {
            emit debug("Failed to initialize gRPC service");
            grpcInitialized = false;
        }
    }
#endif
}

void nordictrackifitadbtreadmill::startGrpcMetricsUpdates() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "startMetricsUpdates",
            "()V"
        );
        emit debug("Started gRPC metrics updates");
    }
#endif
}

void nordictrackifitadbtreadmill::stopGrpcMetricsUpdates() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "stopMetricsUpdates",
            "()V"
        );
        emit debug("Stopped gRPC metrics updates");
    }
#endif
}

double nordictrackifitadbtreadmill::getGrpcSpeed() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentSpeed",
            "()D"
        );
    }
#endif
    return 0.0;
}

double nordictrackifitadbtreadmill::getGrpcIncline() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentIncline",
            "()D"
        );
    }
#endif
    return 0.0;
}

double nordictrackifitadbtreadmill::getGrpcCadence() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentCadence",
            "()D"
        );
    }
#endif
    return 0.0;
}

double nordictrackifitadbtreadmill::getGrpcHeartRate() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jdouble>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getCurrentHeartRate",
            "()D"
        );
    }
#endif
    return 0.0;
}

void nordictrackifitadbtreadmill::setGrpcSpeed(double speed) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "adjustSpeed",
            "(D)V",
            speed - Speed.value()
        );
        emit debug(QString("Set gRPC speed to: %1").arg(speed));
    }
#endif
}

void nordictrackifitadbtreadmill::setGrpcIncline(double incline) {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "adjustIncline",
            "(D)V",
            incline - Inclination.value()
        );
        emit debug(QString("Set gRPC incline to: %1").arg(incline));
    }
#endif
}

void nordictrackifitadbtreadmill::startGrpcWorkoutStateMonitoring() {
#ifdef Q_OS_ANDROID
    emit debug(QString("startGrpcWorkoutStateMonitoring() called, grpcInitialized=%1").arg(grpcInitialized));
    if (grpcInitialized) {
        QAndroidJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "startWorkoutStateMonitoring",
            "()V"
        );
        emit debug(QString("Started gRPC workout state monitoring"));

        // Synchronize QZ with current iFit workout state
        int initialState = getGrpcWorkoutState();
        emit debug(QString("Initial workout state at startup: %1").arg(initialState));

        if (initialState == 3) { // RUNNING
            emit debug("iFit workout already running - QZ already started by default, no action needed");
        } else if (initialState == 4) { // PAUSED
            emit debug("iFit workout paused - auto-pausing QZ recording");
            if (homeform::singleton()) {
                requestPauseOrigin = ORIGIN_GRPC;
                homeform::singleton()->Start(); // This will pause since QZ is already running
            }
        } else if (initialState == 1 || initialState == 5) { // IDLE or RESULTS
            emit debug("iFit workout stopped/idle - auto-stopping QZ recording");
            if (homeform::singleton()) {
                requestStopOrigin = ORIGIN_GRPC;
                homeform::singleton()->Stop();
            }
        }

        // Update previous state to avoid triggering false transitions
        previousWorkoutState = initialState;
    } else {
        emit debug(QString("Cannot start workout state monitoring: gRPC not initialized"));
    }
#endif
}

int nordictrackifitadbtreadmill::getGrpcWorkoutState() {
#ifdef Q_OS_ANDROID
    if (grpcInitialized) {
        return QAndroidJniObject::callStaticMethod<jint>(
            "org/cagnulen/qdomyoszwift/GrpcTreadmillService",
            "getWorkoutState",
            "()I"
        );
    }
#endif
    return 1; // WORKOUT_STATE_IDLE
}
