import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.12
import QtQuick.Extras 1.4
import QtQuick.Dialogs 1.3

Item {
    id: mainItem
    property variant trackingRects: [trackingRect_0, trackingRect_1, trackingRect_2, trackingRect_3, trackingRect_4]
    property int setCenterPoint: 0

    property alias trackingRects: mainItem.trackingRects
    property alias setCenterPoint: mainItem.setCenterPoint
    property alias centerMarkerImg: centerMarkerImg
    property alias crossHairImage: crossHairImage
    property alias inputSource: inputSource
    //    property alias videoPlaybackControlls: videoPlaybackControlls
    property alias fileDialog: fileDialog
    property alias inputIconImage: inputIconImage
    property alias osdView: osdView

    function updateView()
    {
        crossHairImage.opacity = 0.75 + (0.25*atc.isCorssHairMoving())
        crossHairImage.x = atc.getCorssHairX() - (crossHairImage.width*0.5)
        crossHairImage.y = atc.getCorssHairY() - (crossHairImage.height*0.5)
        //        crossHairImage.x =100
        //               crossHairImage.y = 500
        trackingActiveRect.opacity = atc.getTracking()

        //                videoPlaybackControlls.videoPlaybackLabel.text = atc.getPlaybackTime()
        //        if(!videoPlaybackControlls.slider.pressed) { videoPlaybackControlls.slider.value = atc.getVideoSeekerPos() }

                osdView.updateView()
    }

    focus: true
    Keys.onPressed: {
        if(event.key === Qt.Key_Shift)
        {
            atc.setActiveTarget(atc.getNextActiveTracker())
        }

        //        else if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
        //        {
        //            trackingEnabled = !trackingEnabled
        //            atc.setTracking(trackingEnabled)
        //        }

        event.accepted = true
    }

    Image {
        id: centerMarkerImg
        source: "../images/trackerCenter_04.png"
        anchors.fill: parent
        smooth: true
        antialiasing: true
        opacity: 0.6
        visible: false

        Behavior on opacity { PropertyAnimation { duration: 250 }}
    }

    Image{
        id: crossHairImage
        x: (parent.width*0.5) - (width*0.5)
        y: (parent.height*0.5) - (height*0.5)
        width: 72
        height: 72
        smooth: true
        antialiasing: true
        source: "../images/crossHair-01.png"
        opacity: 0.75
        //        onXChanged: { atc.setTrackToCenter(false, crossHairImage.x + (crossHairImage.width*0.5), crossHairImage.y + (crossHairImage.height*0.5)) }
        //        onYChanged: { atc.setTrackToCenter(false, crossHairImage.x + (crossHairImage.width*0.5), crossHairImage.y + (crossHairImage.height*0.5)) }

        Behavior on x { PropertyAnimation { id: chxAni; duration: mainAppWindow.updateTimer.interval }}
        Behavior on y { PropertyAnimation { id: chyAni; duration: mainAppWindow.updateTimer.interval }}
        Behavior on opacity { PropertyAnimation { duration: 250 }}
    }

    TrackerRects{
        id: trackingRect_0
        anchors.fill: parent
        visible: false
        property int myIndex: 0
        opacity: 0.6
    }

    TrackerRects{
        id: trackingRect_1
        anchors.fill: parent
        visible: false
        property int myIndex: 0
        opacity: 0.6
    }

    TrackerRects{
        id: trackingRect_2
        anchors.fill: parent
        visible: false
        property int myIndex: 0
        opacity: 0.6
    }

    TrackerRects{
        id: trackingRect_3
        anchors.fill: parent
        visible: false
        property int myIndex: 0
        opacity: 0.6
    }

    TrackerRects{
        id: trackingRect_4
        anchors.fill: parent
        visible: false
        property int myIndex: 0
        opacity: 0.6
    }

    Rectangle{
        id: trackingActiveRect
        anchors.fill: parent
        anchors.topMargin: parent.height*0.01
        anchors.bottomMargin: parent.height * 0.95
        anchors.rightMargin: parent.width * 0.01
        anchors.leftMargin: parent.width * 0.96
        color: "#28de58"
        radius: width
        opacity: 0

        Behavior on opacity { PropertyAnimation { duration: 250 }}
    }

    Rectangle{
        id: centerLabelBgRect
        x: 0
        y: 0
        width: 80
        height: 30
        color: "#b31a1a1c"
        visible: false

        Label{
            id: centerLabel
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: ""
            font.pixelSize: centerLabelBgRect.height * 0.7 < 8 ? 8 : centerLabelBgRect.height * 0.7
        }
    }

    MouseArea{
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {

            mainItem.focus = true
            if(setCenterPoint == 1)
            {
                setCenterPoint = 0
                atc.setTrackToCenter(false, mouse.x, mouse.y)
                //trackerSettingsView.trackToCenterCheckBox.checked = false
                //trackerSettingsView.trackToButton.text = "Select-Track-Center\n" + (mouse.x * atc.getImgScalingFactor()).toFixed(0) + " x " + (mouse.y * atc.getImgScalingFactor()).toFixed(0)
                centerLabelBgRect.visible = false
            }

            else
            {
                var t_index = atc.isCrossHairOnTarget(mouse.x, mouse.y)
                if(t_index === -1)
                {
                    //                    atc.setTrackTarget(mouse.x - (mainItem.width*0.24*0.5), mouse.y - (mainItem.height*0.15*0.5), mainItem.width*0.24, mainItem.height*0.15)
                    atc.setTrackTarget(mouse.x - (mainItem.width*0.1*0.5), mouse.y - (mainItem.height*0.07*0.5), mainItem.width*0.1, mainItem.height*0.07)
                }
                else { atc.cancelTargetRect(t_index) }
            }
        }

        onPressAndHold: {
            //addRemoveTargetRects(mouseX - (crossHairImage.width*0.5), mouseY - (crossHairImage.height*0.5))
        }

        onPositionChanged:
        {
            if(setCenterPoint == 1)
            {
                centerLabelBgRect.visible = true
                centerLabel.text = (mouse.x * atc.getImgScalingFactor()).toFixed(0) + " x " + (mouse.y * atc.getImgScalingFactor()).toFixed(0)
                centerLabelBgRect.x = mouse.x
                centerLabelBgRect.y = (mouse.y - centerLabelBgRect.height)
            }
        }
    }

    Image{
        id: inputIconImage
        anchors.fill: parent
        anchors.topMargin: parent.height*0.01
        anchors.leftMargin: parent.width*0.01
        anchors.bottomMargin: parent.height*0.95
        anchors.rightMargin: parent.width*0.95
        source: "../images/input_icon.png"
        fillMode: Qt.KeepAspectRatio
        opacity: (0.5*(!atc.isDeployMode())) + (0.5*inputMousearea.containsMouse)

        Behavior on opacity { PropertyAnimation { duration: 250 } }

        MouseArea{
            id: inputMousearea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                inputSource.checkList()
                inputSource.visible = true
            }
        }
    }

    InputSource{
        id: inputSource
        anchors.fill: parent
        anchors.topMargin: parent.height*0.01
        anchors.leftMargin: parent.width*0.01
        anchors.bottomMargin: parent.height*0.6
        anchors.rightMargin: parent.width*0.75
        visible: false
    }

    //    VideoPlaybackControlls{
    //        id: videoPlaybackControlls
    //        anchors.fill: parent
    //        anchors.topMargin: parent.height * 0.95
    //        //visible: false
    //    }

    OSDView{
        id: osdView
        anchors.fill: parent
    }

    FileDialog {
        id: fileDialog
        title: "Please choose a file"
        folder: "file:///home/zulqarnain/Videos/"
        onAccepted: {
            inputSource.visible = false
            atc.setVideoDevice(fileDialog.fileUrl, true)
            //            videoPlaybackControlls.visible = true
            close()
        }
        onRejected: {
            close()
        }
    }
}
