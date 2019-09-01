/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/plottinggl/processors/axisrenderprocessor.h>

#include <modules/plotting/utils/axisutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/rendering/texturequadrenderer.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AxisRenderProcessor::processorInfo_{
    "org.inviwo.AxisRenderProcessor",  // Class identifier
    "Axis Render Processor",           // Display name
    "Plotting",                        // Category
    CodeState::Stable,                 // Code state
    "GL, Plotting, Demo",              // Tags
};
const ProcessorInfo AxisRenderProcessor::getProcessorInfo() const { return processorInfo_; }

AxisRenderProcessor::AxisRenderProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , margins_("margins", "Margins", 5.0f, 5.0f, 55.0f, 60.0f)
    , axisMargin_("axisMargin", "Axis Margin", 15.0f, 0.0f, 50.0f)
    , antialiasing_("antialias", "Antialiasing", true)
    , style_("style", "Global Style")
    , axis1_("axis1", "Axis 1")
    , axis2_("axis2", "Axis 2", AxisProperty::Orientation::Vertical)
    , axis3_("axis3", "Axis 3")
    , axisRenderers_({axis1_, axis2_, axis3_}) {

    inport_.setOptional(true);

    addPort(inport_);
    addPort(outport_);

    addProperty(margins_);
    addProperty(axisMargin_);

    addProperty(antialiasing_);

    axis1_.captionSettings_.setChecked(true);
    axis1_.setCaption("x Axis");

    // flip vertical axis to show labels on the left side
    axis2_.flipped_.set(true);
    axis2_.captionSettings_.setChecked(true);
    axis2_.setCaption("y Axis");

    axis3_.captionSettings_.setChecked(true);
    axis3_.setCaption("Diagonal Axis");

    style_.setCollapsed(true);
    style_.registerProperties(axis1_, axis2_, axis3_);
    addProperties(style_, axis1_, axis2_, axis3_);
}

void AxisRenderProcessor::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, inport_, ImageType::ColorDepth);

    const auto dims = outport_.getDimensions();
    const size2_t lowerLeft(margins_.getLeft(), margins_.getBottom());
    const size2_t upperRight(dims.x - 1 - margins_.getRight(), dims.y - 1 - margins_.getTop());

    const auto padding = axisMargin_.get();

    // draw horizontally
    axisRenderers_[0].render(dims, lowerLeft + size2_t(padding, 0),
                             size2_t(upperRight.x - padding, lowerLeft.y), antialiasing_.get());

    // draw vertically
    axisRenderers_[1].render(dims, lowerLeft + size2_t(0, padding),
                             size2_t(lowerLeft.x, upperRight.y - padding), antialiasing_.get());

    // draw diagonally in upper half
    axisRenderers_[2].render(
        dims, size2_t(lowerLeft.x + padding, (lowerLeft.y + upperRight.y) * 0.5f),
        size2_t(upperRight.x - padding, upperRight.y - padding), antialiasing_.get());

    utilgl::deactivateCurrentTarget();
}

}  // namespace plot

}  // namespace inviwo
