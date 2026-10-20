import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import QtQuick.Window 2.12

HomeForm {
    objectName: "home"
    background: Rectangle {
        anchors.fill: parent
        width: parent.fill
        height: parent.fill
        color: "#303030"

        // VoiceOver accessibility - ignore decorative background
        Accessible.role: Accessible.Pane
        Accessible.ignored: true
    }
    signal start_clicked;
    signal stop_clicked;
    signal plus_clicked(string name)
    signal minus_clicked(string name)
    signal largeButton_clicked(string name)

    Timer {
        id: checkStopFromWeb
        interval: 200
        running: true
        repeat: true
        onTriggered: {
            if (rootItem.stopRequested) {
                rootItem.stopRequested = false;
                stop_clicked();
            }
        }
    }

    start.onClicked: start_clicked()
    stop.onClicked: stop_clicked()

    GridView {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        cellWidth: 175
        cellHeight: 130
        focus: true
        model: appModel
        leftMargin: { if(OS_VERSION === "Android") (Screen.width % cellWidth) / 2; else (parent.width % cellWidth) / 2; }
        anchors.topMargin: rootItem.topBarHeight + 30
        id: gridView
        objectName: "gridview"
        onMovementEnded: { headerToolbar.visible = (contentY == 0); }
        Screen.orientationUpdateMask:  Qt.LandscapeOrientation | Qt.PortraitOrientation
        Screen.onPrimaryOrientationChanged:{
            if(OS_VERSION === "Android")
                gridView.leftMargin = (Screen.width % cellWidth) / 2;
            else
                gridView.leftMargin = (parent.width % cellWidth) / 2;
        }

        Accessible.ignored: true

        delegate: Item {
            id: id1
            width: 170
            height: 125

            visible: visibleItem
            Component.onCompleted: console.log("completed " + objectName)

            // VoiceOver accessibility support
            Accessible.role: largeButton ? Accessible.Button : (writable ? Accessible.Pane : Accessible.StaticText)
            Accessible.name: name + (largeButton ? "" : (": " + value))
            Accessible.description: largeButton ? largeButtonLabel : (secondLine !== "" ? secondLine : (writable ? qsTr("Adjustable. Current value: ") + value : qsTr("Current value: ") + value))
            Accessible.focusable: true

            Behavior on x {
                NumberAnimation { duration: 400; easing.type: Easing.OutBack }
            }

            Behavior on y {
                NumberAnimation { duration: 400; easing.type: Easing.OutBack }
            }

            Rectangle {
                width: 168
                height: 123
                radius: 3
                border.width: 1
                border.color: "#9C27B0"
                color: "#303030"
                id: rect

                // Ignore for VoiceOver - decorative background only
                Accessible.ignored: true
            }

            DropShadow {
                visible: true
                anchors.fill: rect
                cached: true
                horizontalOffset: 3
                verticalOffset: 3
                radius: 8.0
                samples: 16
                color: "#9C27B0"
                source: rect
            }

            Timer {
                id: toggleIconTimer
                interval: 500; running: true; repeat: true
                onTriggered: { if(identificator === "inclination" && rootItem.autoInclinationEnabled()) myIcon.visible = !myIcon.visible; else myIcon.visible = !largeButton; }
            }

            Image {
                id: myIcon
                x: 5
                anchors {
                         bottom: id1.bottom
                }
                width: 48
                height: 48
                source: icon
                visible: !largeButton

                // Ignore for VoiceOver - decorative only
                Accessible.ignored: true
            }
            Text {
                objectName: "value"
                id: myValue
                color: valueFontColor
                y: 0
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: value
                horizontalAlignment: Text.AlignHCenter
                width: Math.max(50, parent.width - (writable ? 100 : 12))
                height: 58
                font.pointSize: valueFontSize
                fontSizeMode: Text.Fit
                minimumPointSize: 10
                font.bold: true
                visible: !largeButton

                // Ignore for VoiceOver - parent Item handles accessibility
                Accessible.ignored: true
            }
            Text {
                objectName: "secondLine"
                id: secondLineText
                color: "white"
                y: myValue.bottom
                anchors {
                    top: myValue.bottom
                    horizontalCenter: parent.horizontalCenter
                }
                text: secondLine
                horizontalAlignment: Text.AlignHCenter
                width: Math.max(50, parent.width - 12)
                height: 24
                font.pointSize: 12
                fontSizeMode: Text.Fit
                minimumPointSize: 7
                font.bold: false
                visible: !largeButton

                // Ignore for VoiceOver - parent Item handles accessibility
                Accessible.ignored: true
            }
            Text {
                id: myText
                anchors {
                    top: myIcon.top
                    horizontalCenter: undefined
                    left: parent.left
                }
                font.bold: true
                font.pointSize: labelFontSize
                fontSizeMode: Text.Fit
                minimumPointSize: 8
                color: "white"
                text: name
                horizontalAlignment: Text.AlignLeft
                anchors.leftMargin: 55
                width: Math.max(40, parent.width - 61)
                height: 40
                anchors.topMargin: 20
                visible: !largeButton

                // Ignore for VoiceOver - parent Item handles accessibility
                Accessible.ignored: true
            }
            RoundButton {
                objectName: minusName
                autoRepeat: true
                text: "-"
                onClicked: minus_clicked(objectName)
                visible: writable && !largeButton
                anchors.top: myValue.top
                anchors.left: parent.left
                anchors.leftMargin: 2
                width: 48
                height: 48

                // VoiceOver accessibility
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Decrease ") + name
                Accessible.description: qsTr("Decrease the value of ") + name
                Accessible.focusable: true
                Accessible.onPressAction: { minus_clicked(objectName) }
            }
            RoundButton {
                autoRepeat: true
                objectName: plusName
                text: "+"
                onClicked: plus_clicked(objectName)
                visible: writable && !largeButton
                anchors.top: myValue.top
                anchors.right: parent.right
                anchors.rightMargin: 2
                width: 48
                height: 48

                // VoiceOver accessibility
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Increase ") + name
                Accessible.description: qsTr("Increase the value of ") + name
                Accessible.focusable: true
                Accessible.onPressAction: { plus_clicked(objectName) }
            }
            RoundButton {
                autoRepeat: true
                objectName: identificator
                text: largeButtonLabel
                onClicked: largeButton_clicked(objectName)
                visible: largeButton
                anchors.fill: rect
                      background: Rectangle {
                          color: largeButtonColor
                            radius: 20
                            }
                font.pointSize: 20

                // VoiceOver accessibility
                Accessible.role: Accessible.Button
                Accessible.name: largeButtonLabel
                Accessible.description: name + ": " + largeButtonLabel
                Accessible.focusable: true
                Accessible.onPressAction: { largeButton_clicked(objectName) }
            }
        }
    }



}
