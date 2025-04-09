import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14

Item {
    id: mainItem

    property string offColor: "#6e6e6e"
    property string onColor: "#12c98c"
    property bool blink: false
    property bool blinkState: false
    property int offTime: 1000
    property int onTime: 1000
    property bool toggle: false

    property alias animation: animation
    property alias color: rect.color
    property alias radius: rect.radius
    property alias text: label.text

    function on() { rect.color = onColor; }
    function off() { rect.color = offColor }

    function makeRound() {
        rect.radius = mainItem.width > mainItem.height ? mainItem.width * 0.5 : mainItem.height * 0.5
        rect.anchors.topMargin = mainItem.height > mainItem.width ? (mainItem.height-mainItem.width)*0.5 : 0
        rect.anchors.bottomMargin =  mainItem.height > mainItem.width ? (mainItem.height-mainItem.width)*0.5 : 0
        rect.anchors.leftMargin = mainItem.width > mainItem.height ? (mainItem.width - mainItem.height)*0.5 : 0
        rect.anchors.rightMargin = mainItem.width > mainItem.height ? (mainItem.width - mainItem.height)*0.5 : 0
    }

    function makeRoundedRect(_radius) {
        rect.radius = _radius
        rect.anchors.topMargin = rect.anchors.bottomMargin = 0
        rect.anchors.leftMargin = rect.anchors.rightMargin = 0
    }

    NumberAnimation {
        id: blinkAni
        duration: offTime
        loops: 1
        onFinished: {
            blinkState = !blinkState
            if(blinkState) { on(); blinkAni.duration = onTime }
            else { off(); blinkAni.duration = offTime }
             blinkAni.start()
        }
    }

    onBlinkChanged: {
        if(blink) { blinkAni.start() }
        else { blinkAni.stop(); off(); }
    }

    onToggleChanged: {
        if(toggle) { on() }
        else { off() }
    }

    Rectangle {
        id: rect
        anchors.fill: parent
        radius: mainItem.width > mainItem.height ? mainItem.width * 0.5 : mainItem.height * 0.5
        anchors.topMargin: mainItem.height > mainItem.width ? (mainItem.height-mainItem.width)*0.5 : 0
        anchors.bottomMargin: mainItem.height > mainItem.width ? (mainItem.height-mainItem.width)*0.5 : 0
        anchors.leftMargin: mainItem.width > mainItem.height ? (mainItem.width - mainItem.height)*0.5 : 0
        anchors.rightMargin: mainItem.width > mainItem.height ? (mainItem.width - mainItem.height)*0.5 : 0
        color: offColor

        Behavior on color { ColorAnimation { id: animation; duration: 100 } }
    }

    Label{
        id: label
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.15
        anchors.bottomMargin: parent.height * 0.15
        anchors.leftMargin: parent.width * 0.1
        anchors.rightMargin: parent.width * 0.1
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        minimumPointSize: 8
        font.pointSize: 24
        fontSizeMode: Text.Fit
        text: ""
        visible: (label.text.length > 0)
    }
}
