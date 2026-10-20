#ifndef QZSETTINGS_H
#define QZSETTINGS_H

#include <QString>

class QZSettings {
  private:
    QZSettings() {}

  public:

    /**
     *@brief Disable (true) or use (false) the device's heart rate service.
     */
    static const QString bike_heartrate_service;
    static constexpr bool default_bike_heartrate_service = false;

    /**
     *@brief Used to invoke a workaround whereby negative inclination is multiplied by 2.
     */
    static const QString zwift_negative_inclination_x2;
    static constexpr bool default_zwift_negative_inclination_x2 = false;

    /**
     *@brief An offset that will be applied to the inclination received from the client application.
     * calculated_inclination = raw_inclination * zwift_inclination_gain + zwift_inclination_offset
     */
    static const QString zwift_inclination_offset;
    static constexpr float default_zwift_inclination_offset = 0;

    /**
     *@brief A gain that will be applied to the inclination received from the client application.
     * calculated_inclination = raw_inclination * zwift_inclination_gain + zwift_inclination_offset
     */
    static const QString zwift_inclination_gain;
    static constexpr float default_zwift_inclination_gain = 0.5;

    /**
     *@brief Specifies whether or not to use miles (false) or kilometers (true) as the unit of distance.
     */
    static const QString miles_unit;
    static constexpr bool default_miles_unit = true;

    static const QString pause_on_start_treadmill;
    static constexpr bool default_pause_on_start_treadmill = false;

    /**
     *@brief Flag to indicate if it should be ignored (true) that the user has stopped doing work.
     */
    static const QString continuous_moving;
    static constexpr bool default_continuous_moving = false;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED.
     */
    static const QString speed_offset;
    static constexpr float default_speed_offset = 0;

    /**
     * @brief Adjusts value in a metric object that's configured specifically for measuring SPEED.
     */
    static const QString speed_gain;
    static constexpr float default_speed_gain = 1;

    /**
     *@brief Enable/disable the virtual device that connects QZ to the client app.
     */
    static const QString virtual_device_enabled;
    static constexpr bool default_virtual_device_enabled = true;
    /**
     *@brief Enable/disable the Bluetooth connectivity of the virtual device that connects QZ to the client app.
     */
    static const QString virtual_device_bluetooth;
    static constexpr bool default_virtual_device_bluetooth = true;

    static const QString android_wakelock;
    static constexpr bool default_android_wakelock = true;
    /**
     *@brief Specifies if the debug log file will be written.
     */
    static const QString log_debug;
    static constexpr bool default_log_debug = false;

    static const QString treadmill_step_speed;
    static constexpr float default_treadmill_step_speed = 0.1;

    static const QString treadmill_step_incline;
    static constexpr float default_treadmill_step_incline = 0.5;

    /**
     *@brief Enable the Wahoo Dircon device.
     */
    static const QString dircon_yes;
    static constexpr bool default_dircon_yes = true;

    static const QString dircon_server_base_port;
    static constexpr int default_dircon_server_base_port = 36866;

    /**
     *@brief Count of the number of times the app has been opened.
     */
    static const QString app_opening;
    static constexpr int default_app_opening = 0;

    static const QString floating_width;
    static constexpr int default_floating_width = 370;

    static const QString floating_height;
    static constexpr int default_floating_height = 210;

    static const QString floating_transparency;
    static constexpr int default_floating_transparency = 80;

    static const QString floating_startup;
    static constexpr int default_floating_startup = false;

