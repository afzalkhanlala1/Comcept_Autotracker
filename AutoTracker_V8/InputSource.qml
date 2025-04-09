import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    Rectangle {
        id: bgRect
        color: "#101111"
        anchors.fill: parent
        opacity: 0.75
    }

    function checkList()
    {
        inputListModel.clear()
        for(var c = 0; c < atc.getInputCameraCount(); c++){
            inputListModel.append({"sourceName": atc.getInputCameraName(c)})
        }

        inputListModel.append({"sourceName": "Video File"})
        inputListModel.append({"sourceName": "Close"})
    }

    function selectInput(index)
    {
        // open video file
        if(index === inputListModel.count-2){
            trackerView.fileDialog.open()
        }

        //close button
        if(index === inputListModel.count-1){}

        //set video device
        else{
            atc.setVideoDevice(inputListModel.get(index).sourceName, false)
//            trackerView.videoPlaybackControlls.visible = false
        }

        trackerView.inputSource.visible = false
    }

    ListModel {
        id: inputListModel
    }

    ListView {
        id: inputListView
        model: inputListModel
        anchors.fill: parent
        spacing: 10
        clip: true

        delegate: Item {
            id: sourceListDelegate
            width: inputListView.width
            height: inputListView.height * 0.15

            InputSourceListView {
                id: sourceListItem
                width: sourceListDelegate.width
                height: sourceListDelegate.height
            }
        }
    }
}
