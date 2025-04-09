import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    Rectangle{
        id: bgRect
        color: "#26282b"
        anchors.fill: parent
    }

    Label{
        id: sourceNameLabel
        text: sourceName
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 10
        color: "white"
    }

    MouseArea{
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onContainsMouseChanged: {
            if(containsMouse) { bgRect.color = "#363b3e" }
            else { bgRect.color = "#26282b" }
        }

        onClicked: {
            trackerView.inputSource.selectInput(index)
        }
    }
}
