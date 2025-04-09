import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

Item {
    id: mainItem

    property int tsh_spacing: mainItem.width*0.0075
    property int tsv_spacing: mainItem.height*0.01

    Rectangle{
        id: bgRect
        color: "#26292b"
        anchors.fill: parent
    }

    Rectangle{
        id: imageBgRect
        anchors.fill: parent
        anchors.leftMargin: tsh_spacing*0.5
        anchors.topMargin: tsv_spacing*0.5
        anchors.bottomMargin: tsv_spacing*0.5
        anchors.rightMargin: parent.width - (height * (4/3))
        color: "black"
        visible: false

        Image {
            id: noImage
            anchors.fill: parent
            source: "../images/no-image-icon.png"
            fillMode: Qt.KeepAspectRatio
            anchors.margins: parent.height*0.25
        }
    }

    GridLayout{
        id: gridLayout
        anchors.fill: parent
        anchors.bottomMargin: tsv_spacing
        anchors.topMargin: tsv_spacing
        anchors.rightMargin: parent.width*0.5
        anchors.leftMargin: tsh_spacing
        rowSpacing: tsv_spacing*2
        columns: 1

        ToggleSwitch{
            id: ir_tv_ts
            leftText: "IR"
            rightText: "TV"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("TVTI", toggle) }
        }

        ToggleSwitch{
            id: run_align_ts
            leftText: "RUN"
            rightText: "ALIGN"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("GBST", toggle) }
        }

        ToggleSwitch{
            id: w_n_fov_ts
            leftText: "WFOV"
            rightText: "NFOV"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("IRWN", toggle) }
        }

        ToggleSwitch{
            id: lsm_lma_ts
            leftText: "LSA"
            rightText: "LSM"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("LSAM", toggle) }
        }

        ToggleSwitch{
            id: ira_irm_ts
            leftText: "IRA"
            rightText: "IRM"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("IRAM", toggle) }
        }

        IndicatorButton{
            id: reset_ib
            label.text: "RESET"
            Layout.fillHeight: true
            Layout.fillWidth: true
            onToggleChanged: { atc.setConsole("ARES", toggle) }
        }
    }

    FourWayButton{
        id: vh_corr_buttons
        anchors.fill: parent
        anchors.bottomMargin: tsv_spacing
        anchors.topMargin: tsv_spacing
        anchors.rightMargin: tsh_spacing
        anchors.leftMargin: parent.width*0.5
        upText: "V-"
        downText: "V+"
        leftText: "H-"
        rightText: "H+"

        onUpPressedChanged: { atc.setConsole("CRVN", upPressed) }
        onDownPressedChanged: { atc.setConsole("CRVP", downPressed) }
        onLeftPressedChanged: { atc.setConsole("CRHN", leftPressed) }
        onRightPressedChanged: { atc.setConsole("CRHP", rightPressed) }

    }
}
