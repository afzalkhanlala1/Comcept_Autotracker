import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtCharts 2.3

Item {
    id: mainItem

    property real mouseStartX: 0
    property real mouseStartY: 0
    property bool autoScroll: true
    property real zoomFactor: 1.0

    property alias chartView: chartView
    property alias xAxis: xAxis
    property alias yAxis: yAxis
    property alias mouseArea: mouseArea
    property alias autoScroll: mainItem.autoScroll
    property alias xMarker: xMarker

    Component.onCompleted: {

    }

    function setXMarker(xVal)
    {
//        if(xVal > charts.xAxis.max)
//            charts.chartView.scrollRight(interval * (chartView.plotArea.width / (xAxis.max - xAxis.min)))
        if(xVal > charts.xAxis.max)
            charts.chartView.scrollRight((xVal - charts.xAxis.max) * (chartView.plotArea.width / (xAxis.max - xAxis.min)))
        else if(xVal < charts.xAxis.min)
             charts.chartView.scrollLeft((charts.xAxis.min - xVal) * (chartView.plotArea.width / (xAxis.max - xAxis.min)))

       xMarker.x = ((chartView.plotArea.width / (xAxis.max - xAxis.min)) * (xVal - xAxis.min)) + chartView.plotArea.x
    }

    ChartView {
        id: chartView
        title: ""
        anchors.fill: parent
        antialiasing: false
        smooth: false
        legend.labelColor: "#ffffff"
        legend.markerShape: Legend.MarkerShapeCircle
        backgroundColor: "#222526"
        legend.font.pointSize: 9
        margins.bottom: 0
        margins.top: 0
        margins.left: 0
        margins.right: 0


        ValueAxis {
            id: xAxis
            min: 0
            max: 10
            tickCount: 11
            color: "#ffffff"
            labelsColor: "#ffffff"
            gridLineColor: "#0fffffff"
            labelsFont:Qt.font({pointSize: 9})
            labelFormat: "%.1f"
        }

        ValueAxis {
            id: yAxis
            min: -500
            max: 500
            tickCount: 11
            gridLineColor: "#0fffffff"
            color: "#ffffff"
            labelsColor: "#ffffff"
            labelsFont:Qt.font({pointSize: 9})
            labelFormat: "%.1f"
        }

        Rectangle{
            id: xMarker
            x: 0 + parent.plotArea.x
            y: parent.plotArea.y
            height: parent.plotArea.height
            color: "#A0ffffff"
            width: 2
            antialiasing: true
            visible: false

            Behavior on x { PropertyAnimation  {duration: 50} }
        }
    }

    MouseArea{
        id: mouseArea
        anchors.fill: parent
        onPressed: {
            mouseStartX = mouseArea.mouseX
            mouseStartY = mouseArea.mouseY
            mainItem.focus = true
        }

        onPositionChanged: {
            var dx = mouseStartX - mouseArea.mouseX
            var dy = mouseStartY - mouseArea.mouseY

            if(dx > 0) chartView.scrollRight(dx)
            else chartView.scrollLeft(-dx)

            //            if(dy > 0) chartView.scrollDown(dy)
            //            else chartView.scrollUp(-dy)

            mouseStartX = mouseArea.mouseX
            mouseStartY = mouseArea.mouseY
        }

        onDoubleClicked: {
            autoScroll = !autoScroll
        }
    }

    Keys.forwardTo: [trackerView]
}
