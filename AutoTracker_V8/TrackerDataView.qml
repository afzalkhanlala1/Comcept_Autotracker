import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property alias opVelView: opVelView
    property alias dasiamrpnVelView: dasiamrpnVelView

    function updateData()
    {
//        opVelView.updateVelocity(atc.getOPVelX()/200.0, atc.getOPVelY()/200.0)
//        opVelView.title.text = "OPFLow: " + atc.getOPVelX().toFixed(3) + ", " + atc.getOPVelY().toFixed(3)

        dasiamrpnVelView.updateVelocity(atc.getDaSiamRPNVelX()/200.0, atc.getDaSiamRPNVelY()/200.0)
        dasiamrpnVelView.title.text = "DaSiamRPN: " + atc.getDaSiamRPNVelX().toFixed(3) + ", " + atc.getDaSiamRPNVelY().toFixed(3)
    }

    Rectangle{
        id: bgRect
        color: "#242627"
        anchors.fill: parent
    }

    VelocityView{
        id: opVelView
        anchors.fill: parent
        anchors.bottomMargin: (parent.height*0.5) - 2.5
        title.text: "OPFlow: 0, 0"
    }

    VelocityView{
        id: dasiamrpnVelView
        anchors.fill: parent
        anchors.topMargin: (parent.height*0.5) + 5
        title.text: "DaSiamRPN: 0, 0"
    }
}
