/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>                  // for boundingBox
#include <inviwo/core/datastructures/coordinatetransformer.h>   // for StructuredCoordinateTrans...
#include <inviwo/core/datastructures/image/imagetypes.h>        // for ImageType, ImageType::Col...
#include <inviwo/core/datastructures/unitsystem.h>              // for Axis
#include <inviwo/core/interaction/cameratrackball.h>            // for CameraTrackball
#include <inviwo/core/network/networklock.h>                    // for NetworkLock
#include <inviwo/core/ports/imageport.h>                        // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                       // for VolumeInport
#include <inviwo/core/processors/processor.h>                   // for Processor
#include <inviwo/core/processors/processorinfo.h>               // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>              // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>               // for Tags
#include <inviwo/core/properties/boolcompositeproperty.h>       // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>              // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>           // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>              // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>              // for OptionPropertyOption, Opt...
#include <inviwo/core/properties/ordinalproperty.h>             // for FloatProperty
#include <inviwo/core/properties/property.h>                    // for updateDefaultState, Overw...
#include <inviwo/core/properties/propertysemantics.h>           // for PropertySemantics, Proper...
#include <inviwo/core/properties/stringproperty.h>              // for StringProperty
#include <inviwo/core/properties/valuewrapper.h>                // for PropertySerializationMode
#include <inviwo/core/util/formats.h>                           // for DataFloat32
#include <inviwo/core/util/glmmat.h>                            // for dmat4, dmat3
#include <inviwo/core/util/glmutils.h>                          // for Matrix
#include <inviwo/core/util/glmvec.h>                            // for dvec3, dvec2, vec3, dvec4
#include <inviwo/core/util/raiiutils.h>                         // for KeepTrueWhileInScope
#include <inviwo/core/util/staticstring.h>                      // for operator+
#include <inviwo/core/util/zip.h>                               // for enumerate, zipIterator
#include <modules/opengl/texture/textureutils.h>                // for activateAndClearTarget
#include <modules/plotting/datastructures/majorticksettings.h>  // for TickStyle, TickStyle::Out...
#include <modules/plotting/properties/axisproperty.h>           // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>      // for AxisStyleProperty
#include <modules/plotting/properties/plottextproperty.h>       // for PlotTextProperty
#include <modules/plotting/properties/tickproperty.h>           // for MajorTickProperty, MinorT...
#include <modules/plottinggl/utils/axisrenderer.h>              // for AxisRenderer3D

#include <algorithm>         // for max_element
#include <cstddef>           // for size_t
#include <initializer_list>  // for initializer_list
#include <memory>            // for shared_ptr, shared_ptr<>:...
#include <type_traits>       // for remove_extent_t

#include <fmt/core.h>                // for basic_string_view, format
#include <glm/common.hpp>            // for sign
#include <glm/detail/type_vec1.hpp>  // for operator-
#include <glm/geometric.hpp>         // for dot, normalize, cross
#include <glm/mat3x3.hpp>            // for operator*
#include <glm/mat4x4.hpp>            // for operator*
#include <glm/matrix.hpp>            // for inverse, transpose
#include <glm/vec3.hpp>              // for operator*, operator-, vec...
#include <glm/vec4.hpp>              // for operator*, operator+, ope...

namespace inviwo {

namespace util {

struct AxisParams {
    dvec3 start;
    dvec3 stop;
    dvec3 tickDir;
};

std::array<AxisParams, 3> findAxisPositions(dvec3 viewDirection) {
    constexpr dvec3 center = {0.5, 0.5, 0.5};
    constexpr std::array<dvec3, 8> corners = {
        {{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}}};
    struct AI {
        std::array<size_t, 2> idx;
        std::array<dvec3, 2> faceNormals;
    };
    // clang-format off
    constexpr std::array<std::array<AI, 4>, 3> meta =
        {{{
            AI{{0, 4}, {dvec3{0, -1, 0}, dvec3{0, 0, -1}}},
            AI{{1, 5}, {dvec3{0, -1, 0}, dvec3{0, 0,  1}}},
            AI{{2, 6}, {dvec3{0, 0, -1}, dvec3{0, 1,  0}}},
            AI{{3, 7}, {dvec3{0, 1,  0}, dvec3{0, 0,  1}}}
        },{
            AI{{0, 2}, {dvec3{-1, 0, 0}, dvec3{0, 0, -1}}},
            AI{{1, 3}, {dvec3{-1, 0, 0}, dvec3{0, 0,  1}}},
            AI{{4, 6}, {dvec3{0, 0, -1}, dvec3{1, 0,  0}}},
            AI{{5, 7}, {dvec3{1, 0,  0}, dvec3{0, 0,  1}}}
        },{
            AI{{0, 1}, {dvec3{-1, 0, 0}, dvec3{0, -1, 0}}},
            AI{{2, 3}, {dvec3{-1, 0, 0}, dvec3{0,  1, 0}}},
            AI{{4, 5}, {dvec3{0, -1, 0}, dvec3{1,  0, 0}}},
            AI{{6, 7}, {dvec3{1,  0, 0}, dvec3{0,  1, 0}}}
        }}};
    // clang-format on

    const auto onEdge = [&](const AI& edge) {
        return glm::sign(glm::dot(edge.faceNormals[0], viewDirection)) !=
               glm::sign(glm::dot(edge.faceNormals[1], viewDirection));
    };

    const auto dist = [&](const AI& edge) {
        return 0.5 * (glm::dot(corners[edge.idx[0]] - center, viewDirection) +
                      glm::dot(corners[edge.idx[1]] - center, viewDirection));
    };

    const auto tickDir = [&](const AI& edge) -> dvec3 {
        const auto normal = glm::dot(edge.faceNormals[0], viewDirection) > 0 ? edge.faceNormals[0]
                                                                             : edge.faceNormals[1];
        const auto axis = corners[edge.idx[1]] - corners[edge.idx[0]];
        auto tickDir = glm::cross(axis, normal);
        return tickDir * glm::sign(glm::dot(center - corners[edge.idx[1]], tickDir));
    };

    const auto find = [&](const std::array<AI, 4>& axis) -> AxisParams {
        const auto min = *std::max_element(axis.begin(), axis.end(), [&](const AI& a, const AI& b) {
            return (dist(a) + (onEdge(a) ? 0 : -100.0)) < (dist(b) + (onEdge(b) ? 0 : -100.0));
        });

        return {corners[min.idx[0]], corners[min.idx[1]], tickDir(min)};
    };

    return {find(meta[0]), find(meta[1]), find(meta[2])};
}

}  // namespace util

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
    , axisOffset_{"axisOffset", "Axis Offset (%)", 0.1f, 0.0f, 10.0f}
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

