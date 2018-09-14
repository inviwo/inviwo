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

#include <modules/basegl/processors/imageprocessing/imagescaling.h>

#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/util/raiiutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

const ProcessorInfo ImageScaling::processorInfo_{
    "org.inviwo.ImageScaling",  // Class identifier
    "Image Scaling",            // Display name
    "Image Operation",          // Category
    CodeState::Stable,          // Code state
    "GL, Image",                // Tags
};
const ProcessorInfo ImageScaling::getProcessorInfo() const { return processorInfo_; }

ImageScaling::ImageScaling()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , enabled_("enabled", "Enabled", true)
    , scalingFactor_("scalingFactor", "Scaling Factor",
                     {
                         {"one-eighth", "12.5%", 0.125},
                         {"one-fourth", "25%", 0.25},
                         {"one-half", "50%", 0.5},
                         {"three-quarters", "75%", 0.75},
                         {"one", "100%", 1.0},
                         {"three-halves", "150%", 1.5},
                         {"two", "200%", 2.0},
                         {"four", "400%", 4.0},
                         {"eight", "800%", 8.0},
                         {"custom", "Custom", -1.0},
                     },
                     4)
    , customFactor_("customFactor", "Custom Factor", 1.0, 1.0 / 32.0, 32.0) {

    addPort(inport_);
    addPort(outport_);

    addProperty(enabled_);
    addProperty(scalingFactor_);
    addProperty(customFactor_);

    customFactor_.setVisible(false);

    scalingFactor_.onChange([this]() {
        if (enabled_) {
            customFactor_.setVisible(scalingFactor_.get() < 0.0);
            resizeInports();
        }
    });
    enabled_.onChange([this]() { resizeInports(); });
    customFactor_.onChange([this]() { resizeInports(); });
}

void ImageScaling::process() {
    if (!enabled_ || (scalingFactor_.getIdentifier() == "one")) {
        // no scaling necessary
        outport_.setData(inport_.getData());
    } else {
        // TODO: do proper filtering here, in particular for down-scaling
        utilgl::activateTargetAndCopySource(outport_, inport_);
        utilgl::deactivateCurrentTarget();
    }
}

void ImageScaling::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;

    if (event->hash() == ResizeEvent::chash()) {
        // cache size of the resize event
        lastValidOutputSize_ = event->getAs<ResizeEvent>()->size();

        if (enabled_) {
            event->markAsVisited(this);

            if (resizeInports()) event->markAsUsed();
            return;
        }
    }

    Processor::propagateEvent(event, source);
}

void ImageScaling::deserialize(Deserializer& d) {
    util::KeepTrueWhileInScope deserializing(&deserializing_);
    Processor::deserialize(d);
}

size2_t ImageScaling::calcInputImageSize() const {
    if (!enabled_) return lastValidOutputSize_;

    const double factor = (scalingFactor_.get() < 0.0) ? customFactor_.get() : scalingFactor_.get();
    return size2_t(dvec2(lastValidOutputSize_) / factor);
}

bool ImageScaling::resizeInports() {
    if (deserializing_) return false;

    ResizeEvent e(calcInputImageSize());

    bool used = e.hasBeenUsed();
    for (auto inport : getInports()) {
        if (e.shouldPropagateTo(inport, this, &outport_)) {
            inport->propagateEvent(&e);
            used |= e.hasBeenUsed();
            e.markAsUnused();
        }
    }
    return used;
}

}  // namespace inviwo
