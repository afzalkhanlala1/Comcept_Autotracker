import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem
    //Ropacity: 0.6*!settingsView.isOpen

    function updateView()
    {
        if(atc.isNetworkConnected()) { connectionIcon.source = "../images/conn-blue.png" }
        else { connectionIcon.source = "../images/conn-red.png" }
    }

    Image {
        id: settingsIcon
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.75
        anchors.rightMargin: parent.width*0.01
        fillMode: Qt.KeepAspectRatio
        source: "../images/Settings-icon.png"
        antialiasing: true
        smooth: true

        visible: (opacity > 0)

        Behavior on opacity { PropertyAnimation { duration: 250 } }

        MouseArea{
            id: settingsMouseArea
            anchors.fill: parent
            onClicked: {
                settingsView.visible = !settingsView.visible
            }
        }
    }

    Image {
        id: connectionIcon
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.5
        anchors.rightMargin: parent.width*0.25
        antialiasing: true
        smooth: true
        fillMode: Qt.KeepAspectRatio
        cache: false
        source: "../images/conn-red.png"

        MouseArea{
            id: connectionMouseArea
            anchors.fill: parent
            onClicked: {
                connectionView.visible = !connectionView.visible
            }
        }
    }

    Image {
        id: dataViewIcon
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.25
        anchors.rightMargin: parent.width*0.5
        antialiasing: true
        smooth: true
        fillMode: Qt.KeepAspectRatio
        cache: false
        source: "../images/telemetry-icon.png"

        MouseArea{
            id: dataViewMouseArea
            anchors.fill: parent
            onClicked: {
                controlPanel.dataView.visible = !controlPanel.dataView.visible
            }
        }
    }

    Image {
        id: consoleIcon
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.01
        anchors.rightMargin: parent.width*0.75
        antialiasing: true
        smooth: true
        fillMode: Qt.KeepAspectRatio
        cache: false
        source: "../images/console.png"

        MouseArea{
            id: consoleMouseArea
            anchors.fill: parent
            onClicked: {
                if(consoleView.visible) { consoleView.close() }
                else { consoleView.open() }
            }
        }
    }
}
