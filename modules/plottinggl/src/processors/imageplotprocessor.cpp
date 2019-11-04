/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/plottinggl/processors/imageplotprocessor.h>

#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImagePlotProcessor::processorInfo_{
    "org.inviwo.ImagePlotProcessor",  // Class identifier
    "Image Plot",                     // Display name
    "Plotting",                       // Category
    CodeState::Stable,                // Code state
    "GL, Plotting",                   // Tags
};
const ProcessorInfo ImagePlotProcessor::getProcessorInfo() const { return processorInfo_; }

ImagePlotProcessor::ImagePlotProcessor()
    : Processor()
    , imgInport_("image", true)
    , bgInport_("bg")
    , outport_("outport")
    , margins_("margins", "Margins", 5.0f, 5.0f, 30.0f, 50.0f)
    , axisMargin_("axisMargin", "Axis Margin", 15.0f, 0.0f, 100.0f)
    , rangeMode_("rangeMode", "Axis Range Mode",
                 {{"dims", "Image Dimensions (pixel)", AxisRangeMode::ImageDims},
                  {"custom", "Custom", AxisRangeMode::Custom}})
    , customRanges_("customRanges", "Custom Ranges")
    , rangeXaxis_("rangeX", "X Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max())
    , rangeYaxis_("rangeY", "Y Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max())
    , axisStyle_("axisStyle", "Global Axis Style")
    , xAxis_("xAxis", "X Axis")
    , yAxis_("yAxis", "Y Axis", AxisProperty::Orientation::Vertical)
    , imageInteraction_("imageInteraction", "Image Interaction", true)
    , axisRenderers_({{xAxis_, yAxis_}})
    , imgRenderer_{}
    , viewManager_{}
    , viewport_{0, 0, 1, 1}
    , imgDims_{1, 1} {

    bgInport_.setOptional(true);

    addPort(imgInport_);
    addPort(bgInport_);
    addPort(outport_);

    rangeXaxis_.setSemantics(PropertySemantics::Text);
    rangeYaxis_.setSemantics(PropertySemantics::Text);

    customRanges_.addProperties(rangeXaxis_, rangeYaxis_);
    customRanges_.setCollapsed(true);

    axisStyle_.registerProperties(xAxis_, yAxis_);
    axisStyle_.setCollapsed(true);
    axisStyle_.labelFormat_.set("%g");
    axisStyle_.setCurrentStateAsDefault();

    addProperties(margins_, axisMargin_, rangeMode_, customRanges_, axisStyle_, xAxis_, yAxis_,
                  imageInteraction_);

    auto updateCallback = [&]() { updateViewport(); };

    margins_.onChange(updateCallback);
    axisMargin_.onChange(updateCallback);

    xAxis_.setCaption("x");
    yAxis_.setCaption("y");
    yAxis_.flipped_.set(true);

    auto linkAxisRanges = [this](DoubleMinMaxProperty& from, DoubleMinMaxProperty& to) {
        auto func = [&]() {
            if (!propertyUpdate_ && (rangeMode_.getSelectedValue() == AxisRangeMode::Custom)) {
                util::KeepTrueWhileInScope b(&propertyUpdate_);
                to.set(from.get());
            }
        };
        return func;
    };
    // "link" custom ranges with individual axis ranges
    xAxis_.range_.onChange(linkAxisRanges(xAxis_.range_, rangeXaxis_));
    yAxis_.range_.onChange(linkAxisRanges(yAxis_.range_, rangeYaxis_));
    rangeXaxis_.onChange(linkAxisRanges(rangeXaxis_, xAxis_.range_));
    rangeYaxis_.onChange(linkAxisRanges(rangeYaxis_, yAxis_.range_));

    // adjust axis ranges when input volume, i.e. its basis, changes
    imgInport_.onChange([this]() { adjustRanges(); });
    // sync ranges when custom range is enabled or disabled
    rangeMode_.onChange([this]() { adjustRanges(); });

    viewManager_.push_back(viewport_);
}

void ImagePlotProcessor::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, bgInport_);

    utilgl::DepthFuncState depthfunc(GL_ALWAYS);

    const auto dims = outport_.getDimensions();
    const size2_t lowerLeft(margins_.getLeft(), margins_.getBottom());
    const size2_t upperRight(dims.x - 1 - margins_.getRight(), dims.y - 1 - margins_.getTop());

    const auto padding = axisMargin_.get();

    // draw horizontal axis
    axisRenderers_[0].render(dims, lowerLeft + size2_t(padding, 0),
                             size2_t(upperRight.x - padding, lowerLeft.y));
    // draw vertical axis
    axisRenderers_[1].render(dims, lowerLeft + size2_t(0, padding),
                             size2_t(lowerLeft.x, upperRight.y - padding));

    const auto bounds = calcImageBounds(dims);
    // ensure that the image is rendered in the background with maximum depth
    mat4 m = glm::translate(vec3(0.0f, 0.0f, 2.0f));
    imgRenderer_.renderToRect(*imgInport_.getData(), bounds.pos, bounds.extent, dims,
                              LayerType::Color, m);

    utilgl::deactivateCurrentTarget();
}

