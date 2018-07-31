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

#include <modules/animationqt/animationeditorqt.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/zip.h>

#include <modules/animation/animationmodule.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/animationcontroller.h>

#include <modules/animationqt/widgets/trackwidgetqt.h>
#include <modules/animationqt/widgets/keyframewidgetqt.h>
#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>

#include <modules/animationqt/factories/trackwidgetqtfactory.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QWidget>
#include <QPen>
#include <warn/pop>

namespace inviwo {

namespace animation {

AnimationEditorQt::AnimationEditorQt(AnimationController& controller,
                                     TrackWidgetQtFactory& widgetFactory)
    : QGraphicsScene(), controller_(controller), widgetFactory_{widgetFactory} {
    auto& animation = controller_.getAnimation();
    animation.addObserver(this);

    // Add Property tracks
    for (auto&& item : util::enumerate(animation)) {
        auto trackQt = std::make_unique<TrackWidgetQt>(item.second());
        trackQt->setPos(0, timelineHeight + trackHeight * item.first() + trackHeight * 0.5);
        this->addItem(trackQt.get());
        tracks_[&item.second()] = std::move(trackQt);
    }

    // Add drag&drop indicator
    QPen timePen;
    timePen.setColor(QColor(255, 128, 0));
    timePen.setWidthF(1.0);
    timePen.setCosmetic(true);
    timePen.setStyle(Qt::DashLine);
    dropIndicatorLine = addLine(10, 0, 10, 1000, timePen);
    if (dropIndicatorLine) {
        dropIndicatorLine->setZValue(1);
        dropIndicatorLine->setVisible(false);
    }

    // Add drag&drop hint - this one still has a number of issue. It should probably not be in the
    // scene, not be affected by zoom.
    dropIndicatorText = new QGraphicsSimpleTextItem(dropIndicatorLine);
    if (dropIndicatorText) {
        dropIndicatorText->setPos(0, timelineHeight);
        dropIndicatorText->setZValue(1);
        dropIndicatorText->setVisible(false);
    }

    updateSceneRect();
}

AnimationEditorQt::~AnimationEditorQt() = default;

std::unique_ptr<TrackWidgetQt> AnimationEditorQt::createTrackWidget(Track& track) const {
    auto widgetId = widgetFactory_.getWidgetId(track.getClassIdentifier());
    return widgetFactory_.create(widgetId, track);
}

void AnimationEditorQt::onTrackAdded(Track* track) {
    auto trackQt = std::make_unique<TrackWidgetQt>(*track);
    trackQt->setPos(0, timelineHeight + trackHeight * tracks_.size() + trackHeight * 0.5);
    this->addItem(trackQt.get());
    tracks_[track] = std::move(trackQt);
    updateSceneRect();
}

void AnimationEditorQt::onTrackRemoved(Track* track) {
    tracks_.erase(track);

    for (auto&& item : util::enumerate(tracks_)) {
        item.second().second->setY(timelineHeight + trackHeight * item.first() + trackHeight * 0.5);
    }
    updateSceneRect();
}

void AnimationEditorQt::keyPressEvent(QKeyEvent* keyEvent) {
    int k = keyEvent->key();
    if (k == Qt::Key_Delete) {  // Delete selected
        QList<QGraphicsItem*> itemList = selectedItems();
        for (auto& elem : itemList) {
            if (auto keyqt = qgraphicsitem_cast<KeyframeWidgetQt*>(elem)) {
                auto& animation = controller_.getAnimation();
                animation.remove(&(keyqt->getKeyframe()));
            } else if (auto seqqt = qgraphicsitem_cast<KeyframeSequenceWidgetQt*>(elem)) {
                auto& animation = controller_.getAnimation();
                animation.remove(&(seqqt->getKeyframeSequence()));
            }
        }
    } else if (k == Qt::Key_Space) {
        switch (controller_.getState()) {
            case AnimationState::Paused:
                controller_.play();
                break;
            case AnimationState::Playing:
                controller_.pause();
                break;
        }
    }
}

void AnimationEditorQt::dragEnterEvent(QGraphicsSceneDragDropEvent* event) {
    // Only accept PropertyWidgets from a processor
    auto source = dynamic_cast<PropertyWidget*>(event->source());
    event->setAccepted(source != nullptr && source->getProperty() != nullptr &&
                       source->getProperty()->getOwner()->getProcessor() != nullptr);
}

void AnimationEditorQt::dragLeaveEvent(QGraphicsSceneDragDropEvent*) {
    if (dropIndicatorLine) dropIndicatorLine->setVisible(false);
}

void AnimationEditorQt::dragMoveEvent(QGraphicsSceneDragDropEvent* event) {
    // Must override for drop events to occur. Do not call QGraphicsScene::dragMoveEvent

    // Indicate position
    if (dropIndicatorLine) {
        QGraphicsView* pView = views().empty() ? nullptr : views().first();
        const qreal snapX =
            getSnapTime(event->scenePos().x(), pView ? pView->transform().m11() : 1);
        dropIndicatorLine->setLine(snapX, 0, snapX, pView ? pView->height() : height());
        dropIndicatorLine->setVisible(true);
    }

    // Indicate insertion mode: keyframe or keyframe sequence.
    if (dropIndicatorText) {
        QString Text = (event->modifiers() & Qt::ControlModifier)
                           ? "Insert new keyframe sequence (Alt for non-snapping time)"
                           : "Insert new keyframe (Ctrl for sequence, Alt for non-snapping time)";

        dropIndicatorText->setText(Text);
        dropIndicatorText->setVisible(true);
    }

    event->accept();
}

void AnimationEditorQt::dropEvent(QGraphicsSceneDragDropEvent* event) {

    // Switch off drag&drop indicator
    if (dropIndicatorLine) dropIndicatorLine->setVisible(false);

    // Drop it into the scene
    auto source = dynamic_cast<PropertyWidget*>(event->source());
    if (source) {
        auto property = source->getProperty();
        auto app = controller_.getInviwoApplication();

        // Get time
        QGraphicsView* pView = views().empty() ? nullptr : views().first();
        const qreal snapX =
            getSnapTime(event->scenePos().x(), pView ? pView->transform().m11() : 1);
        const qreal time = snapX / static_cast<double>(widthPerSecond);

        // Use AnimationManager for adding keyframe or keyframe sequence.
        auto& am = app->template getModuleByType<AnimationModule>()->getAnimationManager();
        if (event->modifiers() & Qt::ControlModifier) {
            am.addSequenceCallback(property, Seconds(time));
        } else {
            am.addKeyframeCallback(property, Seconds(time));
        }

        event->acceptProposedAction();
    }
}

void AnimationEditorQt::updateSceneRect() {
    setSceneRect(
        0.0, 0.0,
        static_cast<double>(controller_.getAnimation().getLastTime().count() * widthPerSecond),
        static_cast<double>(controller_.getAnimation().size() * trackHeight + timelineHeight));
}

void AnimationEditorQt::onFirstMoved() { updateSceneRect(); }

void AnimationEditorQt::onLastMoved() { updateSceneRect(); }

}  // namespace animation

}  // namespace inviwo
