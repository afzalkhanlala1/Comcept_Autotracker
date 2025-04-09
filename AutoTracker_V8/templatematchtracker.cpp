#include "templatematchtracker.h"

using namespace std;
using namespace cv;

bool sortErrs(QVector3D &lhs, QVector3D &rhs)
{
    return lhs.z() < rhs.z();
}

bool sortVel(QVector2D &lhs, QVector2D &rhs)
{
    return (lhs.length() < rhs.length());
}

TemplateMatchTracker::TemplateMatchTracker(QObject *parent) : QObject{parent}
{
    op_config.numLevels = 5;
    op_config.pyrScale = 0.5;
    op_config.fastPyramids = false;
    op_config.winSize = 13;
    op_config.numIters = 10;
    op_config.polyN = 5;
    op_config.polySigma = 1.1;
    op_config.flags = 0;
}

void TemplateMatchTracker::init()
{
#ifdef USE_GPU
    opticalFlow = cv::cuda::FarnebackOpticalFlow::create(op_config.numLevels, op_config.pyrScale, op_config.fastPyramids, op_config.winSize,
                                                         op_config.numIters, op_config.polyN, op_config.polySigma, op_config.flags);
#else
    opticalFlow = cv::FarnebackOpticalFlow::create(op_config.numLevels, op_config.pyrScale, op_config.fastPyramids, op_config.winSize,
                                                   op_config.numIters, op_config.polyN, op_config.polySigma, op_config.flags);
#endif
    trackerCreated = initialized = true;
}

void TemplateMatchTracker::setROI(QRect rect)
{
    if(rect.width() == -1 || rect.height() == -1) { cout << "Disabling Tracker" << endl; return; }

    if(!isRoiInImageArea(rect)) { /*qDebug() << "tm" <<"Roi is not valid" << endl;*/  return;}
    roiSet = false;
    prev_matches.clear();
    contourRect = opRect = tmRect = curr_tmRect = trackingRect = roi = rect;
    tejectory.clear();
    last_frameCount = frameCount = 0;
    world_y = world_x = 0.0f;

    tm_HWin = 36;
    tm_VWin = 24;
    //
    roiCenter = roi.center();
    center_region = QRectF(roiCenter.x() - r, roiCenter.y() - r, 2 * r, 2 * r);
    center_rect = curr_frame.copy(center_region.toRect());
    center_image = qImageToCV(center_rect);
    meanVal = cv::mean (center_image);
    c_mean = meanVal.val[0];
    if(c_mean < 127) { lb = c_mean * 1.25f; }
    else { lb = c_mean * 0.90f; }

    int hWin = 4, vWin = 4;;
    QRectF updated_temp = roi.adjusted(-hWin, -vWin, hWin, vWin);

    //    if(updated_temp.width() > 100) { updated_temp.setWidth(100); }
    //    if(updated_temp.height() > 100) { updated_temp.setHeight(100); }
    QImage expanded_temp_image = curr_frame.copy(updated_temp.toRect());
    Mat expanded_temp = qImageToCV(expanded_temp_image);

    cv::threshold(expanded_temp, templateEdges, lb, 255, cv::THRESH_BINARY);
    if(c_mean < 127) { cv::bitwise_not(templateEdges, templateEdges); }
    templateSeg = cvToQImage(templateEdges).copy();

    int min_x = templateSeg.width(), max_x = 0;
    int min_y = templateSeg.height(), max_y = 0;
    int validPixelFound = 0;
    for(int x = 0; x < templateSeg.width(); x++)
    {
        for(int y = 0; y < templateSeg.height(); y++)
        {
            int lightness = templateSeg.pixelColor(x,y).lightness();

            if (lightness > 10)
            {
                validPixelFound++;

                if (x < min_x){ min_x = x; }
                if (x > max_x){ max_x = x;}

                if (y < min_y){ min_y = y; }
                if (y > max_y){ max_y = y;}

            }
        }
    }

    if(validPixelFound < 50) {return; }

    QRectF bfr = QRectF(QPoint(min_x, min_y), QPoint(max_x+1 , max_y+1));
    if(bfr.height() < 20) { bfr.setHeight(20); }
    if(bfr.width() < 20) { bfr.setWidth(20); }

    QRectF curr_target_rect = QRectF(bfr.x() + updated_temp.x() + hWin, bfr.y() + updated_temp.y() + vWin, bfr.width(), bfr.height());

    //    bbox_copy = QRectF( (bbox_copy.x()*0.8f) + (curr_target_rect.x()*0.2f),
    //                       (bbox_copy.y()*0.8f) + (curr_target_rect.y()*0.2f),
    //                       (bbox_copy.width()*0.8f) + (curr_target_rect.width()*0.2f),
    //                       (bbox_copy.height()*0.8f) + (curr_target_rect.height()*0.2f));

    tempVal = cv::mean(expanded_temp, templateEdges);
    temp_avg = tempVal.val[0];
    //
    //ori_template = curr_frame.copy(rect.adjusted(-4, -4, 4, 4));
    ori_template = curr_frame.copy(rect);
    curr_template = ori_template;

    ori_template_mat = Mat(ori_template.height(), ori_template.width(), CV_8U, (void*)ori_template.constBits(), ori_template.bytesPerLine());
    ori_template_mat.copyTo(curr_template_mat);
    prev_OF_frame = cv::Mat();

    dtTimer.start();
    roiSet = true;
    //cout << "============================ " << "roiRect: " << trackingRect.x() << ", " << trackingRect.y() << " | " << trackingRect.width() << " | " << trackingRect.height() << endl;
}

