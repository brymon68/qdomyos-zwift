#include "qdebugfixup.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include "qzsettings.h"

KeepAwakeHelper::KeepAwakeHelper() {
    QSettings settings;
    bool wake = settings.value(QZSettings::android_wakelock, QZSettings::default_android_wakelock).toBool();
    if (!wake) {
        return;
    }

    activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity",
                                                         "()Landroid/app/Activity;");
    if (activity.isValid()) {
        QAndroidJniObject serviceName =
            QAndroidJniObject::getStaticObjectField<jstring>("android/content/Context", "POWER_SERVICE");
        if (serviceName.isValid()) {
            QAndroidJniObject powerMgr = activity.callObjectMethod(
                "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", serviceName.object<jobject>());
            if (powerMgr.isValid()) {
                jint levelAndFlags =
                    QAndroidJniObject::getStaticField<jint>("android/os/PowerManager", "SCREEN_DIM_WAKE_LOCK");

                QAndroidJniObject tag = QAndroidJniObject::fromString("My Tag");

                m_wakeLock =
                    powerMgr.callObjectMethod("newWakeLock", "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
                                              levelAndFlags, tag.object<jstring>());
            }
        }
    }

    if (m_wakeLock.isValid()) {
        m_wakeLock.callMethod<void>("acquire", "()V");
        qDebug() << "Locked device, can't go to standby anymore";
    } else {
        assert(false);
    }

    keepScreenOn(true);
}

void KeepAwakeHelper::keepScreenOn(bool on) {
    QtAndroid::runOnAndroidThread([on] {
        QAndroidJniObject activity = QtAndroid::androidActivity();
        if (activity.isValid()) {
            QAndroidJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

            if (window.isValid()) {
                const int FLAG_KEEP_SCREEN_ON = 128;
                if (on) {
                    window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                    qDebug() << "Activated : Keep screen ON";
                } else {
                    window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                    qDebug() << "Deactivated : Keep screen ON";
                }
            }
        }
        QAndroidJniEnvironment env;
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
    });
}

KeepAwakeHelper::~KeepAwakeHelper() {
    if (m_wakeLock.isValid()) {
        m_wakeLock.callMethod<void>("release", "()V");

        qDebug() << "Unlocked device, can now go to standby";

        keepScreenOn(false);
    }
}

#endif
