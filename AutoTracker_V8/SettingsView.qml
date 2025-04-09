import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14

Item {
    id: mainItem

    Behavior on anchors.leftMargin { PropertyAnimation { duration: 150 } }
    Behavior on anchors.rightMargin { PropertyAnimation { duration: 150 } }

    function loadSettings()
    {
        pidTunerView.loadSettings()
    }

    function updateView()
    {

    }

    Rectangle{
        id: bgRect
        color: "#212126"
        anchors.fill: parent
    }

    GridLayout{
        id: gridLayout
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.02
        anchors.rightMargin: parent.width * 0.02
        columns: 2
    }

    TabBar {
        id: bar
        anchors.fill: parent
        anchors.bottomMargin: parent.height*0.95

        TabButton {
            text: qsTr("PID")
        }
        TabButton {
            text: qsTr("Enhance")
        }
//        TabButton {
//            text: qsTr("IS-OPFlow")
//        }
    }

    StackLayout {
        anchors.fill: parent
        anchors.topMargin: bar.height*1.25
        currentIndex: bar.currentIndex

        PIDTunerView{
            id: pidTunerView
        }

        PreProcessingView{
            id: preProcessingView
        }

//        ISOPFlowSettingsView{
//            id: isopFlowSettingsView
//        }
    }
}
