/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/base/processors/ordinalpropertyanimator.h>

#include <inviwo/core/network/networklock.h>         // for NetworkLock
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>   // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>    // for Tags, Tags::CPU
#include <inviwo/core/properties/buttonproperty.h>   // for ButtonProperty
#include <inviwo/core/properties/optionproperty.h>   // for OptionProperty, OptionPropertySize_t
#include <inviwo/core/properties/ordinalproperty.h>  // for OrdinalProperty, IntProperty
#include <inviwo/core/properties/property.h>         // for Property
#include <inviwo/core/properties/valuewrapper.h>     // for PropertySerializationMode, PropertyS...
#include <inviwo/core/util/foreacharg.h>             // for for_each_type
#include <inviwo/core/util/stringconversion.h>       // for toString
#include <inviwo/core/util/timer.h>                  // for Timer

#include <chrono>   // for milliseconds
#include <cstddef>  // for size_t
#include <ostream>  // for operator<<

#include <glm/common.hpp>             // for clamp
#include <glm/vec2.hpp>               // for operator*, operator+, operator-, ope...
#include <glm/vec3.hpp>               // for operator*, operator+, operator-, ope...
#include <glm/vec4.hpp>               // for operator*, operator+, operator-, ope...
#include <glm/vector_relational.hpp>  // for greaterThanEqual, lessThan

namespace inviwo {
class Deserializer;

const ProcessorInfo OrdinalPropertyAnimator::processorInfo_{
    "org.inviwo.OrdinalPropertyAnimator",  // Class identifier
    "Property Animator",                   // Display name
    "Various",                             // Category
    CodeState::Experimental,               // Code state
    Tags::CPU,                             // Tags
};
const ProcessorInfo OrdinalPropertyAnimator::getProcessorInfo() const { return processorInfo_; }

OrdinalPropertyAnimator::OrdinalPropertyAnimator()
    : Processor()
    , type_("property", "Property")
    , create_("add", "Add")
    , delay_("delay", "Delay (ms)", 16, 10, 10000, 1)
    , play_("play", "Play")
    , timer_(std::chrono::milliseconds{delay_}, [this]() {
        NetworkLock lock(this);
        for (auto p : props_) {
            p->update();
        }
    }) {

    util::for_each_type<Types>{}(TypeFunctor{}, *this);
    type_.setSelectedIndex(0);
    type_.setCurrentStateAsDefault();

    addProperty(type_);
    addProperty(create_);
    addProperty(delay_);
    addProperty(play_);

    create_.onChange([&]() {
        auto p = factory_[type_.getSelectedIndex()]();

        // make the id unique
        size_t count = 1;
        const auto& base = p->getIdentifier();
        auto id = base;
        auto displayname = base;
        while (getPropertyByIdentifier(id) != nullptr) {
            id = base + toString(count);
            displayname = base + " " + toString(count);
            ++count;
        }
        p->setIdentifier(id);
        p->setDisplayName(displayname);

        p->setSerializationMode(PropertySerializationMode::All);
        props_.push_back(static_cast<BaseOrdinalAnimationProperty*>(p.get()));
        addProperty(p.release(), true);
    });

    delay_.onChange([&]() { timer_.setInterval(std::chrono::milliseconds{delay_}); });

    play_.onChange([&]() {
        if (timer_.isRunning()) {
            timer_.stop();
            play_.setDisplayName("Play");
        } else {
            timer_.start();
            play_.setDisplayName("Stop");
        }
    });
}

void OrdinalPropertyAnimator::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    for (auto prop : *this) {
        if (auto p = dynamic_cast<BaseOrdinalAnimationProperty*>(prop)) {
            props_.push_back(p);
        }
    }
}

std::ostream& operator<<(std::ostream& ss, BoundaryType bt) {
    switch (bt) {
        case BoundaryType::Stop:
            ss << "Stop";
            break;
        case BoundaryType::Periodic:
            ss << "Periodic";
            break;
        case BoundaryType::Mirror:
            ss << "Mirror";
            break;
        default:
            break;
    }
    return ss;
}

}  // namespace inviwo
