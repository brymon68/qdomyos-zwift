#include "qzsettings.h"
#include <QDebug>
#include <QSettings>
#include <algorithm>
#include <cstdint>
#include <vector>
//--------------------------------------------------------------------------------------------
const QString QZSettings::bike_heartrate_service = QStringLiteral("bike_heartrate_service");
const QString QZSettings::zwift_negative_inclination_x2 = QStringLiteral("zwift_negative_inclination_x2");
const QString QZSettings::zwift_inclination_offset = QStringLiteral("zwift_inclination_offset");
const QString QZSettings::zwift_inclination_gain = QStringLiteral("zwift_inclination_gain");
const QString QZSettings::miles_unit = QStringLiteral("miles_unit");
const QString QZSettings::pause_on_start_treadmill = QStringLiteral("pause_on_start_treadmill");
const QString QZSettings::continuous_moving = QStringLiteral("continuous_moving");
const QString QZSettings::speed_offset = QStringLiteral("speed_offset");
const QString QZSettings::speed_gain = QStringLiteral("speed_gain");
const QString QZSettings::virtual_device_enabled = QStringLiteral("virtual_device_enabled");
const QString QZSettings::virtual_device_bluetooth = QStringLiteral("virtual_device_bluetooth");
const QString QZSettings::android_wakelock = QStringLiteral("android_wakelock");
const QString QZSettings::log_debug = QStringLiteral("log_debug");
const QString QZSettings::treadmill_step_speed = QStringLiteral("treadmill_step_speed");
const QString QZSettings::treadmill_step_incline = QStringLiteral("treadmill_step_incline");
// from version 2.10.23
// not used anymore because it's an elliptical not a treadmill. Don't remove this
// it will cause corruption in the settings
// const QString QZSettings:: nordictrack_fs5i_treadmill = QStringLiteral("nordictrack_fs5i_treadmill");
//
const QString QZSettings::dircon_yes = QStringLiteral("dircon_yes");
const QString QZSettings::dircon_server_base_port = QStringLiteral("dircon_server_base_port");
const QString QZSettings::app_opening = QStringLiteral("app_opening");
// from version 2.11.22
const QString QZSettings::floating_height = QStringLiteral("floating_height");
const QString QZSettings::floating_width = QStringLiteral("floating_width");
const QString QZSettings::floating_transparency = QStringLiteral("floating_transparency");
const QString QZSettings::floating_startup = QStringLiteral("floating_startup");
const QString QZSettings::treadmill_inclination_override_0 = QStringLiteral("treadmill_inclination_override_0");
const QString QZSettings::treadmill_inclination_override_05 = QStringLiteral("treadmill_inclination_override_05");
const QString QZSettings::treadmill_inclination_override_10 = QStringLiteral("treadmill_inclination_override_10");
const QString QZSettings::treadmill_inclination_override_15 = QStringLiteral("treadmill_inclination_override_15");
const QString QZSettings::treadmill_inclination_override_20 = QStringLiteral("treadmill_inclination_override_20");
const QString QZSettings::treadmill_inclination_override_25 = QStringLiteral("treadmill_inclination_override_25");
const QString QZSettings::treadmill_inclination_override_30 = QStringLiteral("treadmill_inclination_override_30");
const QString QZSettings::treadmill_inclination_override_35 = QStringLiteral("treadmill_inclination_override_35");
const QString QZSettings::treadmill_inclination_override_40 = QStringLiteral("treadmill_inclination_override_40");
const QString QZSettings::treadmill_inclination_override_45 = QStringLiteral("treadmill_inclination_override_45");
const QString QZSettings::treadmill_inclination_override_50 = QStringLiteral("treadmill_inclination_override_50");
const QString QZSettings::treadmill_inclination_override_55 = QStringLiteral("treadmill_inclination_override_55");
const QString QZSettings::treadmill_inclination_override_60 = QStringLiteral("treadmill_inclination_override_60");
const QString QZSettings::treadmill_inclination_override_65 = QStringLiteral("treadmill_inclination_override_65");
const QString QZSettings::treadmill_inclination_override_70 = QStringLiteral("treadmill_inclination_override_70");
const QString QZSettings::treadmill_inclination_override_75 = QStringLiteral("treadmill_inclination_override_75");
const QString QZSettings::treadmill_inclination_override_80 = QStringLiteral("treadmill_inclination_override_80");
const QString QZSettings::treadmill_inclination_override_85 = QStringLiteral("treadmill_inclination_override_85");
const QString QZSettings::treadmill_inclination_override_90 = QStringLiteral("treadmill_inclination_override_90");
const QString QZSettings::treadmill_inclination_override_95 = QStringLiteral("treadmill_inclination_override_95");
const QString QZSettings::treadmill_inclination_override_100 = QStringLiteral("treadmill_inclination_override_100");
const QString QZSettings::treadmill_inclination_override_105 = QStringLiteral("treadmill_inclination_override_105");
const QString QZSettings::treadmill_inclination_override_110 = QStringLiteral("treadmill_inclination_override_110");
const QString QZSettings::treadmill_inclination_override_115 = QStringLiteral("treadmill_inclination_override_115");
const QString QZSettings::treadmill_inclination_override_120 = QStringLiteral("treadmill_inclination_override_120");
const QString QZSettings::treadmill_inclination_override_125 = QStringLiteral("treadmill_inclination_override_125");
const QString QZSettings::treadmill_inclination_override_130 = QStringLiteral("treadmill_inclination_override_130");
const QString QZSettings::treadmill_inclination_override_135 = QStringLiteral("treadmill_inclination_override_135");
const QString QZSettings::treadmill_inclination_override_140 = QStringLiteral("treadmill_inclination_override_140");
const QString QZSettings::treadmill_inclination_override_145 = QStringLiteral("treadmill_inclination_override_145");
const QString QZSettings::treadmill_inclination_override_150 = QStringLiteral("treadmill_inclination_override_150");
const QString QZSettings::treadmill_inclination_ovveride_gain = QStringLiteral("treadmill_inclination_ovveride_gain");
const QString QZSettings::treadmill_inclination_ovveride_offset =
    QStringLiteral("treadmill_inclination_ovveride_offset");
