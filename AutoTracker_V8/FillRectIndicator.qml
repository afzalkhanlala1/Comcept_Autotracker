import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14

Item {
    id: mainItem

    property real from: -1
    property real to: 1
    property real value: 0
    property real rectWidth: 0
    property real zeroX: 0
    property string name: ""

    property alias from: mainItem.from
    property alias to: mainItem.to
    property alias value: mainItem.value
    property alias fillColor: fillRect.color

    function constrain(val, min, max)
    {
        if(val < min) { return min }
        if(val > max) { return max }
        return val
    }

    function updateRanges()
    {
        zeroX = (frameRect.width - (frameRect.border.width*4)) * (Math.abs(from) / (Math.abs(to) + Math.abs(from)))
        fillRect.x = zeroX
        fillRect.y = frameRect.border.width*2
        fillRect.height = frameRect.height - (frameRect.border.width*4)

        centerMarker.x = zeroX - (centerMarker.width*0.5)

        updateWidth()
    }

    function updateWidth()
    {
        rectWidth = (constrain(value, from, to) / (to - from)) * (frameRect.width - (frameRect.border.width*4))
        if(value < 0)
        {
            fillRect.x = zeroX + rectWidth
            fillRect.width = -rectWidth
        }

        else
        {
            fillRect.x = zeroX
            fillRect.width = rectWidth
        }
    }

    Component.onCompleted: { updateRanges(); updateWidth(); }
    onHeightChanged: { updateRanges(); updateWidth(); }
    onWidthChanged: { updateRanges(); updateWidth(); }

    onFromChanged: { updateRanges(); updateWidth(); }
    onToChanged: { updateRanges(); updateWidth(); }

    onValueChanged:
    {
        updateWidth()
        if(Math.abs(value) < 1) { valueLabel.text = name + "\t" + value.toFixed(3) }
        else if(Math.abs(value) < 100) { valueLabel.text = name + "\t" + value.toFixed(2) }
        else if(Math.abs(value) < 1000) { valueLabel.text = name + "\t" + value.toFixed(1) }
        else { valueLabel.text = name + "\t" + value.toFixed(0) }
    }

    Rectangle{
        id: frameRect
        anchors.fill: parent
        color: "#00ffffff"
        border.color: "#4791a2"
        border.width: 3
        radius: height * 0.05
    }

    Rectangle{
        id: fillRect
        x: (frameRect.width - (frameRect.border.width*4)) * (from / (to - from))
        y: frameRect.border.width*2
        height: frameRect.height - (frameRect.border.width*4)
        width: 0
        color: "#19c49f"
        radius: frameRect.radius

    }

    Rectangle{
        id: centerMarker
        y: frameRect.border.width*2
        height: frameRect.height - (frameRect.border.width*4)
        color: "#80ffffff"
        width: 5
        radius: width
    }

    Label{
        id: valueLabel
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.2
        anchors.bottomMargin: parent.height * 0.2
        anchors.leftMargin: parent.width * 0.1
        anchors.rightMargin: parent.width * 0.1
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        style: Text.Outline
        styleColor: "black"
        minimumPointSize: 8
        font.pointSize: 24
        fontSizeMode: Text.Fit
        text: name + "\t" + value.toFixed(2)
    }
}
