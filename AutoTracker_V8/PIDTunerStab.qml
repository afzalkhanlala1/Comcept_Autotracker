import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property alias maxStab: maxStabSpinbox
    property alias maxStabi: maxStabiSpinbox
    property alias pStab: pStabSlider
    property alias iStab: iStabSlider
    property alias dStab: dStabSlider

    Rectangle{
        id: limitsRectBg
        anchors.fill: parent
        anchors.bottomMargin: parent.height * 0.7
        color: "#1b1c1f"

        GridLayout{
            id: gridLayoutLimits
            anchors.fill: parent
            anchors.leftMargin: limitsRectBg.width * 0.01
            anchors.rightMargin: limitsRectBg.width * 0.01
            anchors.topMargin: limitsRectBg.height * 0.01
            anchors.bottomMargin: limitsRectBg.height * 0.0
            columns: 2

            Label{
                id: maxStabLabel
                text: "MAX_STAB"
                font.pointSize: 9
                antialiasing: true
                smooth: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            SpinBox{
                id: maxStabSpinbox
                font.pointSize: 9
                Layout.fillHeight: true
                Layout.fillWidth: true
                from: 0
                to: 8192
                stepSize: 64
                value: 4096
                editable: true
            }

            Label{
                id: maxStabiLabel
                text: "MAX_I_STAB"
                font.pointSize: 9
                antialiasing: true
                smooth: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            SpinBox{
                id: maxStabiSpinbox
                Layout.fillHeight: true
                Layout.fillWidth: true
                font.pointSize: 9
                from: 0
                to: 1024
                stepSize: 32
                value: 512
                editable: true
            }
        }
    }

    Rectangle{
        id: controlsRectBg
        anchors.fill: parent
        anchors.topMargin: (parent.height * 0.3)
        color: "#1c1d23"

        GridLayout{
            id: gridLayoutControls
            anchors.fill: parent
            anchors.leftMargin: mainItem.width * 0.01
            anchors.rightMargin: mainItem.width * 0.01
            anchors.bottomMargin: mainItem.height * 0.0
            columns: 2

            Label{
                id: pStabLabel
                text: "P_STAB: " + pStabSlider.value.toFixed(3)
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 9
                antialiasing: true
                smooth: true
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Slider{
                id: pStabSlider
                Layout.fillHeight: true
                Layout.fillWidth: true
                //Layout.maximumHeight: gridLayoutControls.height * 0.25
                from: 0
                to: 800
                stepSize: 10
                value: 200

            }


            Label{
                id: iStabLabel
                text: "I_STAB: " + iStabSlider.value.toFixed(3)
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 9
                antialiasing: true
                smooth: true
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Slider{
                id: iStabSlider
                Layout.fillHeight: true
                Layout.fillWidth: true
                //Layout.maximumHeight: gridLayoutControls.height * 0.25
                from: 0
                to: 10
                stepSize: 0.5
                value: 4.0
            }

            Label{
                id: dStabLabel
                text: "D_STAB: " + dStabSlider.value.toFixed(3)
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 9
                antialiasing: true
                smooth: true
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Slider{
                id: dStabSlider
                Layout.fillHeight: true
                Layout.fillWidth: true
                //Layout.maximumHeight: gridLayoutControls.height * 0.25
                from: 0
                to: 4
                stepSize: 0.1
                value: 0.0
            }
        }
    }
}
