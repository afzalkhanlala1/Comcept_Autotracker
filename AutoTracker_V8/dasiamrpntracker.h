#ifndef DASIAMRPNTRACKER_H
#define DASIAMRPNTRACKER_H

#include "app_includes.h"

/*


"{ help     h  |   | Print help message }"
            "{ input    i  |   | Full path to input video folder, the specific camera index. (empty for camera 0) }"
            "{ net         | dasiamrpn_model.onnx | Path to onnx model of net}"
            "{ kernel_cls1 | dasiamrpn_kernel_cls1.onnx | Path to onnx model of kernel_r1 }"
            "{ kernel_r1   | dasiamrpn_kernel_r1.onnx | Path to onnx model of kernel_cls1 }"
            "{ backend     | 0 | Choose one of computation backends: "
                                "0: automatically (by default), "
                                "1: Halide language (http://halide-lang.org/), "
                                "2: Intel's Deep Learning Inference Engine (https://software.intel.com/openvino-toolkit), "
                                "3: OpenCV implementation, "
                                "4: VKCOM, "
                                "5: CUDA },"
            "{ target      | 0 | Choose one of target computation devices: "
                                "0: CPU target (by default), "
                                "1: OpenCL, "
                                "2: OpenCL fp16 (half-float precision), "
                                "3: VPU, "
                                "4: Vulkan, "
                                "6: CUDA, "
                                "7: CUDA fp16 (half-float preprocess) }"


*/


class DaSiamRPNTracker : public QObject
{
    Q_OBJECT
public:
    explicit DaSiamRPNTracker(QObject *parent = nullptr);

    cv::Ptr<cv::TrackerDaSiamRPN> tracker;
    cv::TrackerDaSiamRPN::Params daSiamRPNParams;
    bool trackerCreated = false;
    bool initialized = false;
    bool roiSet = false;
    bool enabled = false;
    float DasRPN_score;
    bool trackerSuccessful = false;
    bool busy = false;
//    float score;

    QElapsedTimer pTimer;
    double processingTime = 0;
    float lp_gain = 0.8f;

    QRectF trackingRect;
    ObjectState objectState;

public slots:
    void init();
    void setROI(QRect rect);
    void track(TrackerFrame trackerFrame, double dt, QVector2D worldDisplacement);
    void setParam(QStringList params);

signals:
    void tracked(QRectF trackedRect, ObjectState objectState, bool success, int tracker_type);

private:
    bool isRoiInImageArea(QRect &rect);
    cv::Mat currMatFrame;
    cv::Rect trackedRect;

};

#endif // DASIAMRPNTRACKER_H
