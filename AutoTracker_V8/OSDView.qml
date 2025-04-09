import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    function updateView()
    {
        mainItem.visible = atc.getShowOSD()
        if(!mainItem.visible) { return }

        azLabel.text = "AZ:" + atc.getHoriLead().toFixed(1)
        elLabel.text = "EL:" + atc.getVertLead().toFixed(1)
        ammoLabel.text = atc.getAmmo()
        rfBg.visible = atc.getExtReady()
        distanceLabel.text = atc.getExtDistance().toFixed(0)
        gunnerModeBg.visible = atc.getGunnerMode()
    }

//    Rectangle{
//        id: azBg
//        x: parent.width*(32/640)
//        y: parent.height*(4/480)
//        width: parent.width*(122/640)
//        height: parent.height*(31/480)
//        color: "black"

//        Label{
//            id: azLabel
//            anchors.fill: parent
//            color: "white"
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            text: ""
//            font.pointSize: mainItem.height*0.032 < 8 ? 8 : mainItem.height*0.032
//        }
//    }

//    Rectangle{
//        id: elBg
//        x: parent.width*(182/640)
//        y: parent.height*(4/480)
//        width: parent.width*(122/640)
//        height: parent.height*(31/480)
//        color: "black"

//        Label{
//            id: elLabel
//            anchors.fill: parent
//            color: "white"
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            text: ""
//            font.pointSize: mainItem.height*0.032 < 8 ? 8 : mainItem.height*0.032
//        }
//    }

//    Rectangle{
//        id: ammoBg
//        x: parent.width*(124/640)
//        y: parent.height*(404/480)
//        width: parent.width*(30/640)
//        height: parent.height*(31/480)
//        color: "black"

//        Label{
//            id: ammoLabel
//            anchors.fill: parent
//            color: "white"
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            text: ""
//            font.pointSize: mainItem.height*0.032 < 8 ? 8 : mainItem.height*0.032
//        }
//    }

//    Rectangle{
//        id: rfBg
//        x: parent.width*(184/640)
//        y: parent.height*(404/480)
//        width: parent.width*(30/640)
//        height: parent.height*(31/480)
//        color: "black"

//        Label{
//            id: rfLabel
//            anchors.fill: parent
//            color: "white"
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            text: "RF"
//            font.pointSize: mainItem.height*0.032 < 8 ? 8 : mainItem.height*0.032
//        }
//    }

//    Rectangle{
//        id: disatanceBg
//        x: parent.width*(256/640)
//        y: parent.height*(404/480)
//        width: parent.width*(62/640)
//        height: parent.height*(31/480)
//        color: "black"

//        Label{
//            id: distanceLabel
//            anchors.fill: parent
//            color: "white"
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            text: ""
//            font.pointSize: mainItem.height*0.032 < 8 ? 8 : mainItem.height*0.032
//        }
//    }

//    Rectangle{
//        id: gunnerModeBg
//        x: parent.width*(362/640)
//        y: parent.height*(404/480)
//        width: parent.width*(62/640)
//        height: parent.height*(31/480)
//        color: "black"
//        Label{
//            id: gunnerModeLabel
//            anchors.fill: parent
//            color: "white"
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            text: "SFCS"
//            font.pointSize: mainItem.height*0.032 < 8 ? 8 : mainItem.height*0.032
//        }
//    }
}
