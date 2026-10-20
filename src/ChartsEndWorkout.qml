import QtQuick 2.4

ChartsEndWorkoutForm {
    Timer {
        id: screenshotTimer
        interval: 100
        repeat: false
        onTriggered: rootItem.save_screenshot_chart(heartChart, "heartChart")
    }

    Component.onCompleted: {
        headerToolbar.visible = true;
        for (var i = 0; i < rootItem.workout_sample_points; i += 10) {
            heartSeries.append(i * 1000, rootItem.workout_heart_points[i]);
        }
        screenshotTimer.start();
    }
}
