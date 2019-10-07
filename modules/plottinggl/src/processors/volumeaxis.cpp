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

#include <modules/plottinggl/processors/volumeaxis.h>

#include <modules/plotting/utils/axisutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeAxis::processorInfo_{
    "org.inviwo.VolumeAxis",  // Class identifier
    "Volume Axis",            // Display name
    "Plotting",               // Category
    CodeState::Stable,        // Code state
    "GL, Plotting",           // Tags
};

const ProcessorInfo VolumeAxis::getProcessorInfo() const { return processorInfo_; }

VolumeAxis::VolumeAxis()
    : Processor()
    , inport_("volume")
    , imageInport_("imageInport")
    , outport_("outport")
    , axisOffset_("axisOffset", "Axis Offset", 0.1f, 0.0f, 10.0f)
    , rangeMode_("rangeMode", "Axis Range Mode",
                 {{"dims", "Volume Dimensions (voxel)", AxisRangeMode::VolumeDims},
                  {"basis", "Volume Basis", AxisRangeMode::VolumeBasis},
                  {"basisOffset", "Volume Basis & Offset", AxisRangeMode::VolumeBasisOffset},
                  {"custom", "Custom", AxisRangeMode::Custom}})
    , customRanges_("customRanges", "Custom Ranges")
    , rangeXaxis_("rangeX", "X Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max())
    , rangeYaxis_("rangeY", "Y Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max())
    , rangeZaxis_("rangeZ", "Z Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max())
    , axisStyle_("axisStyle", "Global Axis Style")
    , xAxis_("xAxis", "X Axis")
    , yAxis_("yAxis", "Y Axis")
    , zAxis_("zAxis", "Z Axis")
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , axisRenderers_({{xAxis_, yAxis_, zAxis_}})
    , propertyUpdate_(false) {
    imageInport_.setOptional(true);
    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);

    addProperty(axisOffset_);
    addProperty(rangeMode_);

    customRanges_.addProperty(rangeXaxis_);
    customRanges_.addProperty(rangeYaxis_);
    customRanges_.addProperty(rangeZaxis_);
    customRanges_.setCollapsed(true);
    addProperty(customRanges_);

    rangeXaxis_.setSemantics(PropertySemantics::Text);
    rangeYaxis_.setSemantics(PropertySemantics::Text);
    rangeZaxis_.setSemantics(PropertySemantics::Text);

    xAxis_.setCaption("x");
    yAxis_.setCaption("y");
    zAxis_.setCaption("z");

    axisStyle_.setCollapsed(true);
    axisStyle_.registerProperties(xAxis_, yAxis_, zAxis_);
    addProperties(axisStyle_, xAxis_, yAxis_, zAxis_);

    addProperty(camera_);
    addProperty(trackball_);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    // initialize axes
    const float majorTick = 0.3f;
    const float minorTick = 0.15f;
    for (auto property : {&xAxis_, &yAxis_, &zAxis_}) {
        property->captionSettings_.offset_.set(0.7f);
        property->captionSettings_.position_.set(0.5f);
        property->labelSettings_.offset_.set(0.7f);

        property->majorTicks_.tickLength_.set(majorTick);
        property->majorTicks_.tickWidth_.set(1.5f);
        property->majorTicks_.style_.set(TickStyle::Outside);
        property->majorTicks_.setCurrentStateAsDefault();

        property->minorTicks_.tickLength_.set(minorTick);
        property->minorTicks_.tickWidth_.set(1.3f);
        property->minorTicks_.style_.set(TickStyle::Outside);
        property->minorTicks_.setCurrentStateAsDefault();
    }

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
    zAxis_.range_.onChange(linkAxisRanges(zAxis_.range_, rangeZaxis_));
    rangeXaxis_.onChange(linkAxisRanges(rangeXaxis_, xAxis_.range_));
    rangeYaxis_.onChange(linkAxisRanges(rangeYaxis_, yAxis_.range_));
    rangeZaxis_.onChange(linkAxisRanges(rangeZaxis_, zAxis_.range_));

    // adjust axis ranges when input volume, i.e. its basis, changes
    inport_.onChange([this]() { adjustRanges(); });
    // sync ranges when custom range is enabled or disabled
    rangeMode_.onChange([this]() { adjustRanges(); });

    customRanges_.setVisible(rangeMode_.getSelectedValue() == AxisRangeMode::Custom);
}

void VolumeAxis::process() {
    if (imageInport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    const auto dims = outport_.getDimensions();

    auto volume = inport_.getData();

    const auto offset = volume->getOffset();
    const auto basis = volume->getBasis();

    // x axis
    vec3 tickDir(0.0f, 1.0f, 1.0f);
    vec3 origin = offset - glm::normalize(tickDir) * axisOffset_.get();
    axisRenderers_[0].render(&camera_.get(), dims, origin, origin + basis[0], tickDir);
    // y axis
    tickDir = vec3(1.0f, 0.0f, 1.0f);
    origin = offset - glm::normalize(tickDir) * axisOffset_.get();
    axisRenderers_[1].render(&camera_.get(), dims, origin, origin + basis[1], tickDir);
    // z axis
    tickDir = vec3(1.0f, 1.0f, 0.0f);
    origin = offset - glm::normalize(tickDir) * axisOffset_.get();
    axisRenderers_[2].render(&camera_.get(), dims, origin, origin + basis[2], tickDir);

    utilgl::deactivateCurrentTarget();
}

void VolumeAxis::adjustRanges() {
    dvec3 volDims(1.0);
    dvec3 offset(0.0);
    dvec3 basisLen(1.0);
    auto volume = inport_.getData();
    if (volume) {
        volDims = dvec3(volume->getDimensions());
        for (size_t i = 0; i < 3; ++i) {
            basisLen[i] = glm::length(volume->getBasis()[i]);
        }
        offset = volume->getOffset();
    }

    util::KeepTrueWhileInScope b(&propertyUpdate_);
    switch (rangeMode_.get()) {
        case AxisRangeMode::VolumeDims:
            xAxis_.range_.set(dvec2(0.0, volDims.x));
            yAxis_.range_.set(dvec2(0.0, volDims.y));
            zAxis_.range_.set(dvec2(0.0, volDims.z));
            break;
        case AxisRangeMode::VolumeBasis:
            xAxis_.range_.set(dvec2(0.0, basisLen.x));
            yAxis_.range_.set(dvec2(0.0, basisLen.y));
            zAxis_.range_.set(dvec2(0.0, basisLen.z));
            break;
        case AxisRangeMode::VolumeBasisOffset:
            xAxis_.range_.set(dvec2(offset.x, offset.x + basisLen.x));
            yAxis_.range_.set(dvec2(offset.y, offset.y + basisLen.y));
            zAxis_.range_.set(dvec2(offset.z, offset.z + basisLen.z));
            break;
        case AxisRangeMode::Custom:
            xAxis_.range_.set(rangeXaxis_.get());
            yAxis_.range_.set(rangeYaxis_.get());
            zAxis_.range_.set(rangeZaxis_.get());
            break;
        default:
            break;
    }

    customRanges_.setVisible(rangeMode_.getSelectedValue() == AxisRangeMode::Custom);
}

}  // namespace plot

}  // namespace inviwo
