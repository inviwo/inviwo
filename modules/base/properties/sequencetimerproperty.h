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

#ifndef IVW_SEQUENCETIMERPROPERTY_H
#define IVW_SEQUENCETIMERPROPERTY_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {

class IVW_MODULE_BASE_API SequenceTimerProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();

    SequenceTimerProperty(std::string identifier, std::string displayName,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                          PropertySemantics semantics = PropertySemantics::Default);
    SequenceTimerProperty(const SequenceTimerProperty& rhs);
    SequenceTimerProperty& operator=(const SequenceTimerProperty& that);
    virtual SequenceTimerProperty* clone() const override;
    virtual ~SequenceTimerProperty() = default;

    void updateMax(size_t max);

    IntProperty index_;
    BoolProperty play_;
    IntProperty framesPerSecond_;
    EventProperty playPause_;
    Timer timer_;

private:
    void onTimerEvent();
    void onPlaySequenceToggled();
};

}  // namespace

#endif  // IVW_SEQUENCETIMERPROPERTY_H
