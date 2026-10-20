#ifndef ZWIFTWORKOUT_H
#define ZWIFTWORKOUT_H
#include "trainprogram.h"

class zwiftworkout {

  public:
    static QList<trainrow> load(const QString &filename, QString *description = nullptr, QString *tags = nullptr);
    static QList<trainrow> load(const QByteArray &input, QString *description = nullptr, QString *tags = nullptr);
    static QList<trainrow> loadJSON(const QString &input, QString *description = nullptr, QString *tags = nullptr);
    static bool isZwiftWorkoutFile(const QString &filename, const QString &extension = QString());
    static double normalizeIncline(double incline);

};

#endif // ZWIFTWORKOUT_H