void TemplateMatchTracker::updatePos(QRectF objectTrackedRect)
{
    QRectF rect2 = objectTrackedRect;
    roiCenter = objectTrackedRect.center();
    center_region = QRectF(roiCenter.x() - r, roiCenter.y() - r, 2 * r, 2 * r);
    center_rect = curr_frame.copy(center_region.toRect());
    center_image = qImageToCV(center_rect);
    meanVal = cv::mean (center_image);
    c_mean = meanVal.val[0];
    if(c_mean < 127) { lb = c_mean * 1.25f; }
    else { lb = c_mean * 0.90f; }

    int hWin = 4, vWin = 4;;
    QRectF updated_temp = roi.adjusted(-hWin, -vWin, hWin, vWin);

    //    if(updated_temp.width() > 100) { updated_temp.setWidth(100); }
    //    if(updated_temp.height() > 100) { updated_temp.setHeight(100); }
    QImage expanded_temp_image = curr_frame.copy(updated_temp.toRect());
    Mat expanded_temp = qImageToCV(expanded_temp_image);

    cv::threshold(expanded_temp, templateEdges, lb, 255, cv::THRESH_BINARY);
    if(c_mean < 127) { cv::bitwise_not(templateEdges, templateEdges); }
    templateSeg = cvToQImage(templateEdges).copy();

    int min_x = templateSeg.width(), max_x = 0;
    int min_y = templateSeg.height(), max_y = 0;
    int validPixelFound = 0;

    for(int x = 0; x < templateSeg.width(); x++)
    {
        for(int y = 0; y < templateSeg.height(); y++)
        {
            int lightness = templateSeg.pixelColor(x,y).lightness();

            if (lightness > 10)
            {
                validPixelFound++;

                if (x < min_x){ min_x = x; }
                if (x > max_x){ max_x = x;}

                if (y < min_y){ min_y = y; }
                if (y > max_y){ max_y = y;}

            }
        }
    }

    qDebug() << "UPDATE_POS_TM";
    if(validPixelFound < 50) { return; }

    QRectF bfr = QRectF(QPoint(min_x, min_y), QPoint(max_x+1 , max_y+1));
    if(bfr.height() < 20) { bfr.setHeight(20); }
    if(bfr.width() < 20) { bfr.setWidth(20); }

    QRectF curr_target_rect = QRectF(bfr.x() + updated_temp.x() + hWin, bfr.y() + updated_temp.y() + vWin, bfr.width(), bfr.height());

    //    bbox_copy = QRectF( (bbox_copy.x()*0.8f) + (curr_target_rect.x()*0.2f),
    //                       (bbox_copy.y()*0.8f) + (curr_target_rect.y()*0.2f),
    //                       (bbox_copy.width()*0.8f) + (curr_target_rect.width()*0.2f),
    //                       (bbox_copy.height()*0.8f) + (curr_target_rect.height()*0.2f));

    tempVal = cv::mean(expanded_temp, templateEdges);
    temp_avg = tempVal.val[0];

    tmConfidence = updateTM(curr_frame, rect2);
    last_trackingRect = rect2;

    float tm_gain = 0.6f;
    float op_gain = 1.0f - tm_gain;
    float ct_gain = 1.0f - tm_gain;

    //trackingRect = QRectF( (tmRect.topLeft()*tm_gain) + (opRect.topLeft()*op_gain), (tmRect.bottomRight()*tm_gain) + (opRect.bottomRight()*op_gain) );
    //trackingRect = QRectF((tmRect.topLeft()*tm_gain) + (contourRect.topLeft()*ct_gain), (tmRect.bottomRight()*tm_gain) + (contourRect.bottomRight()*ct_gain));
    rect2 = tmRect;

    if(rect2.x() < 0) { rect2.translate(-rect2.x(), 0); }
    else if(rect2.right() > curr_frame.width() ) { rect2.translate(curr_frame.width() - rect2.right(), 0); }

    if(rect2.y() < 0) { rect2.translate(0, -rect2.y()); }
    else if(rect2.bottom() > curr_frame.height()) {rect2.translate(0, curr_frame.height() -rect2.bottom()); }

    //trackingRect = opRect;
    trackingRectDisp = QVector2D(rect2.center() - last_trackingRect.center());
    trackerSuccessful = (confidence >= match_thres);
    //    qDebug() << "TM2";
//    qDebug() << "conf" << confidence << "match_thresh" << match_thres;

    emit tracked(rect2, objectState, trackerSuccessful, TRACKER_TYPE::TEMPLATE_MATCHER);
}

