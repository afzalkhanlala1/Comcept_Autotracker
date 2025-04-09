import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14
import AutoTrackerControllerClass 1.0

Window {
    id: mainAppWindow
    width: 1920
    height: 1080
    visible: true
    title: qsTr("AutoTracker")

    property int dataUpdateDelay: 40
    property int commTime: 50
    property bool settingsLoaded: false
    property bool wideScreen: true

    property alias atc: atc
    property alias trackerView: trackerView
    property alias updateTimer: updateTimer
    property alias dataUpdateDelay: mainAppWindow.dataUpdateDelay

    Material.theme: Material.Dark
    Material.accent: Material.Teal

    Component.onCompleted:
    {
        for (var i = Qt.application.screens.length-1;i>=0;i--)
        {
            if (Qt.application.screens[i].width === 2560)
            {
                x= Qt.application.screens[i].virtualX;
                y= Qt.application.screens[i].virtualY;
                break
            }
        }

        //showFullScreen()
        atc.init()
        if(atc.isHeadlessMode()) { visible = false; return; }
        updateTimer.start()
    }

    function loadSettings()
    {
        controlPanel.loadSettings()
        settingsLoaded = true
    }

    AutoTrackerController{
        id: atc
        anchors.fill: parent
        anchors.rightMargin: (parent.width > parent.height) ? parent.width - (parent.height * (4.0/3.0)) : 0
        anchors.bottomMargin: (parent.height > parent.width) ? parent.height - (parent.width* (3.0/4.0)) : 0

        TrackerView {
            id: trackerView
            anchors.fill: parent
        }
    }

    ControlPanel{
        id: controlPanel
        anchors.fill: parent
        anchors.leftMargin: atc.anchors.rightMargin > 0 ? atc.width+5 : 0
        anchors.topMargin: atc.anchors.bottomMargin > 0 ? atc.height+5 : 0
    }

    Timer{
        id: updateTimer
        interval: 1000/25
        running: false
        repeat: true
        onTriggered: {
            if( !settingsLoaded) { loadSettings() }
            //            loadSettings()
            atc.update()
            trackerView.updateView()
            controlPanel.updateView()
            mainAppWindow.title = atc.getSystemStatsStr()
        }
    }
}


