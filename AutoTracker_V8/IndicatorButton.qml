import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14

Item {
    id: mainItem

    property string bgColor: "#404547"
    property string hoverColor: "#386769"
    property string clickColor: "#ebc634"
    property string borderColor: "#0ec4e8"
    property bool toggleType: false
    property bool toggled: false
    property bool toggle: false

    //    property alias bgColor: mainItem.bgColor
    //    property alias hoverColor: mainItem.hoverColor
    //    property alias clickColor: mainItem.clickColor
    property alias label: buttonLabel
    property alias radius: bgRect.radius

    Rectangle{
        id: bgRect
        color: bgColor
        border.width: width > height ? height * 0.1 : width * 0.1
        border.color: "#00000000"
        anchors.fill: parent
        radius: width > height ? height * 0.08 : width * 0.08

        Behavior on color { ColorAnimation { duration: 100 } }
    }

    Label{
        id: buttonLabel
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.075
        anchors.bottomMargin: parent.height * 0.075
        anchors.leftMargin: parent.width * 0.05
        anchors.rightMargin: parent.width * 0.05
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        minimumPointSize: 8
        font.pointSize: 20
        fontSizeMode: Text.Fit
        text: "Button"
    }

    MouseArea{
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onContainsMouseChanged: {
            if(toggleType && toggled){  }
            else
            {
                if(containsMouse) { bgRect.color = hoverColor }
                else { bgRect.color = bgColor }
            }
        }

        onPressed:{
            bgRect.color = clickColor;
            toggled = !toggled

            if(toggleType && toggled) { bgRect.border.color = borderColor }
            else { bgRect.border.color = "#00000000" }
            toggle = true
        }
        onReleased:{
            if(toggleType && toggled){  }
            else
            {
                if(containsMouse) { bgRect.color = hoverColor }
                else { bgRect.color = bgColor }
            }
            toggle = false
        }
    }
}