void TemplateMatchTracker::track(QImage frame, double dt, QVector2D _worldDisp)
{
    if(busy || frame.isNull()) { return; }

    busy = true;
    pTimer.start();
    curr_frame = frame;
    if(curr_frame.format() != QImage::Format_Grayscale8) { curr_frame.convertTo(QImage::Format_Grayscale8); }
    if(curr_frame.isNull() || !trackerCreated || !enabled || !initialized || !roiSet) { busy = false; return;}
    frame_dt = dt;
    frameCount++;
    velocityThresh = velocityThreshPixel / frame_dt;
    accelThresh = accelThreshPixel / 0.04f;
    worldFrameVel = (worldFrameVel*0.6f) + (((_worldDisp - worldFrameDisp) / frame_dt)*0.4f);
    worldFrameDisp = _worldDisp;
    world_x += worldFrameDisp.x();
    world_y += worldFrameDisp.y();

    //findTargetContours(curr_frame, trackingRect);
    tmConfidence = updateTM(curr_frame, trackingRect);
    //opSuccess = updateOP(curr_frame, trackingRect);
    last_trackingRect = trackingRect;

    float tm_gain = 0.6f;
    float op_gain = 1.0f - tm_gain;
    float ct_gain = 1.0f - tm_gain;

    //trackingRect = QRectF( (tmRect.topLeft()*tm_gain) + (opRect.topLeft()*op_gain), (tmRect.bottomRight()*tm_gain) + (opRect.bottomRight()*op_gain) );
    //trackingRect = QRectF((tmRect.topLeft()*tm_gain) + (contourRect.topLeft()*ct_gain), (tmRect.bottomRight()*tm_gain) + (contourRect.bottomRight()*ct_gain));
    trackingRect = tmRect;
    if(trackingRect.x() < 0) { trackingRect.translate(-trackingRect.x(), 0); }
    else if(trackingRect.right() > frame.width() ) { trackingRect.translate(frame.width() - trackingRect.right(), 0); }

    if(trackingRect.y() < 0) { trackingRect.translate(0, -trackingRect.y()); }
    else if(trackingRect.bottom() > frame.height()) { trackingRect.translate(0, frame.height() - trackingRect.bottom()); }

    //trackingRect = opRect;
    trackingRectDisp = QVector2D(trackingRect.center() - last_trackingRect.center());
    trackerSuccessful = (confidence >= match_thres);
    processingTime = (processingTime*0.9f) + ( ((double)pTimer.nsecsElapsed()/1000000.0f )*0.1f);

//    qDebug() << "conf" << confidence << "match_thresh" << match_thres;

    emit tracked(trackingRect, objectState, trackerSuccessful, TRACKER_TYPE::TEMPLATE_MATCHER);
    //qDebug() << "TM: " << processingTime;
    busy = false;
}


