import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.12
import QtCharts 2.3
import QtQuick.Extras 1.4
import QtQuick.Window 2.12

Item {
    id: mainItem
    property int myIndex: 0
    property real xTime: 0
    property alias dataTimer: dataTimer
    property real data_dt: 0.04

    property real azi_yMin: -100
    property real azi_yMax: 100
    property real azi_yMax_time: 0
    property real azi_yMin_time: 0
    property real azi_val: 0

    property real elev_yMin: -100
    property real elev_yMax: 100
    property real elev_yMax_time: 0
    property real elev_yMin_time: 0
    property real elev_val: 0

    property int data_per_chart: 2

    onVisibleChanged:
    {

    }

    Component.onCompleted:
    {

    }

    function loadDataFields()
    {
        for(var l = 0; l < atc.getLocalFieldCount(); l++)
        {
            if(l < data_per_chart)
            {
                azi_charts.chartView.createSeries(ChartView.SeriesTypeLine, atc.getLocalFieldName(l), azi_charts.xAxis, azi_charts.yAxis);
                azi_charts.chartView.series(l).useOpenGL = true
                azi_charts.chartView.series(l).width = 3
            }

            else
            {
                elev_charts.chartView.createSeries(ChartView.SeriesTypeLine, atc.getLocalFieldName(l), elev_charts.xAxis, elev_charts.yAxis);
                elev_charts.chartView.series(l-2).useOpenGL = true
                elev_charts.chartView.series(l-2).width = 3
            }
        }
    }

    function loadSettings()
    {
        loadDataFields()
    }

    Rectangle{
        id: bgRect
        color: "#151b21"
        border.width: 0
        anchors.fill: parent
    }

    ScrollableChartView{
        id: azi_charts
        anchors.fill: parent
        anchors.bottomMargin: parent.height*0.5
        yAxis.min: azi_yMin
        yAxis.max: azi_yMax
        yAxis.tickCount: 11
    }

    ScrollableChartView{
        id: elev_charts
        anchors.fill: parent
        anchors.topMargin: parent.height*0.5
        yAxis.min: elev_yMin
        yAxis.max: elev_yMax
        yAxis.tickCount: 11
    }

    Timer{
        id: dataTimer
        interval: mainAppWindow.dataUpdateDelay
        repeat: true
        running: false
        onTriggered:
        {
            atc.setGraphUpdating(true)
            xTime += data_dt

            if((xTime - azi_yMax_time) > 10.0) { azi_yMax *= 0.95 }
            if((xTime - azi_yMin_time) > 10.0) { azi_yMin *= 0.95}

            if(atc.getLocalDataReady())
            {
                for(var vl = 0; vl < atc.getLocalFieldCount(); vl++)
                {
                    if(vl < data_per_chart)
                    {
                        azi_val = atc.getLocalData(vl)
                        azi_charts.chartView.series(atc.getLocalFieldName(vl)).append(xTime, azi_val)

                        if(azi_val > azi_yMax) { azi_yMax = azi_val*1.05; azi_yMax_time = xTime }
                        if(azi_val < azi_yMin) { azi_yMin = azi_val*1.05; azi_yMin_time = xTime }
                    }

                    else
                    {
                        elev_val = atc.getLocalData(vl)
                        elev_charts.chartView.series(atc.getLocalFieldName(vl)).append(xTime, elev_val)

                        if(elev_val > elev_yMax) { elev_yMax = elev_val*1.05; elev_yMax_time = xTime }
                        if(elev_val < elev_yMin) { elev_yMin = elev_val*1.05; elev_yMin_time = xTime }
                    }
                }
            }

            if(xTime > azi_charts.xAxis.max && !azi_charts.mouseArea.pressed && azi_charts.autoScroll){
                azi_charts.chartView.scrollRight(data_dt * (azi_charts.chartView.plotArea.width / (azi_charts.xAxis.max - azi_charts.xAxis.min)))
            }

            if(xTime > elev_charts.xAxis.max && !elev_charts.mouseArea.pressed && elev_charts.autoScroll){
                elev_charts.chartView.scrollRight(data_dt * (elev_charts.chartView.plotArea.width / (elev_charts.xAxis.max - elev_charts.xAxis.min)))
            }

            for(var ch = 0; ch < azi_charts.chartView.count; ch++)
            {
                if(azi_charts.chartView.series(ch).count > 400)
                {
                    azi_charts.chartView.series(ch).remove(0)
                }
            }

            for(var elev_ch = 0; ch < elev_charts.chartView.count; ch++)
            {
                if(elev_charts.chartView.series(elev_ch).count > 400)
                {
                    elev_charts.chartView.series(elev_ch).remove(0)
                }
            }

            atc.setGraphUpdating(false)
        }
    }

    Keys.forwardTo: [trackerView]
}
