/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "ordinalpropertyanimator.h"
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

ProcessorClassIdentifier(OrdinalPropertyAnimator, "org.inviwo.OrdinalPropertyAnimator");
ProcessorDisplayName(OrdinalPropertyAnimator,  "Property Animator");
ProcessorTags(OrdinalPropertyAnimator, Tags::CPU);
ProcessorCategory(OrdinalPropertyAnimator, "Various");
ProcessorCodeState(OrdinalPropertyAnimator, CODE_STATE_EXPERIMENTAL);


OrdinalPropertyAnimator::OrdinalPropertyAnimator()
    : Processor()
    , type_("property", "Property")
    , delay_("delay", "Delay (ms)", 50, 1, 10000, 1)
    , pbc_("pbc", "Periodic", true)
    , active_("active", "Active", false) 
{
    
    timer_ = InviwoApplication::getPtr()->createTimer();
    if (timer_ == nullptr) {
        throw Exception("Failed to create timer", IvwContext);
    } else{
        timer_->setElapsedTimeCallback(this, &OrdinalPropertyAnimator::timerEvent);
    }
    

    delay_.onChange(this, &OrdinalPropertyAnimator::updateTimerInterval);

    properties_.push_back(new PrimProp<float>("org.inviwo.FloatProperty", "org.inviwo.FloatProperty"));
    properties_.push_back(new VecProp<vec2>("org.inviwo.FloatVec2Property", "org.inviwo.FloatVec2Property"));
    properties_.push_back(new VecProp<vec3>("org.inviwo.FloatVec3Property", "org.inviwo.FloatVec3Property"));
    properties_.push_back(new VecProp<vec4>("org.inviwo.FloatVec4Property", "org.inviwo.FloatVec4Property"));
    properties_.push_back(new PrimProp<double>("org.inviwo.DoubleProperty","org.inviwo.DoubleProperty"));
    properties_.push_back(new VecProp<dvec2>("org.inviwo.DoubleVec2Property", "org.inviwo.DoubleVec2Property"));
    properties_.push_back(new VecProp<dvec3>("org.inviwo.DoubleVec3Property", "org.inviwo.DoubleVec3Property"));
    properties_.push_back(new VecProp<dvec4>("org.inviwo.DoubleVec4Property", "org.inviwo.DoubleVec4Property"));
    properties_.push_back(new PrimProp<int>("org.inviwo.IntProperty", "org.inviwo.IntProperty"));
    properties_.push_back(new VecProp<ivec2>("org.inviwo.IntVec2Property", "org.inviwo.IntVec2Property"));
    properties_.push_back(new VecProp<ivec3>("org.inviwo.IntVec3Property", "org.inviwo.IntVec3Property"));
    properties_.push_back(new VecProp<ivec4>("org.inviwo.IntVec4Property", "org.inviwo.IntVec4Property"));

    addProperty(type_);
    addProperty(active_);
    addProperty(delay_);
    addProperty(pbc_);

    std::vector<BaseProp*>::const_iterator itBegin = properties_.begin(); 
    for (std::vector<BaseProp*>::const_iterator it = itBegin; it != properties_.end(); ++it) {
        type_.addOption((*it)->classname_, (*it)->displayName_, std::distance(itBegin, it));
        Property* prop = (*it)->getProp();
        Property* delta = (*it)->getDelta();

        addProperty(prop);
        addProperty(delta);
        prop->setVisible(false);
        delta->setVisible(false);
    }
    type_.setSelectedIndex(0);
    type_.setCurrentStateAsDefault();
    
    type_.onChange(this, &OrdinalPropertyAnimator::changeProperty);

    active_.onChange(this, &OrdinalPropertyAnimator::changeActive);

    changeProperty();

    setAllPropertiesCurrentStateAsDefault();
}

OrdinalPropertyAnimator::~OrdinalPropertyAnimator() {
    timer_->stop();
    delete timer_;
}

void OrdinalPropertyAnimator::initialize() {
    changeProperty();
    updateTimerInterval();
}

void OrdinalPropertyAnimator::deinitialize() {
    timer_->stop();
}

void OrdinalPropertyAnimator::process() {
    if (!active_.get()) timer_->stop();
}

void OrdinalPropertyAnimator::updateTimerInterval() {
    timer_->stop();
    if(active_.get())
        timer_->start(delay_.get());
}

void OrdinalPropertyAnimator::timerEvent() {
    int ind = type_.get();
    properties_[ind]->update(pbc_.get());
}

void OrdinalPropertyAnimator::changeProperty() {
    int ind = type_.get();
    
    for (std::vector<BaseProp*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it) {
        Property* prop = (*it)->getProp();
        Property* delta = (*it)->getDelta();
        prop->setVisible(false);
        delta->setVisible(false);
    }
    
    properties_[ind]->getProp()->setVisible(true);
    properties_[ind]->getDelta()->setVisible(true);
}

void OrdinalPropertyAnimator::changeActive() {
    if (active_.get()) {
        updateTimerInterval();
    } else {
        timer_->stop();
    }
}

} // namespace