float TemplateMatchTracker::updateTM(QImage &frame, QRectF &bbox)
{
    QRectF bbox_copy = bbox;
    QRectF bbox_copy2 = bbox;
    QRectF searchWindow = bbox.adjusted(-tm_HWin, -tm_VWin, tm_HWin, tm_VWin);
    QImage searchImage = frame.copy(searchWindow.toRect());
    Mat inFrame = Mat(searchImage.height(), searchImage.width(), CV_8U, (void*)searchImage.constBits(), searchImage.bytesPerLine());
    Point matchLoc, maxMatchLoc;
    //    float confidence = 0.0f, maxConfidence = 0.0f;
    int prev_match_i = -1;

    confidence = templateMatching(inFrame, curr_template_mat, matchLoc);

    //    for(int t = 0; t < tejectory.size(); t++){
    //        tejectory[t].pos.translate(worldFrameDisp.toPointF());
    //    }

    bool matchFound = false;

    //ADAPTIVE THRESHOLD
    //Window Average
    cv::Mat mask = cv::Mat(inFrame.size(), CV_8UC1);
    Point m_pt1 = qPointToCVPoint(bbox_copy2.topLeft() - searchWindow.topLeft());
    Point m_pt2 = qPointToCVPoint(bbox_copy2.bottomRight() - searchWindow.topLeft());

    cv::rectangle(mask, m_pt1, m_pt2, cv::Scalar(0), cv::FILLED);

    WindowVal = cv::mean(inFrame, mask);
    window_avg = WindowVal.val[0];
    if(temp_avg == 0) { return match_thres; }

    //Relative Contrast
    if (window_avg > temp_avg) { rel_cont = (temp_avg / window_avg); }
    else { rel_cont = (window_avg / temp_avg); }

    curr_at = 0.65f + ((rel_cont + 0.1f)*0.25f);

    if(curr_at < 0.65f) { curr_at = 0.65f;}
    else if(curr_at > 0.90f) { curr_at = 0.90f;}
    //    at = (at * 0.9f) + (curr_at * 0.1f);
    match_thres = (match_thres * 0.9f) + (curr_at * 0.1f);

    //    match_thres = (confidence > at);
    //

    if(confidence >= match_thres)
    {
        failed = 0;
        updateOCVMotion(frame, bbox, matchLoc, true);
        tm_HWin = 36.0f;
        tm_VWin = 24.0f;
        matchFound = true;
    }

    else
    {
        failed++;
        cv::Mat curr_tmpl;
        for(int i = 0; i < prev_matches.size(); i++)
        {
            if(prev_matches[i].isNull()) { continue; }

            curr_tmpl = Mat(prev_matches[i].height(), prev_matches[i].width(), CV_8U, (void*)prev_matches[i].constBits(), prev_matches[i].bytesPerLine());
            confidence = templateMatching(inFrame, curr_tmpl, matchLoc);
            if( (confidence >= match_thres) && (confidence > maxConfidence))
            {
                prev_match_i = i;
                maxMatchLoc = matchLoc;
                maxConfidence = confidence;
            }
        }

        if(prev_match_i != -1)
        {
            updateOCVMotion(frame, bbox, maxMatchLoc, false);
            confidence = maxConfidence;
            curr_template = prev_matches[prev_match_i];
            matchFound = true;
        }
    }

    //    if(!matchFound)
    //    {
    //        if(failed == 0 && tejectory.size() > 2)
    //        {
    //            QVector2D sum_vel(0, 0);
    //            QVector2D sum_accel(0, 0);
    //            QVector2D c_vel, l_vel;
    //            double vel_count = 0.0f, accel_count = 0.0f;
    //            double _dt = 0.0f;

    //            for(int i = 1; i < tejectory.size(); i++)
    //            {
    //                _dt = (tejectory[i].time - tejectory[i-1].time);
    //                c_vel = QVector2D((tejectory[i].pos.center() - tejectory[i-1].pos.center()) / _dt);
    //                //if(abs(c_vel.length()) < 0.2f) { continue; }

    //                sum_vel += c_vel;

    //                if(i >= 2) { sum_accel += QVector2D((c_vel - l_vel) / _dt); accel_count++; }

    //                l_vel = c_vel;
    //                vel_count++;
    //            }

    //            tm_vel = (sum_vel / vel_count);
    //            if(accel_count > 1.0f) { tm_accel = sum_accel / accel_count; }

    //            //tm_vel = tm_accel = QVector2D();
    //        }

    //        else
    //        {
    //            //            if(tm_HWin < 48) { tm_HWin *= 1.05f; }
    //            //            if(tm_VWin < 48) { tm_VWin *= 1.02f; }

    //            if(failed > 100)
    //            {
    //                tm_vel *= 0.99f; if(abs(tm_vel.length()) < 0.05f) { tm_vel = QVector2D(); }
    //                tm_accel *= 0.98f; if(abs(tm_accel.length()) < 0.01f) { tm_accel = QVector2D(); }
    //            }
    //        }

    //        failed++;
    //    }

    //    displacement = (displacement*0.2f) + (((tm_vel*frame_dt) + (tm_accel*frame_dt*frame_dt*0.5f))*0.8f);
    displacement = (tm_vel*frame_dt) + (tm_accel*frame_dt*frame_dt*0.5f);
    tmRect.translate(displacement.toPointF());

    tmRect = curr_tmRect;

    if(tmRect.x() < 0) { tmRect.translate(-tmRect.x(), 0); }
    else if(tmRect.right() > frame.width() ) { tmRect.translate(frame.width() - tmRect.right(), 0); }

    if(tmRect.y() < 0) { tmRect.translate(0, -tmRect.y()); }
    else if(tmRect.bottom() > frame.height()) { tmRect.translate(0, frame.height() - tmRect.bottom()); }

    if(matchFound && (prev_match_i == -1) && (failed == 0) )
    {
        curr_template = frame.copy(tmRect.toRect());
        //curr_template_mat = Mat(curr_template.height(), curr_template.width(), CV_8U, (void*)curr_template.constBits(), curr_template.bytesPerLine()).clone();
    }

    //qDebug() << failed << confidence << " | " << processingTime << "    |   " << tmRect << "     |    " << displacement;
    return confidence;
}

