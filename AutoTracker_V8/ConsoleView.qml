import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14

Item {
    id: mainItem
    visible: false
    anchors.fill: parent

    function open()
    {
        visible = true
    }

    function close()
    {
        visible = false
    }

    Rectangle{
        id: bgRect
        color: "#1d1d1e"
        anchors.fill: parent
    }

    GunnerMonitor{
        id: gunnerMonitor
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.005
        anchors.bottomMargin: parent.height*0.65
        anchors.leftMargin: parent.width * 0.01
        anchors.rightMargin: parent.width * 0.01
    }

    GunnerConsole{
        id: gunnerConsole
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.355
        anchors.bottomMargin: parent.height*0.255
        anchors.leftMargin: parent.width * 0.01
        anchors.rightMargin: parent.width * 0.01
    }

    TrackingErrView{
        id: trackingErrView
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.75
        anchors.bottomMargin: parent.height * 0.0
        anchors.leftMargin: parent.width * 0.005
        anchors.rightMargin: parent.width * 0.35
    }

    TrackerInfoView{
        id: trackerInfoView
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.75
        anchors.bottomMargin: parent.height * 0.0
        anchors.leftMargin: parent.width * 0.655
        anchors.rightMargin: parent.width * 0.002
    }

    Button{
        id: consoleCloseButton
        anchors.fill: parent
        anchors.topMargin: parent.height*0.0075
        anchors.bottomMargin: parent.height*0.85
        anchors.rightMargin: parent.width*0.05
        anchors.leftMargin: parent.width*0.75
        visible: false
        text: "Close"
        onClicked: {
            close()
        }
    }

    Timer{
        id: updateTimer
        running: mainItem.visible
        repeat: true
        interval: 40
        onTriggered: {
            //trackerInfoView.updateView()
            //trackingErrView.updateView()
            //updateTimer.interval = atc.getDataDt()
        }
    }
}
