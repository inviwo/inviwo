/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2020 Inviwo Foundation
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
#include <inviwo/core/network/networklock.h>

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
    , inport_{"volume"}
    , imageInport_{"imageInport"}
    , outport_{"outport"}
    , axisOffset_{"axisOffset", "Axis Offset", 0.1f, 0.0f, 10.0f}
    , rangeMode_{"rangeMode",
                 "Axis Range Mode",
                 {{"dims", "Volume Dimensions (voxel)", AxisRangeMode::VolumeDims},
                  {"basis", "Volume Basis", AxisRangeMode::VolumeBasis},
                  {"basisOffset", "Volume Basis & Offset", AxisRangeMode::VolumeBasisOffset},
                  {"custom", "Custom", AxisRangeMode::Custom}}}
    , customRanges_{"customRanges", "Custom Ranges"}
    , rangeXaxis_{"rangeX", "X Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max()}
    , rangeYaxis_{"rangeY", "Y Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max()}
    , rangeZaxis_{"rangeZ", "Z Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max()}

    , visibility_{"visibility", "Axis Visibility"}
    , presets_{"visibilityPresets",
               "Presets",
               {{"default", "Default (origin)", "default"},
                {"all", "All", "all"},
                {"none", "None", "none"}}}
    , visibleAxes_{{
          // x axis
          {"negYnegZ", "X -Y-Z", true},
          {"posYnegZ", "X +Y-Z", false},
          {"posYposZ", "X +Y+Z", false},
          {"negYposZ", "X -Y+Z", false},
          // y axis
          {"negXnegZ", "Y -X-Z", true},
          {"posXnegZ", "Y +X-Z", false},
          {"posXposZ", "Y +X+Z", false},
          {"negXposZ", "Y -X+Z", false},
          // z axis
          {"negXnegY", "Z -X-Y", true},
          {"posXnegY", "Z +X-Y", false},
          {"posXposY", "Z +X+Y", false},
          {"negXposY", "Z -X+Y", false},
      }}
    , axisStyle_{"axisStyle", "Global Axis Style"}
    , xAxis_{"xAxis", "X Axis"}
    , yAxis_{"yAxis", "Y Axis"}
    , zAxis_{"zAxis", "Z Axis"}
    , camera_{"camera", "Camera", util::boundingBox(inport_)}
    , trackball_{&camera_}
    , axisRenderers_{{xAxis_, yAxis_, zAxis_}}
    , propertyUpdate_{false} {
    imageInport_.setOptional(true);
    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);

    rangeXaxis_.setSemantics(PropertySemantics::Text);
    rangeYaxis_.setSemantics(PropertySemantics::Text);
    rangeZaxis_.setSemantics(PropertySemantics::Text);

    xAxis_.setCaption("x");
    yAxis_.setCaption("y");
    zAxis_.setCaption("z");

    customRanges_.addProperties(rangeXaxis_, rangeYaxis_, rangeZaxis_);
    customRanges_.setCollapsed(true);

    presets_.setSerializationMode(PropertySerializationMode::None);
    presets_.onChange([&]() {
        NetworkLock lock(this);
        if (presets_.getSelectedValue() == "all") {
            for (auto& p : visibleAxes_) {
                p.set(true);
            }
        } else {
            for (auto& p : visibleAxes_) {
                p.set(false);
            }
            if (presets_.getSelectedValue() == "default") {
                visibleAxes_[0].set(true);
                visibleAxes_[4].set(true);
                visibleAxes_[8].set(true);
            }
        }
    });

    visibility_.addProperty(presets_);
    for (auto& p : visibleAxes_) {
        visibility_.addProperty(p);
    }

    axisStyle_.registerProperties(xAxis_, yAxis_, zAxis_);
    addProperties(axisOffset_, rangeMode_, customRanges_, visibility_, axisStyle_, xAxis_, yAxis_,
                  zAxis_, camera_, trackball_);

    axisStyle_.setCollapsed(true);
    visibility_.setCollapsed(true);
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
        
        property->minorTicks_.tickLength_.set(minorTick);
        property->minorTicks_.tickWidth_.set(1.3f);
        property->minorTicks_.style_.set(TickStyle::Outside);
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

    setAllPropertiesCurrentStateAsDefault();
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

    struct AxisParams {
        vec3 axisoffset;
        vec3 tickdir;
    };

    std::array<AxisParams, 12> lookup{{
        // x axis
        {{0.f, 0.f, 0.f}, {0.f, 1.f, 1.f}},
        {{basis[1]}, {0.f, -1.f, 1.f}},
        {{basis[1] + basis[2]}, {0.f, -1.f, -1.f}},
        {{basis[2]}, {0.f, 1.f, -1.f}},
        // y axis
        {{0.f, 0.f, 0.f}, {1.f, 0.f, 1.f}},
        {{basis[0]}, {-1.f, 0.f, 1.f}},
        {{basis[0] + basis[2]}, {-1.f, 0.f, -1.f}},
        {{basis[2]}, {1.f, 0.f, -1.f}},
        // z axis
        {{0.f, 0.f, 0.f}, {1.f, 1.f, 0.f}},
        {{basis[0]}, {-1.f, 1.f, 0.f}},
        {{basis[0] + basis[1]}, {-1.f, -1.f, 0.f}},
        {{basis[1]}, {1.f, -1.f, 0.f}},
    }};

    for (size_t i = 0; i < visibleAxes_.size(); ++i) {
        if (!visibleAxes_[i].get()) continue;
        const size_t axis = i / 4;
        vec3 origin =
            offset + lookup[i].axisoffset - glm::normalize(lookup[i].tickdir) * axisOffset_.get();
        axisRenderers_[axis].render(&camera_.get(), dims, origin, origin + basis[axis],
                                    lookup[i].tickdir);
    }

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