const QString QZSettings::zwift_username = QStringLiteral("zwift_username");
const QString QZSettings::default_zwift_username = QStringLiteral("");
const QString QZSettings::zwift_password = QStringLiteral("zwift_password");
const QString QZSettings::default_zwift_password = QStringLiteral("");
const QString QZSettings::treadmill_incline_min = QStringLiteral("treadmill_incline_min");
const QString QZSettings::treadmill_incline_max = QStringLiteral("treadmill_incline_max");
const QString QZSettings::treadmill_speed_max = QStringLiteral("treadmill_speed_max");
const QString QZSettings::treadmill_speed_min = QStringLiteral("treadmill_speed_min");
const QString QZSettings::android_documents_folder = QStringLiteral("android_documents_folder");
const QString QZSettings::zwift_api_autoinclination = QStringLiteral("zwift_api_autoinclination");
const QString QZSettings::zwift_api_poll = QStringLiteral("zwift_api_poll");
const QString QZSettings::inclination_delay_seconds = QStringLiteral("inclination_delay_seconds");
const QString QZSettings::dircon_id = QStringLiteral("dircon_id");
const QString QZSettings::min_inclination = QStringLiteral("min_inclination");









const QString QZSettings::real_inclination_to_virtual_treamill_bridge = QStringLiteral("real_inclination_to_virtual_treamill_bridge");

















const QString QZSettings::floatingwindow_type = QStringLiteral("floatingwindow_type");


const QString QZSettings::shortcuts_enabled = QStringLiteral("shortcuts_enabled");
const QString QZSettings::shortcut_speed_plus = QStringLiteral("shortcut_speed_plus");
const QString QZSettings::default_shortcut_speed_plus = QStringLiteral("");
const QString QZSettings::shortcut_speed_minus = QStringLiteral("shortcut_speed_minus");
const QString QZSettings::default_shortcut_speed_minus = QStringLiteral("");
const QString QZSettings::shortcut_inclination_plus = QStringLiteral("shortcut_inclination_plus");
const QString QZSettings::default_shortcut_inclination_plus = QStringLiteral("");
const QString QZSettings::shortcut_inclination_minus = QStringLiteral("shortcut_inclination_minus");
const QString QZSettings::default_shortcut_inclination_minus = QStringLiteral("");
const QString QZSettings::shortcut_start_stop = QStringLiteral("shortcut_start_stop");
const QString QZSettings::default_shortcut_start_stop = QStringLiteral("");
const QString QZSettings::shortcut_stop = QStringLiteral("shortcut_stop");
const QString QZSettings::default_shortcut_stop = QStringLiteral("");

