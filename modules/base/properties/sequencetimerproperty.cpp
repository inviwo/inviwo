/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "sequencetimerproperty.h"
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/action.h>

namespace inviwo {

PropertyClassIdentifier(SequenceTimerProperty, "org.inviwo.SequenceTimerProperty");

SequenceTimerProperty::SequenceTimerProperty(std::string identifier, std::string displayName,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , index_("selectedSequenceIndex", "Sequence Index", 1, 1, 1, 1)
    , play_("playSequence", "Play Sequence", false)
    , framesPerSecond_("volumesPerSecond", "Frame rate", 30, 1, 60, 1, InvalidationLevel::Valid)
    , playPause_(
          "playPause", "Play / Pause",
          new KeyboardEvent('P', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action([this](Event* e){play_.set(!play_.get());}))
    , timer_(1000 / framesPerSecond_.get(), [this]() { onTimerEvent(); }) {
    play_.onChange(this, &SequenceTimerProperty::onPlaySequenceToggled);

    framesPerSecond_.onChange([this]() { timer_.setInterval(1000 / framesPerSecond_.get()); });
    index_.setSerializationMode(PropertySerializationMode::ALL);
    addProperty(index_);
    addProperty(play_);
    addProperty(framesPerSecond_);
    addProperty(playPause_);
}

SequenceTimerProperty::SequenceTimerProperty(const SequenceTimerProperty& rhs)
    : CompositeProperty(rhs)
    , index_(rhs.index_)
    , play_(rhs.play_)
    , framesPerSecond_(rhs.framesPerSecond_)
    , playPause_(rhs.playPause_)
    , timer_(1000 / framesPerSecond_.get(), [this]() { onTimerEvent(); }) {
    framesPerSecond_.onChange([this]() { timer_.setInterval(1000 / framesPerSecond_.get()); });
    index_.setSerializationMode(PropertySerializationMode::ALL);
    addProperty(index_);
    addProperty(play_);
    addProperty(framesPerSecond_);
    addProperty(playPause_);
}

SequenceTimerProperty& SequenceTimerProperty::operator=(const SequenceTimerProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        index_ = that.index_;
        play_ = that.play_;
        framesPerSecond_ = that.framesPerSecond_;
        playPause_ = that.playPause_;
        
    }
    return *this;
}

SequenceTimerProperty* SequenceTimerProperty::clone() const {
    return new SequenceTimerProperty(*this);
}

void SequenceTimerProperty::updateMax(size_t max) {
    index_.setMaxValue(static_cast<int>(max));
    index_.set(1);
    index_.setCurrentStateAsDefault();
}

void inviwo::SequenceTimerProperty::onTimerEvent() {
    index_ = (index_ < index_.getMaxValue() ? index_ + 1 : 1);
}

void inviwo::SequenceTimerProperty::onPlaySequenceToggled() {
    if (index_.getMaxValue() > 1) {
        if (play_.get()) {
            timer_.start(1000 / framesPerSecond_.get());
            index_.setReadOnly(true);
        } else {
            timer_.stop();
            index_.setReadOnly(false);
        }
    }
}

}  // namespace
