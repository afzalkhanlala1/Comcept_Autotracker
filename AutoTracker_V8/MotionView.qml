import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    function updateData()
    {
        velXLabel.text = "VEL-X: " + atc.getObjectVelX().toFixed(2)
        velYLabel.text = "VEL-Y: " + atc.getObjectVelY().toFixed(2)
        accelXLabel.text = "ACCEL-X: " + atc.getObjectAccelX().toFixed(2)
        accelYLabel.text = "ACCEL-Y: " + atc.getObjectAccelY().toFixed(2)

        velXRect.width = (Math.abs(atc.getObjectVelX()) / 250) * velXFrame.width
        velYRect.width = (Math.abs(atc.getObjectVelY()) / 250) * velYFrame.width
        accelXRect.width = (Math.abs(atc.getObjectAccelX()) / 250) * accelXFrame.width
        accelYRect.width = (Math.abs(atc.getObjectAccelY()) / 250) * accelYFrame.width
    }

    Rectangle{
        id: bgRect
        color: "#2c2d2f"
        anchors.fill: parent
    }

    Frame{
        id: velXFrame
        anchors.fill: parent
        anchors.bottomMargin: parent.height * 0.75
        anchors.leftMargin: parent.width * 0.02
        anchors.rightMargin: parent.width * 0.02

        Rectangle{
            id: velXRect
            x: 5
            y: 0
            width: 0
            height: velXFrame.height - 20
            color: "#d9ab2e"
            radius: 5

            Behavior on width { PropertyAnimation{ duration: 40 } }
        }

        Label{
            id: velXLabel
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "VEL-X: 0"
        }
    }

    Frame{
        id: velYFrame
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.25
        anchors.bottomMargin: parent.height * 0.5
        anchors.leftMargin: parent.width * 0.02
        anchors.rightMargin: parent.width * 0.02

        Rectangle{
            id: velYRect
            x: 5
            y: 0
            width: 0
            height: velYFrame.height - 20
            color: "#d9362e"
            radius: 5

            Behavior on width { PropertyAnimation{ duration: 40 } }
        }

        Label{
            id: velYLabel
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "VEL-Y: 0"
        }
    }

    Frame{
        id: accelXFrame
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.5
        anchors.bottomMargin: parent.height * 0.25
        anchors.leftMargin: parent.width * 0.02
        anchors.rightMargin: parent.width * 0.02

        Rectangle{
            id: accelXRect
            x: 5
            y: 0
            width: 0
            height: accelXFrame.height - 20
            color: "#4fa1b0"
            radius: 5

            Behavior on width { PropertyAnimation{ duration: 40 } }
        }

        Label{
            id: accelXLabel
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "ACCEL-X: 0"
        }
    }

    Frame{
        id: accelYFrame
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.75
        anchors.bottomMargin: parent.height * 0
        anchors.leftMargin: parent.width * 0.02
        anchors.rightMargin: parent.width * 0.02

        Rectangle{
            id: accelYRect
            x: 5
            y: 0
            width: 0
            height: accelYFrame.height - 20
            color: "#50b58e"
            radius: 5

            Behavior on width { PropertyAnimation{ duration: 40 } }
        }

        Label{
            id: accelYLabel
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "ACCEL-Y: 0"
        }
    }
}
