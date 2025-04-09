import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

Item {
    id: mainItem

    property int tsh_spacing: mainItem.width*0.01
    property int tsv_spacing: mainItem.height*0.02

    Rectangle{
        id: bgRect
        color: "#31373a"
        anchors.fill: parent
    }

    FourWayButton{
        id: vh_buttons
        anchors.fill: parent
        anchors.leftMargin: tsh_spacing
        anchors.topMargin: tsv_spacing
        anchors.bottomMargin: tsv_spacing
        anchors.rightMargin: parent.width - height

        upText: "CRV-"
        downText: "CRV+"
        leftText: "CRH-"
        rightText: "CRH+"

        onUpPressedChanged: { atc.setConsole("GCUP", upPressed) }
        onDownPressedChanged: { atc.setConsole("GDWN", downPressed) }
        onLeftPressedChanged: { atc.setConsole("GLFT", leftPressed) }
        onRightPressedChanged: { atc.setConsole("GRGH", rightPressed) }
    }

    GridLayout{
        id: gridLayout
        anchors.fill: parent
        anchors.leftMargin: vh_buttons.width + (tsh_spacing*2)
        anchors.rightMargin: tsh_spacing
        anchors.bottomMargin: tsv_spacing
        anchors.topMargin: tsv_spacing
        columns: 1

        ToggleSwitch{
            id: auto_man_lock_ts
            leftText: "A-LOCK"
            rightText: "M-LOCK"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("GAML", toggle) }
        }

        ToggleSwitch{
            id: far_near_lock_ts
            leftText: "FAR"
            rightText: "NEAR"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("GFNL", toggle) }
        }

        IndicatorButton{
            id: lock_ib
            label.text: "LOCK"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("GLOC", toggle) }
        }

        IndicatorButton{
            id: unlock_ib
            label.text: "UN-LOCK"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("GULO", toggle) }
        }

        IndicatorButton{
            id: autoLoader_rdy_ib
            label.text: "AUTO-LOADER"
            Layout.fillHeight: true
            Layout.fillWidth: true
            toggleType: true
            onToggledChanged: { atc.setConsole("ALRY", toggled) }
        }
    }
}