void TemplateMatchTracker::updateOCVMotion(QImage &frame, QRectF &bbox, cv::Point &matchLoc, bool updatePrev)
{
    curr_tmRect = QRectF(matchLoc.x, matchLoc.y, ori_template.width(), ori_template.height());
    curr_tmRect.translate(bbox.x()-tm_HWin, bbox.y()-tm_VWin);
    tm_curr_vel = QVector2D(curr_tmRect.center() - tmRect.center()) / frame_dt;
    tm_curr_vel += worldFrameVel;

    if(abs(tm_curr_vel.length()) > velocityThresh) { tm_curr_vel *= velocityThresh / abs(tm_curr_vel.length()); }
    tm_curr_accel = ( tm_curr_vel - tm_vel ) / frame_dt;
    if(abs(tm_curr_accel.length()) > accelThresh) { tm_curr_accel *= accelThresh / abs(tm_curr_accel.length()); }
    tm_vel = (tm_vel*0.5f) + (tm_curr_vel*0.5f);
    tm_accel = (tm_accel*0.5f) + (tm_curr_accel*0.5f);

    if( /*confidence >= constrain(match_thres*1.05f, 0.0f, 0.99f) ||*/ ((frameCount - last_frameCount) >= tejectoryKeyFrame) && updatePrev )
    {
        TrackHistroy hist;
        hist.pos = tmRect;
        hist.time = (double)dtTimer.nsecsElapsed()/1000000000.0f;
        hist.worldDisplacement = QVector2D(world_x, world_y);

        tejectory.append(hist);
        if(tejectory.size() > maxTejectoryHistory) { tejectory.takeFirst(); }

        if(failed == 0) { prev_matches.prepend(frame.copy(bbox.toRect())); }
        if(prev_matches.size() > max_pre_matches) { prev_matches.removeLast(); }

        last_frameCount = frameCount;
    }
}

bool TemplateMatchTracker::updateOP(QImage &_frame, QRectF &bbox)
{
    QRectF searchWindow = bbox.adjusted(-op_HWin, -op_VWin, op_HWin, op_VWin);
    QImage frame = _frame.copy(searchWindow.toRect());

    if(blurRadius > 0)
    {
        cv::Mat inFrame = cv::Mat(frame.height(), frame.width(), CV_8U, (void*)frame.bits(), frame.bytesPerLine());
        cv::Mat outFrame(frame.height(), frame.width(), CV_8U);
        cv::blur(inFrame, outFrame, cv::Size(blurRadius, blurRadius));
        frame = QImage(outFrame.data, outFrame.cols, outFrame.rows, static_cast<int>(outFrame.step), QImage::Format_Grayscale8).copy();
    }

    curr_OF_frame = cv::Mat(frame.height(), frame.width(), CV_8U, (void*)frame.bits(), frame.bytesPerLine());

    if(prev_OF_frame.empty() || (prev_OF_frame.size != curr_OF_frame.size) )
    {
        prev_OF_frame = curr_OF_frame.clone();
        busy = false;
        return false;
    }
#ifdef USE_GPU
    cuda::GpuMat d_frameL(prev_OF_frame), d_frameR(curr_OF_frame);
    opticalFlow->calc(d_frameL, d_frameR, d_flow);
    cuda::split(d_flow, planes);
    planes[0].download(flowx);
    planes[1].download(flowy);
#else
    opticalFlow->calc(prev_OF_frame, curr_OF_frame, d_flow);
    cv::split(d_flow, planes);
    planes[0].copyTo(flowx);
    planes[1].copyTo(flowy);

#endif

    calVelocityGrid(flowx, flowy);
    filterVelocityGrid();
    drawVelocityGrid(flowImage, flowx.cols, flowx.rows);
    calcBlobPos(_frame, flowImage, bbox);

    curr_OF_frame.copyTo(prev_OF_frame);
    return opRectValid;
}

