#include "metric.h"
#include "qdebugfixup.h"
#include "qzsettings.h"
#include <QSettings>

#ifdef TEST
static uint32_t random_value_uint32 = 0;
static uint8_t random_value_uint8 = 0;
#endif

metric::metric() {}

void metric::setType(_metric_type t, BLUETOOTH_TYPE bt) {
    m_type = t;
    m_bluetooth_type = bt;
}

void metric::setValue(double v, bool applyGainAndOffset) {
    QSettings settings;
    // these special cases are also in the valueRaw
    if (applyGainAndOffset) {
        if (m_type == METRIC_SPEED) {
            if (v > 0) {
                v *= settings.value(QZSettings::speed_gain, QZSettings::default_speed_gain).toDouble();
                v += settings.value(QZSettings::speed_offset, QZSettings::default_speed_offset).toDouble();
            }
        }
    }

    QDateTime now = QDateTime::currentDateTime();
    if (v != m_value && v != INFINITY) {
        m_valueChanged = now;
        if (m_last5.count() > 1) {
            double diff = v - m_value;
            double diffFromLastValue = qAbs(now.msecsTo(m_lastChanged));
            if (diffFromLastValue > 0)
                m_rateAtSec = diff * (1000.0 / diffFromLastValue);
            else
                m_rateAtSec = 0;
        }
    }

    // Refresh the sample timestamp even when the measured value is unchanged.
    m_lastChanged = now;

    m_value = v;

    if (paused) {
        return;
    }

    if (value() != 0 && value() != INFINITY) {
        m_countValue++;
        m_lapCountValue++;
        m_totValue += value();
        m_lapTotValue += value();
        m_last3.append(value());
        m_last5.append(value());
        m_last20.append(value());

        if (m_last3.count() > 3)
            m_last3.removeAt(0);

        if (m_last5.count() > 5)
            m_last5.removeAt(0);

        if (m_last20.count() > 20)
            m_last20.removeAt(0);

        if (value() < m_min) {
            m_min = value();
        }

        if (value() < m_lapMin) {
            m_lapMin = value();
        }

        if (value() > m_max) {
            m_max = value();
        }

        if (value() > m_lapMax) {
            m_lapMax = value();
        }
    }
}

void metric::clear(bool accumulator) {
    if (accumulator) {
        m_offset = m_value;
    }
    m_max = 0;
    m_totValue = 0;
    m_countValue = 0;
    m_min = 999999999;
    m_last3.clear();
    m_last5.clear();
    m_last20.clear();
    clearLap(accumulator);
#ifdef TEST
    random_value_uint8 = 0;
    random_value_uint32 = 0;
#endif
}

double metric::valueRaw() {
    double v = m_value;
    QSettings settings;

    if (m_type == METRIC_SPEED) {
        if (v > 0) {
            v /= settings.value(QZSettings::speed_gain, QZSettings::default_speed_gain).toDouble();
            v -= settings.value(QZSettings::speed_offset, QZSettings::default_speed_offset).toDouble();
        }
    }
    return v;
}

double metric::value() {
#ifdef TEST
    if (m_type != METRIC_ELAPSED) {
        return (double)(rand() % 256);
    } else {
        return (double)(random_value_uint32++);
    }

#endif
    return m_value - m_offset;
}

double metric::lapValue() { return m_value - m_lapOffset; }

double metric::average() {
    if (m_countValue == 0) {
        return 0;
    } else {
        return (m_totValue / m_countValue);
    }
}

double metric::lapAverage() {
    if (m_lapCountValue == 0) {
        return 0;
    } else {
        return (m_lapTotValue / m_lapCountValue);
    }
}

double metric::average5s() {
    if (m_last5.count() == 0)
        return 0;
    else {
        double sum = 0;
        uint8_t c = 0;
        QMutableListIterator<double> i(m_last5);
        while (i.hasNext()) {
            double b = i.next();
            sum += b;
            c++;
        }

        if (c > 0)
            return (sum / c);
        else
            return 0;
    }
}

double metric::average5sHarmonic() {
    // If current value is 0, return 0 immediately (user stopped pedaling)
    if (m_value == 0.0)
        return 0.0;

    if (m_last5.count() == 0)
        return 0;
    else {
        double reciprocalSum = 0;
        uint8_t c = 0;
        QMutableListIterator<double> i(m_last5);
        while (i.hasNext()) {
            double b = i.next();
            // If any value in buffer is 0, return 0 immediately
            if (b == 0.0)
                return 0.0;
            reciprocalSum += (1.0 / b);
            c++;
        }

        if (c > 0)
            return (c / reciprocalSum);  // Harmonic mean: n / (1/x1 + 1/x2 + ... + 1/xn)
        else
            return 0;
    }
}

double metric::average3sHarmonic() {
    // If current value is 0, return 0 immediately (user stopped pedaling)
    if (m_value == 0.0)
        return 0.0;

    if (m_last3.count() == 0)
        return 0;
    else {
        double reciprocalSum = 0;
        uint8_t c = 0;
        QMutableListIterator<double> i(m_last3);
        while (i.hasNext()) {
            double b = i.next();
            // If any value in buffer is 0, return 0 immediately
            if (b == 0.0)
                return 0.0;
            reciprocalSum += (1.0 / b);
            c++;
        }

        if (c > 0)
            return (c / reciprocalSum);  // Harmonic mean: n / (1/x1 + 1/x2 + ... + 1/xn)
        else
            return 0;
    }
}

double metric::average20s() {
    if (m_last20.count() == 0)
        return 0;
    else {
        double sum = 0;
        uint8_t c = 0;
        QMutableListIterator<double> i(m_last20);
        while (i.hasNext()) {
            double b = i.next();
            sum += b;
            c++;
        }

        if (c > 0)
            return (sum / c);
        else
            return 0;
    }
}


void metric::operator=(double v) { setValue(v); }

void metric::operator+=(double v) { setValue(m_value + v); }

double metric::min() { return m_min; }

double metric::max() { return m_max; }

double metric::lapMin() { return m_lapMin; }

double metric::lapMax() { return m_lapMax; }

void metric::setPaused(bool p) { paused = p; }

void metric::clearLap(bool accumulator) {
    if (accumulator) {
        m_lapOffset = m_value;
    }
    m_lapMax = 0;
    m_lapTotValue = 0;
    m_lapCountValue = 0;
    m_lapMin = 999999999;
}

void metric::setLap(bool accumulator) { clearLap(accumulator); }
