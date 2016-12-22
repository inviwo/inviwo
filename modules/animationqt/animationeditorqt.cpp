/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/animationcontroller.h>

#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPainter>
#include <QGraphicsItem>
#include <warn/pop>

namespace inviwo {

namespace animation {

static constexpr double LineWidth = 0.5;

AnimationEditorQt::AnimationEditorQt(AnimationController& controller)
    : QGraphicsScene(), controller_(controller) {
    auto& animation = *controller_.getAnimation();
    animation.addObserver(this);

    for (size_t i = 0; i < animation.size(); ++i) {
        auto& track = animation[i];
        auto trackQt = new TrackQt(track);
        trackQt->setPos(0, TimelineHeight + TrackHeight * i + TrackHeight * 0.5);
        this->addItem(trackQt);
    }

    setSceneRect(0.0, 0.0, animation.lastTime().count() * WidthPerTimeUnit,
                 animation.size() * TrackHeight + TimelineHeight);
}

void AnimationEditorQt::onTrackAdded(Track* track) {
    auto trackQt = new TrackQt(*track);
    auto i = items().size();
    trackQt->setPos(0, TimelineHeight + TrackHeight * i + TrackHeight * 0.5);
    this->addItem(trackQt);
}

void AnimationEditorQt::onTrackRemoved(Track* track) {}

}  // namespace

}  // namespace
