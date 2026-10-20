#include "../../src/x22i_defaults.h"

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QTemporaryDir>
#include <QtCore/QStringList>
#include <cmath>
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

static QStringList expectedKeys() {
    QStringList keys = {
        "miles_unit",
        "dircon_yes",
        "dircon_server_base_port",
        "virtual_device_bluetooth",
        "virtual_device_enabled",
        "zwift_api_autoinclination",
        "zwift_api_poll",
        "zwift_inclination_gain",
        "zwift_inclination_offset",
        "zwift_negative_inclination_x2",
        "real_inclination_to_virtual_treamill_bridge",
        "continuous_moving",
        "speed_gain",
        "speed_offset",
        "treadmill_inclination_ovveride_gain",
        "treadmill_inclination_ovveride_offset",
        "treadmill_step_incline",
        "inclination_delay_seconds",
        "treadmill_incline_min",
        "treadmill_incline_max",
        "min_inclination",
    };
    for (int halfSteps = 0; halfSteps <= 30; ++halfSteps) {
        const QString suffix = halfSteps == 1 ? QStringLiteral("05") : QString::number(halfSteps * 5);
        keys.append(QStringLiteral("treadmill_inclination_override_") + suffix);
    }
    keys.sort();
    return keys;
}

static void requireNumber(QSettings &settings, const char *key, double expected) {
    bool converted = false;
    const double actual = settings.value(QString::fromLatin1(key)).toDouble(&converted);
    require(converted && std::isfinite(actual) && std::abs(actual - expected) < 0.000001,
            "A required public numeric default is missing or incorrect.");
}

static void checkFreshSettings(QSettings &settings) {
    QStringList actualKeys = settings.allKeys();
    actualKeys.sort();
    require(actualKeys == expectedKeys() && actualKeys.size() == 52,
            "Fresh settings do not match the exact 52-key public allowlist.");
    require(settings.value("miles_unit").toBool(),
            "The fresh unit preference is incorrect.");
    require(settings.value("dircon_yes").toBool() &&
                settings.value("virtual_device_bluetooth").toBool() &&
                settings.value("virtual_device_enabled").toBool(),
            "Both virtual-device transports must be enabled.");
    require(settings.value("zwift_api_autoinclination").toBool(),
            "Fresh auto-incline must be enabled.");
    requireNumber(settings, "dircon_server_base_port", 36866);
    requireNumber(settings, "zwift_api_poll", 5);
    requireNumber(settings, "treadmill_step_incline", 0.5);
    requireNumber(settings, "zwift_inclination_gain", 0.5);
    requireNumber(settings, "zwift_inclination_offset", 0.0);
    requireNumber(settings, "treadmill_inclination_ovveride_gain", 0.01);
    requireNumber(settings, "treadmill_inclination_ovveride_offset", 0.1);
    requireNumber(settings, "inclination_delay_seconds", 3.0);
    requireNumber(settings, "treadmill_incline_min", 0.0);
    requireNumber(settings, "treadmill_incline_max", 100.0);
    requireNumber(settings, "min_inclination", -999.0);

    for (int halfSteps = 0; halfSteps <= 30; ++halfSteps) {
        const QString suffix = halfSteps == 1 ? QStringLiteral("05") : QString::number(halfSteps * 5);
        const QByteArray key = (QStringLiteral("treadmill_inclination_override_") + suffix).toLatin1();
        requireNumber(settings, key.constData(), halfSteps / 2.0);
    }

    const QStringList excluded = {
        "zwift_username", "zwift_password", "strava_accesstoken", "peloton_password",
        "cryptoKeySettingsProfiles", "app_opening", "profile_name", "bluetooth_lastdevice_address",
    };
    for (const QString &key : excluded)
        require(!settings.contains(key), "Fresh settings include a private or runtime key.");
}

