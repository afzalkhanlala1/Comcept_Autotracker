import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14

Item {
    id: mainItem

    property int tsh_spacing: mainItem.width*0.01
    property int tsv_spacing: mainItem.height*0.1

    Rectangle{
        id: bgRect
        color: "#2f3336"
        anchors.fill: parent
    }

    FillRectIndicator{
        id: xErrIndicator
        anchors.fill: parent
        anchors.topMargin: tsv_spacing
        anchors.bottomMargin: (parent.height*0.5) + (tsv_spacing*0.5)
        anchors.leftMargin: tsh_spacing
        anchors.rightMargin: parent.width*0.1
        name: "X-Err"
        from: -5
        to: 5
        value: 0
    }

    FillRectIndicator{
        id: yErrIndicator
        anchors.fill: parent
        anchors.topMargin: (parent.height*0.5) + (tsv_spacing*0.5)
        anchors.bottomMargin: tsv_spacing
        anchors.leftMargin: tsh_spacing
        anchors.rightMargin: parent.width*0.1
        name: "Y-Err"
        from: -5
        to: 5
        value: 0
    }

    LedIndicator{
        id: xDirLed
        anchors.fill: parent
        anchors.topMargin: tsv_spacing
        anchors.bottomMargin: (parent.height*0.5) + (tsv_spacing*0.5)
        anchors.leftMargin: xErrIndicator.width + (tsh_spacing*2)
        anchors.rightMargin: tsh_spacing
    }

    LedIndicator{
        id: yDirLed
        anchors.fill: parent
        anchors.topMargin: (parent.height*0.5) + (tsv_spacing*0.5)
        anchors.bottomMargin: tsv_spacing
        anchors.leftMargin: yErrIndicator.width + (tsh_spacing*2)
        anchors.rightMargin: tsh_spacing
    }
}
