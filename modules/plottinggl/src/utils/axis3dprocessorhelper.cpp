/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/plottinggl/utils/axis3dprocessorhelper.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glm.h>
#include <modules/opengl/texture/textureutils.h>

#include <fmt/core.h>

namespace inviwo {

namespace {

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

}  // namespace

namespace plot {

Axis3DProcessorHelper::Axis3DProcessorHelper(Processor& owner,
                                             std::function<std::optional<mat4>()> getBoundingBox,
                                             DimsRangeMode useDimsRange)
    : offsetScaling_{"offsetScaling",
                     "Offset Scaling",
                     R"(Offset scaling affects tick lengths and offsets of axis captions and labels.
                        + **None** No scaling, offsets and lengths are given in world coordinates.
                        + **Min** Relative scaling based on the shortest extent of the bounding box. Useful 
                                when visualizing growing data.
                        + **Max** Relative scaling based on the longest extent of the bounding box. Useful 
                                when visualizing shrinking data.
                        + **Mean** Relative scaling basd on the mean bounding box extent.
                        + **Diagonal** Relative scaling based on the diagonal of the bounding box.
                      )"_unindentHelp,
                     {{"none", "None (absolute World coordinates)", OffsetScaling::None},
                      {"min", "Min Extent", OffsetScaling::MinExtent},
                      {"max", "Max Extent", OffsetScaling::MaxExtent},
                      {"mean", "Mean Extent", OffsetScaling::MeanExtent},
                      {"diagonal", "Bounding Box Diagonal", OffsetScaling::Diagonal}},
                     1}
    , axisOffset_{"axisOffset", "Axis Offset",
                  util::ordinalLength(10.0f, 50.0f)
                      .set(
                          "Offset between each axis and the data considering the Offset Scaling mode"_help)}
    , rangeMode_{"rangeMode", "Axis Range Mode", "Determines axis ranges"_help}
    , customRanges_{"customRanges", "Custom Ranges"}
    , rangeXaxis_{"rangeX", "X Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max()}
    , rangeYaxis_{"rangeY", "Y Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max()}
    , rangeZaxis_{"rangeZ", "Z Axis", 0.0, 1.0, DataFloat32::lowest(), DataFloat32::max()}
    , visibility_{"visibility", "Axis Visibility",
                  "Visibility of all available axes (default: all axis start at the origin)"_help,
                  true}
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
    , xAxis_{"xAxis", "X Axis", "Axis properties for x"_help, AxisSettings::Orientation::Horizontal,
             false}
    , yAxis_{"yAxis", "Y Axis", "Axis properties for y"_help, AxisSettings::Orientation::Horizontal,
             false}
    , zAxis_{"zAxis", "Z Axis", "Axis properties for y"_help, AxisSettings::Orientation::Horizontal,
             false}
    , camera_{"camera", "Camera", std::move(getBoundingBox)}
    , trackball_{&camera_}
    , axisRenderers_{xAxis_, yAxis_, zAxis_}
    , propertyUpdate_{false} {

    rangeXaxis_.setSemantics(PropertySemantics::Text);
    rangeYaxis_.setSemantics(PropertySemantics::Text);
    rangeZaxis_.setSemantics(PropertySemantics::Text);

    xAxis_.setCaption("x");
    yAxis_.setCaption("y");
    zAxis_.setCaption("z");

    std::vector<OptionPropertyOption<AxisRangeMode>> rangeOptions = {
        {"basis", "Basis", AxisRangeMode::Basis},
        {"basisOffset", "Basis & Offset", AxisRangeMode::BasisOffset},
        {"world", "Model & World Transform", AxisRangeMode::World},
        {"custom", "Custom", AxisRangeMode::Custom}};
    if (useDimsRange == DimsRangeMode::Yes) {
        rangeOptions.insert(rangeOptions.begin(),
                            {"dims", "Volume Dimensions (voxel)", AxisRangeMode::Dims});
    }
    rangeMode_.replaceOptions(rangeOptions);

    customRanges_.addProperties(rangeXaxis_, rangeYaxis_, rangeZaxis_);
    customRanges_.setCollapsed(true);

    presets_.setSerializationMode(PropertySerializationMode::None);
    presets_.onChange([&]() {
        NetworkLock lock(&owner);
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
    owner.addProperties(offsetScaling_, axisOffset_, rangeMode_, customRanges_, visibility_,
                        axisStyle_, xAxis_, yAxis_, zAxis_, camera_, trackball_);

    axisStyle_.setCollapsed(true);
    visibility_.setCollapsed(true);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    // initialize axes
    for (auto property : {&xAxis_, &yAxis_, &zAxis_}) {
        property->majorTicks_.tickWidth_.set(1.5f);
        property->majorTicks_.style_.set(TickStyle::Outside);

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

    // adjust scaling factor for label offsets and tick lengths
    // offsetScaling_.onChange([&]() { adjustScalingFactor(); });

    //// adjust axis ranges when input volume, i.e. its basis, changes
    // inport_.onChange([&]() {
    //     adjustScalingFactor();
    //     adjustRanges();
    // });
    //// sync ranges when custom range is enabled or disabled
    // rangeMode_.onChange([this]() { adjustRanges(); });

    customRanges_.setVisible(rangeMode_.getSelectedValue() == AxisRangeMode::Custom);

    owner.setAllPropertiesCurrentStateAsDefault();
}

void Axis3DProcessorHelper::renderAxes(size2_t outputDims, const SpatialEntity& entity) {

    const dmat4 m = entity.getCoordinateTransformer().getDataToWorldMatrix();
    const dmat3 nm = glm::transpose(glm::inverse(m));
    // the mean length of the three basis vectors is used for a relative axis offset (%)
    const double offset = axisOffset_.get() * xAxis_.getScalingFactor();

    const auto render = [&](const AxisParams& axis, size_t axisIdx) {
        const dvec3 center{0.5, 0.5, 0.5};
        const auto offsetDir =
            glm::normalize(dmat3(m) * glm::normalize(axis.start - center + axis.stop - center));
        const vec3 start{dvec3{m * dvec4(axis.start, 1)} + offset * offsetDir};
        const vec3 stop{dvec3{m * dvec4(axis.stop, 1)} + offset * offsetDir};
        const vec3 tickDir{nm * axis.tickDir};
        axisRenderers_[axisIdx].render(&camera_.get(), outputDims, start, stop, tickDir);
    };

    if (visibility_.isChecked()) {  // automatic selection
        // transform camera to data space since findAxisPositions uses a uniform cube centered at
        // the origin to determine the visible faces
        const mat4 trafo{entity.getCoordinateTransformer().getWorldToDataMatrix()};
        const auto axes =
            findAxisPositions(glm::normalize(vec3(trafo * vec4(camera_.getLookFrom(), 1.0f)) -
                                             vec3(trafo * vec4(camera_.getLookTo(), 1.0f))));
        for (auto&& [i, axis] : util::enumerate(axes)) {
            render(axis, i);
        }
    } else {
        constexpr std::array<AxisParams, 12> axes{{
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
}

void Axis3DProcessorHelper::adjustScalingFactor(const SpatialEntity* entity) {
    if (entity) {
        const mat4 m{entity->getCoordinateTransformer().getDataToWorldMatrix()};

        float factor = [l = vec3{glm::length(m[0]), glm::length(m[1]), glm::length(m[2])}, &m,
                        mode = offsetScaling_.get()]() {
            switch (mode) {
                case OffsetScaling::MinExtent:
                    return glm::compMin(l) / 100.0f;
                case OffsetScaling::MaxExtent:
                    return glm::compMax(l) / 100.0f;
                case OffsetScaling::MeanExtent:
                    return glm::compAdd(l) / (3.0f * 100.0f);
                case OffsetScaling::Diagonal:
                    return glm::length(m[0] + m[1] + m[2]) / 100.0f;
                case OffsetScaling::None:
                default:
                    return 1.0f;
            }
        }();
        xAxis_.scalingFactor_.set(factor);
        yAxis_.scalingFactor_.set(factor);
        zAxis_.scalingFactor_.set(factor);
    } else {
        xAxis_.scalingFactor_.set(1.0f);
        yAxis_.scalingFactor_.set(1.0f);
        zAxis_.scalingFactor_.set(1.0f);
    }
}

void Axis3DProcessorHelper::adjustRanges(const SpatialEntity* entity) {
    dvec3 volDims(1.0);
    dvec3 offset(0.0);
    dvec3 basisLen(1.0);
    mat4 worldTrafo{1.0f};

    if (entity) {
        for (size_t i = 0; i < 3; ++i) {
            basisLen[i] = glm::length(entity->getBasis()[i]);
        }
        offset = entity->getOffset();

        worldTrafo = entity->getCoordinateTransformer().getDataToWorldMatrix();
    }

    std::string xCaption{"x"};
    std::string yCaption{"y"};
    std::string zCaption{"z"};

    if (auto volume = dynamic_cast<const Volume*>(entity)) {
        volDims = dvec3(volume->getDimensions());

        xCaption = fmt::format("{}{: [}", volume->axes[0].name, volume->axes[0].unit);
        yCaption = fmt::format("{}{: [}", volume->axes[1].name, volume->axes[1].unit);
        zCaption = fmt::format("{}{: [}", volume->axes[2].name, volume->axes[2].unit);
    }

    util::KeepTrueWhileInScope b(&propertyUpdate_);
    switch (rangeMode_.get()) {
        case AxisRangeMode::Dims:
            xAxis_.range_.set(dvec2(0.0, volDims.x));
            yAxis_.range_.set(dvec2(0.0, volDims.y));
            zAxis_.range_.set(dvec2(0.0, volDims.z));
            break;
        case AxisRangeMode::Basis:
            xAxis_.range_.set(dvec2(0.0, basisLen.x));
            yAxis_.range_.set(dvec2(0.0, basisLen.y));
            zAxis_.range_.set(dvec2(0.0, basisLen.z));
            break;
        case AxisRangeMode::BasisOffset:
            xAxis_.range_.set(dvec2(offset.x, offset.x + basisLen.x));
            yAxis_.range_.set(dvec2(offset.y, offset.y + basisLen.y));
            zAxis_.range_.set(dvec2(offset.z, offset.z + basisLen.z));
            break;
        case AxisRangeMode::World:
            xAxis_.range_.set(
                dvec2(worldTrafo[3].x, worldTrafo[3].x + glm::length(vec3(worldTrafo[0]))));
            yAxis_.range_.set(
                dvec2(worldTrafo[3].y, worldTrafo[3].y + glm::length(vec3(worldTrafo[1]))));
            zAxis_.range_.set(
                dvec2(worldTrafo[3].z, worldTrafo[3].z + glm::length(vec3(worldTrafo[2]))));
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
