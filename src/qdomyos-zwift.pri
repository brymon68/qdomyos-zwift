include(../defaults.pri)
QT += bluetooth widgets xml positioning quick websockets texttospeech location multimedia
QTPLUGIN += qavfmediaplayer
QT+= charts core-private sql concurrent

qtHaveModule(httpserver) {
    QT += httpserver
    DEFINES += Q_HTTPSERVER
    SOURCES += webserverinfosender.cpp
    HEADERS += webserverinfosender.h

    # android and iOS are using ChartJS
    unix:android: {
        QT+= webview
        DEFINES += CHARTJS
    }
    ios: {
        QT+= webview
        DEFINES += CHARTJS
    }
#	 win32: {
#	     DEFINES += CHARTJS
#		}
}

CONFIG += c++17 console app_bundle optimize_full ltcg

CONFIG += qmltypes

#win32: CONFIG += webengine
#unix:!android: CONFIG += webengine

win32:DEFINES += _ITERATOR_DEBUG_LEVEL=0
win32:!mingw:LIBS += -llibprotobuf -llibprotoc -labseil_dll -llibprotobuf-lite -ldbghelp -L$$PWD 

QML_IMPORT_NAME = org.cagnulein.qdomyoszwift
QML_IMPORT_MAJOR_VERSION = 1
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

win32:QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc -llibcrypto-1_1-x64 -llibssl-1_1-x64 -L$$PWD/../windows_openssl
win32:QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc -llibcrypto-1_1-x64 -llibssl-1_1-x64 -L$$PWD/../windows_openssl

QMAKE_LFLAGS_RELEASE += -s
QMAKE_CXXFLAGS += -fno-sized-deallocation
mingw: QMAKE_CXXFLAGS += -Wa,-mbig-obj
msvc {
   win32:QMAKE_CXXFLAGS_DEBUG += /RTC1
}
unix:android: {
    CONFIG -= optimize_size
    QMAKE_CFLAGS_OPTIMIZE_FULL -= -Oz
    QMAKE_CFLAGS_OPTIMIZE_FULL += -O3
}
macx: CONFIG += debug
win32: CONFIG += debug
macx: CONFIG += static
macx {
    QMAKE_INFO_PLIST = macx/Info.plist
}
INCLUDEPATH += qmdnsengine/src/include

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS IO_UNDER_QT NOMINMAX


# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# include(../qtzeroconf/qtzeroconf.pri)

SOURCES += \
    $$PWD/androidqlog.cpp \
    $$PWD/fitdatabaseprocessor.cpp \
    $$PWD/logwriter.cpp \
    $$PWD/fitbackupwriter.cpp \
    $$PWD/filesearcher.cpp \
    $$PWD/mqtt/qmqttauthenticationproperties.cpp \
    $$PWD/mqtt/qmqttclient.cpp \
    $$PWD/mqtt/qmqttconnection.cpp \
    $$PWD/mqtt/qmqttconnectionproperties.cpp \
    $$PWD/mqtt/qmqttcontrolpacket.cpp \
    $$PWD/mqtt/qmqttmessage.cpp \
    $$PWD/mqtt/qmqttpublishproperties.cpp \
    $$PWD/mqtt/qmqttsubscription.cpp \
    $$PWD/mqtt/qmqttsubscriptionproperties.cpp \
    $$PWD/mqtt/qmqtttopicfilter.cpp \
    $$PWD/mqtt/qmqtttopicname.cpp \
    $$PWD/mqtt/qmqtttype.cpp \
    $$PWD/osc.cpp \
    $$PWD/workoutloaderworker.cpp \
    $$PWD/workoutmodel.cpp \
