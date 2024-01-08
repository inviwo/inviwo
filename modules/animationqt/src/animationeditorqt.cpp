/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/properties/property.h>                       // for Property
#include <inviwo/core/properties/propertyowner.h>                  // for PropertyOwner
#include <inviwo/core/properties/propertywidget.h>                 // for PropertyWidget
#include <inviwo/core/util/exception.h>                            // for Exception
#include <inviwo/core/util/indirectiterator.h>                     // for IndirectIterator
#include <inviwo/core/util/sourcecontext.h>                        // for IVW_CONTEXT
#include <inviwo/core/util/zip.h>                                  // for enumerate, zipIterator
#include <modules/animation/animationcontroller.h>                 // for AnimationController
#include <modules/animation/animationcontrollerobserver.h>         // for AnimationControllerObs...
#include <modules/animation/datastructures/animation.h>            // for Animation
#include <modules/animation/datastructures/animationstate.h>       // for AnimationState, Animat...
#include <modules/animation/datastructures/animationtime.h>        // for Seconds
#include <modules/animation/datastructures/keyframe.h>             // IWYU pragma: keep
#include <modules/animation/datastructures/track.h>                // for Track
#include <modules/animationqt/factories/trackwidgetqtfactory.h>    // for TrackWidgetQtFactory
#include <modules/animationqt/widgets/editorconstants.h>           // for getSnapTime, trackHeight
#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>  // for KeyframeSequenceWidgetQt
#include <modules/animationqt/widgets/keyframewidgetqt.h>          // for KeyframeWidgetQt
#include <modules/animationqt/widgets/trackwidgetqt.h>             // for TrackWidgetQt
#include <modules/qtwidgets/textlabeloverlay.h>                    // for TextLabelOverlay

#include <chrono>   // for milliseconds
#include <string>   // for basic_string, string
#include <utility>  // for move, pair

#include <QColor>                       // for QColor
#include <QFlags>                       // for QFlags
#include <QGraphicsItem>                // for qgraphicsitem_cast
#include <QGraphicsLineItem>            // for QGraphicsLineItem
#include <QGraphicsSceneDragDropEvent>  // for QGraphicsSceneDragDrop...
#include <QGraphicsView>                // for QGraphicsView
#include <QKeyEvent>                    // for QKeyEvent
#include <QList>                        // for QList, QList<>::iterator
#include <QPen>                         // for QPen
#include <QPointF>                      // for QPointF
#include <QTransform>                   // for QTransform
#include <QWidget>                      // for QWidget
#include <Qt>                           // for ControlModifier, DashLine
#include <QtGlobal>                     // for qreal
#include <fmt/core.h>                   // for format

class QGraphicsSceneDragDropEvent;

namespace inviwo {

namespace animation {

AnimationEditorQt::AnimationEditorQt(AnimationController& controller,
                                     TrackWidgetQtFactory& widgetFactory, TextLabelOverlay& overlay)
    : QGraphicsScene(), controller_(controller), widgetFactory_{widgetFactory}, overlay_{overlay} {
    auto& animation = controller_.getAnimation();
    animation.addObserver(this);
    controller_.AnimationControllerObservable::addObserver(this);

    // Add Property tracks
    for (auto& track : animation) {
        onTrackAdded(&track);
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

    updateSceneRect();
}

AnimationEditorQt::~AnimationEditorQt() = default;

void AnimationEditorQt::onAnimationChanged(AnimationController*, Animation* oldAnim,
                                           Animation* newAnim) {
    oldAnim->removeObserver(this);
    for (auto& track : *oldAnim) {
        onTrackRemoved(&track);
    }
    for (auto& track : *newAnim) {
        onTrackAdded(&track);
    }
    newAnim->addObserver(this);
}

std::unique_ptr<TrackWidgetQt> AnimationEditorQt::createTrackWidget(Track& track) const {
    auto widgetId = widgetFactory_.getWidgetId(track.getClassIdentifier());
    return widgetFactory_.create(widgetId, track);
}

void AnimationEditorQt::onTrackAdded(Track* track) {
    if (!track) return;
    if (auto trackWidget = createTrackWidget(*track)) {
        trackWidget->setPos(0, trackHeight * tracks_.size() + trackHeight * 0.5);
        this->addItem(trackWidget.get());
        tracks_[track] = std::move(trackWidget);
        updateSceneRect();
    } else {
        throw Exception(IVW_CONTEXT, "Not able to create widget for track: {} of type: {}",
                        track->getName(), track->getClassIdentifier());
    }
}

void AnimationEditorQt::onTrackRemoved(Track* track) {
    tracks_.erase(track);

    for (auto&& item : util::enumerate(tracks_)) {
        item.second().second->setY(trackHeight * item.first() + trackHeight * 0.5);
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
            case AnimationState::Rendering:
                break;
            default:
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
    overlay_.clear();
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
    overlay_.setText((event->modifiers() & Qt::ControlModifier)
                         ? "Insert new keyframe sequence (Alt for non-snapping time)"
                         : "Insert new keyframe (Ctrl for sequence, Alt for non-snapping time)",
                     std::chrono::milliseconds(1000));

    event->accept();
}

void AnimationEditorQt::dropEvent(QGraphicsSceneDragDropEvent* event) {

    // Switch off drag&drop indicator
    if (dropIndicatorLine) dropIndicatorLine->setVisible(false);

    // Drop it into the scene
    auto source = dynamic_cast<PropertyWidget*>(event->source());
    if (source) {
        auto property = source->getProperty();

        // Get time
        QGraphicsView* pView = views().empty() ? nullptr : views().first();
        const qreal snapX =
            getSnapTime(event->scenePos().x(), pView ? pView->transform().m11() : 1);
        const qreal time = snapX / static_cast<double>(widthPerSecond);

        if (event->modifiers() & Qt::ControlModifier) {
            controller_.getAnimation().addKeyframeSequence(property, Seconds(time));
        } else {
            controller_.getAnimation().addKeyframe(property, Seconds(time));
        }

        event->acceptProposedAction();
    }
}

void AnimationEditorQt::updateSceneRect() {
    setSceneRect(
        0.0, 0.0,
        static_cast<double>(controller_.getAnimation().getLastTime().count() * widthPerSecond) +
            5 * widthPerSecond,
        static_cast<double>(controller_.getAnimation().size() * trackHeight));
}

void AnimationEditorQt::onFirstMoved() { updateSceneRect(); }

void AnimationEditorQt::onLastMoved() { updateSceneRect(); }

}  // namespace animation

}  // namespace inviwo