void ImagePlotProcessor::propagateEvent(Event* event, Outport* source) {
    if (event->hasVisitedProcessor(this)) return;
    event->markAsVisited(this);

    invokeEvent(event);
    if (event->hasBeenUsed()) return;

    if (event->hash() == ResizeEvent::chash()) {
        auto resizeEvent = static_cast<ResizeEvent*>(event);

        updateViewports(resizeEvent->size(), true);

        bgInport_.propagateEvent(resizeEvent);

        if (imgInport_.isConnected()) {
            ResizeEvent e(size2_t(viewManager_[0].size));
            imgInport_.propagateEvent(&e);
        }
    } else {
        if (imageInteraction_.get() && imgInport_.isConnected()) {
            bool overlayHandlesEvent =
                (viewManager_.propagateEvent(event, [&](Event* newEvent, size_t /*ind*/) {
                    imgInport_.propagateEvent(newEvent);
                }));

            if (overlayHandlesEvent || event->hasBeenUsed()) {
                return;
            }
        }

        if (event->shouldPropagateTo(&bgInport_, this, source)) {
            bgInport_.propagateEvent(event);
        }
    }
}

void ImagePlotProcessor::updateViewport() {
    auto bounds = calcImageBounds(imgDims_);
    viewport_ = ivec4(bounds.pos, bounds.extent.x, bounds.extent.y);
    onStatusChange();
}

void ImagePlotProcessor::updateViewports(size2_t dim, bool force) {
    if (!force && (imgDims_ == dim)) return;  // no changes

    auto bounds = calcImageBounds(dim);
    viewport_ = ivec4(bounds.pos, bounds.extent.x, bounds.extent.y);

    imgDims_ = dim;
    onStatusChange();
}

void ImagePlotProcessor::onStatusChange() {
    // update viewport stored in view manager
    viewManager_.replace(0, viewport_);

    if (imgInport_.isConnected()) {
        ResizeEvent e(size2_t(viewManager_[0].size));
        imgInport_.propagateEvent(&e, imgInport_.getConnectedOutport());
    }
}

ImagePlotProcessor::ImageBounds ImagePlotProcessor::calcImageBounds(const size2_t& dims) const {
    // adjust all margins by axis margin
    const auto padding = axisMargin_.get();
    const ivec2 lowerLeft(margins_.getLeft() + padding, margins_.getBottom() + padding);
    ivec2 upperRight(dims.x - 1 - (margins_.getRight() + padding),
                     dims.y - 1 - (margins_.getTop() + padding));

    // ensure positive extent
    upperRight = glm::max(upperRight, lowerLeft);

    return {lowerLeft, size2_t{upperRight - lowerLeft}};
}

void ImagePlotProcessor::adjustRanges() {
    size2_t dims{100, 100};
    if (imgInport_.hasData()) {
        dims = imgInport_.getData()->getDimensions();
    }

    util::KeepTrueWhileInScope b(&propertyUpdate_);
    switch (rangeMode_.get()) {
        case AxisRangeMode::ImageDims:
            xAxis_.range_.set(dvec2(0.0, dims.x));
            yAxis_.range_.set(dvec2(0.0, dims.y));
            break;
        case AxisRangeMode::Custom:
            xAxis_.range_.set(rangeXaxis_.get());
            yAxis_.range_.set(rangeYaxis_.get());
            break;
        default:
            break;
    }

    customRanges_.setReadOnly(rangeMode_.getSelectedValue() != AxisRangeMode::Custom);
}

}  // namespace plot

}  // namespace inviwo