int main(int argc, char *argv[]) {
    QCoreApplication application(argc, argv);
    try {
        QTemporaryDir directory;
        require(directory.isValid(), "Cannot create an isolated settings test directory.");
        const QString freshPath = directory.filePath(QStringLiteral("fresh.ini"));
        QVariantMap freshValues;
        {
            QSettings settings(freshPath, QSettings::IniFormat);
            settings.setFallbacksEnabled(false);
            require(settings.allKeys().isEmpty(), "The fresh fixture is not empty.");
            applyX22iDefaults(settings);
            checkFreshSettings(settings);
            for (const QString &key : settings.allKeys())
                freshValues.insert(key, settings.value(key));
            applyX22iDefaults(settings);
            checkFreshSettings(settings);
            for (auto it = freshValues.constBegin(); it != freshValues.constEnd(); ++it)
                require(settings.value(it.key()) == it.value(), "Repeated seeding changed a fresh value.");
            settings.sync();
            require(settings.status() == QSettings::NoError, "Cannot persist the fresh fixture.");
        }
        {
            QSettings settings(freshPath, QSettings::IniFormat);
            settings.setFallbacksEnabled(false);
            checkFreshSettings(settings);
            for (auto it = freshValues.constBegin(); it != freshValues.constEnd(); ++it)
                require(settings.value(it.key()).toString() == it.value().toString(),
                        "A fresh value changed after reloading.");
            applyX22iDefaults(settings);
            checkFreshSettings(settings);
            for (auto it = freshValues.constBegin(); it != freshValues.constEnd(); ++it)
                require(settings.value(it.key()).toString() == it.value().toString(),
                        "Reseeding after a reload changed a fresh value.");
        }

        // Synthetic fixtures only: never load application settings or real credentials.
        const QVariantMap saved = {
            {QStringLiteral("treadmill_step_incline"), 0.25},
            {QStringLiteral("zwift_api_autoinclination"), false},
            {QStringLiteral("zwift_inclination_gain"), 0.0},
            {QStringLiteral("treadmill_inclination_override_55"), 4.25},
            {QStringLiteral("zwift_username"), QStringLiteral("synthetic-user@example.invalid")},
            {QStringLiteral("zwift_password"), QStringLiteral("")},
            {QStringLiteral("app_opening"), 17},
        };
        QStringList savedKeys = expectedKeys();
        savedKeys << QStringLiteral("zwift_username") << QStringLiteral("zwift_password")
                  << QStringLiteral("app_opening");
        savedKeys.sort();
        const QString savedPath = directory.filePath(QStringLiteral("saved.ini"));
        {
            QSettings settings(savedPath, QSettings::IniFormat);
            settings.setFallbacksEnabled(false);
            for (auto it = saved.constBegin(); it != saved.constEnd(); ++it)
                settings.setValue(it.key(), it.value());
            settings.sync();
            require(settings.status() == QSettings::NoError, "Cannot persist the saved-value fixture.");
        }
        // Reopening between passes checks first-run migration and later restarts.
        for (int pass = 0; pass < 2; ++pass) {
            QSettings settings(savedPath, QSettings::IniFormat);
            settings.setFallbacksEnabled(false);
            for (auto it = saved.constBegin(); it != saved.constEnd(); ++it)
                require(settings.contains(it.key()) && settings.value(it.key()).toString() == it.value().toString(),
                        "A saved fixture value changed during persistence.");
            applyX22iDefaults(settings);
            applyX22iDefaults(settings);
            QStringList actualKeys = settings.allKeys();
            actualKeys.sort();
            require(actualKeys == savedKeys, "Seeding existing settings added unexpected keys.");
            for (auto it = saved.constBegin(); it != saved.constEnd(); ++it)
                require(settings.contains(it.key()) && settings.value(it.key()).toString() == it.value().toString(),
                        "Seeding overwrote a saved preference or synthetic credential.");
            requireNumber(settings, "treadmill_inclination_ovveride_gain", 0.01);
            requireNumber(settings, "inclination_delay_seconds", 3.0);
            settings.sync();
            require(settings.status() == QSettings::NoError, "Cannot persist seeded settings.");
        }
        std::cout << "X22i defaults tests passed: fresh settings, preservation, persistence, and idempotence.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "X22i defaults test failed: " << error.what() << '\n';
        return 1;
    }
}
