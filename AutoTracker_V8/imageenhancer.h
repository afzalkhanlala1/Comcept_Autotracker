#ifndef IMAGEENHANCER_H
#define IMAGEENHANCER_H

#include "app_includes.h"
#include "colortransfer.h"

class ImageEnhancer : public QObject
{
    Q_OBJECT
public:
    explicit ImageEnhancer(QObject *parent = nullptr);
    ~ImageEnhancer();
    ColorTransfer *colorTransfer = nullptr;
    QThread colorTransferThread;
    TrackerFrame trackerFrame;
    QElapsedTimer pTimer;
    cv::Mat f_lutFrame;

    cv::Mat inFrame;
    cv::Mat lutFrame;

    bool initialized = false;

    bool enableAdaptiveThresh = false;
    bool enableEmboss = false;
    bool enableSharpen = false;
    bool enableEdge = false;
    bool applyPreNR = false;
    bool applyPostNR = false;
    bool applyLuts = true;
    bool applyCLAHE = false;
    float noiseH = 6;
    int noiseSearchWindow = 11;
    int noiseBlockSize = 11;

    double lut_lpf = 0.95f;
    int medianKSize = 3;

    float effectStrength = 1.0f;
    float lutStrength = 1.0f;

    double pTime = 0.0f;
    bool updatingCLAHE = false;
    float lut_clahe_factor = 0.75f;

#ifdef USE_GPU
    cv::Ptr<cv::cuda::Convolution> convolver;
    cv::Ptr<cv::cuda::LookUpTable> lut;
    cv::Ptr<cv::cuda::CLAHE> clahe;
#else
    cv::Ptr<cv::CLAHE> clahe;
#endif
    cv::Mat sharpen_kernel;
    cv::Mat emboss_kernel;
    cv::Mat edge_kernel;
    cv::Mat blur_kernel;

    int lut_channelCount = 3;

    QVector<float> lutArr_f;
    QVector<float> currLutArr_f;
    QVector<float> oriLutArr;
    cv::Mat lutArr = cv::Mat(1, 256, CV_8UC1);
    cv::Mat currLutArr = cv::Mat(1, 256, CV_8UC1);

    QVector<QVector<float>> rgblutArr_f;
    QVector<QVector<float>> rgbcurrLutArr_f;
    QVector<QVector<float>> rgboriLutArr;
    cv::Mat rgblutArr = cv::Mat(3, 256, CV_8UC1);
    cv::Mat rgbcurrLutArr = cv::Mat(3, 256, CV_8UC1);

public slots:
    void init();
    void processImage(TrackerFrame trackerFrame, double frame_dt);
//    void processImage(QImage image, double frame_dt);
    void rgblutUpdated(QVector<QVector<int>> rgbnewLut);
    void lutUpdated(QVector<int> lut);
    void updateCLAHE(int gridRows, int gridCols, double clipLimit);

signals:
    void initColorTransfer();
    void processLut(QImage frame);
    void frameReady(TrackerFrame trackerFrame);

private:
    void initLUT();
    void initrgbLUT();
    inline void resizeAndConvImage(QImage &frameImg);
    inline void imagergbApplyLut(cv::Mat &inFrame, cv::Mat &lutFrame);
    inline void imageApplyLut(cv::Mat &inFrame, cv::Mat &lutFrame);
    inline void imageApplyCLAHE(cv::Mat &inFrame, cv::Mat &lutFrame);
    inline void applyAdaptiveThreshold(cv::Mat &lutFrame);
    inline void removeNoiseGPU(cv::Mat &inFrame, float _noiseH = 8, int _noiseSearchWindow = 11, int _noiseBlockSize = 7);
    inline void applyConvolutionFilters(cv::Mat &inFrame);
    inline void blendWithOriginalImage();
};

#endif // IMAGEENHANCER_H
