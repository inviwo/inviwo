/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/animationqt/widgets/trackwidgetqt.h>
#include <modules/animation/datastructures/keyframesequence.h>

#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>

namespace inviwo {

namespace animation {

TrackWidgetQt::TrackWidgetQt(Track& track) : QGraphicsItem(), track_(track) {
    for (size_t i = 0; i < track_.size(); ++i) {
        sequences_[&track_[i]] = std::make_unique<KeyframeSequenceWidgetQt>(track_[i], this);
    }
    track_.addObserver(this);
}

TrackWidgetQt::~TrackWidgetQt() = default;

void TrackWidgetQt::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

Track& TrackWidgetQt::getTrack() { return track_; }

const Track& TrackWidgetQt::getTrack() const { return track_; }

QRectF TrackWidgetQt::boundingRect() const { return childrenBoundingRect(); }

void TrackWidgetQt::onKeyframeSequenceAdded(Track*, KeyframeSequence* s) {
    sequences_[s] = std::make_unique<KeyframeSequenceWidgetQt>(*s, this);
}

void TrackWidgetQt::onKeyframeSequenceRemoved(Track*, KeyframeSequence* sequence) {
    sequences_.erase(sequence);
}

}  // namespace animation

}  // namespace inviwo
