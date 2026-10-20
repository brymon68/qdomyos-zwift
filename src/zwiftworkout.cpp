#include "zwiftworkout.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <cmath>

QList<trainrow> zwiftworkout::load(const QString &filename, QString *description, QString *tags) {
    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly))
        return {};
    return load(input.readAll(), description, tags);
}

bool zwiftworkout::isZwiftWorkoutFile(const QString &filename, const QString &extension) {
    const QString ext = extension.isEmpty() ? QFileInfo(filename).suffix().toUpper() : extension.toUpper();
    if (ext == QStringLiteral("ZWO"))
        return true;

    if (ext != QStringLiteral("XML"))
        return false;

    QFile input(filename);
    if (!input.open(QIODevice::ReadOnly))
        return false;

    QXmlStreamReader stream(&input);
    while (!stream.atEnd()) {
        stream.readNext();
        if (stream.isStartElement()) {
            const QString rootName = stream.name().toString();
            return rootName.compare(QStringLiteral("workout_file"), Qt::CaseInsensitive) == 0 ||
                   rootName.compare(QStringLiteral("Workout"), Qt::CaseInsensitive) == 0;
        }
    }

    return false;
}

double zwiftworkout::normalizeIncline(double incline) {
    if (incline == -100)
        return incline;

    // Zwift workout files may store incline either as a fraction (0.02 = 2%)
    // or as a percent value (2.0 = 2%). QZ trainrows use percent values.
    if (qAbs(incline) <= 0.40)
        return incline * 100.0;

    return incline;
}

static bool isRunningSport(const QString &sport) {
    return sport.isEmpty() || sport.compare(QStringLiteral("run"), Qt::CaseInsensitive) == 0 ||
           sport.compare(QStringLiteral("running"), Qt::CaseInsensitive) == 0;
}

static bool directStep(const QJsonObject &values, bool speedInMetersPerSecond, trainrow &row) {
    // Relative intensity and race-pace targets require user data this app does not keep.
    for (const QString &key : values.keys()) {
        if (key.contains(QStringLiteral("power"), Qt::CaseInsensitive) ||
            key.compare(QStringLiteral("pace"), Qt::CaseInsensitive) == 0)
            return false;
    }
    const double duration = values.value(QStringLiteral("duration")).toDouble(-1);
    const bool hasSpeed = values.contains(QStringLiteral("speed"));
    const bool hasIncline = values.contains(QStringLiteral("incline"));
    const double speed = values.value(QStringLiteral("speed")).toDouble(-1);
    const double incline = values.value(QStringLiteral("incline")).toDouble(-100);
    if (!std::isfinite(duration) || duration < 1 || duration >= 86400 || (!hasSpeed && !hasIncline) ||
        (hasSpeed && (!values.value(QStringLiteral("speed")).isDouble() || !std::isfinite(speed) || speed < 0)) ||
        (hasIncline && (!values.value(QStringLiteral("incline")).isDouble() || !std::isfinite(incline))))
        return false;
    row.duration = QTime(0, 0).addSecs(static_cast<int>(duration));
    if (hasSpeed) {
        row.speed = speedInMetersPerSecond ? speed * 3.6 : speed;
        row.forcespeed = 1;
    }
    if (hasIncline)
        row.inclination = zwiftworkout::normalizeIncline(incline);
    return true;
}

QList<trainrow> zwiftworkout::loadJSON(const QString &input, QString *description, QString *tags) {
    if (description) description->clear();
    if (tags) tags->clear();
    const QJsonDocument document = QJsonDocument::fromJson(input.toUtf8());
    if (!document.isObject()) return {};
    const QJsonObject workout = document.object();
    if (!isRunningSport(workout.value(QStringLiteral("sportType")).toString())) return {};
    const QString durationType = workout.value(QStringLiteral("durationType")).toString();
    if (!durationType.isEmpty() && durationType.compare(QStringLiteral("time"), Qt::CaseInsensitive) != 0)
        return {};
    if (description) *description = workout.value(QStringLiteral("description")).toString();
    if (tags) {
        for (const QJsonValue &tag : workout.value(QStringLiteral("tags")).toArray())
            tags->append("#" + tag.toObject().value(QStringLiteral("name")).toString() + " ");
    }
    QList<trainrow> rows;
    for (const QJsonValue &entry : workout.value(QStringLiteral("workout")).toArray()) {
        QJsonObject values;
        const QJsonObject object = entry.toObject();
        for (const QString &key : object.keys()) values.insert(key.toLower(), object.value(key));
        const QString type = values.value(QStringLiteral("type")).toString().toLower();
        trainrow row;
        if ((type != QStringLiteral("step") && type != QStringLiteral("steadystate")) ||
            !directStep(values, type == QStringLiteral("steadystate"), row)) {
            qWarning() << "Unsupported workout: explicit running speed or incline is required";
            return {};
        }
        rows.append(row);
    }
    return rows;
}

QList<trainrow> zwiftworkout::load(const QByteArray &input, QString *description, QString *tags) {
    if (description) description->clear();
    if (tags) tags->clear();
    QList<trainrow> rows;
    QXmlStreamReader stream(input);
    while (!stream.atEnd()) {
        stream.readNext();
        if (!stream.isStartElement()) continue;
        const QString name = stream.name().toString().toLower();
        const QXmlStreamAttributes attributes = stream.attributes();
        if (name == QStringLiteral("sporttype")) {
            if (!isRunningSport(stream.readElementText())) return {};
        } else if (name == QStringLiteral("durationtype")) {
            if (stream.readElementText().compare(QStringLiteral("time"), Qt::CaseInsensitive) != 0) return {};
        } else if (name == QStringLiteral("description")) {
            const QString text = stream.readElementText();
            if (description) *description = text;
        } else if (name == QStringLiteral("tag")) {
            if (tags) tags->append("#" + attributes.value(QStringLiteral("name")).toString() + " ");
        } else if (name == QStringLiteral("textevent")) {
            if (!rows.isEmpty()) {
                trainrow::TextEvent event;
                event.timeoffset = attributes.value(QStringLiteral("timeoffset")).toUInt();
                event.message = attributes.value(QStringLiteral("message")).toString();
                rows.last().textEvents.append(event);
            }
        } else if (name == QStringLiteral("step") || name == QStringLiteral("steadystate")) {
            QJsonObject values;
            for (const QXmlStreamAttribute &attribute : attributes) {
                bool valid = false;
                const double value = attribute.value().toDouble(&valid);
                values.insert(attribute.name().toString().toLower(), valid ? QJsonValue(value) : QJsonValue());
            }
            trainrow row;
            if (!directStep(values, name == QStringLiteral("steadystate"), row)) {
                qWarning() << "Unsupported workout: explicit running speed or incline is required";
                return {};
            }
            rows.append(row);
        } else if (name == QStringLiteral("intervalst") || name == QStringLiteral("ramp") ||
                   name == QStringLiteral("warmup") || name == QStringLiteral("cooldown") ||
                   name == QStringLiteral("freeride")) {
            qWarning() << "Unsupported workout segment:" << name;
            return {};
        }
    }
    return stream.hasError() ? QList<trainrow>() : rows;
}
