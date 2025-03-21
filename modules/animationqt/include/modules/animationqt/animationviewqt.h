/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/
#pragma once

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIONQT_API

#include <modules/animation/animationcontrollerobserver.h>    // for AnimationControllerObserver
#include <modules/animation/datastructures/animationstate.h>  // for AnimationState
#include <modules/animation/datastructures/animationtime.h>   // for Seconds

#include <QGraphicsView>  // for QGraphicsView
#include <QRectF>         // for QRectF

class QKeyEvent;
class QMouseEvent;
class QPainter;
class QRectF;
class QWheelEvent;
class QWidget;

namespace inviwo {

namespace animation {

class AnimationController;
class AnimationEditorQt;

class IVW_MODULE_ANIMATIONQT_API AnimationViewQt : public QGraphicsView,
                                                   public AnimationControllerObserver {
public:
    AnimationViewQt(AnimationController& controller, AnimationEditorQt* scene);
    virtual ~AnimationViewQt() = default;

    void setTimelinePos(double x);
    AnimationController& getController();

protected:
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void wheelEvent(QWheelEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;
    virtual void keyReleaseEvent(QKeyEvent* keyEvent) override;

    void zoom(double dz);
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;

    virtual void onStateChanged(AnimationController* controller, AnimationState oldState,
                                AnimationState newState) override;
    virtual void onTimeChanged(AnimationController* controller, Seconds oldTime,
                               Seconds newTime) override;

    AnimationEditorQt* scene_;
    AnimationController& controller_;
    bool pressingOnTimeline_ = false;
    QWidget* timeline_;
};

}  // namespace animation

}  // namespace inviwo
