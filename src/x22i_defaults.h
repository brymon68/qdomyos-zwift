#ifndef X22I_DEFAULTS_H
#define X22I_DEFAULTS_H

#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QVariant>

inline void applyX22iDefaults(QSettings &settings) {
    const QMap<QString, QVariant> defaults = {
        {QStringLiteral("nordictrack_2950_ip"), QStringLiteral("localhost")},
        {QStringLiteral("nordictrack_x22i"), true},
        {QStringLiteral("nordictrack_ifit_adb_remote"), false},
        {QStringLiteral("miles_unit"), true},
        {QStringLiteral("dircon_yes"), true},
        {QStringLiteral("dircon_server_base_port"), 36866},
        {QStringLiteral("virtual_device_bluetooth"), true},
        {QStringLiteral("virtual_device_enabled"), true},
        {QStringLiteral("virtual_device_force_bike"), false},
        {QStringLiteral("virtual_device_force_treadmill"), false},
        {QStringLiteral("zwift_api_autoinclination"), true},
        {QStringLiteral("zwift_api_poll"), 5},
        {QStringLiteral("zwift_inclination_gain"), 0.5},
        {QStringLiteral("zwift_inclination_offset"), 0.0},
        {QStringLiteral("zwift_negative_inclination_x2"), false},
        // Keep legacy key spellings compatible with existing configurations.
        {QStringLiteral("real_inclination_to_virtual_treamill_bridge"), false},
        {QStringLiteral("continuous_moving"), false},
        {QStringLiteral("speed_gain"), 1.0},
        {QStringLiteral("speed_offset"), 0.0},
        {QStringLiteral("treadmill_force_speed"), false},
        {QStringLiteral("zwift_ocr"), false},
        {QStringLiteral("zwift_workout_ocr"), false},
        {QStringLiteral("zwift_ocr_climb_portal"), false},
        {QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")},
        {QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled")},
        {QStringLiteral("treadmill_inclination_ovveride_gain"), 0.01},
        {QStringLiteral("treadmill_inclination_ovveride_offset"), 0.1},
        {QStringLiteral("treadmill_step_incline"), 0.5},
        {QStringLiteral("inclination_delay_seconds"), 3.0},
        {QStringLiteral("treadmill_incline_min"), 0.0},
        // Preserve the saved value; 100 is not a physical hardware limit.
        {QStringLiteral("treadmill_incline_max"), 100.0},
        {QStringLiteral("min_inclination"), -999.0},
        {QStringLiteral("trainprogram_incline_min"), 0.0},
        {QStringLiteral("trainprogram_incline_max"), 15.0},
        {QStringLiteral("trainprogram_speed_min"), 8.0},
        {QStringLiteral("trainprogram_speed_max"), 16.0},
        {QStringLiteral("treadmill_inclination_override_0"), 0.0},
        {QStringLiteral("treadmill_inclination_override_05"), 0.5},
        {QStringLiteral("treadmill_inclination_override_10"), 1.0},
        {QStringLiteral("treadmill_inclination_override_15"), 1.5},
        {QStringLiteral("treadmill_inclination_override_20"), 2.0},
        {QStringLiteral("treadmill_inclination_override_25"), 2.5},
        {QStringLiteral("treadmill_inclination_override_30"), 3.0},
        {QStringLiteral("treadmill_inclination_override_35"), 3.5},
        {QStringLiteral("treadmill_inclination_override_40"), 4.0},
        {QStringLiteral("treadmill_inclination_override_45"), 4.5},
        {QStringLiteral("treadmill_inclination_override_50"), 5.0},
        {QStringLiteral("treadmill_inclination_override_55"), 5.5},
        {QStringLiteral("treadmill_inclination_override_60"), 6.0},
        {QStringLiteral("treadmill_inclination_override_65"), 6.5},
        {QStringLiteral("treadmill_inclination_override_70"), 7.0},
        {QStringLiteral("treadmill_inclination_override_75"), 7.5},
        {QStringLiteral("treadmill_inclination_override_80"), 8.0},
        {QStringLiteral("treadmill_inclination_override_85"), 8.5},
        {QStringLiteral("treadmill_inclination_override_90"), 9.0},
        {QStringLiteral("treadmill_inclination_override_95"), 9.5},
        {QStringLiteral("treadmill_inclination_override_100"), 10.0},
        {QStringLiteral("treadmill_inclination_override_105"), 10.5},
        {QStringLiteral("treadmill_inclination_override_110"), 11.0},
        {QStringLiteral("treadmill_inclination_override_115"), 11.5},
        {QStringLiteral("treadmill_inclination_override_120"), 12.0},
        {QStringLiteral("treadmill_inclination_override_125"), 12.5},
        {QStringLiteral("treadmill_inclination_override_130"), 13.0},
        {QStringLiteral("treadmill_inclination_override_135"), 13.5},
        {QStringLiteral("treadmill_inclination_override_140"), 14.0},
        {QStringLiteral("treadmill_inclination_override_145"), 14.5},
        {QStringLiteral("treadmill_inclination_override_150"), 15.0},
    };

    for (auto it = defaults.constBegin(); it != defaults.constEnd(); ++it) {
        if (!settings.contains(it.key()))
            settings.setValue(it.key(), it.value());
    }
}

#endif // X22I_DEFAULTS_H
