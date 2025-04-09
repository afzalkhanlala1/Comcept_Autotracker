import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property alias title: title

    function updateVelocity(xVel, yVel)
    {
        xRect.width = xVel * ((frameRect.width*0.5) - 10)
        yRect.height = yVel * ((frameRect.height*0.5) - 10)

        if(xVel >= 0){ xRect.x = frameRect.width * 0.5 }
        else { xRect.width *= -1; xRect.x = (frameRect.width * 0.5) - (xRect.width) }

        if(yVel >= 0){ yRect.y = frameRect.height*0.5 }
        else{ yRect.height *= -1; yRect.y = (frameRect.height * 0.5) - (yRect.height) }
    }

    Rectangle{
        id: frameRect
        color: "#141414"
        border.color: "#787a7c"
        border.width: 2
        anchors.fill: parent

        Rectangle{
            id: xAxisRect
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.495
            anchors.bottomMargin: parent.height * 0.495
            anchors.leftMargin: parent.width * 0.05
            anchors.rightMargin: parent.width * 0.05
            radius: height
            opacity: 0.6
        }

        Rectangle{
            id: yAxisRect
            anchors.fill: parent
            anchors.topMargin: parent.height * 0.05
            anchors.bottomMargin: parent.height * 0.05
            anchors.leftMargin: parent.width * 0.498
            anchors.rightMargin: parent.width * 0.498
            radius: height
            opacity: 0.6
        }

        Rectangle{
            id: xRect
            x: parent.width * 0.5
            y: (parent.height*0.5) - (height*0.5)
            height: parent.height * 0.1
            width: 0
            color: "#32a871"
            radius: height * 0.25
            opacity: 0.8
        }

        Rectangle{
            id: yRect
            x: (parent.width*0.5) - (width*0.5)
            y: (parent.height*0.5)
            height: 0
            width: parent.height * 0.1
            color: "#27d6e3"
            radius: width*0.25
            opacity: 0.8
        }

        Label{
            id: title
            anchors.fill: parent
            anchors.topMargin: frameRect.border.width*1.5
            anchors.leftMargin: frameRect.border.width*1.5
            anchors.bottomMargin: frameRect.height * 0.9
            anchors.rightMargin: frameRect.width * 0.85
            font.pointSize: 8
            text: ""
        }
    }
}
