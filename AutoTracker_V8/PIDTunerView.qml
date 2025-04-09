import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    function loadSettings()
    {
        xPIDTuner.maxStab.value = atc.getSetting("PXME")
        xPIDTuner.maxStabi.value = atc.getSetting("PXMI")
        xPIDTuner.pStab.value = atc.getSetting("PXKP")
        xPIDTuner.iStab.value = atc.getSetting("PXKI")
        xPIDTuner.dStab.value = atc.getSetting("PXKD")

        yPIDTuner.maxStab.value = atc.getSetting("PYME")
        yPIDTuner.maxStabi.value = atc.getSetting("PYMI")
        yPIDTuner.pStab.value = atc.getSetting("PYKP")
        yPIDTuner.iStab.value = atc.getSetting("PYKI")
        yPIDTuner.dStab.value = atc.getSetting("PYKD")

        invertAziCheckBox.checked = atc.getSetting("INAA")
        invertPitCheckBox.checked = atc.getSetting("INPA")
    }

    PIDTunerStab{
        id: xPIDTuner
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.02
        anchors.rightMargin: parent.width*0.02
        anchors.topMargin: parent.height * 0.01
        anchors.bottomMargin: parent.height * 0.56

        maxStab.to: 4096
        maxStab.stepSize: 8
        maxStabi.to: 2048
        maxStabi.stepSize: 4
        maxStab.value: 4096
        maxStabi.value: 128

        pStab.to: 400.0
        pStab.stepSize: 0.2
        pStab.value: 128.0
        iStab.to: 50.0
        iStab.stepSize: 0.001
        iStab.value: 0.0
        dStab.to: 50
        dStab.stepSize: 0.001
        dStab.value: 0.0

        maxStab.onValueModified: {
            atc.set("PXME", maxStab.value, false)
            atc.setPIDxMax(maxStab.value)
        }

        maxStabi.onValueModified: {
            atc.set("PXMI", maxStabi.value, false)
            atc.setPIDxMaxI(maxStabi.value)
        }

        pStab.onPressedChanged: {
            if(!pStab.pressed)
            {
                atc.set("PXKP", pStab.value, false)
                atc.setPIDxKP(pStab.value)
            }
        }

        iStab.onPressedChanged: {
            if(!iStab.pressed)
            {
                atc.set("PXKI", iStab.value, false)
                atc.setPIDxKI(iStab.value)
            }
        }

        dStab.onPressedChanged: {
            if(!dStab.pressed)
            {
                atc.set("PXKD", dStab.value, false)
                atc.setPIDxKD(dStab.value)
            }
        }
    }

    PIDTunerStab{
        id: yPIDTuner
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.02
        anchors.rightMargin: parent.width*0.02
        anchors.topMargin: (parent.height*0.44) + 2
        anchors.bottomMargin: parent.height*0.12

        maxStab.to: 4096
        maxStab.stepSize: 8
        maxStabi.to: 2048
        maxStabi.stepSize: 4
        maxStab.value: 4096
        maxStabi.value: 128

        pStab.to: 400.0
        pStab.stepSize: 0.2
        pStab.value: 128.0
        iStab.to: 50.0
        iStab.stepSize: 0.001
        iStab.value: 0.0
        dStab.to: 50
        dStab.stepSize: 0.001
        dStab.value: 0.0

        maxStab.onValueModified: {
            atc.set("PYME", maxStab.value, false)
            atc.setPIDyMax(maxStab.value)
        }

        maxStabi.onValueModified: {
            atc.set("PYMI", maxStabi.value, false)
            atc.setPIDyMaxI(maxStabi.value)
        }

        pStab.onPressedChanged: {
            if(!pStab.pressed){
                atc.set("PYKP", pStab.value, false)
                atc.setPIDyKP(pStab.value)
            }
        }

        iStab.onPressedChanged: {
            if(!iStab.pressed){
                atc.set("PYKI", iStab.value, false)
                atc.setPIDyKI(iStab.value)
            }
        }

        dStab.onPressedChanged: {
            if(!dStab.pressed){
                atc.set("PYKD", dStab.value, false)
                atc.setPIDyKD(dStab.value)
            }
        }
    }

    Rectangle{
        id: analogGainRect
        color: "#232425"
        anchors.fill: parent
        anchors.leftMargin: parent.width*0.02
        anchors.rightMargin: parent.width*0.02
        anchors.topMargin: (parent.height*0.88) + 5
        anchors.bottomMargin: parent.height*0.01

        CheckBox{
            id: invertAziCheckBox
            anchors.fill: parent
            anchors.topMargin: parent.height*0.1
            anchors.leftMargin: parent.width*0.05
            anchors.rightMargin: parent.width*0.6
            font.pointSize: 14
            text: "AZI-INV"
            onCheckedChanged: {
                atc.set("INAA", checked)
            }
        }

        CheckBox{
            id: invertPitCheckBox
            anchors.fill: parent
            anchors.topMargin: parent.height*0.1
            anchors.leftMargin: parent.width*0.6
            anchors.rightMargin: parent.width*0.05
            font.pointSize: 14
            text: "ELEV-INV"
            onCheckedChanged: {
                atc.set("INPA", checked)
            }
        }
    }
}

