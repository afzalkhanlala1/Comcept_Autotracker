import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

Item
{
    id: mainItem

    property variant rects: [csrtRect, kcfRect, goturnRect, daSiamRPNRect, opFeatureRect, templateMatcherRect, opFlowRect, objectRect]
    property alias csrtRect: csrtRect
    property alias kcfRect: kcfRect
    property alias goturnRect: goturnRect
    property alias daSiamRPNRect: daSiamRPNRect
    property alias opFlowRect: opFlowRect
    property alias objectRect: objectRect
    property alias opFeatureRect: opFeatureRect
    property alias templateMatcherRect: templateMatcherRect
    property alias rects: mainItem.rects

    Rectangle{
        id: csrtRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#00000000"
        border.width: 2
        border.color: "#00f2cc50"
        radius: border.width
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }

    Rectangle{
        id: kcfRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#00000000"
        border.width: 2
        border.color: "#0050f29e"
        radius: border.width
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }

    Rectangle{
        id: goturnRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#00000000"
        border.width: 2
        border.color: "#006e50f2"
        radius: border.width
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }

    Rectangle{
        id: daSiamRPNRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#00000000"
        border.width: 2
        border.color: "#00e550f2"
        radius: border.width
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }

    Rectangle{
        id: opFeatureRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#007b00ff"
        border.width: 2
        border.color: "#007b00ff"
        radius: border.width
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }

    Rectangle{
        id: templateMatcherRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#00000000"
        border.width: 2
        border.color: "#00fbb111"
        radius: border.width
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }

    Rectangle{
        id: opFlowRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#00000000"
        border.width: 2
        border.color: "#00a6ff17"
        radius: border.width
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }

    Rectangle{
        id: objectRect
        x: 0
        y: 0
        width: 0
        height: 0
        color: "#00000000"
        border.width: 5
        border.color: "#12c0c9"
        radius: border.width
        opacity: 0.75
        property int myIndex: 0

//        Behavior on x { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on y { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on width { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
//        Behavior on height { PropertyAnimation { duration: mainAppWindow.updateTimer.interval }}
    }
}
