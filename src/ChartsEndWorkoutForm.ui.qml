import QtQuick 2.4
import QtCharts 2.2
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15

Item {
    anchors.fill: parent

    property alias heartSeries: heartSeries
    property alias heartChart: heartChart


    Row {
        id: row
        anchors.fill: parent

        Text {
            id: date
            text: rootItem.workoutStartDate
            font.pixelSize: 16
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            anchors.top: date.bottom
            id: title
            text: rootItem.workoutName
            font.pixelSize: 24
            color: "yellow"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }



        ScrollView {
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: title.bottom
            anchors.bottom: parent.bottom
            contentHeight: heartChart.height


            ChartView {
                id: heartChart
                height: 400
                width: parent.width
                antialiasing: true
                legend.visible: false
                anchors.top: parent.top
                title: "Heart Rate"
                titleFont.pixelSize: 20

                DateTimeAxis {
                    id: valueAxisXHR
                    tickCount: 7
                    min: new Date(0)
                    max: new Date(rootItem.workout_sample_points * 1000)
                    format: "mm:ss"
                    //labelsVisible: false
                    gridVisible: false
                    //lineVisible: false
                    labelsFont.pixelSize: 10
                }

                ValueAxis {
                    id: valueAxisYHR
                    min: 60
                    max: 220
                    tickCount: 6
                    labelFormat: "%.0f"
                    //labelsVisible: false
                    //gridVisible: false
                    //lineVisible: false
                    labelsFont.pixelSize: 10
                }

                LineSeries {
                    id: heartSeries
                    visible: true
                    axisX: valueAxisXHR
                    axisY: valueAxisYHR
                    color: "black"
                    width: 1
                }
            }



        }
    }
}
