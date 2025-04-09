import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property bool markTargetMode: true
    property alias markTargetMode: mainItem.markTargetMode
    property real xJoyInv: 1.0
    property real yJoyInv: 1.0

    function updateView()
    {
        atc.setGuiJoystick(touchJoystick.xPos*joySpeedSlider.value*xJoyInv, touchJoystick.yPos*joySpeedSlider.value*yJoyInv)
    }

    Rectangle{
        id: showCenterMarker
        color: "#0f0f0f"
        anchors.fill: parent
        anchors.topMargin: parent.height*0.02
        anchors.leftMargin: parent.width*0.02
        anchors.bottomMargin: parent.height*0.88
        anchors.rightMargin: parent.width*0.75

        Image {
            id: centerMarkerImage
            source: "../images/centermarker-bold.png"
            anchors.fill: parent
            anchors.margins: parent.height*0.1
            fillMode: Qt.KeepAspectRatio
            antialiasing: true
        }

        MouseArea{
            id: centerMarkerMouseArea
            anchors.fill: parent
            onClicked: {
                if(showCenterMarker.color == "#0f0f0f")
                {
                    trackerView.centerMarkerImg.visible = true
                    showCenterMarker.color = "#3e6b7a"
                }
                else
                {
                    trackerView.centerMarkerImg.visible = false
                    showCenterMarker.color = "#0f0f0f"
                }
            }
        }
    }

    Rectangle{
        id: selectTargetModeRect
        color: "#3e6b7a"
        anchors.fill: parent
        anchors.topMargin: parent.height*0.02
        anchors.leftMargin: parent.width*0.27
        anchors.bottomMargin: parent.height*0.88
        anchors.rightMargin: parent.width*0.50

        Image {
            id: selectTargetModeImage
            source: "../images/crossHair-01.png"
            anchors.fill: parent
            anchors.margins: parent.height*0.1
            fillMode: Qt.KeepAspectRatio
            antialiasing: true
        }

        MouseArea{
            id: selectTargetModeMouseArea
            anchors.fill: parent
            onClicked: {
                if(selectTargetModeRect.color == "#0f0f0f")
                {
                    selectTargetModeRect.color = "#3e6b7a"
                    trackerView.crossHairImage.visible = markTargetMode = true
                }
                else
                {
                    selectTargetModeRect.color = "#0f0f0f"
                    trackerView.crossHairImage.visible = markTargetMode = false
                }
            }
        }
    }

    Rectangle{
        id: enableTrackingRect
        color: "#091827"
        anchors.fill: parent
        anchors.topMargin: parent.height*0.02
        anchors.bottomMargin: parent.height*0.88
        anchors.leftMargin: parent.width*0.52
        anchors.rightMargin: parent.width*0.02

        Label{
            id: enableTrackingLabel
            anchors.fill: parent
            text: "Start\nTracking"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPointSize: 8
            font.pointSize: 20
            fontSizeMode: Text.Fit
            color: "white"
        }

        MouseArea{
            id: enableTrackingMouseArea
            anchors.fill: parent
            onClicked: {
                if(enableTrackingRect.color == "#091827")
                {
                    enableTrackingRect.color = "#cf7917"
                    enableTrackingLabel.text = "Stop\nTracking"
                    enableTrackingLabel.font.bold = true
                    atc.setUserEnableTracking(true)
                }

                else
                {
                    enableTrackingRect.color = "#091827"
                    enableTrackingLabel.text = "Start\nTracking"
                    enableTrackingLabel.font.bold = false
                    atc.setUserEnableTracking(false)
                }
            }
        }
    }

    Rectangle{
        id: nextTargetRect
        color: "#111a24"
        anchors.fill: parent
        anchors.topMargin: (parent.height*0.145)
        anchors.leftMargin: parent.width*0.02
        anchors.bottomMargin: parent.height*0.7
        anchors.rightMargin: (parent.width*0.50)

        Behavior on color { ColorAnimation { duration: 100 } }

        Label {
            id: nextTargetLabel
            anchors.fill: parent
            anchors.margins: parent.height*0.1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPointSize: 8
            font.pointSize: 20
            fontSizeMode: Text.Fit
            text: "Switch\nTarget"
            color: "white"
        }

        MouseArea{
            id: nextTargetMouseArea
            anchors.fill: parent
            onClicked: { atc.switchToNextTarget() }
            onPressed: { nextTargetRect.color = "#edb42f" }
            onReleased: { nextTargetRect.color = "#111a24" }
        }
    }

    Rectangle{
        id: cancelAllTargetRect
        color: "#111a24"
        anchors.fill: parent
        anchors.topMargin: (parent.height*0.145)
        anchors.leftMargin: (parent.width*0.52)
        anchors.bottomMargin: parent.height*0.7
        anchors.rightMargin: parent.width*0.02

        Behavior on color { ColorAnimation { duration: 100 } }

        Label {
            id: cancelAllTargetLabel
            anchors.fill: parent
            anchors.margins: parent.height*0.1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPointSize: 8
            font.pointSize: 20
            fontSizeMode: Text.Fit
            text: "Cancel\nTracking"
            color: "white"
        }

        MouseArea{
            id: cancelTargetMouseArea
            anchors.fill: parent
            onClicked: { atc.cancelAllTargetRect(); }
            onPressed: { cancelAllTargetRect.color = "#edb42f" }
            onReleased: { cancelAllTargetRect.color = "#111a24" }
        }
    }

    TouchJoystick{
        id: touchJoystick
        anchors.fill: parent
        anchors.topMargin: parent.height*0.335
        anchors.bottomMargin: (parent.height-anchors.topMargin) - width
        anchors.leftMargin: parent.width*0.06
        anchors.rightMargin: parent.width*0.06
        //        onXPosChanged: { atc.setGuiJoyX(xPos) }
        //        onYPosChanged: { atc.setGuiJoyY(yPos) }
    }

    Rectangle{
        id: joyControlRect
        color: "#131518"
        anchors.fill: parent
        anchors.topMargin: parent.height*0.38 + touchJoystick.height
        anchors.bottomMargin: parent.height*0.005
        anchors.leftMargin: parent.width*0.01
        anchors.rightMargin: parent.width*0.01


        GridLayout{
            id: joyControlJoystick
            anchors.fill: parent
            anchors.leftMargin: parent.width*0.05
            anchors.rightMargin: parent.width*0.05
            columns: 2

            CheckBox{
                id: joyInvertX
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment:  Qt.AlignVCenter | Qt.AlignHCenter
                font.pointSize: 14
                text: "INV-Joy-X"
                onCheckedChanged: {
                    if(checked) { xJoyInv = -1.0 }
                    else { xJoyInv = 1.0 }

                }
            }

            CheckBox{
                id: joyInvertY
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment:  Qt.AlignVCenter | Qt.AlignHCenter
                font.pointSize: 14
                text: "INV-Joy-Y"
                onCheckedChanged: {
                    if(checked) { yJoyInv = -1.0 }
                    else { yJoyInv = 1.0 }
                }
            }

            Slider{
                id: joySpeedSlider
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 2
                from: 0
                to: 4096
                value: 4096
            }
        }
    }
}
