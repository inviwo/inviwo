/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/animation/animationcontrollerobserver.h>       // for AnimationControllerObserver
#include <modules/animation/datastructures/animationobserver.h>  // for AnimationObserver

#include <QGraphicsScene>  // for QGraphicsScene

#include <memory>         // for unique_ptr
#include <unordered_map>  // for unordered_map

class QGraphicsLineItem;
class QGraphicsSceneDragDropEvent;
class QKeyEvent;

namespace inviwo {

class TextLabelOverlay;

namespace animation {

class Animation;
class Track;
class AnimationController;
class TrackWidgetQt;
class TrackWidgetQtFactory;

class IVW_MODULE_ANIMATIONQT_API AnimationEditorQt : public QGraphicsScene,
                                                     public AnimationObserver,
                                                     public AnimationControllerObserver {
public:
    AnimationEditorQt(AnimationController& controller, TrackWidgetQtFactory& widgetFactory,
                      TextLabelOverlay& overlay);
    virtual ~AnimationEditorQt();

    // AnimationControllerObserver overload
    void onAnimationChanged(AnimationController*, Animation* oldAnim, Animation* newAnim) override;

protected:
    void updateSceneRect();

    // AnimationObserver overloads
    virtual void onFirstMoved() override;
    virtual void onLastMoved() override;
    virtual void onTrackAdded(Track* track) override;
    virtual void onTrackRemoved(Track* track) override;

    // QGraphicsScene overloads
    virtual void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;

    AnimationController& controller_;
    TrackWidgetQtFactory& widgetFactory_;
    std::unordered_map<Track*, std::unique_ptr<TrackWidgetQt>> tracks_;

    /// Indicator line for the drag&drop of properties.
    /// Shows a timeline indicating where the item will be dropped.
    /// Manipulated in the drag* and drop* functions.
    QGraphicsLineItem* dropIndicatorLine;

    TextLabelOverlay& overlay_;
    std::unique_ptr<TrackWidgetQt> createTrackWidget(Track& track) const;
};

}  // namespace animation

}  // namespace inviwo