    , visibility_{"visibility", "Axis Visibility", true}
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

    addPorts(inport_, imageInport_, outport_);

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

    visibility_.getBoolProperty()->setDisplayName("Automatic");
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
    const auto volume = inport_.getData();
    const dmat4 m = volume->getCoordinateTransformer().getDataToModelMatrix();
    const dmat3 nm = glm::transpose(glm::inverse(m));
    // the mean length of the three basis vectors is used for a relative axis offset (%)
    const double offset =
        axisOffset_.get() / 100 *
        (glm::length(dvec3(m[0])) + glm::length(dvec3(m[1])) + glm::length2(dvec3(m[2]))) / 3.0;

    const auto render = [&](const util::AxisParams& axis, size_t axisIdx) {
        const dvec3 center{0.5, 0.5, 0.5};
        const auto offsetDir =
            glm::normalize(dmat3(m) * glm::normalize(axis.start - center + axis.stop - center));
        const vec3 start{dvec3{m * dvec4(axis.start, 1)} + offset * offsetDir};
        const vec3 stop{dvec3{m * dvec4(axis.stop, 1)} + offset * offsetDir};
        const vec3 tickDir{nm * axis.tickDir};
        axisRenderers_[axisIdx].render(&camera_.get(), dims, start, stop, tickDir);
    };

    if (visibility_.isChecked()) {  // automatic selection
        const auto axes =
            util::findAxisPositions(glm::normalize(camera_.getLookFrom() - camera_.getLookTo()));
        for (auto&& [i, axis] : util::enumerate(axes)) {
            render(axis, i);
        }
    } else {
        constexpr std::array<util::AxisParams, 12> axes{{
            // x axis
            {{0., 0., 0.}, {1., 0., 0.}, {0., 1., 1.}},
            {{0., 1., 0.}, {1., 1., 0.}, {0., -1., 1.}},
            {{0., 1., 1.}, {1., 1., 1.}, {0., -1., -1.}},
            {{0., 0., 1.}, {1., 0., 1.}, {0., 1., -1.}},
            // y axis
            {{0., 0., 0.}, {0., 1., 0.}, {1., 0., 1.}},
            {{1., 0., 0.}, {1., 1., 0.}, {-1., 0., 1.}},
            {{1., 0., 1.}, {1., 1., 1.}, {-1., 0., -1.}},
            {{0., 0., 1.}, {0., 1., 1.}, {1., 0., -1.}},
            // z axis
            {{0., 0., 0.}, {0., 0., 1.}, {1., 1., 0.}},
            {{1., 0., 0.}, {1., 0., 1.}, {-1., 1., 0.}},
            {{1., 1., 0.}, {1., 1., 1.}, {-1., -1., 0.}},
            {{0., 1., 0.}, {0., 1., 1.}, {1., -1., 0.}},
        }};
        for (auto&& [i, axis] : util::enumerate(axes)) {
            if (!visibleAxes_[i].get()) continue;
            render(axis, i / 4);
        }
    }

    utilgl::deactivateCurrentTarget();
}

void VolumeAxis::adjustRanges() {
    dvec3 volDims(1.0);
    dvec3 offset(0.0);
    dvec3 basisLen(1.0);

    std::string xCaption{"x"};
    std::string yCaption{"y"};
    std::string zCaption{"z"};

    if (auto volume = inport_.getData()) {
        volDims = dvec3(volume->getDimensions());
        for (size_t i = 0; i < 3; ++i) {
            basisLen[i] = glm::length(volume->getBasis()[i]);
        }
        offset = volume->getOffset();

        xCaption = fmt::format("{}{: [}", volume->axes[0].name, volume->axes[0].unit);
        yCaption = fmt::format("{}{: [}", volume->axes[1].name, volume->axes[1].unit);
        zCaption = fmt::format("{}{: [}", volume->axes[2].name, volume->axes[2].unit);
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

    util::updateDefaultState(xAxis_.captionSettings_.title_, xCaption, util::OverwriteState::No);
    util::updateDefaultState(yAxis_.captionSettings_.title_, yCaption, util::OverwriteState::No);
    util::updateDefaultState(zAxis_.captionSettings_.title_, zCaption, util::OverwriteState::No);

    customRanges_.setVisible(rangeMode_.getSelectedValue() == AxisRangeMode::Custom);
}

}  // namespace plot

}  // namespace inviwo