void TemplateMatchTracker::calVelocityGrid(const cv::Mat &u, const cv::Mat &v)
{
    int gridRows = u.rows / gridBlockSize;
    int gridCols = u.cols / gridBlockSize;

    if(velocityGrid.size() != gridRows)
    {
        velocityGrid = QVector<QVector<QVector2D>>(gridRows);
        for(int i = 0; i < velocityGrid.size(); i++)
            velocityGrid[i] = QVector<QVector2D>(gridCols);

        f_velocityGrid = QVector<QVector<QVector2D>>(gridRows);
        for(int i = 0; i < f_velocityGrid.size(); i++)
            f_velocityGrid[i] = QVector<QVector2D>(gridCols);
    }

    Mat u_resized = Mat(gridRows, gridCols, u.type());
    Mat v_resized = Mat(gridRows, gridCols, v.type());

    cv::resize(u, u_resized, cv::Size(gridCols, gridRows), 0, 0, cv::INTER_AREA);
    cv::resize(v, v_resized, cv::Size(gridCols, gridRows), 0, 0, cv::INTER_AREA);

    for(int y = 0; y < velocityGrid.size(); y++)
    {
        for(int x = 0; x < velocityGrid[y].size(); x++)
        {
            velocityGrid[y][x].setX(u_resized.at<float>(y, x));
            velocityGrid[y][x].setY(v_resized.at<float>(y, x));
            velocityGrid[y][x] -= worldFrameDisp;
            velocityGrid[y][x] += trackingRectDisp;
        }
    }
}

void TemplateMatchTracker::filterVelocityGrid()
{
    float n_vf_gain = 1.0f - vf_gain;
    QVector<QVector2D> sortedVel;
    for(int y = 0; y < velocityGrid.size(); y++)
    {
        for(int x = 0; x < velocityGrid[y].size(); x++)
        {
            if(abs(velocityGrid[y][x].length()) < 0.2f) { velocityGrid[y][x] = QVector2D(); }
            else if(abs(velocityGrid[y][x].length()) > 8.0f) { velocityGrid[y][x] *= 8.0f / abs(velocityGrid[y][x].length()); }

            f_velocityGrid[y][x] = (f_velocityGrid[y][x]*vf_gain) + (velocityGrid[y][x]*n_vf_gain);
            sortedVel.append(f_velocityGrid[y][x]);
        }
    }
}

