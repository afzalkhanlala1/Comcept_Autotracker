import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12

Item{
    id: mainItem

    property bool upPressed: false
    property bool downPressed: false
    property bool leftPressed: false
    property bool rightPressed: false

    property alias upButton: upMouseArea
    property alias downButton: downMouseArea
    property alias leftButton: leftMouseArea
    property alias rightButton: rightMouseArea

    property alias upText: upLabel.text
    property alias downText: downLabel.text
    property alias leftText: leftLabel.text
    property alias rightText: rightLabel.text

    Rectangle{
        id: mcrTabRect
        color: "#002d363b"
        anchors.fill: parent
    }

    Image {
        id: arrowImage
        source: "../images/dir_arrow.png"
        anchors.fill: parent
        fillMode: Qt.KeepAspectRatio
        cache: false
    }

    MouseArea{
        id: upMouseArea
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.3
        anchors.rightMargin: parent.width * 0.3
        anchors.bottomMargin: parent.height * 0.7
        anchors.topMargin: parent.height * 0.05
        onPressed: { arrowImage.source = "../images/dir_arrow_up.png"; upPressed = true }
        onReleased: { arrowImage.source = "../images/dir_arrow.png"; upPressed = false }

        Label{
            id: upLabel
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.6
            anchors.bottomMargin: -parent.height * 0.1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPointSize: 8
            font.pointSize: mainItem.height*0.05 < 8 ? 8 : mainItem.height*0.05
            fontSizeMode: Text.Fit
            text: "UP"
        }
    }

    MouseArea{
        id: leftMouseArea
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.02
        anchors.rightMargin: parent.width * 0.65
        anchors.bottomMargin: parent.height * 0.25
        anchors.topMargin: parent.height * 0.25
        onPressed : { arrowImage.source = "../images/dir_arrow_left.png"; leftPressed = true }
        onReleased: { arrowImage.source = "../images/dir_arrow.png"; leftPressed = false }

        Label{
            id: leftLabel
            anchors.fill: parent
            anchors.leftMargin: parent.width*0.6
            anchors.rightMargin: -parent.width*0
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
//            minimumPointSize: 8
//            font.pointSize: 24
//            fontSizeMode: Text.Fit
            font.pointSize: upLabel.font.pointSize
            text: "LEFT"
        }
    }

    MouseArea{
        id: rightMouseArea
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.65
        anchors.rightMargin: parent.width * 0.02
        anchors.bottomMargin: parent.height * 0.25
        anchors.topMargin: parent.height * 0.25
        onPressed : { arrowImage.source = "../images/dir_arrow_right.png"; rightPressed = true }
        onReleased: { arrowImage.source = "../images/dir_arrow.png"; rightPressed = false }

        Label{
            id: rightLabel
            anchors.fill: parent
            anchors.leftMargin: -parent.width*0
            anchors.rightMargin: parent.width*0.6
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
//            minimumPointSize: 8
//            font.pointSize: 24
//            fontSizeMode: Text.Fit
            font.pointSize: upLabel.font.pointSize
            text: "RIGHT"
        }
    }

    MouseArea{
        id: downMouseArea
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.3
        anchors.rightMargin: parent.width * 0.3
        anchors.bottomMargin: parent.height * 0.05
        anchors.topMargin: parent.height * 0.7
        onPressed : { arrowImage.source = "../images/dir_arrow_down.png"; downPressed = true }
        onReleased: { arrowImage.source = "../images/dir_arrow.png"; downPressed = false }

        Label{
            id: downLabel
            anchors.fill: parent
            anchors.topMargin: -parent.height * 0.3
            anchors.bottomMargin: parent.height * 0.8
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
//            minimumPointSize: 8
//            font.pointSize: 24
//            fontSizeMode: Text.Fit
            font.pointSize: upLabel.font.pointSize
            text: "DOWN"
        }
    }
}
