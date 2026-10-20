import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtMultimedia 5.15
import org.cagnulein.qdomyoszwift 1.0
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
    title: Qt.platform.os === "ios" ? "" : qsTr("qDomyos-Zwift")

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
        // Add padding for iPadOS multi-window mode (Stage Manager, Split View, Slide Over)
        // to avoid overlap with window control buttons (red/yellow/green)
        // Check both the native detection and window size comparison for reactivity
        if (Qt.platform.os === "ios") {
            var isMultiWindow = (typeof rootItem !== "undefined" && rootItem && rootItem.iPadMultiWindowMode) ||
                                (window.width < Screen.width - 10);  // Window smaller than screen = multi-window
            if (isMultiWindow) {
                return 15;  // Space for window control buttons
            }
        }
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

    signal gpx_open_clicked(url name)
    signal gpxpreview_open_clicked(url name)
    signal trainprogram_open_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal gpx_open_other_folder(url name)
    signal trainprogram_preview(url name)
    signal trainprogram_zwo_loaded(string s)
    signal trainprogram_autostart_requested()
    signal fitfile_preview(string s)
    signal restart()
    signal floatingOpen()
    signal openFloatingWindowBrowser();

    property bool settings_restart_to_apply: false

    Loader {
      id: googleMapUI
      source:"GoogleMap.qml";
      active: false
      onLoaded: { console.log("googleMapUI loaded"); stackView.push(googleMapUI.item); }
    }

    ToastManager {
        id: toast
    }

    property bool lapPromptVisible: false
    property string lapPromptText: ""

    function isLapPromptMessage(message) {
        var lowerMessage = message.toLowerCase()
        return (lowerMessage.indexOf("press") >= 0 && lowerMessage.indexOf("lap") >= 0) ||
               (lowerMessage.indexOf("lap") >= 0 && lowerMessage.indexOf("continue") >= 0 &&
                lowerMessage.indexOf("received") < 0)
    }

    Rectangle {
        id: lapPromptOverlay
        z: Infinity
        visible: window.lapPromptVisible
        anchors.centerIn: parent
        width: Math.min(parent.width - 32, 520)
        height: Math.max(96, lapPromptLabel.implicitHeight + 44)
        radius: 8
        color: "#9C27B0"
        border.color: "white"
        border.width: 3
        opacity: visible ? 1 : 0

        Label {
            id: lapPromptLabel
            anchors.fill: parent
            anchors.margins: 16
            text: window.lapPromptText
            color: "white"
            font.bold: true
            font.pixelSize: 26
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        SequentialAnimation on scale {
            running: lapPromptOverlay.visible
            loops: Animation.Infinite
            NumberAnimation { from: 1.0; to: 1.05; duration: 450; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 1.05; to: 1.0; duration: 450; easing.type: Easing.InOutQuad }
        }
    }

    Timer {
        id: lapPromptAutoClose
        interval: 15000
        repeat: false
        onTriggered: window.lapPromptVisible = false
    }

    Timer {
        interval: 1
        repeat: false
        running: (rootItem.toastRequested !== "")
        onTriggered: {
            if (window.isLapPromptMessage(rootItem.toastRequested)) {
                window.lapPromptText = rootItem.toastRequested;
                window.lapPromptVisible = true;
                lapPromptAutoClose.restart();
            } else {
                toast.show(rootItem.toastRequested);
            }
            rootItem.toastRequested = "";
        }
    }

    /*
    Timer {
        interval: 1000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            toast.show("This timer has triggered " + (++i) + " times!");
        }
    }

    Timer {
        interval: 3000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            toast.show("This important message has been shown " + (++i) + " times.", 5000);
        }
    }*/

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

    Popup {
	    id: popup
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
		     text: qsTr("Program has been loaded correctly. Press start to begin!")
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

    MessageDialog {
        text: "Clipboard Workout"
        informativeText: "Workout found in clipboard:\n" + rootItem.clipboardWorkoutPromptName +
                         "\n\nDo you want to open the workout preview?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            var workoutUrl = rootItem.clipboard_workout_url()
            rootItem.clipboard_accept_workout_prompt()
            var page = CHARTJS
                    ? stackView.push("TrainingProgramsListJS.qml", { initialWorkoutUrl: workoutUrl })
                    : stackView.push("TrainingProgramsList.qml", { initialWorkoutUrl: workoutUrl })
            page.trainprogram_open_clicked.connect(trainprogram_open_clicked)
            page.trainprogram_open_other_folder.connect(trainprogram_open_other_folder)
            page.trainprogram_preview.connect(trainprogram_preview)
            if (page.trainprogram_autostart_requested) {
                page.trainprogram_autostart_requested.connect(trainprogram_autostart_requested)
            }
            page.trainprogram_open_clicked.connect(function(url) {
                stackView.pop();
            });
        }
        onNoClicked: { rootItem.clipboard_dismiss_workout_prompt(); }
        visible: rootItem.clipboardWorkoutPromptRequested
    }

    MessageDialog {
        text: "Clipboard Workout"
        informativeText: "The clipboard workout has ended.\n\nDo you want to delete the file?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: rootItem.clipboard_delete_finished_workout()
        onNoClicked: rootItem.clipboard_keep_finished_workout()
        visible: rootItem.clipboardWorkoutDeletePromptRequested
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
            function loadMaps() {
                if(rootItem.currentCoordinateValid) {
                    console.log("coordinate is valid for map");
                    if(googleMapUI.status === Loader.Ready)
                        stackView.push(googleMapUI.item);
                    else
                        googleMapUI.active = true;

                } else {
                    console.log("coordinate is NOT valid for map");
                }
            }
            id: toolButtonMaps
            icon.source: ( "icons/icons/maps-icon-16.png" )
            onClicked: { loadMaps(); }
            anchors.right: toolButtonChart.left
            visible: rootItem.mapsVisible
        }

        ToolButton {
            function loadVideo() {
                if(rootItem.currentCoordinateValid || rootItem.trainProgramLoadedWithVideo) {
                    console.log("coordinate is valid for map");
                    //stackView.push("videoPlayback.qml");
                    rootItem.videoVisible = !rootItem.videoVisible
                } else {
                    console.log("coordinate is NOT valid for map");
                }
            }
            id: toolButtonVideo
            icon.source: ( "icons/icons/video.png" )
            onClicked: { loadVideo(); }
            anchors.right: toolButtonMaps.left
            visible: rootItem.videoIconVisible
        }

        ToolButton {
            id: toolButtonChart
            icon.source: ( "icons/icons/chart.png" )
            onClicked: { rootItem.chartFooterVisible = !rootItem.chartFooterVisible }
            anchors.right: toolButtonAutoResistance.left
            visible: rootItem.chartIconVisible
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
                    text: qsTr("Charts")
                    width: parent.width
                    onClicked: {
                        console.log(CHARTJS)
                        if(CHARTJS)
                            stackView.push("ChartJsTest.qml")
                        else
                            stackView.push("ChartsEndWorkout.qml")
                        drawer.close()
                    }
                }
                ItemDelegate {
                    id: gpx_open
                    text: qsTr("Open GPX")
                    width: parent.width
                    onClicked: {
                        stackView.push("GPXList.qml")
                        stackView.currentItem.trainprogram_open_clicked.connect(gpx_open_clicked)
                        stackView.currentItem.trainprogram_open_other_folder.connect(gpx_open_other_folder)
                        stackView.currentItem.trainprogram_preview.connect(gpxpreview_open_clicked)
                        stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                            stackView.pop();
                            popup.open();
                         });
                        drawer.close()
                    }
                }
                ItemDelegate {
                    id: trainprogram_open
                    text: qsTr("Open Train Program")
                    width: parent.width
                    onClicked: {
                        if(CHARTJS)
                            stackView.push("TrainingProgramsListJS.qml")
                        else
                            stackView.push("TrainingProgramsList.qml")
                        stackView.currentItem.trainprogram_open_clicked.connect(trainprogram_open_clicked)
                        stackView.currentItem.trainprogram_open_other_folder.connect(trainprogram_open_other_folder)
                        stackView.currentItem.trainprogram_preview.connect(trainprogram_preview)
                        stackView.currentItem.trainprogram_autostart_requested.connect(trainprogram_autostart_requested)
                        stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                            stackView.pop();
                         });
                        drawer.close()
                    }
                }
                ItemDelegate {
                    text: qsTr("Workout Editor")
                    width: parent.width
                    onClicked: {
                        var editorPage = stackView.push("WorkoutEditor.qml")
                        if (editorPage) {
                            editorPage.closeRequested.connect(function() {
                                stackView.pop()
                            })
                            // Close editor when workout is started from Save & Start
                            trainprogram_autostart_requested.connect(function() {
                                console.log("[main.qml] trainprogram_autostart_requested received, closing editor")
                                editorPage.closeRequested()
                            })
                        }
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

                    FileDialog {
                        id: fileDialogGPX
                         title: "Please choose a file"
                         folder: "file://" + rootItem.getWritableAppDir() + 'gpx'
                         onAccepted: {
                             console.log("You chose: " + fileDialogGPX.fileUrl)
                              gpx_open_clicked(fileDialogGPX.fileUrl)
                              fileDialogGPX.close()
                              popup.open()
                            }
                         onRejected: {
                             console.log("Canceled")
                              fileDialogGPX.close()
                            }
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