void TemplateMatchTracker::drawVelocityGrid(QImage &img, int _width, int _height)
{
    img = QImage(_width, _height, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.setPen(Qt::transparent);

    float r = 0, g = 0, b = 0, a = 0;
    float norm_x = 1.0f, norm_y = 1.0f;
    float vx = 0, vy = 0;
    for(int y = 0; y < f_velocityGrid.size(); y++)
    {
        for(int x = 0; x < f_velocityGrid[y].size(); x++)
        {
            vx = f_velocityGrid[y][x].x();
            vy = f_velocityGrid[y][x].y();

            norm_x = (vx / (op_config.winSize / 10.0f));
            norm_y = (vy / (op_config.winSize / 10.0f));

            b = constrain(abs(norm_x) * 255.0f, 0.0f, 255.0f);
            g = constrain(abs(norm_y) * 255.0f, 0.0f, 255.0f);
            r = 0;
            if(norm_x < 0) { r += abs(norm_x)*255.0f; }
            if(norm_y < 0) { r += abs(norm_y)*255.0f; }
            r = constrain(r, 0.0f, 255.0f);

            a = constrain((r+b+g)*2, 0.0f, 255.0f);
            //qDebug() << r << ", " << g << ", " << b  << ", " << a;

            painter.setBrush(QColor(r, g, b, a));
            painter.drawRect(x*gridBlockSize, y*gridBlockSize, gridBlockSize, gridBlockSize);
        }
    }
}

Point TemplateMatchTracker::qPointToCVPoint(QPointF qpoint)
{
    return Point(qpoint.x(), qpoint.y());
}

Mat TemplateMatchTracker::qImageToCV(QImage &frame)
{
    return cv::Mat(frame.height(), frame.width(), CV_8UC1, (void*)frame.constBits(), frame.bytesPerLine());
}

QImage TemplateMatchTracker::cvToQImage(cv::Mat &cvImage, QImage::Format format)
{
    return QImage(cvImage.data, cvImage.cols, cvImage.rows, static_cast<int>(cvImage.step), format);
}

void TemplateMatchTracker::calcBlobPos(QImage &_frame, QImage &flowImage, QRectF &bbox)
{
    opRectValid = false;
    //flowImage.convertTo(QImage::Format_RGB888);

    QColor pixelColor;
    double validCount = 0;
    float rx1 = flowImage.width(), ry1 = flowImage.height();
    float rx2 = 1, ry2 = 1;
    double sum_x = 0.0f, sum_y = 0.0f;
    double cx = 0.0f, cy = 0.0f;
    float l_gain = 1.0f;

    for(int y = 0; y < flowImage.height(); y++)
    {
        for(int x = 0; x < flowImage.width(); x++)
        {
            pixelColor = flowImage.pixelColor(x, y);
            if(pixelColor.lightness() > 25)
            {
                if(x < rx1) { rx1 = x; }
                if(y < ry1) { ry1 = y; }
                if(x > rx2) { rx2 = x; }
                if(y > ry2) { ry2 = y; }

                l_gain = constrain((float)pixelColor.lightnessF()/0.5f, 0.0f, 1.0f);
                sum_x += (x * l_gain);
                sum_y += (y * l_gain);
                validCount++;
            }
        }
    }

    //qDebug() << validCount;

    if(validCount > 8)
    {
        cx = ((sum_x / validCount)*0.5f) + ((rx2-rx1)*0.5f);
        cy = ((sum_y / validCount)*0.5f) + ((ry2-ry1)*0.5f);
        cx += (bbox.x()-op_HWin);
        cy += (bbox.y()-op_VWin);

        op_curr_vel = QVector2D(QPointF(cx, cy) - opRect.center()) / frame_dt;
        if(abs(op_curr_vel.length()) > velocityThresh) { op_curr_vel *= (velocityThresh / abs(op_curr_vel.length())) * 0.15f; }
        op_curr_accel = ( op_curr_vel - op_vel ) / frame_dt;
        if(abs(op_curr_accel.length()) > accelThresh) { op_curr_accel *= (accelThresh / abs(op_curr_accel.length())) * 0.15f; }
        op_vel = (op_vel*0.7f) + (op_curr_vel*0.3f);
        op_accel = (op_accel*0.9f) + (op_curr_accel*0.1f);

        opRectValid = true;
    }

    else
    {
        op_vel *= 0.99f; if(abs(op_vel.length()) < 0.1f) { op_vel = QVector2D(); }
        op_accel *= 0.9f; if(abs(op_accel.length()) < 0.1f) { op_accel = QVector2D(); }
    }

    opDisplacement = opDisplacement*0.6f + (((op_vel*frame_dt) + (op_accel*frame_dt*frame_dt*0.5f))*0.4f);
    opRect.translate(opDisplacement.toPointF());

    if(opRect.x() < 0) { opRect.translate(-opRect.x(), 0); }
    else if(opRect.right() > _frame.width() ) { opRect.translate(_frame.width() - opRect.right(), 0); }

    if(opRect.y() < 0) { opRect.translate(0, -opRect.y()); }
    else if(opRect.bottom() > _frame.height()) { opRect.translate(0, _frame.height() - opRect.bottom()); }
}

void TemplateMatchTracker::findTargetContours(QImage &frame, QRectF &bbox)
{
    validContour = false;
    QRectF searchWindow = bbox.adjusted(-ct_HWin, -ct_VWin, ct_HWin, ct_VWin);
    QImage searchImage = frame.copy(searchWindow.toRect());
    QRect imageRect = searchImage.rect();
    Mat src_gray = Mat(searchImage.height(), searchImage.width(), CV_8U, (void*)searchImage.constBits(), searchImage.bytesPerLine());

    Mat canny_output;
    cv::Canny(src_gray, canny_output, canny_thresh1, canny_thresh2, 3, true);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    //cv::Point offset(bbox.x()-ct_HWin, bbox.y()-ct_VWin);
    cv::findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3 );
    Scalar color(255, 255, 255);
    for( size_t i = 0; i< contours.size(); i++ ){
        drawContours( drawing, contours, (int)i, color, 1, LINE_8, hierarchy, 0 );
    }

    cv::Mat outFrame(drawing.size(), drawing.type());
    cv::blur(drawing, outFrame, cv::Size(4, 4));
    cvtColor(outFrame, outFrame, COLOR_RGB2GRAY);
    cv::threshold(outFrame, outFrame, 30, 255, cv::THRESH_BINARY);

    cv::Canny(outFrame, canny_output, canny_thresh1, canny_thresh2, 3, true);
    cv::findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    //contourImage = QImage(outFrame.data, outFrame.cols, outFrame.rows, static_cast<int>(outFrame.step), QImage::Format_Grayscale8).copy();

    //if(contours.empty()) { qDebug() << "No Contours Found"; return; }

    double sum_dist = 0.0f, avg_dist = 0.0f, min_dist = DBL_MAX;
    int mc = 0;

    QVector2D centerPoint = QVector2D(imageRect.center().x(), imageRect.center().y());
    for( size_t i = 0; i< contours.size(); i++ )
    {
        for(size_t c = 0; c < contours[i].size(); c++)
        {
            sum_dist = abs(QVector2D(contours[i][c].x, contours[i][c].y).distanceToPoint(centerPoint));
        }
        avg_dist = sum_dist / (double)contours[i].size();
        if(avg_dist < min_dist)
        {
            min_dist = avg_dist;
            mc = i;
        }
    }

    QVector<QPointF> polyPoints;
    float abs_x_dist = 0.0f, abs_y_dist = 0.0f;
    float min_x = 1000.0f, max_x = -1;
    float min_y = 1000.0f, max_y = -1;

    for(size_t i = 0; i < contours[mc].size(); i++)
    {
        abs_x_dist = abs(contours[mc][i].x - centerPoint.x());
        abs_y_dist = abs(contours[mc][i].y - centerPoint.y());
        if( (abs_x_dist < (bbox.width()*0.6f)) && (abs_y_dist < (bbox.height()*0.6f)) )
        {
            polyPoints.append(QPointF(contours[mc][i].x, contours[mc][i].y));

            if(contours[mc][i].x < min_x) { min_x = contours[mc][i].x; }
            else if(contours[mc][i].x > max_x) { max_x = contours[mc][i].x; }

            if(contours[mc][i].y < min_y) { min_y = contours[mc][i].y; }
            else if(contours[mc][i].y > max_y) { max_y = contours[mc][i].y; }
        }
    }

    //if(!contours[mc].empty()) { polyPoints[polyPointCount-1] = QPoint(contours[min_contour][0].x, contours[min_contour][0].y); }

    if(polyPoints.isEmpty()) { contourImage = QImage(); return; }

    QPolygonF polygon(polyPoints);
    double polygonArea = getPolygonArea(polygon);

    if(polygonArea < (16*16)) { contourImage = QImage(); return; }

    QRectF curr_rect( QPointF(min_x, min_y), QPointF(max_x, max_y));
    curr_rect.translate(bbox.x()-ct_HWin, bbox.y()-ct_VWin);
    contourImage = QImage(searchImage.size(), QImage::Format_Grayscale8);
    contourImage.fill(Qt::black);

    QPainter p(&contourImage);
    p.setPen(Qt::transparent);
    p.setBrush(QBrush(Qt::white, Qt::BrushStyle::SolidPattern));
    p.drawPolygon(polyPoints.data(), (int)polyPoints.size());

    templateMaskImage = contourImage.scaledToWidth( contourImage.width()*1.2f, Qt::SmoothTransformation);
    int _x = (templateMaskImage.width() - contourImage.width())*0.5f;
    int _y = (templateMaskImage.height() - contourImage.height())*0.5f;
    templateMaskImage = templateMaskImage.copy(QRect(_x, _y, contourImage.width(), contourImage.height()));

    contourRect = QRectF( (contourRect.x()*0.65f) + (curr_rect.x()*0.35f), (contourRect.y()*0.65f) + (curr_rect.y()*0.35f),
                         (contourRect.width()*0.8f) + (curr_rect.width()*0.2f), (contourRect.height()*0.8f) + (curr_rect.height()*0.2f));

    validContour = true;

    //    contourImage = QImage(drawing.data, drawing.cols, drawing.rows, static_cast<int>(drawing.step), QImage::Format_RGB888).copy();
}

