#ifndef KEEPAWAKEHELPER_H
#define KEEPAWAKEHELPER_H

#include <QtCore>

#ifdef Q_OS_ANDROID

#include <QApplication>
#include <QSettings>
#include <QtAndroidExtras>

static QAndroidJniObject activity;

class KeepAwakeHelper {
  public:
    KeepAwakeHelper();
    virtual ~KeepAwakeHelper();
    void keepScreenOn(bool on);

  private:
    QAndroidJniObject m_wakeLock;
};

#endif
#endif // KEEPAWAKEHELPER_H
