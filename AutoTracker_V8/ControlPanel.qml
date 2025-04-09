import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property alias dataView: dataView
    property alias settingsView: settingsView
    property alias consoleView: consoleView
    property alias controlView: controlView

    function updateView()
    {
        settingsView.updateView()
        dataView.trackerDataView.updateData()
        dataView.motionView.updateData()
        controlView.updateView()

        infoView.infoLabel.text = atc.getSystemStatsInfoStr()
        infoView.infoLabel.text += "\n\n" + atc.getInfo()
        infoView.trackInfoLabel.text = atc.getTrackInfo()
    }

    function loadSettings()
    {
        dataView.telemetryView.loadSettings()
//        settingsView.loadSettings()
        dataView.telemetryView.dataTimer.start()
    }

    Rectangle{
        id: bgRect
        color: "#141618"
        anchors.fill: parent
    }

    Rectangle{
        id: controlViewRect
        anchors.fill: parent
        anchors.bottomMargin: parent.height*0.25
        color: "#30353b"
//        visible: false

        ControlView{
            id: controlView
            anchors.fill: parent
            visible: true
        }

        DataView{
            id: dataView
            anchors.fill: parent
            visible: false
        }

        SettingsView{
            id: settingsView
            anchors.fill: parent
            visible: false
        }

        ConsoleView{
            id: consoleView
            anchors.fill: parent
            visible: false
        }
    }

    InfoView{
        id: infoView
        anchors.fill: parent
        anchors.topMargin: parent.height*0.76
        anchors.bottomMargin: parent.height*0.1
    }

    IconsView{
        id: iconsView
        anchors.fill: parent
        anchors.topMargin: parent.height*0.92
        anchors.bottomMargin: parent.height*0.02
        anchors.leftMargin: parent.width*0.05
        anchors.rightMargin: parent.width*0.05
    }
}