float TemplateMatchTracker::templateMatching(cv::Mat &inFrame, cv::Mat &tmpl, Point &matchLoc)
{
    Mat result_mat;
    int match_method = TemplateMatchModes::TM_SQDIFF_NORMED;
    cv::matchTemplate(inFrame, tmpl, result_mat, match_method);
    //cv::normalize(result_mat, result_mat, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

    double minVal; double maxVal;
    Point minLoc, maxLoc;
    float confidence = 0.0f;
    minMaxLoc(result_mat, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

    if( match_method  ==  TemplateMatchModes::TM_SQDIFF || match_method == TemplateMatchModes::TM_SQDIFF_NORMED ) { matchLoc = minLoc; confidence = 1.0f - minVal; }
    else { matchLoc = maxLoc; confidence = maxVal; }

    return confidence;
}

void TemplateMatchTracker::setParam(QStringList params){}

bool TemplateMatchTracker::isRoiInImageArea(QRect &rect)
{
    if(curr_frame.isNull()) { return false; }
    return curr_frame.rect().contains(rect, true);
}

void TemplateMatchTracker::applyImageMask(QImage &image, QImage &mask)
{
    if(image.size() != mask.size() || image.isNull() || mask.isNull()) { return; }

    QColor color;
    for(int y = 0; y < image.height(); y++)
    {
        for(int x = 0; x < image.width(); x++)
        {
            color = image.pixelColor(x, y);
            color.setHsl( color.hue(), color.saturation(), color.lightness() * ((float)mask.pixelColor(x, y).lightness()/255.0f));
            image.setPixelColor(x, y, color);
        }
    }

}
