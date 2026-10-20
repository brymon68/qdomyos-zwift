import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtQuick.Window 2.12
import Qt.labs.platform 1.1
import AndroidStatusBar 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visibility: Qt.WindowFullScreen
    visible: true
	 objectName: "stack"
    title: qsTr("X22i")

    // Force update on orientation change
    property int currentOrientation: Screen.orientation
    onCurrentOrientationChanged: {
        if (Qt.platform.os === "android") {
            console.log("Orientation changed to:", currentOrientation)
            // Force property binding updates by accessing the properties
            var temp = AndroidStatusBar.height + AndroidStatusBar.navigationBarHeight + AndroidStatusBar.leftInset + AndroidStatusBar.rightInset
        }
    }

    // Helper functions for cleaner padding calculations
    function getTopPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.PortraitOrientation || Screen.orientation === Qt.InvertedPortraitOrientation) ?
               AndroidStatusBar.height : AndroidStatusBar.leftInset;
    }

    function getBottomPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.PortraitOrientation || Screen.orientation === Qt.InvertedPortraitOrientation) ?
               AndroidStatusBar.navigationBarHeight : AndroidStatusBar.rightInset;
    }

    function getLeftPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.LandscapeOrientation || Screen.orientation === Qt.InvertedLandscapeOrientation) ?
               AndroidStatusBar.leftInset : 0;
    }

    function getRightPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.LandscapeOrientation || Screen.orientation === Qt.InvertedLandscapeOrientation) ?
               AndroidStatusBar.rightInset : 0;
    }

    signal restart()
    signal floatingOpen()
    signal openFloatingWindowBrowser();

    property bool settings_restart_to_apply: false

    ToastManager {
        id: toast
    }

    Timer {
        interval: 1
        repeat: false
        running: (rootItem.toastRequested !== "")
        onTriggered: {
            toast.show(rootItem.toastRequested);
            rootItem.toastRequested = "";
        }
    }

    Keys.onBackPressed: {
        if(OS_VERSION === "Android") {
            toast.show("Pressed it quickly to close the app!")
            timer.pressBack();
        }
    }
    Timer{
        id: timer

        property bool backPressed: false
        repeat: false
        interval: 200//ms
        onTriggered: backPressed = false
        function pressBack(){
            if(backPressed){
                timer.stop()
                backPressed = false
                Qt.callLater(Qt.quit)
            }
            else{
                backPressed = true
                timer.start()
            }
        }
    }

    MessageDialog {
        id: popupRestartApp
        text: qsTr("Settings changed")
        informativeText: qsTr("In order to apply the changes you need to restart the app.\nDo you want to do it now?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: Qt.callLater(Qt.quit)
        onNoClicked: this.visible = false;
        visible: false
    }

    header: ToolBar {
        contentHeight: toolButton.implicitHeight
        Material.primary: "#800080"
        id: headerToolbar
        property bool settingsPageActive: stackView.currentItem && stackView.currentItem.objectName === "settingsPage"
        topPadding: getTopPadding()
        leftPadding: getLeftPadding()
        rightPadding: getRightPadding()

        ToolButton {
            id: toolButton
            icon.source: "icons/icons/icon.png"
            text: stackView.depth > 1 ? "◄" : "◄"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    if(window.settings_restart_to_apply === true) {
                        window.settings_restart_to_apply = false;
                        popupRestartApp.visible = true;
                    }

                    stackView.pop()
                    rootItem.sortTiles()
                } else {
                    drawer.open()
                }
            }
        }

        ToolButton {
            id: toolButtonFloating
            icon.source: "icons/icons/mini-display.png"
            onClicked: { console.log("floating!"); floatingOpen(); }
            anchors.left: toolButton.right
            visible: OS_VERSION === "Android" ? true : false
        }

        Popup {
            id: popupAutoResistance
             parent: Overlay.overlay

             x: Math.round((parent.width - width) / 2)
             y: Math.round((parent.height - height) / 2)
             width: 380
             height: 60
             modal: true
             focus: true
             palette.text: "white"
             closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
             enter: Transition
             {
                 NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
             }
             exit: Transition
             {
                 NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
             }
             Column {
                 anchors.horizontalCenter: parent.horizontalCenter
             Label {
                 anchors.horizontalCenter: parent.horizontalCenter
                 text: "Automatic incline " + (rootItem.autoResistance ? "enabled" : "disabled")
                }
             }
        }

        Timer {
            id: popupAutoResistanceAutoClose
            interval: 2000; running: false; repeat: false
            onTriggered: popupAutoResistance.close();
        }

        ToolButton {
            id: toolButtonAutoResistance
            icon.source: ( rootItem.autoResistance ? "icons/icons/resistance.png" : "icons/icons/pause.png")
            onClicked: { rootItem.autoResistance = !rootItem.autoResistance; console.log("auto resistance toggled " + rootItem.autoResistance); popupAutoResistance.open(); popupAutoResistanceAutoClose.running = true; }
            anchors.right: parent.right
            visible: !headerToolbar.settingsPageActive
            width: visible ? implicitWidth : 0
        }

        Label {
            text: stackView.currentItem.title
            anchors.centerIn: parent
        }
    }

    Drawer {
        id: drawer
        width: window.width * 0.66
        height: window.height
        topPadding: getTopPadding()
        bottomPadding: getBottomPadding()
        leftPadding: getLeftPadding()
        rightPadding: getRightPadding()
        Accessible.ignored: !drawer.opened

        ScrollView {
            contentWidth: -1
            focus: true
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.fill: parent

            Column {
                anchors.fill: parent
                spacing: 3

                ItemDelegate {
                    text: qsTr("Settings")
                    width: parent.width
                    onClicked: {
                        stackView.push("settings.qml")

                         drawer.close()
                    }
                }

                ItemDelegate {
                    id: launch_ifit
                    text: qsTr("Launch iFit App")
                    width: parent.width
                    visible: OS_VERSION === "Android" ? true : false
                    onClicked: {
                        rootItem.launchIFitApp();
                        drawer.close()
                    }
                }
                ItemDelegate {
                    text: qsTr("Credits")
                    width: parent.width
                    onClicked: {
                        stackView.push("Credits.qml")
                        drawer.close()
                    }
                }
                ItemDelegate {
                    text: qsTr("Quit")
                    width: parent.width
                    visible: OS_VERSION === "Other" ? true : false
                    onClicked: {
                        console.log("closing...")
                        Qt.callLater(Qt.quit)
                    }
                }

                ItemDelegate {
                    text: "version 2.21.5"
                    width: parent.width
                }


            }
        }
    }

    // Wrapper Item to prevent ApplicationWindow from capturing all VoiceOver focus
    Item {
        anchors.fill: parent
        Accessible.ignored: true

        StackView {
            id: stackView
            initialItem: "Home.qml"
            anchors.fill: parent
            anchors.bottomMargin: (Screen.orientation === Qt.PortraitOrientation || Screen.orientation === Qt.InvertedPortraitOrientation) ? getBottomPadding() : 0
            anchors.rightMargin: getRightPadding()
            anchors.leftMargin: getLeftPadding()
            focus: true

        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
