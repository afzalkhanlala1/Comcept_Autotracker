#ifndef TARGETRENDERER_H
#define TARGETRENDERER_H

#include "autotrackercontroller.h"

inline void renderTemplateMatcher(AutoTrackerController *atc, QPainter *painter, QRectF boundingRect)
{
    painter->setBrush(Qt::transparent);

    for(int i = 0; i < atc->objectTrackers.size(); i++)
    {
        if(atc->objectTrackers[i] == nullptr) continue;
        if(!atc->objectTrackers[i]->enabled) continue;
        //        if(atc->objectTrackers[i]->templateTracker == nullptr) continue;
        painter->setOpacity(1.0);

        if(atc->showTragetRect && atc->objectTrackers[i]->enabled)
        //if(0)
        {
            //if(!atc->objectTrackers[i]->templateTracker->trackingRect.isNull())
            if(0)
            {
                float tm_HWin = atc->objectTrackers[i]->templateTracker->tm_HWin;
                float tm_VWin = atc->objectTrackers[i]->templateTracker->tm_VWin;
                float rx1 = (atc->objectTrackers[i]->templateTracker->trackingRect.left()) / atc->imgScalingFactor;
                float ry1 = (atc->objectTrackers[i]->templateTracker->trackingRect.top()) / atc->imgScalingFactor;
                float rx2 = (atc->objectTrackers[i]->templateTracker->trackingRect.right()) / atc->imgScalingFactor;
                float ry2 = (atc->objectTrackers[i]->templateTracker->trackingRect.bottom()) / atc->imgScalingFactor;

                if(atc->showTragetRect)
                {
                    if(i == atc->activeTarget) { painter->setPen(QPen(QBrush(Qt::red), 4)); }
                    else { painter->setPen(QPen(QBrush(Qt::cyan), 4)); }
                    painter->drawRect(QRect(QPoint(rx1, ry1), QPoint(rx2, ry2)));
                }

                if(atc->showTragetRectWindow)
                {
                    rx1 = rx1 - (tm_HWin / atc->imgScalingFactor);
                    ry1 = ry1 - (tm_VWin / atc->imgScalingFactor);
                    rx2 = rx2 + (tm_HWin / atc->imgScalingFactor);
                    ry2 = ry2 + (tm_VWin / atc->imgScalingFactor);
                    painter->setPen(QPen(QBrush(Qt::darkCyan), 4));
                    painter->drawRect(QRect(QPoint(rx1, ry1), QPoint(rx2, ry2)));
                }

                if(atc->showTargetMask && !atc->objectTrackers[i]->templateTracker->templateMaskImage.isNull())
                {
                    int ct_HWin = atc->objectTrackers[i]->templateTracker->ct_HWin;
                    int ct_VWin = atc->objectTrackers[i]->templateTracker->ct_VWin;

                    painter->setOpacity(0.5);
                    painter->drawImage((atc->targetRects[i][TRACKER_TYPE::FUSION].x()+atc->x_win_offset-ct_HWin)/atc->imgScalingFactor,
                                       (atc->targetRects[i][TRACKER_TYPE::FUSION].y()+atc->y_win_offset-ct_VWin)/atc->imgScalingFactor,
                                       atc->objectTrackers[i]->templateTracker->templateMaskImage.scaledToWidth(atc->objectTrackers[i]->templateTracker->templateMaskImage.width()/atc->imgScalingFactor));
                }

                if(atc->showTargetFlow && !atc->objectTrackers[i]->templateTracker->flowImage.isNull())
                {
                    int op_HWin = atc->objectTrackers[i]->templateTracker->op_HWin;
                    int op_VWin = atc->objectTrackers[i]->templateTracker->op_VWin;

                    painter->setOpacity(0.75);
                    painter->drawImage((atc->targetRects[i][TRACKER_TYPE::FUSION].x()+atc->x_win_offset-op_HWin)/atc->imgScalingFactor,
                                       (atc->targetRects[i][TRACKER_TYPE::FUSION].y()+atc->y_win_offset-op_VWin)/atc->imgScalingFactor,
                                       atc->objectTrackers[i]->templateTracker->flowImage.scaledToWidth(atc->objectTrackers[i]->templateTracker->flowImage.width()/atc->imgScalingFactor));
                }

                if(atc->showTargetTejectory)
                {
                    painter->setOpacity(0.5);
                    for(int r = 0; r < atc->objectTrackers[i]->templateTracker->tejectory.size(); r++)
                    {
                        int rx = (atc->objectTrackers[i]->templateTracker->tejectory[r].pos.center().x()+atc->x_win_offset)/atc->imgScalingFactor;
                        int ry = (atc->objectTrackers[i]->templateTracker->tejectory[r].pos.center().y()+atc->y_win_offset)/atc->imgScalingFactor;
                        painter->setPen(Qt::green);
                        painter->drawRect(rx-2, ry-2, 4, 4);
                    }
                }

                if(atc->showTragetHistory)
                {
                    painter->setOpacity(1.0);
                    for(int i = 0; i < atc->objectTrackers.size(); i++)
                    {
                        if(atc->objectTrackers[i] == nullptr) continue;
                        if(atc->objectTrackers[i]->templateTracker == nullptr) continue;
                        if(i != atc->activeTarget) { continue; }

                        int p = 0;
                        for(; p < atc->objectTrackers[i]->templateTracker->prev_matches.size(); p++)
                        {
                            painter->drawImage(p*atc->objectTrackers[i]->templateTracker->prev_matches[p].width(), boundingRect.height()*0.6, atc->objectTrackers[i]->templateTracker->prev_matches[p]);
                        }

                        if(!atc->objectTrackers[i]->templateTracker->curr_template.isNull() && (p > 0))
                        {
                            painter->drawImage((p+1)*atc->objectTrackers[i]->templateTracker->prev_matches[p-1].width(), boundingRect.height()*0.6, atc->objectTrackers[i]->templateTracker->curr_template);
                        }
                    }
                }
            }

            else
            {
                float rx1 = (atc->objectTrackers[i]->trackedRects.last().left()) / atc->imgScalingFactor;
                float ry1 = (atc->objectTrackers[i]->trackedRects.last().top()) / atc->imgScalingFactor;
                float rx2 = (atc->objectTrackers[i]->trackedRects.last().right()) / atc->imgScalingFactor;
                float ry2 = (atc->objectTrackers[i]->trackedRects.last().bottom()) / atc->imgScalingFactor;

                if(i == atc->activeTarget) { painter->setPen(QPen(QBrush(Qt::red), 4));}
                else { painter->setPen(QPen(QBrush(Qt::cyan), 4)); }
                painter->drawRect(QRect(QPoint(rx1, ry1), QPoint(rx2, ry2)));


                //                float rx = atc->countourBB.x() / atc->imgScalingFactor;
                //                float ry =atc->countourBB.y() / atc->imgScalingFactor;
                //                float rw =atc->countourBB.width() / atc->imgScalingFactor;
                //                float rh = atc->countourBB.height() / atc->imgScalingFactor;

                //                float rx = atc->countourBB.x();
                //                float ry =atc->countourBB.y();
                //                float rw =atc->countourBB.width();
                //                float rh = atc->countourBB.height();
                //                QRect contRect = QRect(rx, ry, rw, rh);
                //                painter->setPen(QPen(QBrush(Qt::blue), 2));
                //                painter-> drawRect(contRect);

                //                atc->outRect = QRect(rx, ry, rw, rh);
                //                painter->setPen(QPen(QBrush(Qt::blue), 2));
                //                painter-> drawRect(atc->outRect);

            }
        }
    }
}

#endif // TARGETRENDERER_H