QTelnet.cpp \
virtualdevices/virtualdevice.cpp \
androidactivityresultreceiver.cpp \
androidadblog.cpp \
localipaddress.cpp \
windows_zwift_workout_paddleocr_thread.cpp \
PathController.cpp \
characteristics/characteristicnotifier2a53.cpp \
characteristics/characteristicnotifier2acc.cpp \
characteristics/characteristicnotifier2acd.cpp \
characteristics/characteristicnotifier2ad9.cpp \
characteristics/characteristicwriteprocessor.cpp \
devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.cpp \
qmdnsengine/src/src/abstractserver.cpp \
qmdnsengine/src/src/bitmap.cpp \
qmdnsengine/src/src/browser.cpp \
qmdnsengine/src/src/cache.cpp \
qmdnsengine/src/src/dns.cpp \
qmdnsengine/src/src/hostname.cpp \
qmdnsengine/src/src/mdns.cpp \
qmdnsengine/src/src/message.cpp \
qmdnsengine/src/src/prober.cpp \
qmdnsengine/src/src/provider.cpp \
qmdnsengine/src/src/query.cpp \
qmdnsengine/src/src/record.cpp \
qmdnsengine/src/src/resolver.cpp \
qmdnsengine/src/src/server.cpp \
qmdnsengine/src/src/service.cpp \
devices/bluetooth.cpp \
devices/bluetoothdevice.cpp \
characteristics/characteristicnotifier2a37.cpp \
characteristics/characteristicnotifier2ad2.cpp \
characteristics/characteristicwriteprocessor2ad9.cpp \
devices/dircon/dirconmanager.cpp \
devices/dircon/dirconpacket.cpp \
devices/dircon/dirconprocessor.cpp \
filedownloader.cpp \
fit-sdk/fit.cpp \
fit-sdk/fit_accumulated_field.cpp \
fit-sdk/fit_accumulator.cpp \
fit-sdk/fit_buffer_encode.cpp \
fit-sdk/fit_buffered_mesg_broadcaster.cpp \
fit-sdk/fit_buffered_record_mesg_broadcaster.cpp \
fit-sdk/fit_crc.cpp \
fit-sdk/fit_date_time.cpp \
fit-sdk/fit_decode.cpp \
fit-sdk/fit_developer_field.cpp \
fit-sdk/fit_developer_field_definition.cpp \
fit-sdk/fit_developer_field_description.cpp \
fit-sdk/fit_encode.cpp \
fit-sdk/fit_factory.cpp \
fit-sdk/fit_field.cpp \
fit-sdk/fit_field_base.cpp \
fit-sdk/fit_field_definition.cpp \
fit-sdk/fit_mesg.cpp \
fit-sdk/fit_mesg_broadcaster.cpp \
fit-sdk/fit_mesg_definition.cpp \
fit-sdk/fit_mesg_with_event_broadcaster.cpp \
fit-sdk/fit_profile.cpp \
fit-sdk/fit_protocol_validator.cpp \
fit-sdk/fit_unicode.cpp \
gpx.cpp \
homeform.cpp \
keepawakehelper.cpp \
main.cpp \
metric.cpp \
qfit.cpp \
qzsettings.cpp \
screencapture.cpp \
sessionline.cpp \
signalhandler.cpp \
tcpclientinfosender.cpp \
templateinfosender.cpp \
templateinfosenderbuilder.cpp \
devices/treadmill.cpp \
trainprogram.cpp \
virtualdevices/virtualtreadmill.cpp \
scanrecordresult.cpp \
windows_zwift_incline_paddleocr_thread.cpp \
zwiftworkout.cpp
   
macx: SOURCES += macos/lockscreen.mm

