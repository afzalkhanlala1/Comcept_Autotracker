import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14

Item {
    id: mainItem
    property int toggle: 0
    property string selectedColor: "#416a82"
    property string deSelectedColor: "#191e21"
    property bool pushToMake: false
    property bool verticalOrientation: false

    property alias leftText: label_01.text
    property alias rightText: label_02.text

    onVerticalOrientationChanged: {
        if(verticalOrientation)
        {
            rect_01.anchors.bottomMargin = mainItem.height * 0.495
            rect_01.anchors.rightMargin = mainItem.width * 0.05

            rect_02.anchors.leftMargin = mainItem.width * 0.05
            rect_02.anchors.topMargin = mainItem.height * 0.505
        }

        else
        {
            rect_01.anchors.bottomMargin = mainItem.height * 0.05
            rect_01.anchors.rightMargin = mainItem.width*0.5 + (mainItem.width*0.005)

            rect_02.anchors.leftMargin = mainItem.width*0.5 + (mainItem.width*0.005)
            rect_02.anchors.topMargin = mainItem.height * 0.05
        }
    }

    Rectangle{
        id: bgRect
        anchors.fill: parent
        color: "#a8a8a8"
        radius: rect_01.radius

    }

    Rectangle{
        id: rect_01
        color: selectedColor
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.05
        anchors.bottomMargin: parent.height * 0.05
        anchors.leftMargin: parent.width * 0.05
        anchors.rightMargin: parent.width*0.5 + (mainItem.width*0.005)
        radius: width > height ? height * 0.1 : width*0.1

        Behavior on color { ColorAnimation { duration: 150 } }

        Label{
            id: label_01
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.075
            anchors.bottomMargin: parent.height * 0.075
            anchors.leftMargin: parent.width * 0.05
            anchors.rightMargin: parent.width * 0.05
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPointSize: 8
            font.pointSize: 24
            fontSizeMode: Text.Fit
            text: "1"
        }

        MouseArea{
            id: mouseArea_01
            anchors.fill: parent

            onPressed: {
                if(pushToMake) {}
                else { toggle = 0; rect_01.color = selectedColor; rect_02.color = deSelectedColor }
            }
        }
    }

    Rectangle{
        id: rect_02
        color: deSelectedColor
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.5 + (mainItem.width*0.005)
        anchors.topMargin: parent.height * 0.05
        anchors.bottomMargin: parent.height * 0.05
        anchors.rightMargin: parent.width * 0.05
        radius: width > height ? height * 0.1 : width*0.1

        Behavior on color { ColorAnimation { duration: 150 } }

        Label{
            id: label_02
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.075
            anchors.bottomMargin: parent.height * 0.075
            anchors.leftMargin: parent.width * 0.05
            anchors.rightMargin: parent.width * 0.05
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPointSize: 8
            font.pointSize: 24
            fontSizeMode: Text.Fit
            text: "2"
        }

        MouseArea{
            id: mouseArea_02
            anchors.fill: parent

            onPressed: { toggle = 1; rect_02.color = selectedColor; rect_01.color = deSelectedColor }
            onReleased:{
                if(pushToMake) { toggle = 0; rect_01.color = selectedColor; rect_02.color = deSelectedColor }
            }
        }
    }
}
