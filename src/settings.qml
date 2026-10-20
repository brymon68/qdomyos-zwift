import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    id: settingsPane
    objectName: "settingsPage"
    property string title: "X22i Settings"
    contentWidth: -1
    anchors.fill: parent

    Settings {
        id: settings
        property bool zwift_negative_inclination_x2: false
        property real zwift_inclination_offset: 0.0
        property real zwift_inclination_gain: 0.5
        property bool miles_unit: true
        property real speed_offset: 0.0
        property real speed_gain: 1.0
        property bool virtual_device_enabled: true
        property bool virtual_device_bluetooth: true
        property real treadmill_step_speed: 0.1
        property real treadmill_step_incline: 0.5
        property bool dircon_yes: true
        property int dircon_server_base_port: 36866
        property double treadmill_inclination_override_0: 0.0
        property double treadmill_inclination_override_05: 0.5
        property double treadmill_inclination_override_10: 1.0
        property double treadmill_inclination_override_15: 1.5
        property double treadmill_inclination_override_20: 2.0
        property double treadmill_inclination_override_25: 2.5
        property double treadmill_inclination_override_30: 3.0
        property double treadmill_inclination_override_35: 3.5
        property double treadmill_inclination_override_40: 4.0
        property double treadmill_inclination_override_45: 4.5
        property double treadmill_inclination_override_50: 5.0
        property double treadmill_inclination_override_55: 5.5
        property double treadmill_inclination_override_60: 6.0
        property double treadmill_inclination_override_65: 6.5
        property double treadmill_inclination_override_70: 7.0
        property double treadmill_inclination_override_75: 7.5
        property double treadmill_inclination_override_80: 8.0
        property double treadmill_inclination_override_85: 8.5
        property double treadmill_inclination_override_90: 9.0
        property double treadmill_inclination_override_95: 9.5
        property double treadmill_inclination_override_100: 10.0
        property double treadmill_inclination_override_105: 10.5
        property double treadmill_inclination_override_110: 11.0
        property double treadmill_inclination_override_115: 11.5
        property double treadmill_inclination_override_120: 12.0
        property double treadmill_inclination_override_125: 12.5
        property double treadmill_inclination_override_130: 13.0
        property double treadmill_inclination_override_135: 13.5
        property double treadmill_inclination_override_140: 14.0
        property double treadmill_inclination_override_145: 14.5
        property double treadmill_inclination_override_150: 15.0
        property double treadmill_inclination_ovveride_gain: 0.01
        property double treadmill_inclination_ovveride_offset: 0.1
        property string zwift_username: ""
        property string zwift_password: ""
        property real treadmill_incline_min: 0.0
        property real treadmill_incline_max: 100.0
        property bool zwift_api_autoinclination: true
        property int zwift_api_poll: 5
        property real inclination_delay_seconds: 3.0
        property real min_inclination: -999.0
        property bool real_inclination_to_virtual_treamill_bridge: false
        property real treadmill_speed_max: 100.0
        property real treadmill_speed_min: 0.0
    }

    ColumnLayout {
        width: settingsPane.availableWidth
        spacing: 12

        Label {
            text: "X22i Settings"
            font.pixelSize: 24
            Layout.fillWidth: true
        }

        Label {
            text: "X22i connection"
            font.bold: true
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.topMargin: 12
        }

        SwitchDelegate {
            text: "Share treadmill with Zwift"
            checked: settings.virtual_device_enabled
            Layout.fillWidth: true
            onClicked: {
                settings.virtual_device_enabled = checked
                window.settings_restart_to_apply = true
            }
        }
        SwitchDelegate {
            text: "Bluetooth treadmill connection"
            checked: settings.virtual_device_bluetooth
            Layout.fillWidth: true
            onClicked: {
                settings.virtual_device_bluetooth = checked
                window.settings_restart_to_apply = true
            }
        }
        SwitchDelegate {
            text: "Wi-Fi treadmill connection (DIRCON)"
            checked: settings.dircon_yes
            Layout.fillWidth: true
            onClicked: {
                settings.dircon_yes = checked
                window.settings_restart_to_apply = true
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "DIRCON port"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: dircon_server_base_portField
                text: settings.dircon_server_base_port
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseInt(dircon_server_base_portField.text)
                    if (!isFinite(value)) return
                    if (value < 1 || value > 65535) return
                    settings.dircon_server_base_port = value
                    window.settings_restart_to_apply = true
                    toast.show("Setting saved!")
                }
            }
        }

        Label {
            text: "Zwift terrain"
            font.bold: true
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.topMargin: 12
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Zwift username"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: zwift_usernameField
                text: settings.zwift_username
                Layout.preferredWidth: 150
            }
            Button {
                text: "Save"
                onClicked: {
                    settings.zwift_username = zwift_usernameField.text
                    window.settings_restart_to_apply = true
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Zwift password"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: zwift_passwordField
                text: settings.zwift_password
                Layout.preferredWidth: 150
                echoMode: TextInput.Password
            }
            Button {
                text: "Save"
                onClicked: {
                    settings.zwift_password = zwift_passwordField.text
                    window.settings_restart_to_apply = true
                    toast.show("Setting saved!")
                }
            }
        }
        SwitchDelegate {
            text: "Follow Zwift terrain automatically"
            checked: settings.zwift_api_autoinclination
            Layout.fillWidth: true
            onClicked: {
                settings.zwift_api_autoinclination = checked
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Terrain polling interval (seconds)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: zwift_api_pollField
                text: settings.zwift_api_poll
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseInt(zwift_api_pollField.text)
                    if (!isFinite(value)) return
                    if (value < 5) return
                    settings.zwift_api_poll = value
                    window.settings_restart_to_apply = true
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Terrain incline multiplier"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: zwift_inclination_gainField
                text: settings.zwift_inclination_gain
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(zwift_inclination_gainField.text)
                    if (!isFinite(value)) return
                    settings.zwift_inclination_gain = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Terrain incline offset (%)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: zwift_inclination_offsetField
                text: settings.zwift_inclination_offset
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(zwift_inclination_offsetField.text)
                    if (!isFinite(value)) return
                    settings.zwift_inclination_offset = value
                    toast.show("Setting saved!")
                }
            }
        }
        SwitchDelegate {
            text: "Double downhill terrain incline"
            checked: settings.zwift_negative_inclination_x2
            Layout.fillWidth: true
            onClicked: {
                settings.zwift_negative_inclination_x2 = checked
            }
        }
        SwitchDelegate {
            text: "Report physical incline to Zwift"
            checked: settings.real_inclination_to_virtual_treamill_bridge
            Layout.fillWidth: true
            onClicked: {
                settings.real_inclination_to_virtual_treamill_bridge = checked
            }
        }

        Label {
            text: "Motion controls"
            font.bold: true
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.topMargin: 12
        }
        SwitchDelegate {
            text: "Display miles and mph"
            checked: settings.miles_unit
            Layout.fillWidth: true
            onClicked: {
                settings.miles_unit = checked
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Manual speed increment (km/h)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: treadmill_step_speedField
                text: settings.treadmill_step_speed
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(treadmill_step_speedField.text)
                    if (!isFinite(value)) return
                    settings.treadmill_step_speed = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Manual incline increment (%)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: treadmill_step_inclineField
                text: settings.treadmill_step_incline
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(treadmill_step_inclineField.text)
                    if (!isFinite(value)) return
                    settings.treadmill_step_incline = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Incline change delay (seconds)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: inclination_delay_secondsField
                text: settings.inclination_delay_seconds
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(inclination_delay_secondsField.text)
                    if (!isFinite(value)) return
                    settings.inclination_delay_seconds = value
                    toast.show("Setting saved!")
                }
            }
        }

        Label {
            text: "Motion limits"
            font.bold: true
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.topMargin: 12
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Minimum speed (km/h)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: treadmill_speed_minField
                text: settings.treadmill_speed_min
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(treadmill_speed_minField.text)
                    if (!isFinite(value)) return
                    settings.treadmill_speed_min = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Maximum speed (km/h)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: treadmill_speed_maxField
                text: settings.treadmill_speed_max
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(treadmill_speed_maxField.text)
                    if (!isFinite(value)) return
                    settings.treadmill_speed_max = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Minimum incline (%)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: treadmill_incline_minField
                text: settings.treadmill_incline_min
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(treadmill_incline_minField.text)
                    if (!isFinite(value)) return
                    settings.treadmill_incline_min = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Maximum incline (%)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: treadmill_incline_maxField
                text: settings.treadmill_incline_max
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(treadmill_incline_maxField.text)
                    if (!isFinite(value)) return
                    settings.treadmill_incline_max = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Minimum reported incline (%)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: min_inclinationField
                text: settings.min_inclination
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(min_inclinationField.text)
                    if (!isFinite(value)) return
                    settings.min_inclination = value
                    toast.show("Setting saved!")
                }
            }
        }

        Label {
            text: "Speed calibration"
            font.bold: true
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.topMargin: 12
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Speed multiplier"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: speed_gainField
                text: settings.speed_gain
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(speed_gainField.text)
                    if (!isFinite(value)) return
                    settings.speed_gain = value
                    toast.show("Setting saved!")
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Speed offset (km/h)"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            TextField {
                id: speed_offsetField
                text: settings.speed_offset
                Layout.preferredWidth: 150
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }
            Button {
                text: "Save"
                onClicked: {
                    var value = parseFloat(speed_offsetField.text)
                    if (!isFinite(value)) return
                    settings.speed_offset = value
                    toast.show("Setting saved!")
                }
            }
        }

        Button {
            text: "Incline calibration table"
            Layout.fillWidth: true
            onClicked: stackView.push("settings-treadmill-inclination-override.qml")
        }
    }
}