    static const QString treadmill_inclination_override_0;
    static constexpr double default_treadmill_inclination_override_0 = 0;
    static const QString treadmill_inclination_override_05;
    static constexpr double default_treadmill_inclination_override_05 = 0.5;
    static const QString treadmill_inclination_override_10;
    static constexpr double default_treadmill_inclination_override_10 = 1.0;
    static const QString treadmill_inclination_override_15;
    static constexpr double default_treadmill_inclination_override_15 = 1.5;
    static const QString treadmill_inclination_override_20;
    static constexpr double default_treadmill_inclination_override_20 = 2.0;
    static const QString treadmill_inclination_override_25;
    static constexpr double default_treadmill_inclination_override_25 = 2.5;
    static const QString treadmill_inclination_override_30;
    static constexpr double default_treadmill_inclination_override_30 = 3.0;
    static const QString treadmill_inclination_override_35;
    static constexpr double default_treadmill_inclination_override_35 = 3.5;
    static const QString treadmill_inclination_override_40;
    static constexpr double default_treadmill_inclination_override_40 = 4.0;
    static const QString treadmill_inclination_override_45;
    static constexpr double default_treadmill_inclination_override_45 = 4.5;
    static const QString treadmill_inclination_override_50;
    static constexpr double default_treadmill_inclination_override_50 = 5.0;
    static const QString treadmill_inclination_override_55;
    static constexpr double default_treadmill_inclination_override_55 = 5.5;
    static const QString treadmill_inclination_override_60;
    static constexpr double default_treadmill_inclination_override_60 = 6.0;
    static const QString treadmill_inclination_override_65;
    static constexpr double default_treadmill_inclination_override_65 = 6.5;
    static const QString treadmill_inclination_override_70;
    static constexpr double default_treadmill_inclination_override_70 = 7.0;
    static const QString treadmill_inclination_override_75;
    static constexpr double default_treadmill_inclination_override_75 = 7.5;
    static const QString treadmill_inclination_override_80;
    static constexpr double default_treadmill_inclination_override_80 = 8.0;
    static const QString treadmill_inclination_override_85;
    static constexpr double default_treadmill_inclination_override_85 = 8.5;
    static const QString treadmill_inclination_override_90;
    static constexpr double default_treadmill_inclination_override_90 = 9.0;
    static const QString treadmill_inclination_override_95;
    static constexpr double default_treadmill_inclination_override_95 = 9.5;
    static const QString treadmill_inclination_override_100;
    static constexpr double default_treadmill_inclination_override_100 = 10.0;
    static const QString treadmill_inclination_override_105;
    static constexpr double default_treadmill_inclination_override_105 = 10.5;
    static const QString treadmill_inclination_override_110;
    static constexpr double default_treadmill_inclination_override_110 = 11.0;
    static const QString treadmill_inclination_override_115;
    static constexpr double default_treadmill_inclination_override_115 = 11.5;
    static const QString treadmill_inclination_override_120;
    static constexpr double default_treadmill_inclination_override_120 = 12.0;
    static const QString treadmill_inclination_override_125;
    static constexpr double default_treadmill_inclination_override_125 = 12.5;
    static const QString treadmill_inclination_override_130;
    static constexpr double default_treadmill_inclination_override_130 = 13.0;
    static const QString treadmill_inclination_override_135;
    static constexpr double default_treadmill_inclination_override_135 = 13.5;
    static const QString treadmill_inclination_override_140;
    static constexpr double default_treadmill_inclination_override_140 = 14.0;
    static const QString treadmill_inclination_override_145;
    static constexpr double default_treadmill_inclination_override_145 = 14.5;
    static const QString treadmill_inclination_override_150;
    static constexpr double default_treadmill_inclination_override_150 = 15.0;

    static const QString treadmill_inclination_ovveride_gain;
    static constexpr double default_treadmill_inclination_ovveride_gain = 0.01;

    static const QString treadmill_inclination_ovveride_offset;
    static constexpr double default_treadmill_inclination_ovveride_offset = 0.1;

    static const QString zwift_username;
    static const QString default_zwift_username;

    static const QString zwift_password;
    static const QString default_zwift_password;

    static const QString treadmill_incline_min;
    static constexpr float default_treadmill_incline_min = 0.0;

    static const QString treadmill_incline_max;
    static constexpr float default_treadmill_incline_max = 100.0;

    static const QString treadmill_speed_max;
    static constexpr float default_treadmill_speed_max = 100.0;

    static const QString treadmill_speed_min;
    static constexpr float default_treadmill_speed_min = 0.0;

    static const QString android_documents_folder;
    static constexpr bool default_android_documents_folder = false;

    static const QString zwift_api_autoinclination;
    static constexpr bool default_zwift_api_autoinclination = true;

    static const QString zwift_api_poll;
    static constexpr int default_zwift_api_poll = 5;

    static const QString inclination_delay_seconds;
    static constexpr float default_inclination_delay_seconds = 3.0;

    static const QString dircon_id;
    static constexpr int default_dircon_id = 0;

    static const QString min_inclination;
    static constexpr double default_min_inclination = -999.0;

    static const QString real_inclination_to_virtual_treamill_bridge;
    static constexpr bool default_real_inclination_to_virtual_treamill_bridge = false;

    /**
     * @brief Type of floating window to use. 0 = classic, 1 = horizontal
     */
    static const QString floatingwindow_type;
    static constexpr int default_floatingwindow_type = 0;

    /**
     * @brief Enable keyboard shortcuts
     */
    static const QString shortcuts_enabled;
    static constexpr bool default_shortcuts_enabled = false;

    static const QString shortcut_speed_plus;
    static const QString default_shortcut_speed_plus;
    static const QString shortcut_speed_minus;
    static const QString default_shortcut_speed_minus;

    static const QString shortcut_inclination_plus;
    static const QString default_shortcut_inclination_plus;
    static const QString shortcut_inclination_minus;
    static const QString default_shortcut_inclination_minus;

    static const QString shortcut_start_stop;
    static const QString default_shortcut_start_stop;

    static const QString shortcut_stop;
    static const QString default_shortcut_stop;

    /**
     * @brief Write the QSettings values using the constants from this namespace.
     * @param showDefaults Optionally indicates if the default should be shown with the key.
     */
    static void qDebugAllSettings(bool showDefaults = false);

    /**
     * @brief Restore the default value to all the settings
     */
    static void restoreAll();
};

#endif
