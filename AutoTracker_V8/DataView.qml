import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    property alias telemetryView: telemetryView
    property alias trackerDataView: trackerDataView
    property alias motionView: motionView

    TabBar {
        id: bar
        anchors.fill: parent
        anchors.bottomMargin: parent.height * 0.94
        clip: true

        TabButton {
            text: "Plots"
            font.pointSize: 9
            width: implicitWidth
        }
        TabButton {
            text: "Velocities"
            font.pointSize: 9
            width: implicitWidth
        }

        TabButton{
            text: "Motion"
            font.pointSize: 9
            width: implicitWidth
        }
    }

    StackLayout {
        id: stackLayout
        anchors.fill: parent
        anchors.topMargin: parent.height * 0.06
        currentIndex: bar.currentIndex

        onCurrentIndexChanged: {

        }

        Item {
            id: plotsTab
            TelemetryView{
                id: telemetryView
                anchors.fill: parent
            }
        }

        Item {
            id: velocitiesTab
            TrackerDataView{
                id: trackerDataView
                anchors.fill: parent
            }
        }

        Item {
            id: motionTab
            MotionView{
                id: motionView
                anchors.fill: parent
            }
        }
    }
}
