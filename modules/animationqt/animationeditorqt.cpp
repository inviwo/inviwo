/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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
#include <modules/animationqt/trackqt.h>
#include <modules/animationqt/keyframeqt.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/animationcontroller.h>

#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPainter>
#include <QGraphicsItem>
#include <QKeyEvent>
#include <warn/pop>

namespace inviwo {

namespace animation {

static constexpr double LineWidth = 0.5;

AnimationEditorQt::AnimationEditorQt(AnimationController& controller)
    : QGraphicsScene(), controller_(controller) {
    auto& animation = *controller_.getAnimation();
    animation.addObserver(this);

    for (size_t i = 0; i < animation.size(); ++i) {
        auto trackQt = std::make_unique<TrackQt>(animation[i]);
        trackQt->setPos(0, TimelineHeight + TrackHeight * i + TrackHeight * 0.5);
        this->addItem(trackQt.get());
        tracks_.push_back(std::move(trackQt));
    }

    updateSceneRect();
}

AnimationEditorQt::~AnimationEditorQt() = default;

void AnimationEditorQt::onTrackAdded(Track* track) {
    auto trackQt = std::make_unique<TrackQt>(*track);
    trackQt->setPos(0, TimelineHeight + TrackHeight * tracks_.size() + TrackHeight * 0.5);
    this->addItem(trackQt.get());
    tracks_.push_back(std::move(trackQt));
    updateSceneRect();
}

void AnimationEditorQt::onTrackRemoved(Track* track) {
    if (util::erase_remove_if(tracks_, [&](auto& trackqt) {
            if (&(trackqt->getTrack()) == track) {
                removeItem(trackqt.get());
                return true;
            } else {
                return false;
            }
        }) > 0) {

        for (size_t i = 0; i < tracks_.size(); ++i) {
            tracks_[i]->setY(TimelineHeight + TrackHeight * i);
        }
    }
    updateSceneRect();
}

void AnimationEditorQt::keyPressEvent(QKeyEvent* keyEvent) {
    int k = keyEvent->key();
    if (k == Qt::Key_Delete) {  // Delete selected
        QList<QGraphicsItem*> itemList = selectedItems();
        for (auto& elem : itemList) {
            if (auto key = qgraphicsitem_cast<KeyframeQt*>(elem)) {
                auto& animation = *controller_.getAnimation();
                animation.removeKeyframe(&(key->getKeyframe()));
            }
        }
    } else if (k == Qt::Key_Space) {
        switch(controller_.getState()) {
            case AnimationState::Paused:
                controller_.play();
                break;
            case AnimationState::Playing:
                controller_.stop();
                break;
        } 
    }
}

void AnimationEditorQt::updateSceneRect() {
    setSceneRect(0.0, 0.0, controller_.getAnimation()->lastTime().count() * WidthPerSecond,
                 controller_.getAnimation()->size() * TrackHeight + TimelineHeight);
}

void AnimationEditorQt::onFirstMoved() { updateSceneRect(); }

void AnimationEditorQt::onLastMoved() { updateSceneRect(); }

}  // namespace

}  // namespace
