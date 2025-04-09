import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property real mouseXStart: mainItem.width*0.5
    property real mouseYStart: mainItem.height*0.5
    property real stickCenterX: (mainItem.width*0.5) - (stickHead.width*0.5)
    property real stickCenterY: (mainItem.height*0.5) - (stickHead.height*0.5)
    property real moveX: 0
    property real moveY: 0
    property real angle: 0
    property real radius: 200

    property real xPos: 0
    property real yPos: 0

    property real d2r: 0.01745329252
    property real r2d: 57.2957795130

    property alias xPos: mainItem.xPos
    property alias yPos: mainItem.yPos

    function constrain(val, min, max)
    {
        if(val < min) { return min }
        if(val > max) { return max }
        return val
    }

    function getAngle360(x1, y1, x2, y2)
    {
        return (360 - ((Math.atan2(y2 - y1, x2 - x1) * r2d) - 90))
    }

    function getDistance(x1, y1, x2, y2)
    {
        return (Math.sqrt( ((x2 - x1)*(x2 - x1)) + ((y2 - y1)*(y2 - y1)) ) )
    }

    function updateView()
    {

    }

    function updateStick(mouseX, mouseY)
    {
        stickCenterX = (mainItem.width*0.5)
        stickCenterY = (mainItem.height*0.5)

        angle = getAngle360(stickCenterX, stickCenterY, mouseX, mouseY)
        radius = constrain(getDistance(stickCenterX, stickCenterY, mouseX, mouseY), -(mainItem.width*0.5)-(stickHead.width*0.5), (mainItem.width*0.5)-(stickHead.width*0.5))

        moveX =  mouseX - (mainItem.width*0.5)
        moveY =  mouseY - (mainItem.height*0.5)

        stickHead.x = (mainItem.width*0.5) - (stickHead.width*0.5) + (Math.sin(angle*d2r)*radius)
        stickHead.y = (mainItem.height*0.5) - (stickHead.width*0.5) + (Math.cos(angle*d2r)*radius)

        xPos =  constrain(((stickHead.x / (joyAreaImage.width - stickHead.width*0.5))*2) - 1, -1, 1)
        yPos = constrain(((stickHead.y / (joyAreaImage.width - stickHead.height*0.5))*2) - 1, -1, 1)
    }

    Rectangle{
        id: bgRect
        color: "#181a1e"
        anchors.fill: parent
    }

    Image {
        id: joyAreaImage
        anchors.fill: parent
        anchors.margins: parent.width*0.026 < 4 ? 4 : parent.width*0.026
        fillMode: Qt.KeepAspectRatio
        source: "../images/jotarea.png"
    }

    Rectangle{
        id: joyArea
        anchors.fill: parent
        anchors.margins: parent.width*0.025 < 3 ? 3 : parent.width*0.025
        radius: height*0.5
        color: "transparent"
        border.color: "#105c66"
        border.width: height*0.025 < 4 ? 4 : height*0.025

        MouseArea{
            id: joyMouseArea
            anchors.fill: parent
            hoverEnabled: true

            onPressedChanged:
            {
                if(pressed)
                {
                    //xStickAni.duration = yStickAni.duration = 0;
                    mouseXStart = mouseX
                    mouseYStart = mouseY
                    updateStick(mouseX, mouseY)
                }
            }

            onReleased:
            {
                //xStickAni.duration = yStickAni.duration = 250
                stickHead.x = stickCenterX - (stickHead.width*0.5)
                stickHead.y = stickCenterY - (stickHead.height*0.5)
                xPos = yPos = 0
            }

            onPositionChanged:
            {
                if(pressed)
                {
                    updateStick(mouseX, mouseY)
                }
            }
        }
    }

    Rectangle{
        id: stickHead
        width: parent.width*0.15 < 10 ? 10 : parent.width*0.15
        height: width
        border.width: width*0.075 < 5 ? 5 : width*0.075
        radius: width*0.5
        color: "#013a4b"
        border.color: "#56898f"
        x: (parent.width*0.5) - (width*0.5)
        y: (parent.height*0.5) - (height*0.5)

        Behavior on x { PropertyAnimation { id: xStickAni; duration: 500; easing.type: Easing.OutExpo } }
        Behavior on y { PropertyAnimation { id: yStickAni; duration: 500; easing.type: Easing.OutExpo } }

        Component.onCompleted: {
            stickCenterX = (mainItem.width*0.5) - (stickHead.width*0.5)
            stickCenterY = (mainItem.height*0.5) - (stickHead.height*0.5)
        }
    }

    Rectangle{
        id: cm
        width: mainItem.width*0.05 < 8 ? 8 : mainItem.width*0.05
        height: width
        radius: width*0.5
        x: (mainItem.width*0.5) - (width*0.5)
        y: (mainItem.height*0.5) - (width*0.5)
        color: "#56898f"
    }

}
