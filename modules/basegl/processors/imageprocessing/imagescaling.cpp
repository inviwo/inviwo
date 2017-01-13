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

#include <modules/basegl/processors/imageprocessing/imagescaling.h>

#include <inviwo/core/interaction/events/resizeevent.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

const ProcessorInfo ImageScaling::processorInfo_{
    "org.inviwo.ImageScaling",      // Class identifier
    "Image Scaling",                // Display name
    "Image Operation",              // Category
    CodeState::Stable,              // Code state
    Tags::GL,                       // Tags
};
const ProcessorInfo ImageScaling::getProcessorInfo() const {
    return processorInfo_;
}

ImageScaling::ImageScaling()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , enabled_("enabled", "Enabled", true)
    , scalingFactor_("scalingFactor", "Scaling Factor", {
        { "one-eighth", "12.5%", 0.125 },
        { "one-fourth", "25%", 0.25 },
        { "one-half", "50%", 0.5 },
        { "three-quarters", "75%", 0.75 },
        { "one", "100%", 1.0 },
        { "three-halves", "150%", 1.5 },
        { "two", "200%", 2.0 },
        { "four", "400%", 4.0 },
        { "eight", "800%", 8.0 },
        { "custom", "Custom", -1.0 },
    }, 4)
    , customFactor_("customFactor", "Custom Factor", 1.0, 1.0/32.0, 32.0)
{
    
    addPort(inport_);
    addPort(outport_);

    addProperty(enabled_);
    addProperty(scalingFactor_);
    addProperty(customFactor_);

    customFactor_.setVisible(false);

    auto triggerRescale = [this]() {
        ResizeEvent e(calcInputImageSize());
        inport_.propagateEvent(&e);
    };

    enabled_.onChange(triggerRescale);
    scalingFactor_.onChange([this]() {
        customFactor_.setVisible(scalingFactor_.get() < 0.0);
        ResizeEvent e(calcInputImageSize());
        inport_.propagateEvent(&e);
    });
    customFactor_.onChange(triggerRescale);
}
    
void ImageScaling::process() {
    // TODO: do proper filtering here, in particular for down-scaling
    utilgl::activateTargetAndCopySource(outport_, inport_);
    utilgl::deactivateCurrentTarget();
}

void ImageScaling::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;


    if (event->hash() == ResizeEvent::chash()) {
        if (inport_.isConnected()) {
            ResizeEvent e(calcInputImageSize());
            inport_.propagateEvent(&e);
        }
    } else {
        bool used = event->hasBeenUsed();
        if (event->shouldPropagateTo(&inport_, this, source)) {
            inport_.propagateEvent(event);
            used |= event->hasBeenUsed();
        }
        if (used) event->markAsUsed();
    }
}

size2_t ImageScaling::calcInputImageSize() const {
    size2_t size(8, 8);
    if (outport_.hasData()) {
        size = outport_.getDimensions();
    }
    if (enabled_.get()) {
        const double factor = (scalingFactor_.get() < 0.0) ? customFactor_.get() : scalingFactor_.get();
        size = size2_t(dvec2(size) / factor);
    }
    return size;
}

} // namespace inviwo
