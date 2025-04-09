#include "imagestabilization.h"

using namespace std;
using namespace cv;

ImageStabilization::ImageStabilization(QObject *parent) : QObject{parent}
{
    imuData = QVector<float>(IMUDATA::_imuDataCount, 0.0f);
    sumImuData = QVector<float>(IMUDATA::_imuDataCount, 0.0f);
    avgImuData = QVector<float>(IMUDATA::_imuDataCount, 0.0f);
    imuDataCount = 0.0f;
}

void ImageStabilization::init()
{
    qDebug() << "Image-Stabilization - INIT: " << QThread::currentThreadId();
    kalman_opStab = new KalmanOpticalFlowStab;
    kalman_opStab->init(0);
    kalman_opStab_2Pass = new KalmanOpticalFlowStab;
    kalman_opStab_2Pass->init(1);

    imuStab = new IMUStab;
    imuStab->init();

#ifdef USE_GPU
    backSub = cv::cuda::createBackgroundSubtractorMOG2(250, 20, false);
#else
    backSub = cv::createBackgroundSubtractorMOG2(250, 20, false);
#endif
    setMaxAmplitude(win_h, win_v);
    pTimer = new QElapsedTimer;

    pTimer->start();
    initialized = true;
    setEnabled(false);
    emit initCompleted();
}

void ImageStabilization::imuIncomingData(QVector<float> _imuData)
{
    for(int i = 0; i < _imuData.size(); i++){ sumImuData[i] += _imuData[i]; }
    imuDataCount++;
}