#zwift api
msvc {
    SOURCES += zwift-api/zwift_messages.pb.cc
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += fit-sdk/ devices/

HEADERS += \
    $$PWD/fitdatabaseprocessor.h \
    $$PWD/logwriter.h \
    $$PWD/fitbackupwriter.h \
    $$PWD/filesearcher.h \
    $$PWD/osc.h \
    $$PWD/oscpp/client.hpp \
    $$PWD/oscpp/detail/endian.hpp \
    $$PWD/oscpp/detail/host.hpp \
    $$PWD/oscpp/detail/stream.hpp \
    $$PWD/oscpp/error.hpp \
    $$PWD/oscpp/print.hpp \
    $$PWD/oscpp/server.hpp \
    $$PWD/oscpp/types.hpp \
    $$PWD/oscpp/util.hpp \
    $$PWD/mqtt/qmqttauthenticationproperties.h \
    $$PWD/mqtt/qmqttclient.h \
    $$PWD/mqtt/qmqttclient_p.h \
    $$PWD/mqtt/qmqttconnection_p.h \
    $$PWD/mqtt/qmqttconnectionproperties.h \
    $$PWD/mqtt/qmqttconnectionproperties_p.h \
    $$PWD/mqtt/qmqttcontrolpacket_p.h \
    $$PWD/mqtt/qmqttglobal.h \
    $$PWD/mqtt/qmqttmessage.h \
    $$PWD/mqtt/qmqttmessage_p.h \
    $$PWD/mqtt/qmqttpublishproperties.h \
    $$PWD/mqtt/qmqttpublishproperties_p.h \
    $$PWD/mqtt/qmqttsubscription.h \
    $$PWD/mqtt/qmqttsubscription_p.h \
    $$PWD/mqtt/qmqttsubscriptionproperties.h \
    $$PWD/mqtt/qmqtttopicfilter.h \
    $$PWD/mqtt/qmqtttopicname.h \
    $$PWD/mqtt/qmqtttype.h \
    $$PWD/workoutloaderworker.h \
    $$PWD/workoutmodel.h \
QTelnet.h \
windows_zwift_workout_paddleocr_thread.h \
zwift-api/PlayerStateWrapper.h \
zwift-api/zwift_client_auth.h \
virtualdevices/virtualdevice.h \
androidactivityresultreceiver.h \
androidadblog.h \
localipaddress.h \
PathController.h \
characteristics/characteristicnotifier2a53.h \
characteristics/characteristicnotifier2acc.h \
characteristics/characteristicnotifier2acd.h \
characteristics/characteristicnotifier2ad9.h \
definitions.h \
devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.h \
qmdnsengine/src/include/qmdnsengine/abstractserver.h \
qmdnsengine/src/include/qmdnsengine/bitmap.h \
qmdnsengine/src/include/qmdnsengine/browser.h \
qmdnsengine/src/include/qmdnsengine/cache.h \
qmdnsengine/src/include/qmdnsengine/dns.h \
qmdnsengine/src/include/qmdnsengine/hostname.h \
qmdnsengine/src/include/qmdnsengine/mdns.h \
qmdnsengine/src/include/qmdnsengine/message.h \
qmdnsengine/src/include/qmdnsengine/prober.h \
qmdnsengine/src/include/qmdnsengine/provider.h \
qmdnsengine/src/include/qmdnsengine/query.h \
qmdnsengine/src/include/qmdnsengine/record.h \
qmdnsengine/src/include/qmdnsengine/resolver.h \
qmdnsengine/src/include/qmdnsengine/server.h \
qmdnsengine/src/include/qmdnsengine/service.h \
qmdnsengine/src/src/bitmap_p.h \
qmdnsengine/src/src/browser_p.h \
qmdnsengine/src/src/cache_p.h \
qmdnsengine/src/src/hostname_p.h \
qmdnsengine/src/src/message_p.h \
qmdnsengine/src/src/prober_p.h \
qmdnsengine/src/src/provider_p.h \
qmdnsengine/src/src/query_p.h \
qmdnsengine/src/src/record_p.h \
qmdnsengine/src/src/resolver_p.h \
qmdnsengine/src/src/server_p.h \
qmdnsengine/src/src/service_p.h \
devices/bluetooth.h \
devices/bluetoothdevice.h \
characteristics/characteristicnotifier.h \
characteristics/characteristicnotifier2a37.h \
characteristics/characteristicnotifier2ad2.h \
characteristics/characteristicwriteprocessor.h \
characteristics/characteristicwriteprocessor2ad9.h \
devices/dircon/dirconmanager.h \
devices/dircon/dirconpacket.h \
devices/dircon/dirconprocessor.h \
filedownloader.h \
fit-sdk/FitDecode.h \
fit-sdk/FitDeveloperField.h \
fit-sdk/FitEncode.h \
fit-sdk/FitField.h \
fit-sdk/FitFieldDefinition.h \
fit-sdk/FitMesg.h \
fit-sdk/FitMesgDefinition.h \
fit-sdk/fit.hpp \
fit-sdk/fit_accelerometer_data_mesg.hpp \
fit-sdk/fit_accelerometer_data_mesg_listener.hpp \
fit-sdk/fit_accumulated_field.hpp \
fit-sdk/fit_accumulator.hpp \
fit-sdk/fit_activity_mesg.hpp \
fit-sdk/fit_activity_mesg_listener.hpp \
fit-sdk/fit_ant_channel_id_mesg.hpp \
fit-sdk/fit_ant_channel_id_mesg_listener.hpp \
fit-sdk/fit_ant_rx_mesg.hpp \
fit-sdk/fit_ant_rx_mesg_listener.hpp \
fit-sdk/fit_ant_tx_mesg.hpp \
fit-sdk/fit_ant_tx_mesg_listener.hpp \
fit-sdk/fit_aviation_attitude_mesg.hpp \
fit-sdk/fit_aviation_attitude_mesg_listener.hpp \
fit-sdk/fit_barometer_data_mesg.hpp \
fit-sdk/fit_barometer_data_mesg_listener.hpp \
fit-sdk/fit_bike_profile_mesg.hpp \
fit-sdk/fit_bike_profile_mesg_listener.hpp \
fit-sdk/fit_blood_pressure_mesg.hpp \
fit-sdk/fit_blood_pressure_mesg_listener.hpp \
fit-sdk/fit_buffer_encode.hpp \
fit-sdk/fit_buffered_mesg_broadcaster.hpp \
fit-sdk/fit_buffered_record_mesg.hpp \
fit-sdk/fit_buffered_record_mesg_broadcaster.hpp \
fit-sdk/fit_buffered_record_mesg_listener.hpp \
fit-sdk/fit_cadence_zone_mesg.hpp \
fit-sdk/fit_cadence_zone_mesg_listener.hpp \
fit-sdk/fit_camera_event_mesg.hpp \
fit-sdk/fit_camera_event_mesg_listener.hpp \
fit-sdk/fit_capabilities_mesg.hpp \
fit-sdk/fit_capabilities_mesg_listener.hpp \
fit-sdk/fit_climb_pro_mesg.hpp \
fit-sdk/fit_climb_pro_mesg_listener.hpp \
fit-sdk/fit_config.hpp \
fit-sdk/fit_connectivity_mesg.hpp \
fit-sdk/fit_connectivity_mesg_listener.hpp \
fit-sdk/fit_course_mesg.hpp \
fit-sdk/fit_course_mesg_listener.hpp \
fit-sdk/fit_course_point_mesg.hpp \
fit-sdk/fit_course_point_mesg_listener.hpp \
fit-sdk/fit_crc.hpp \
fit-sdk/fit_date_time.hpp \
fit-sdk/fit_decode.hpp \
fit-sdk/fit_developer_data_id_mesg.hpp \
fit-sdk/fit_developer_data_id_mesg_listener.hpp \
fit-sdk/fit_developer_field.hpp \
fit-sdk/fit_developer_field_definition.hpp \
fit-sdk/fit_developer_field_description.hpp \
fit-sdk/fit_developer_field_description_listener.hpp \
fit-sdk/fit_device_info_mesg.hpp \
fit-sdk/fit_device_info_mesg_listener.hpp \
fit-sdk/fit_device_settings_mesg.hpp \
fit-sdk/fit_device_settings_mesg_listener.hpp \
fit-sdk/fit_dive_alarm_mesg.hpp \
fit-sdk/fit_dive_alarm_mesg_listener.hpp \
fit-sdk/fit_dive_gas_mesg.hpp \
fit-sdk/fit_dive_gas_mesg_listener.hpp \
fit-sdk/fit_dive_settings_mesg.hpp \
fit-sdk/fit_dive_settings_mesg_listener.hpp \
fit-sdk/fit_dive_summary_mesg.hpp \
fit-sdk/fit_dive_summary_mesg_listener.hpp \
fit-sdk/fit_encode.hpp \
fit-sdk/fit_event_mesg.hpp \
fit-sdk/fit_event_mesg_listener.hpp \
fit-sdk/fit_exd_data_concept_configuration_mesg.hpp \
fit-sdk/fit_exd_data_concept_configuration_mesg_listener.hpp \
fit-sdk/fit_exd_data_field_configuration_mesg.hpp \
fit-sdk/fit_exd_data_field_configuration_mesg_listener.hpp \
fit-sdk/fit_exd_screen_configuration_mesg.hpp \
fit-sdk/fit_exd_screen_configuration_mesg_listener.hpp \
fit-sdk/fit_exercise_title_mesg.hpp \
fit-sdk/fit_exercise_title_mesg_listener.hpp \
fit-sdk/fit_factory.hpp \
fit-sdk/fit_field.hpp \
fit-sdk/fit_field_base.hpp \
fit-sdk/fit_field_capabilities_mesg.hpp \
fit-sdk/fit_field_capabilities_mesg_listener.hpp \
fit-sdk/fit_field_definition.hpp \
fit-sdk/fit_field_description_mesg.hpp \
fit-sdk/fit_field_description_mesg_listener.hpp \
fit-sdk/fit_file_capabilities_mesg.hpp \
fit-sdk/fit_file_capabilities_mesg_listener.hpp \
fit-sdk/fit_file_creator_mesg.hpp \
fit-sdk/fit_file_creator_mesg_listener.hpp \
fit-sdk/fit_file_id_mesg.hpp \
fit-sdk/fit_file_id_mesg_listener.hpp \
fit-sdk/fit_goal_mesg.hpp \
fit-sdk/fit_goal_mesg_listener.hpp \
fit-sdk/fit_gps_metadata_mesg.hpp \
fit-sdk/fit_gps_metadata_mesg_listener.hpp \
fit-sdk/fit_gyroscope_data_mesg.hpp \
fit-sdk/fit_gyroscope_data_mesg_listener.hpp \
fit-sdk/fit_hr_mesg.hpp \
fit-sdk/fit_hr_mesg_listener.hpp \
fit-sdk/fit_hr_zone_mesg.hpp \
fit-sdk/fit_hr_zone_mesg_listener.hpp \
fit-sdk/fit_hrm_profile_mesg.hpp \
fit-sdk/fit_hrm_profile_mesg_listener.hpp \
fit-sdk/fit_hrv_mesg.hpp \
fit-sdk/fit_hrv_mesg_listener.hpp \
fit-sdk/fit_jump_mesg.hpp \
fit-sdk/fit_jump_mesg_listener.hpp \
fit-sdk/fit_lap_mesg.hpp \
fit-sdk/fit_lap_mesg_listener.hpp \
fit-sdk/fit_length_mesg.hpp \
fit-sdk/fit_length_mesg_listener.hpp \
fit-sdk/fit_magnetometer_data_mesg.hpp \
fit-sdk/fit_magnetometer_data_mesg_listener.hpp \
fit-sdk/fit_memo_glob_mesg.hpp \
fit-sdk/fit_memo_glob_mesg_listener.hpp \
fit-sdk/fit_mesg.hpp \
fit-sdk/fit_mesg_broadcast_plugin.hpp \
fit-sdk/fit_mesg_broadcaster.hpp \
fit-sdk/fit_mesg_capabilities_mesg.hpp \
fit-sdk/fit_mesg_capabilities_mesg_listener.hpp \
fit-sdk/fit_mesg_definition.hpp \
fit-sdk/fit_mesg_definition_listener.hpp \
fit-sdk/fit_mesg_listener.hpp \
fit-sdk/fit_mesg_with_event.hpp \
fit-sdk/fit_mesg_with_event_broadcaster.hpp \
fit-sdk/fit_mesg_with_event_listener.hpp \
fit-sdk/fit_met_zone_mesg.hpp \
fit-sdk/fit_met_zone_mesg_listener.hpp \
fit-sdk/fit_monitoring_info_mesg.hpp \
fit-sdk/fit_monitoring_info_mesg_listener.hpp \
fit-sdk/fit_monitoring_mesg.hpp \
fit-sdk/fit_monitoring_mesg_listener.hpp \
fit-sdk/fit_nmea_sentence_mesg.hpp \
fit-sdk/fit_nmea_sentence_mesg_listener.hpp \
fit-sdk/fit_obdii_data_mesg.hpp \
fit-sdk/fit_obdii_data_mesg_listener.hpp \
fit-sdk/fit_ohr_settings_mesg.hpp \
fit-sdk/fit_ohr_settings_mesg_listener.hpp \
fit-sdk/fit_one_d_sensor_calibration_mesg.hpp \
fit-sdk/fit_one_d_sensor_calibration_mesg_listener.hpp \
fit-sdk/fit_pad_mesg.hpp \
fit-sdk/fit_pad_mesg_listener.hpp \
fit-sdk/fit_power_zone_mesg.hpp \
fit-sdk/fit_power_zone_mesg_listener.hpp \
fit-sdk/fit_profile.hpp \
fit-sdk/fit_protocol_validator.hpp \
fit-sdk/fit_record_mesg.hpp \
fit-sdk/fit_record_mesg_listener.hpp \
fit-sdk/fit_runtime_exception.hpp \
fit-sdk/fit_schedule_mesg.hpp \
fit-sdk/fit_schedule_mesg_listener.hpp \
fit-sdk/fit_sdm_profile_mesg.hpp \
fit-sdk/fit_sdm_profile_mesg_listener.hpp \
fit-sdk/fit_segment_file_mesg.hpp \
fit-sdk/fit_segment_file_mesg_listener.hpp \
fit-sdk/fit_segment_id_mesg.hpp \
fit-sdk/fit_segment_id_mesg_listener.hpp \
fit-sdk/fit_segment_lap_mesg.hpp \
fit-sdk/fit_segment_lap_mesg_listener.hpp \
fit-sdk/fit_segment_leaderboard_entry_mesg.hpp \
fit-sdk/fit_segment_leaderboard_entry_mesg_listener.hpp \
fit-sdk/fit_segment_point_mesg.hpp \
fit-sdk/fit_segment_point_mesg_listener.hpp \
fit-sdk/fit_session_mesg.hpp \
fit-sdk/fit_session_mesg_listener.hpp \
fit-sdk/fit_set_mesg.hpp \
fit-sdk/fit_set_mesg_listener.hpp \
fit-sdk/fit_slave_device_mesg.hpp \
fit-sdk/fit_slave_device_mesg_listener.hpp \
fit-sdk/fit_software_mesg.hpp \
fit-sdk/fit_software_mesg_listener.hpp \
fit-sdk/fit_speed_zone_mesg.hpp \
fit-sdk/fit_speed_zone_mesg_listener.hpp \
fit-sdk/fit_sport_mesg.hpp \
fit-sdk/fit_sport_mesg_listener.hpp \
fit-sdk/fit_stress_level_mesg.hpp \
fit-sdk/fit_stress_level_mesg_listener.hpp \
fit-sdk/fit_three_d_sensor_calibration_mesg.hpp \
fit-sdk/fit_three_d_sensor_calibration_mesg_listener.hpp \
fit-sdk/fit_timestamp_correlation_mesg.hpp \
fit-sdk/fit_timestamp_correlation_mesg_listener.hpp \
fit-sdk/fit_totals_mesg.hpp \
fit-sdk/fit_totals_mesg_listener.hpp \
fit-sdk/fit_training_file_mesg.hpp \
fit-sdk/fit_training_file_mesg_listener.hpp \
fit-sdk/fit_unicode.hpp \
fit-sdk/fit_user_profile_mesg.hpp \
fit-sdk/fit_user_profile_mesg_listener.hpp \
fit-sdk/fit_video_clip_mesg.hpp \
fit-sdk/fit_video_clip_mesg_listener.hpp \
fit-sdk/fit_video_description_mesg.hpp \
fit-sdk/fit_video_description_mesg_listener.hpp \
fit-sdk/fit_video_frame_mesg.hpp \
fit-sdk/fit_video_frame_mesg_listener.hpp \
fit-sdk/fit_video_mesg.hpp \
fit-sdk/fit_video_mesg_listener.hpp \
fit-sdk/fit_video_title_mesg.hpp \
fit-sdk/fit_video_title_mesg_listener.hpp \
fit-sdk/fit_watchface_settings_mesg.hpp \
fit-sdk/fit_watchface_settings_mesg_listener.hpp \
fit-sdk/fit_weather_alert_mesg.hpp \
fit-sdk/fit_weather_alert_mesg_listener.hpp \
fit-sdk/fit_weather_conditions_mesg.hpp \
fit-sdk/fit_weather_conditions_mesg_listener.hpp \
fit-sdk/fit_weight_scale_mesg.hpp \
fit-sdk/fit_weight_scale_mesg_listener.hpp \
fit-sdk/fit_workout_mesg.hpp \
fit-sdk/fit_workout_mesg_listener.hpp \
fit-sdk/fit_workout_session_mesg.hpp \
fit-sdk/fit_workout_session_mesg_listener.hpp \
fit-sdk/fit_workout_step_mesg.hpp \
fit-sdk/fit_workout_step_mesg_listener.hpp \
fit-sdk/fit_zones_target_mesg.hpp \
fit-sdk/fit_zones_target_mesg_listener.hpp \
homeform.h \
ios/lockscreen.h \
keepawakehelper.h \
macos/lockscreen.h \
ios/M3iIOS-Interface.h \
material.h \
metric.h \
qdebugfixup.h \
qfit.h \
qmdnsengine_export.h \
qzsettings.h \
screencapture.h \
sessionline.h \
signalhandler.h \
tcpclientinfosender.h \
templateinfosender.h \
templateinfosenderbuilder.h \
gpx.h \
devices/treadmill.h \
trainprogram.h \
virtualdevices/virtualtreadmill.h \
wobjectdefs.h \
wobjectimpl.h \
scanrecordresult.h \
windows_zwift_incline_paddleocr_thread.h \
zwiftworkout.h


exists(secret.h): HEADERS += secret.h

RESOURCES += \
    icons.qrc \
    qml.qrc

DISTFILES += \
    $$PWD/android/assets/ca_cert.pem \
    $$PWD/android/assets/client_cert.pem \
    $$PWD/android/assets/client_key.pem \
    $$PWD/android/res/xml/device_filter.xml \
    $$PWD/android/src/BleAdvertiser.java \
   $$PWD/android/src/CSafeRowerUSBHID.java \
    $$PWD/android/src/ContentHelper.java \
    $$PWD/android/src/CustomQtActivity.java \
    $$PWD/android/src/GrpcTreadmillService.java \
   $$PWD/android/src/HidBridge.java \
    $$PWD/android/src/LocationHelper.java \
    $$PWD/android/src/MediaProjection.java \
    $$PWD/android/src/NotificationUtils.java \
    $$PWD/android/src/QLog.java \
    $$PWD/android/src/ScreenCaptureService.java \
    $$PWD/android/src/Shortcuts.java \
    $$PWD/android/src/ZwiftAPI.java \
    $$PWD/android/src/main/proto/zwift_hub.proto \
    $$PWD/android/src/main/proto/zwift_messages.proto \
    .clang-format \
   AppxManifest.xml \
   android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
   android/res/layout/floating_layout.xml \
	android/res/values/libs.xml \
   android/src/FloatingHandler.java \
   android/src/FloatingWindowGFG.java \
   android/src/ForegroundService.java \
   android/src/NotificationClient.java \
   android/src/QZAdbRemote.java \
        android/src/ScanRecordResult.java \
        android/src/NativeScanCallback.java \
	android/src/MyActivity.java \
    android/src/Usbserial.java \
   android/src/com/cgutman/adblib/AdbBase64.java \
   android/src/com/cgutman/adblib/AdbConnection.java \
   android/src/com/cgutman/adblib/AdbCrypto.java \
   android/src/com/cgutman/adblib/AdbProtocol.java \
   android/src/com/cgutman/adblib/AdbStream.java \
   android/src/com/cgutman/adblib/package-info.java \
   android/src/com/cgutman/androidremotedebugger/AdbUtils.java \
   android/src/com/cgutman/androidremotedebugger/adblib/AndroidBase64.java \
   android/src/com/cgutman/androidremotedebugger/console/CommandHistory.java \
   android/src/com/cgutman/androidremotedebugger/console/ConsoleBuffer.java \
   android/src/com/cgutman/androidremotedebugger/devconn/DeviceConnection.java \
   android/src/com/cgutman/androidremotedebugger/devconn/DeviceConnectionListener.java \
   android/src/com/cgutman/androidremotedebugger/service/ShellListener.java \
   android/src/com/cgutman/androidremotedebugger/service/ShellService.java \
   android/src/com/cgutman/androidremotedebugger/ui/Dialog.java \
   android/src/com/cgutman/androidremotedebugger/ui/SpinnerDialog.java \
    android/gradle.properties \
    android/proguard-rules.pro \
	ios/AppDelegate.swift \
	ios/BLEPeripheralManager.swift \
	ios/LiveActivityManager.swift

win32: DISTFILES += \
   $$PWD/adb/AdbWinApi.dll \
	$$PWD/adb/AdbWinUsbApi.dll \
	$$PWD/adb/adb.exe \


ios {
    ios_icon.files = $$files($$PWD/icons/ios/*.png)
	 QMAKE_BUNDLE_DATA += ios_icon
}

ios {
    OBJECTIVE_SOURCES += ios/lockscreen.mm \
    ios/ios_app_delegate.mm \
    ios/ios_liveactivity.mm \
	 fit-sdk/FitDecode.mm \
	 fit-sdk/FitDeveloperField.mm \
	 fit-sdk/FitEncode.mm \
	 fit-sdk/FitField.mm \
	 fit-sdk/FitFieldDefinition.mm \
	 fit-sdk/FitMesg.mm \
         fit-sdk/FitMesgDefinition.mm \
         ios/M3iNS.mm \

    SOURCES += ios/M3iNSQT.cpp

    OBJECTIVE_HEADERS += ios/M3iNS.h \
    ios/ios_liveactivity.h

    QMAKE_INFO_PLIST = ios/Info.plist
	 QMAKE_ASSET_CATALOGS = $$PWD/ios/Images.xcassets
	 QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
	 QMAKE_ASSET_CATALOGS_BUILD_PATH = $$PWD/ios/ 

    TARGET = qdomyoszwift
	 QMAKE_TARGET_BUNDLE_PREFIX = org.cagnulein
    
    # iOS Code Signing Configuration - handled manually in Xcode project
    
    DEFINES+=_Nullable_result=_Nullable NS_FORMAT_ARGUMENT\\(A\\)=
}

HEADERS += \
    mqttpublisher.h \
    androidstatusbar.h \
    fontmanager.h

SOURCES += \
    mqttpublisher.cpp \
    androidstatusbar.cpp \
    fontmanager.cpp


WINRT_MANIFEST = AppxManifest.xml

VERSION = 2.21.5