const uint32_t allSettingsCount = 76;

QVariant allSettings[allSettingsCount][2] = {
    {QZSettings::bike_heartrate_service, QZSettings::default_bike_heartrate_service},
    {QZSettings::zwift_negative_inclination_x2, QZSettings::default_zwift_negative_inclination_x2},
    {QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset},
    {QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain},
    {QZSettings::miles_unit, QZSettings::default_miles_unit},
    {QZSettings::pause_on_start_treadmill, QZSettings::default_pause_on_start_treadmill},
    {QZSettings::continuous_moving, QZSettings::default_continuous_moving},
    {QZSettings::speed_offset, QZSettings::default_speed_offset},
    {QZSettings::speed_gain, QZSettings::default_speed_gain},
    {QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled},
    {QZSettings::virtual_device_bluetooth, QZSettings::default_virtual_device_bluetooth},
    {QZSettings::android_wakelock, QZSettings::default_android_wakelock},
    {QZSettings::log_debug, QZSettings::default_log_debug},
    {QZSettings::treadmill_step_speed, QZSettings::default_treadmill_step_speed},
    {QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline},
    {QZSettings::dircon_yes, QZSettings::default_dircon_yes},
    {QZSettings::dircon_server_base_port, QZSettings::default_dircon_server_base_port},
    {QZSettings::app_opening, QZSettings::default_app_opening},
    {QZSettings::floating_height, QZSettings::default_floating_height},
    {QZSettings::floating_width, QZSettings::default_floating_width},
    {QZSettings::floating_transparency, QZSettings::default_floating_transparency},
    {QZSettings::floating_startup, QZSettings::default_floating_startup},
    {QZSettings::treadmill_inclination_override_0, QZSettings::default_treadmill_inclination_override_0},
    {QZSettings::treadmill_inclination_override_05, QZSettings::default_treadmill_inclination_override_05},
    {QZSettings::treadmill_inclination_override_10, QZSettings::default_treadmill_inclination_override_10},
    {QZSettings::treadmill_inclination_override_15, QZSettings::default_treadmill_inclination_override_15},
    {QZSettings::treadmill_inclination_override_20, QZSettings::default_treadmill_inclination_override_20},
    {QZSettings::treadmill_inclination_override_25, QZSettings::default_treadmill_inclination_override_25},
    {QZSettings::treadmill_inclination_override_30, QZSettings::default_treadmill_inclination_override_30},
    {QZSettings::treadmill_inclination_override_35, QZSettings::default_treadmill_inclination_override_35},
    {QZSettings::treadmill_inclination_override_40, QZSettings::default_treadmill_inclination_override_40},
    {QZSettings::treadmill_inclination_override_45, QZSettings::default_treadmill_inclination_override_45},
    {QZSettings::treadmill_inclination_override_50, QZSettings::default_treadmill_inclination_override_50},
    {QZSettings::treadmill_inclination_override_55, QZSettings::default_treadmill_inclination_override_55},
    {QZSettings::treadmill_inclination_override_60, QZSettings::default_treadmill_inclination_override_60},
    {QZSettings::treadmill_inclination_override_65, QZSettings::default_treadmill_inclination_override_65},
    {QZSettings::treadmill_inclination_override_70, QZSettings::default_treadmill_inclination_override_70},
    {QZSettings::treadmill_inclination_override_75, QZSettings::default_treadmill_inclination_override_75},
    {QZSettings::treadmill_inclination_override_80, QZSettings::default_treadmill_inclination_override_80},
    {QZSettings::treadmill_inclination_override_85, QZSettings::default_treadmill_inclination_override_85},
    {QZSettings::treadmill_inclination_override_90, QZSettings::default_treadmill_inclination_override_90},
    {QZSettings::treadmill_inclination_override_95, QZSettings::default_treadmill_inclination_override_95},
    {QZSettings::treadmill_inclination_override_100, QZSettings::default_treadmill_inclination_override_100},
    {QZSettings::treadmill_inclination_override_105, QZSettings::default_treadmill_inclination_override_105},
    {QZSettings::treadmill_inclination_override_110, QZSettings::default_treadmill_inclination_override_110},
    {QZSettings::treadmill_inclination_override_115, QZSettings::default_treadmill_inclination_override_115},
    {QZSettings::treadmill_inclination_override_120, QZSettings::default_treadmill_inclination_override_120},
    {QZSettings::treadmill_inclination_override_125, QZSettings::default_treadmill_inclination_override_125},
    {QZSettings::treadmill_inclination_override_130, QZSettings::default_treadmill_inclination_override_130},
    {QZSettings::treadmill_inclination_override_135, QZSettings::default_treadmill_inclination_override_135},
    {QZSettings::treadmill_inclination_override_140, QZSettings::default_treadmill_inclination_override_140},
    {QZSettings::treadmill_inclination_override_145, QZSettings::default_treadmill_inclination_override_145},
    {QZSettings::treadmill_inclination_override_150, QZSettings::default_treadmill_inclination_override_150},
    {QZSettings::treadmill_inclination_ovveride_gain, QZSettings::default_treadmill_inclination_ovveride_gain},
    {QZSettings::treadmill_inclination_ovveride_offset, QZSettings::default_treadmill_inclination_ovveride_offset},
    {QZSettings::zwift_username, QZSettings::default_zwift_username},
    {QZSettings::zwift_password, QZSettings::default_zwift_password},
    {QZSettings::treadmill_incline_min, QZSettings::default_treadmill_incline_min},
    {QZSettings::treadmill_incline_max, QZSettings::default_treadmill_incline_max},
    {QZSettings::treadmill_speed_max, QZSettings::default_treadmill_speed_max},
    {QZSettings::treadmill_speed_min, QZSettings::default_treadmill_speed_min},
    {QZSettings::android_documents_folder, QZSettings::default_android_documents_folder},
    {QZSettings::zwift_api_autoinclination, QZSettings::default_zwift_api_autoinclination},
    {QZSettings::zwift_api_poll, QZSettings::default_zwift_api_poll},
    {QZSettings::inclination_delay_seconds, QZSettings::default_inclination_delay_seconds},
    {QZSettings::dircon_id, QZSettings::default_dircon_id},
    {QZSettings::min_inclination, QZSettings::default_min_inclination},








    {QZSettings::real_inclination_to_virtual_treamill_bridge, QZSettings::default_real_inclination_to_virtual_treamill_bridge},







    {QZSettings::floatingwindow_type, QZSettings::default_floatingwindow_type},
    {QZSettings::shortcuts_enabled, QZSettings::default_shortcuts_enabled},
    {QZSettings::shortcut_speed_plus, QZSettings::default_shortcut_speed_plus},
    {QZSettings::shortcut_speed_minus, QZSettings::default_shortcut_speed_minus},
    {QZSettings::shortcut_inclination_plus, QZSettings::default_shortcut_inclination_plus},
    {QZSettings::shortcut_inclination_minus, QZSettings::default_shortcut_inclination_minus},
    {QZSettings::shortcut_start_stop, QZSettings::default_shortcut_start_stop},
    {QZSettings::shortcut_stop, QZSettings::default_shortcut_stop},
};

void QZSettings::qDebugAllSettings(bool showDefaults) {
    QSettings settings;
    // make a copy of the settings for sorting
    std::vector<QVariant *> sorted;
    for (uint32_t i = 0; i < allSettingsCount; i++) {
        sorted.push_back(allSettings[i]);
    }
    // sort the settings alphabetically
    struct {
        bool operator()(QVariant *a, QVariant *b) { return a[0].toString() < b[0].toString(); }
    } comparer;
    std::sort(sorted.begin(), sorted.end(), comparer);
    for (uint32_t i = 0; i < sorted.size(); i++) {
        QVariant *item = sorted[i];
        QString key = item[0].toString();
        QVariant defaultValue = item[1];
        if (!showDefaults) {
            qDebug() << key << settings.value(key, defaultValue);
        } else {
            qDebug() << "(" << key << ", " << defaultValue << ") = " << settings.value(key, defaultValue);
        }
    }
}

void QZSettings::restoreAll() {
    qDebug() << QStringLiteral("RESTORING SETTINGS!");
    QSettings settings;
    for (uint32_t i = 0; i < allSettingsCount; i++) {
        settings.setValue(allSettings[i][0].toString(), allSettings[i][1]);
    }
}
