QT += quick multimedia multimediawidgets serialport network gamepad charts datavisualization

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += USE_GPU
#DEFINES += DEPLOY_MODE
#DEFINES += HEADLESS_MODE
DEFINES += JETSON

SOURCES += \
        autolock.cpp \
        autotrackercontroller.cpp \
        colortransfer.cpp \
        coprocessor.cpp \
        dasiamrpntracker.cpp \
        datadevice.cpp \
        frametransmitter.cpp \
        imageenhancer.cpp \
        imagestabilization.cpp \
        imustab.cpp \
        kalmanopticalflowstab.cpp \
        main.cpp \
        objecttracker.cpp \
        opencvvideocapture.cpp \
        opencvvideodevice.cpp \
        pid.cpp \
        pingdevice.cpp \
        remotecontrol.cpp \
        systemstatsreport.cpp \
        templatematchtracker.cpp \
        yoloclienttracker.cpp


RESOURCES += qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    PacketData.h \
    app_connector.h \
    app_includes.h \
    autolock.h \
    autotracker_types.h \
    autotrackercontroller.h \
    colortransfer.h \
    config.h \
    coprocessor.h \
    dasiamrpntracker.h \
    dataFields.h \
    datadevice.h \
    frametransmitter.h \
    imageenhancer.h \
    imagestabilization.h \
    imustab.h \
    kalmanopticalflowstab.h \
    objecttracker.h \
    opencvvideocapture.h \
    opencvvideodevice.h \
    pid.h \
    pingdevice.h \
    remotecontrol.h \
    systemstatsreport.h \
    targetrenderer.h \
    templatematchtracker.h \
    utilityFunctions.h \
    videoframe_convertor.h \
    yoloclienttracker.h


#---------- FFmpeg -----------------

INCLUDEPATH += /usr/include/ffmpeg
LIBS += -lavformat -lavcodec -lavutil -lswscale

# LIBS += -L /usr/lib/x86_64-linux-gnu/ \
#         -lavformat \
#         -lavcodec \
#         -lavutil \
#         -lswscale

# INCLUDEPATH +=  /usr/include/x86_64-linux-gnu/
# DEPENDPATH +=  /usr/include/x86_64-linux-gnu/


#---------- OPENCV -----------------
LIBS += -L/usr/local/cuda/lib64
LIBS += -L/usr/local/cuda-12.6/targets/aarch64-linux/lib/stubs/

INCLUDEPATH += /usr/local/include/opencv4

LIBS += -L/usr/lib/aarch64-linux-gnu \

LIBS += -L/usr/local/cuda/lib64 \
        -L/usr/lib/aarch64-linux-gnu \
        -lcuda \
        -lcudart \
        -lnvinfer \
        -lnvonnxparser

LIBS += -L/usr/local/lib/ \
    -lopencv_core \
    -lopencv_dnn \
    -lopencv_dnn_objdetect \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_objdetect \
    -lopencv_optflow \
    -lopencv_photo \
    -lopencv_video \
    -lopencv_videoio \
    -lopencv_tracking \
    -lopencv_imgproc \
    -lopencv_dnn_superres \
    -lopencv_features2d \
    -lopencv_calib3d \
    -lopencv_bgsegm

INCLUDEPATH += /usr/include/aarch64-linux-gnu
LIBS += -L/usr/lib/aarch64-linux-gnu \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_videoio \
        -lopencv_imgproc \
        -lopencv_dnn \
        -lnvinfer \
        -lnvonnxparser \
        -lnvinfer_plugin

INCLUDEPATH += /usr/local/cuda/include
DEPENDPATH += /usr/local/cuda/include
INCLUDEPATH += /usr/local/include
contains(DEFINES,USE_GPU){
    LIBS += -L/usr/local/lib/ \
        -lopencv_cudafilters \
        -lopencv_cudaarithm \
        -lopencv_cudaimgproc \
        -lopencv_cudaobjdetect \
        -lopencv_cudaoptflow \
        -lopencv_cudafeatures2d \
        -lopencv_cudabgsegm

    !JETSON
    {
        LIBS += -L/usr/local/cuda/targets/x86_64-linux/lib/stubs/ -lnvidia-ml

        INCLUDEPATH += /usr/local/cuda/targets/x86_64-linux/lib/stubs
        DEPENDPATH += /usr/local/cuda/targets/x86_64-linux/lib/stubs

        INCLUDEPATH += /usr/local/cuda/targets/x86_64-linux/include
        DEPENDPATH += /usr/local/cuda/targets/x86_64-linux/include
    }


win32:CONFIG(release, debug|release): LIBS += -L/usr/local/cuda/targets/x86_64-linux/lib/release/ -lnvrtc
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/local/cuda/targets/x86_64-linux/lib/debug/ -lnvrtc
else:unix: LIBS += -L/usr/local/cuda/targets/x86_64-linux/lib/ -lnvrtc

INCLUDEPATH +=  /usr/local/cuda/targets/x86_64-linux/include
DEPENDPATH +=  /usr/local/cuda/targets/x86_64-linux/include
}

DISTFILES +=

win32:CONFIG(release, debug|release): LIBS += -L/usr/local/lib/release/ -lopencv_core
else:win32:CONFIG(debug, debug|release): LIBS += -L/usr/local/lib/debug/ -lopencv_core
else:unix: LIBS += -L/usr/local/lib/ -lopencv_core

INCLUDEPATH += /usr/local/cuda/include
DEPENDPATH += /usr/local/cuda/include
LIBS += -L/usr/local/cuda/lib64 -lnvidia-ml -lnvrtc




