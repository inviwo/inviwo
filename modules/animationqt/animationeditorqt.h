/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_ANIMATIONEDITORQT_H
#define IVW_ANIMATIONEDITORQT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <modules/animation/datastructures/animationobserver.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QApplication>
#include <warn/pop>

class QKeyEvent;

namespace inviwo {

namespace animation {

constexpr int TrackHeight = 25;
constexpr int TimelineHeight = 25;
constexpr int KeyframeWidth = 15;
constexpr int KeyframeHeight = TrackHeight;
constexpr int WidthPerSecond = 96;

///We snap to certain times depending on the scale (zoom) level and keyboard modifiers.
/// It is important to supply scene coordinates to this function!
static qreal getSnapTime(const qreal& actualTime, const qreal& scale) {
    if (QApplication::keyboardModifiers() == Qt::AltModifier) {
        return actualTime;
    }
    qreal snapScale = (scale >= 1) ? round(scale) + 1 : 3-round(1/scale);
    snapScale = std::max(snapScale, 0.0); //not over 1 second
    snapScale = std::min(snapScale, 5.0); //not under 1/32 second
    const qreal snapToGridResolution = WidthPerSecond / pow(2, snapScale);
    const qreal snapTime = round(actualTime / snapToGridResolution) * snapToGridResolution;

    return snapTime;
}

class AnimationController;
class TrackQt;

class IVW_MODULE_ANIMATIONQT_API AnimationEditorQt : public QGraphicsScene,
                                                     public AnimationObserver {
public:
    AnimationEditorQt(AnimationController& controller);
    virtual ~AnimationEditorQt();

    virtual void onTrackAdded(Track* track) override;
    virtual void onTrackRemoved(Track* track) override;

    virtual void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

protected:
    void updateSceneRect();

    virtual void onFirstMoved() override;

    virtual void onLastMoved() override;

    AnimationController& controller_;
    std::vector<std::unique_ptr<TrackQt>> tracks_;

    ///Indicator line for the drag&drop of properties.
    /// Shows a timeline indicating where the item will be dropped.
    /// Manipulated in the drag* and drop* functions.
    QGraphicsLineItem* pDropIndicatorLine;
};

}  // namespace
}  // namespace

#endif  // IVW_ANIMATIONEDITORQT_H
