import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property alias infoLabel: infoLabel
    property alias trackInfoLabel: trackInfoLabel

    Rectangle{
        id: bgRect
        color: "#292f33"
        anchors.fill: parent
        anchors.rightMargin: parent.width * 0.25

        Text{
            id: infoLabel
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.1
            anchors.bottomMargin: parent.height * 0.02
            anchors.leftMargin: parent.width * 0.05
            anchors.rightMargin: parent.width * 0.02
            text: "Info"
            font.pointSize: 10
            lineHeight: 1.2
            color: "#ffffff"
        }
    }

    Rectangle{
        id: bgTrackRect
        color: "#373d40"
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.75

        Text{
            id: trackInfoLabel
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.1
            anchors.bottomMargin: parent.height * 0.02
            anchors.leftMargin: parent.width * 0.05
            anchors.rightMargin: parent.width * 0.05
            text: "Track-Info"
            font.pointSize: 10
            lineHeight: 1.2
            color: "#ffffff"
        }
    }
}
