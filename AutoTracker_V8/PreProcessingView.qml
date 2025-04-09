import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Item {
    id: mainItem

    function loadSettings()
    {

    }

    Rectangle{
        id: checkBoxRect
        color: "#1d1d1e"
        anchors.fill: parent
        anchors.bottomMargin: parent.height*0.8
        anchors.leftMargin: parent.width*0.02
        anchors.rightMargin: parent.width*0.02

        GridLayout{
            id: checkboxGridLayout
            anchors.fill: parent
            anchors.leftMargin: parent.width*0.02
            anchors.rightMargin: parent.width*0.02
            columns: 4

            CheckBox{
                id: showOriImageSwitch
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 1
                text: "ShowOri"
                font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
                checked: false
                onCheckedChanged: {
                    atc.setShowOri(showOriImageSwitch.checked)
                }
            }

            CheckBox{
                id: applyLUTSwitch
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 1
                text: "LUT"
                font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
                checked: true
                onCheckedChanged: {
                    atc.setApplyLut(checked)
                }
            }

            CheckBox{
                id: applyCLAHE
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 1
                checked: false
                text: "CLAHE"
                font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
                onCheckedChanged: {
                    atc.enableCLAHE(applyCLAHE.checked)
                }
            }

            CheckBox{
                id: frameBlendingCheckBox
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 1
                text: "Avg"
                font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
                checked: false
                onCheckedChanged: {
                    atc.setEnableFrameBlending(checked)
                }
            }

            CheckBox{
                id: sharpenCheckBox
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 1
                text: "Sharpen"
                font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
                checked: false
                onCheckedChanged: {
                    atc.setEnableSharpen(checked)
                }
            }

            CheckBox{
                id: preNRCheckBox
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 1
                text: "PreNR"
                font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
                checked: false
                onCheckedChanged: {
                    atc.setPreNR(checked)
                }
            }

            CheckBox{
                id: postNRCheckBox
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: "PostNR"
                font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
                checked: false
                onCheckedChanged: {
                    atc.setPostNR(checked)
                }
            }
        }
    }

    GridLayout{
        id: inputImageGridLayout
        anchors.fill: parent
        anchors.topMargin: parent.height*0.205
        anchors.bottomMargin: parent.height*0.05
        anchors.leftMargin: parent.width*0.02
        anchors.rightMargin: parent.width*0.02
        columns: 4

        Label{
            id: frameSamplingLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter

            text: "Sampling: " + frameSamplingSlider.value.toFixed(3)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: frameSamplingSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 0
            to: 1.0
            value: 0.4
            stepSize: 0.001
            onPressedChanged: {
                if(!pressed)
                    atc.setFrameSamplingRate(value)
            }
        }

        Label{
            id: lutAdaptionLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter

            text: "LUT-Adapt: " + lutAdaptionSlider.value.toFixed(2)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: lutAdaptionSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 0
            to: 0.99
            value: 0.95
            stepSize: 0.01
            onPressedChanged: {
                if(!pressed)
                    atc.setLutAdaption(value)
            }
        }

        Label{
            id: lutStrengthLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: "LUT-Gain: " + lutStrengthSlider.value.toFixed(2)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: lutStrengthSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 0.0
            to: 1.0
            value: 1.0
            stepSize: 0.01
            onPressedChanged: {
                if(!pressed)
                    atc.setLutStrength(value)
            }
        }

        Label{
            id: noiseHueLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: "NoiseHue: " + noiseHueSlider.value.toFixed(0)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: noiseHueSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 1
            to: 30
            value: 6
            stepSize: 1
            onPressedChanged: {
                if(!pressed)
                    atc.setNoiseH(value)
            }
        }

        Label{
            id: noiseSearchWindowLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: "NoiseWin: " + noiseSearchWindowSlider.value.toFixed(0)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: noiseSearchWindowSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 3
            to: 31
            value: 11
            stepSize: 2
            onPressedChanged: {
                if(!pressed)
                    atc.setNoiseSearchWindow(value)
            }
        }

        Label{
            id: noiseBlockSizeLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: "NoiseBlock: " + noiseBlockSizeSlider.value.toFixed(0)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: noiseBlockSizeSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 3
            to: 31
            value: 11
            stepSize: 2
            onPressedChanged: {
                if(!pressed)
                    atc.setNoiseBlockSize(value)
            }
        }

        Label{
            id: claheClipLimitLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "CLAHE-Limit  " + claheClipLimitSlider.value.toFixed(0)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: claheClipLimitSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 1
            to: 128
            value: 24
            stepSize: 1.0
            onPressedChanged: {
                if(!pressed){
                    atc.setClaheParams( claheGridRowsSlider.value, claheGridColsSlider.value, claheClipLimitSlider.value)
                }
            }
        }

        Label{
            id: claheGridSizeLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "CLAHE-Size " + claheGridSizeSlider.value.toFixed(0)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: claheGridSizeSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 1
            to: 128
            value: 24
            stepSize: 1.0
            onPressedChanged: {
                if(!pressed){
                    atc.setClaheParams(claheGridSizeSlider.value, claheClipLimitSlider.value)
                }
            }
        }

        Label{
            id: lutClaheFactorLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "LUT-CLAHE " + lutClaheFactorSlider.value.toFixed(2)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: lutClaheFactorSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 0.0
            to: 1.0
            value: 0.75
            stepSize: 0.01
            onPressedChanged: {
                if(!pressed){
                    atc.setLutClaheFactor(lutClaheFactorSlider.value)
                }
            }
        }

        Label{
            id: effectStrengthLabel
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.25
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: "Enhance: " + effectStrengthSlider.value.toFixed(2)
            minimumPointSize: 8
            font.pointSize: mainItem.width*0.025 < 10 ? 10 : mainItem.width*0.025
            fontSizeMode: Text.Fit
        }

        Slider{
            id: effectStrengthSlider
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: inputImageGridLayout.columns*0.75
            from: 0.0
            to: 1.0
            value: 0.64
            stepSize: 0.01
            onPressedChanged: {
                if(!pressed)
                    atc.setPreProcessingEffectStrength(value)
            }
        }
    }
}
