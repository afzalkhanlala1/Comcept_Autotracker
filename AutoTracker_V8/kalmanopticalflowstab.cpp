#include "kalmanopticalflowstab.h"

using namespace std;
using namespace cv;
using namespace cv::cuda;

KalmanOpticalFlowStab::KalmanOpticalFlowStab(QObject *parent) : QObject{parent}
{

}

void KalmanOpticalFlowStab::init(int _id)
{
    id = _id;
#ifdef USE_GPU
    opFlow = cuda::SparsePyrLKOpticalFlow::create(Size(op_win_h, op_win_v), 3, 30, false);
    detector = cuda::createGoodFeaturesToTrackDetector(CV_8UC1, 500, 0.01, 4);
#else
    opFlow = cv::SparsePyrLKOpticalFlow::create(Size(op_win_h, op_win_v), 3, TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01), false);
#endif
    initialized = true;
}

void KalmanOpticalFlowStab::stabilize(cv::Mat &_prev_grey, cv::Mat &_curr_grey, bool useMask, Mat &mask)
{
    if(!initialized || _curr_grey.empty()) { return; }
    if(_curr_grey.size != _prev_grey.size) { _curr_grey.copyTo(_prev_grey); return; }

    vector<float> err;
    vector<Point2f> prev_corner2, cur_corner2;
    worldDisplacment = QVector2D();

#ifndef USE_GPU
    vector <uchar> status;
    vector <Point2f> prev_corner, cur_corner;
#endif

#ifdef USE_GPU
    GpuMat _prev_grey_cuda(_prev_grey);
    GpuMat _curr_grey_cuda(_curr_grey);
    GpuMat d_prevPts;
    GpuMat d_nextPts;
    GpuMat d_status;
    GpuMat maskGpu(mask);
#endif

    if(useMask)
    {
#ifdef USE_GPU
        detector->detect(_prev_grey_cuda, d_prevPts, maskGpu);
#else
        goodFeaturesToTrack(_prev_grey, prev_corner, 250, 0.02, 16, mask, 3, false, 0.04);
#endif
    }
    else
    {
#ifdef USE_GPU
        detector->detect(_prev_grey_cuda, d_prevPts);
#else
        goodFeaturesToTrack(_prev_grey, prev_corner, 250, 0.02, 16, noArray(), 3, false, 0.04);
#endif
    }

#ifdef USE_GPU
    if(d_prevPts.empty())
    {
        _curr_grey.copyTo(_prev_grey);
        return;
    }

    vector<Point2f> prev_corner(d_prevPts.cols);
    d_prevPts.download(prev_corner);
#endif

    if(prev_corner.empty())
    {
        //qDebug() << ++errCount << "IS: prev_corner empty";
        _curr_grey.copyTo(_prev_grey);
        setMaxAmplitude(win_h, win_v);
        return;
    }

#ifdef USE_GPU
    opFlow->calc(_prev_grey_cuda, _curr_grey_cuda, d_prevPts, d_nextPts, d_status);
    vector<Point2f> cur_corner(d_nextPts.cols);
    d_nextPts.download(cur_corner);

    vector<uchar> status(d_status.cols);
    d_status.download(status);
#else
    opFlow->calc(_prev_grey, _curr_grey, prev_corner, cur_corner, status);
#endif

    for(size_t i=0; i < status.size(); i++)
    {
        if(status[i])
        {
            prev_corner2.push_back(prev_corner[i]);
            cur_corner2.push_back(cur_corner[i]);
        }
    }

    if(prev_corner2.empty() || cur_corner2.empty())
    {
        //qDebug() << ++errCount << "IS: no matching corners";
        setMaxAmplitude(win_h, win_v);
        _curr_grey.copyTo(_prev_grey);
        return;
    }

    _curr_grey.copyTo(_prev_grey);

    //qDebug() << id << prev_corner2.size() << cur_corner2.size();

    // translation + rotation only
    Mat T = cv::estimateAffine2D(prev_corner2, cur_corner2);

    // in rare cases no transform is found. We'll just use the last known good transform.
    if(T.data == NULL){ last_T.copyTo(T); }
    else { T.copyTo(last_T); }

    if(T.empty()) { return; }

    // decompose T
    double dx = T.at<double>(0,2);
    double dy = T.at<double>(1,2);
    double da = atan2(T.at<double>(1,0), T.at<double>(0,0));

    // Accumulated frame to frame transform
    x += dx;
    y += dy;
    a += da;
    z = Trajectory(x,y,a);

    if(k==1)
    {
        // intial guesses
        X = Trajectory(0,0,0); //Initial estimate,  set 0
        P = Trajectory(1,1,1); //set error variance,set
        k = 2;
    }
    else
    {
        //time update prediction
        X_ = X; //X_(k) = X(k-1);
        P_ = P+Q; //P_(k) = P(k-1)+Q;
        // measurement update correction
        K = P_/( P_+R ); //gain;K(k) = P_(k)/( P_(k)+R );
        X = X_+K*(z-X_); //z-X_ is residual,X(k) = X_(k)+K(k)*(z(k)-X_(k));
        P = (Trajectory(1,1,1)-K)*P_; //P(k) = (1-K(k))*P_(k);
    }

    // target - current
    double diff_x = constrain(X.x - x, -op_win_h, op_win_h);
    double diff_y = constrain(X.y - y, -op_win_v, op_win_v);
    double diff_a = constrain(X.a - a, -angle_limit, angle_limit);

//    double diff_x = X.x - x;
//    double diff_y = X.y - y;
//    double diff_a = X.a - a;

    last_x_offset = x_offset;
    last_y_offset = y_offset;

    _x_move = x_offset-diff_x;
    _y_move = y_offset-diff_y;

    worldDisplacment = QVector2D(_x_move - last_x_move, _y_move - last_y_move);

    last_x_move = _x_move;
    last_y_move = _y_move;

    if(!enabled) { return; }
    stab_win.moveTo(_x_move, _y_move);

    corr_x = stab_win.x() - win_center_x;
    corr_y = stab_win.y() - win_center_y;
    corr_a = diff_a - last_a_move;
    last_a_move = (last_a_move*0.99f) + (diff_a*0.01f);

    if( (stab_win.left() > imageSize.width()) || (stab_win.right() < 0) ) { x_offset -= (corr_x); }
    else if( (stab_win.left() < (win_center_x*0.3f)) || (stab_win.right() > (imageSize.width() - (win_center_x*0.3f))) ) { x_offset -= (corr_x*0.015f); }
    else { x_offset -= (corr_x*0.005f); }

    if( (stab_win.top() > imageSize.height()) || (stab_win.bottom() < 0) ) { y_offset -= corr_y; }
    else if( (stab_win.top() < (win_center_y*0.3f)) || (stab_win.bottom() > (imageSize.height() - (win_center_y*0.3f))) ) { y_offset -= (corr_y*0.015f); }
    else { y_offset -= (corr_y*0.005f); }


    //qDebug() << x_offset << "   " << y_offset << "  |  " << corr_x << "    " << corr_y << "    |   " << stab_win.x() << "    " << win_center_x;
}

void KalmanOpticalFlowStab::setMaxAmplitude(float _win_h, float _win_v)
{
    //qDebug() << ++errCount << "Setting Max Amp";

    if(!last_T.empty())
    {
        last_T.at<float>(0, 2) = 0;
        last_T.at<float>(1, 2) = 0;
    }

    win_h = _win_h;
    win_v = _win_v;
    float w = imageSize.width() - (2*win_h);
    float h = imageSize.height() - (2*win_v);
    win_center_x = (imageSize.width() - w)*0.5f;
    win_center_y = (imageSize.height() - h)*0.5f;
    stab_win = QRectF(win_center_x, win_center_y, w, h);
    centerRect = stab_win;

    x_offset = win_center_x;
    y_offset = win_center_y;

    //x = y = a = 0.0f;
    k = 1;
}
