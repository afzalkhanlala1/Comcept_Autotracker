import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14
import DataFields 1.0

Item {
    id: mainItem

    property int hMargin: mainItem.width*0.05
    property int vMargin: mainItem.height*0.02

    function updateView()
    {
        at_gate_indicator.toggle = (atc.getOutputValues(DATA_FIELDS.AT_GATE) > 0.5)
        shapping_indicator.toggle = (atc.getOutputValues(DATA_FIELDS.SHAPPING_SIG) > 0.5)
        at_on_indicator.value = atc.getOutputValues(DATA_FIELDS.AT_ON)
        sight_volt_indicator.value = atc.getOutputValues(DATA_FIELDS.SIGHT_VOLT)
        curr_indicator.value = atc.getOutputValues(DATA_FIELDS.INP_CURRENT)
    }

    Rectangle{
        id: bgRect
        color: "#3c3e40"
        anchors.fill: parent
    }

    GridLayout{
        id: gridLayout
        anchors.fill: parent
        anchors.topMargin: vMargin
        anchors.bottomMargin: vMargin
        anchors.leftMargin: hMargin
        anchors.rightMargin: hMargin
        columns: 1
        rowSpacing: vMargin

        LedIndicator{
            id: at_gate_indicator
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "AT-GATE"
            onColor: "#f25535"

            Component.onCompleted: { at_gate_indicator.makeRoundedRect(10) }
        }

        LedIndicator{
            id: shapping_indicator
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "SHAPPING_SIG"
            onColor: "#1b9bc2"

            Component.onCompleted: { shapping_indicator.makeRoundedRect(10) }
        }

        FillRectIndicator{
            id: at_on_indicator
            Layout.fillHeight: true
            Layout.fillWidth: true
            from: 0
            to: 32
            name: "AT-ON"
            fillColor: "#1bc2a3"

        }

        FillRectIndicator{
            id: sight_volt_indicator
            Layout.fillHeight: true
            Layout.fillWidth: true
            from: 0
            to: 16
            name: "SIGHT"
            fillColor: "#1bc2a3"
        }

        FillRectIndicator{
            id: curr_indicator
            Layout.fillHeight: true
            Layout.fillWidth: true
            from: 0
            to: 20
            name: "AMP"
            fillColor: "#1bc2a3"
        }
    }
}
