import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12

Page {

    title: qsTr("QZ Fitness")
    id: page

    // VoiceOver accessibility - ignore Page itself, only children are accessible
    Accessible.ignored: true

    property alias start: start
    property alias stop: stop
    property alias row: row

    Item {
        width: parent.width
        height: rootItem.topBarHeight
        id: topBar

        Row {
            id: row
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            height: topBar.height - 20
            spacing: 5
            padding: 5

            Rectangle {
                width: 50
                height: row.height
					 color: "#303030"
                Accessible.ignored: true

                Column {
                    id: column
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: row.height
                    spacing: 0
                    padding: 0
                    Accessible.ignored: true

                    Rectangle {
                        width: 50
                        height: row.height
                        color: "#303030"
                        Accessible.ignored: true

                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            id: treadmill_connection
                            width: 48
                            height: row.height - 52
                            source: "icons/icons/bluetooth-icon.png"
                            enabled: rootItem.device
                            smooth: true

                            // VoiceOver accessibility
                            Accessible.role: Accessible.Indicator
                            Accessible.name: qsTr("X22i connection")
                            Accessible.description: rootItem.device ? qsTr("Device connected") : qsTr("Device not connected")
                            Accessible.focusable: true
                        }
                        ColorOverlay {
                            anchors.fill: treadmill_connection
                            source: treadmill_connection
                            color: treadmill_connection.enabled ? "#00000000" : "#B0D3d3d3"
                        }
                    }
                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        id: treadmill_signal
                        width: 24
                        height: row.height - 76
                        source: rootItem.signal
                        smooth: true
                        Accessible.ignored: true
                    }
                }
            }

            Rectangle {
                width: 120
                height: row.height
					 color: "#303030"
                Accessible.ignored: true

                RoundButton {
                    icon.source: rootItem.startIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.startText
                    enabled: true
                    id: start
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.startText
                    Accessible.description: qsTr("Start or pause the treadmill")
                    Accessible.focusable: true
                }
                ColorOverlay {
                    anchors.fill: start
                    source: start
                    color: rootItem.startColor
                    enabled: rootItem.startColor === "red" ? true : false
                }
            }

            Rectangle {
                width: 120
                height: row.height
					 color: "#303030"
                Accessible.ignored: true

                RoundButton {
                    icon.source: rootItem.stopIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.stopText
                    enabled: true
                    id: stop
                    width: 120
                    height: row.height - 4

                    // VoiceOver accessibility
                    Accessible.role: Accessible.Button
                    Accessible.name: rootItem.stopText
                    Accessible.description: qsTr("Stop the treadmill")
                    Accessible.focusable: true
                }
                ColorOverlay {
                    anchors.fill: stop
                    source: stop
                    color: rootItem.stopColor
                    enabled: rootItem.stopColor === "red" ? true : false
                }
            }


        }

        Row {
            id: row1
            width: parent.width
            anchors.bottom: row.bottom
            anchors.bottomMargin: -10

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: rootItem.info
            }
        }

        Label {
            id: lblHelp
            width: parent.width
            anchors.top: row1.bottom
            anchors.topMargin: 30
            text: "Waiting for the X22i iFIT service. Open iFIT on this treadmill, then return to X22i Test."
            wrapMode: Label.WordWrap
            visible: rootItem.labelHelp
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.6600000262260437;height:480;width:640}
}
##^##*/