void ImageStabilization::stabilizeFrame(TrackerFrame trackerFrame)
{
    pTimer->start();
    bool sizeChanged = false;

    if(trackerFrame.useLutFrame && !trackerFrame.lutFrame.isNull()) {
        if(trackerFrame.lutFrame.size() != imageSize.toSize()) { imageSize = trackerFrame.lutFrame.size(); sizeChanged = true; }
    }
    else if(!trackerFrame.frame.isNull()){
        if(trackerFrame.frame.size() != imageSize.toSize()) { imageSize = trackerFrame.frame.size(); sizeChanged = true; }
    }

    else { return; }

    if(firstFrame || sizeChanged)
    {
        qDebug() << "IS - First Frame || sizeChanged" << endl;
        setMaxAmplitude(win_h, win_v);
        emit frameStabilized(trackerFrame);
        firstFrame = false;
        return;
    }

    if(enabled)
    {
        if(trackerFrame.useLutFrame) { cur_grey = cv::Mat(trackerFrame.lutFrame.height(), trackerFrame.lutFrame.width(), CV_8UC1, (void*)trackerFrame.lutFrame.constBits(), trackerFrame.lutFrame.bytesPerLine()); }
        else { cur_grey = cv::Mat(trackerFrame.frame.height(), trackerFrame.frame.width(), CV_8UC1, (void*)trackerFrame.frame.constBits(), trackerFrame.frame.bytesPerLine()); }

        cv::Mat imuRotatedImage;
        if(useIMU)
        {
            if(imuDataCount > 0)
            {
                for(int i = 0; i < sumImuData.size(); i++)
                {
                    avgImuData[i] = sumImuData[i] / imuDataCount;
                    sumImuData[i] = 0.0f;
                }
                imuDataCount = 0.0f;
            }
            imuStab->stabilize(avgImuData, trackerFrame.dt);
            imuCurrStabWin = imuStab->stab_win;
            imu_corr_angle = imuStab->win_offset.z();
            imuMotion = QVector3D(imuStab->dx, -imuStab->dy, imuStab->da);

            useMask = false;
            if(trackerFrame.useLutFrame) { imuImage = trackerFrame.lutFrame.copy(imuCurrStabWin.toRect()); }
            else { imuImage = trackerFrame.frame.copy(imuCurrStabWin.toRect()); }

            imuRotatedImage = cv::Mat(imuImage.height(), imuImage.width(), CV_8UC1, (void*)imuImage.constBits(), imuImage.bytesPerLine());
            cv::Point2f center(imuCurrStabWin.center().x(), imuCurrStabWin.center().y());
            cv::Mat rotation_matix = getRotationMatrix2D(center, -imu_corr_angle, 1.0);

            cv::warpAffine(imuRotatedImage, imuRotatedImage, rotation_matix, imuRotatedImage.size());
            currStabWin = imuCurrStabWin;
            corr_angle = imu_corr_angle;
        }

        else
        {
            imuMotion = QVector3D(0 ,0, 0);
            imu_corr_angle = 0.0f;
            imuCurrStabWin = trackerFrame.frame.rect();
        }

        if(useOpticalFlow)
        {
            if(useMask) { createMask(); }
            if(useIMU) { kalman_opStab->stabilize(prev_grey, imuRotatedImage, useMask, mask); }
            else { kalman_opStab->stabilize(prev_grey, cur_grey, useMask, mask); }

            x_offset = kalman_opStab->x_offset;
            y_offset = kalman_opStab->y_offset;
            opCurrStabWin = kalman_opStab->stab_win;
            op_corr_angle = kalman_opStab->corr_a*90.0f;
            trackerFrame.worldDisplacement = kalman_opStab->worldDisplacment;

            if(use2Passes)
            {
                QRectF cr = kalman_opStab->stab_win;
                QImage pass1Img;

                if(trackerFrame.useLutFrame) { pass1Img = trackerFrame.lutFrame.copy(cr.toRect()); }
                else { pass1Img = trackerFrame.frame.copy(cr.toRect()); }

                cur_grey_2 = cv::Mat(pass1Img.height(), pass1Img.width(), CV_8UC1, (void*)pass1Img.constBits(), pass1Img.bytesPerLine());
                kalman_opStab_2Pass->stabilize(prev_grey_2, cur_grey_2, false, mask);

                x_offset += kalman_opStab_2Pass->x_offset;
                y_offset += kalman_opStab_2Pass->y_offset;
                opCurrStabWin.translate(kalman_opStab_2Pass->worldDisplacment.toPointF());
                op_corr_angle += kalman_opStab_2Pass->corr_a;
                trackerFrame.worldDisplacement += kalman_opStab_2Pass->worldDisplacment;
            }

            op_corr_angle = 0.0f;
            opMotion = QVector3D(softDeadband(opCurrStabWin.x()-lastOPCurrStabWin.x(), 0.3f, 0.2f),
                                 softDeadband(opCurrStabWin.y()-lastOPCurrStabWin.y(), 0.3f, 0.2f),
                                 op_corr_angle - corr_angle);

            lastOPCurrStabWin = opCurrStabWin;

            currStabWin = opCurrStabWin;

            if(useIMU) { corr_angle += op_corr_angle; }
            else { corr_angle = op_corr_angle; }
        }

        else
        {
            opMotion = QVector3D(0,0,0);
            op_corr_angle = 0.0f;
            opCurrStabWin = trackerFrame.frame.rect();
        }

        stabWin.translate(imuMotion.toPointF()+opMotion.toPointF());
        float cen_corr_x = 0.0f, cen_corr_y = 0.0f;

        if(stabWin.left() < 0) { cen_corr_x = -stabWin.left(); }
        else if (stabWin.right() > imageSize.width()) { cen_corr_x = imageSize.width() - stabWin.right(); }

        if(stabWin.top() < 0) { cen_corr_y = -stabWin.top(); }
        else if (stabWin.bottom() > imageSize.height()) { cen_corr_y = imageSize.height() - stabWin.bottom(); }

        stabWin.translate(cen_corr_x, cen_corr_y);

        cent_err = anchorRect.topLeft() - stabWin.topLeft();
        if( (fabs(cent_err.x()) > 1.0f) || (fabs(cent_err.y()) > 1.0f)) { stabWin.translate(cent_err*0.01f); }

        cv::Point2f center(stabWin.center().x(), stabWin.center().y());
        cv::Mat rotation_matix = getRotationMatrix2D(center, -corr_angle, 1.0);
        cv::Mat rotated_image;
        cv::warpAffine(cur_grey, rotated_image, rotation_matix, cur_grey.size());
        stabImage = QImage(rotated_image.data, rotated_image.cols, rotated_image.rows, static_cast<int>(rotated_image.step), QImage::Format_Grayscale8).copy(stabWin.toRect());

        trackerFrame.stab_win = stabWin;

        //qDebug() << stabWin;

        //stabImage = QImage(cur_grey.data, cur_grey.cols, cur_grey.rows, static_cast<int>(cur_grey.step), QImage::Format_Grayscale8).copy(trackerFrame.stab_win.toRect());
        updateFrameSampling(trackerFrame, stabImage);
    }

    else
    {
        trackerFrame.stab_win = trackerFrame.frame.rect();
        trackerFrame.stab_win_valid = true;
        trackerFrame.win_jump = QPointF();

        updateFrameSampling(trackerFrame, trackerFrame.lutFrame);

        kalman_opStab->x = kalman_opStab->y = kalman_opStab->a = 0.0f;
        kalman_opStab->setMaxAmplitude(win_h, win_v);

        kalman_opStab_2Pass->setMaxAmplitude(win_h, win_v);
        kalman_opStab_2Pass->x = kalman_opStab_2Pass->y = kalman_opStab_2Pass->a = 0.0f;
    }

    //if(doFgSeperation) { subtractBackgound(trackerFrame); }

    trackerFrame.stabImage = trackerFrame.stabImage.scaled(trackerFrame.frame.width(), trackerFrame.frame.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    //    qDebug() << trackerFrame.frame.width() <<  trackerFrame.frame.height() ;
    emit frameStabilized(trackerFrame);
    pTime = (pTime*0.9f) + (((double)pTimer->nsecsElapsed() / 1000000.0f)*0.1f);
}

//=====================================================

void ImageStabilization::setMaxAmplitude(float _win_h, float _win_v)
{
    //qDebug() << ++errCount << "Setting Max Amp";

    win_h = _win_h;
    win_v = _win_v;
    float w = imageSize.width() - (2*win_h);
    float h = imageSize.height() - (2*win_v);
    win_center_x = (imageSize.width() - w)*0.5f;
    win_center_y = (imageSize.height() - h)*0.5f;

    anchorRect = stabWin = QRectF(win_h, win_v, w, h);

    imuStab->imageSize = imageSize;
    imuStab->setMaxAmplitude(win_h, win_v);

    if(useIMU)
    {
        kalman_opStab->imageSize = QSizeF(w, h);
        kalman_opStab->setMaxAmplitude(win_h, win_v);

        kalman_opStab_2Pass->imageSize = QSizeF(w - (2*win_h), h - (2*win_v));
        kalman_opStab_2Pass->setMaxAmplitude(win_h, win_v);
    }

    else
    {
        kalman_opStab->imageSize = imageSize;
        kalman_opStab->setMaxAmplitude(win_h, win_v);

        kalman_opStab_2Pass->imageSize = QSizeF(w, h);
        kalman_opStab_2Pass->setMaxAmplitude(win_h, win_v);
    }
}

void ImageStabilization::centerWindow()
{
    qDebug() << "CenterWindow" << endl;
    setMaxAmplitude(win_h, win_v);
}

void ImageStabilization::setTargetRegions(QVector<QRectF> _targetRects, int _activeTarget)
{
    if(_activeTarget < 0) { return; }
    activeTarget = _activeTarget;
    targetRects = _targetRects;
}

void ImageStabilization::setEnabled(bool val)
{
    enabled = val;
    if(kalman_opStab != nullptr) { kalman_opStab->enabled = val; }
    if(kalman_opStab_2Pass != nullptr) { kalman_opStab_2Pass->enabled = (val && use2Passes); }
}

void ImageStabilization::createMask()
{
    if(marginMask.empty() || marginMask.rows != imageSize.height() || marginMask.cols != imageSize.width())
    {
        marginMask = cv::Mat(imageSize.height(), imageSize.width(), CV_8UC1);
        marginMask = cv::Scalar(255);

        if(maskVMargin > 0)
        {
            Rect topMargin = Rect(0, 0, imageSize.width(), imageSize.height()*maskVMargin);
            Rect bottomMargin = Rect(0, imageSize.height() - (imageSize.height()*(maskVMargin*2.0f)), imageSize.width(), imageSize.height()*(maskVMargin*2.0f));
            marginMask(topMargin).setTo(cv::Scalar(0));
            marginMask(bottomMargin).setTo(cv::Scalar(0));
        }

        if(maskHMargin > 0)
        {
            Rect leftMargin = Rect(0, 0, imageSize.width()*maskHMargin, imageSize.height());
            Rect rightMargin = Rect(imageSize.width() - imageSize.width()*maskHMargin, 0, imageSize.width()*maskHMargin, imageSize.height());
            marginMask(leftMargin).setTo(cv::Scalar(0));
            marginMask(rightMargin).setTo(cv::Scalar(0));
        }
    }

    //add masking for target regions

    for(int i = 0; i < targetRects.size(); i++)
    {
        Rect targetRegion = Rect(targetRects[i].x(), targetRects[i].y(), targetRects[i].width(), targetRects[i].height());
        marginMask(targetRegion).setTo(cv::Scalar(0));
    }

    marginMask.copyTo(mask);
}

void ImageStabilization::subtractBackgound(TrackerFrame &trackerFrame)
{
    if(trackerFrame.stabImage.isNull()) { return; }

    QImage stabImg = trackerFrame.stabImage;
    Mat inFrame = cv::Mat(stabImg.height(), stabImg.width(), CV_8UC1, (void*)stabImg.constBits(), stabImg.bytesPerLine());
    Mat fgFrame(inFrame.size(), inFrame.type());

#ifdef USE_GPU
    cuda::GpuMat inFrame_gpu(inFrame);
    cuda::GpuMat fgFrame_cuda(fgFrame);
    backSub->apply(inFrame_gpu, fgFrame_cuda);
    fgFrame_cuda.download(fgFrame);
#else
    backSub->apply(inFrame, fgFrame);
#endif
    trackerFrame.fgMask = QImage(fgFrame.data, fgFrame.cols, fgFrame.rows, static_cast<int>(fgFrame.step), QImage::Format_Grayscale8).copy();
}

void ImageStabilization::updateFrameSampling(TrackerFrame &trackerFrame, QImage &image)
{
    if(f_stabImage.isNull() || f_stabImage.size() != image.size()){
        f_stabImage = QImage(image.size(), QImage::Format_Grayscale8);
    }

    if(useFrameSampling)
    {
        QPainter painter(&f_stabImage);
        painter.setOpacity(frameSamplingRate);
        painter.drawImage(0, 0, image);
        trackerFrame.stabImage = f_stabImage;
        trackerFrame.useFrameSampling = true;
    }

    else
    {
        f_stabImage = image;
        trackerFrame.stabImage = f_stabImage;
        trackerFrame.useFrameSampling = false;
    }
}
