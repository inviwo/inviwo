/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glm.h>

#include <modules/plotting/utils/labelscaling.h>

#include <fmt/core.h>
#include <algorithm>
#include <ranges>

namespace inviwo::plot {

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
            AI{.idx={0, 4}, .faceNormals={dvec3{0, -1, 0}, dvec3{0, 0, -1}}},
            AI{.idx={1, 5}, .faceNormals={dvec3{0, -1, 0}, dvec3{0, 0,  1}}},
            AI{.idx={2, 6}, .faceNormals={dvec3{0, 0, -1}, dvec3{0, 1,  0}}},
            AI{.idx={3, 7}, .faceNormals={dvec3{0, 1,  0}, dvec3{0, 0,  1}}}
        },{
            AI{.idx={0, 2}, .faceNormals={dvec3{-1, 0, 0}, dvec3{0, 0, -1}}},
            AI{.idx={1, 3}, .faceNormals={dvec3{-1, 0, 0}, dvec3{0, 0,  1}}},
            AI{.idx={4, 6}, .faceNormals={dvec3{0, 0, -1}, dvec3{1, 0,  0}}},
            AI{.idx={5, 7}, .faceNormals={dvec3{1, 0,  0}, dvec3{0, 0,  1}}}
        },{
            AI{.idx={0, 1}, .faceNormals={dvec3{-1, 0, 0}, dvec3{0, -1, 0}}},
            AI{.idx={2, 3}, .faceNormals={dvec3{-1, 0, 0}, dvec3{0,  1, 0}}},
            AI{.idx={4, 5}, .faceNormals={dvec3{0, -1, 0}, dvec3{1,  0, 0}}},
            AI{.idx={6, 7}, .faceNormals={dvec3{1,  0, 0}, dvec3{0,  1, 0}}}
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
        auto dir = glm::cross(axis, normal);
        return dir * glm::sign(glm::dot(center - corners[edge.idx[1]], dir));
    };

    const auto find = [&](const std::array<AI, 4>& axis) -> AxisParams {
        const auto& min = *std::ranges::max_element(axis, [&](const AI& a, const AI& b) {
            return (dist(a) + (onEdge(a) ? 0 : -100.0)) < (dist(b) + (onEdge(b) ? 0 : -100.0));
        });

        return {.start = corners[min.idx[0]], .stop = corners[min.idx[1]], .tickDir = tickDir(min)};
    };

    return {find(meta[0]), find(meta[1]), find(meta[2])};
}

auto rangeOptions(bool hasDims, bool hasBoundingBox) {
    using AxisRangeMode = Axis3DProcessorHelper::AxisRangeMode;
    using enum AxisRangeMode;
    std::vector<OptionPropertyOption<AxisRangeMode>> rangeOptions;
    if (hasDims) {
        rangeOptions.emplace_back("dims", "Volume Dimensions (voxel)", Dims);
    }
    rangeOptions.emplace_back("basis", "Basis", Basis);
    rangeOptions.emplace_back("basisOffset", "Basis & Offset", BasisOffset);
    rangeOptions.emplace_back("world", "Model & World Transform", World);
    if (hasBoundingBox) {
        rangeOptions.emplace_back("dataBoundingBox", "Data Bounding Box", DataBoundingBox);
        rangeOptions.emplace_back("modelBoundingBox", "Model Bounding Box", ModelBoundingBox);
        rangeOptions.emplace_back("boundingBox", "World Bounding Box", WorldBoundingBox);
    }
    return rangeOptions;
}

}  // namespace

Axis3DProcessorHelper::Axis3DProcessorHelper(std::function<std::optional<mat4>()> getBoundingBox,
                                             DimsRangeMode useDimsRange)
    : offsetScaling_{"offsetScaling",
                     "Offset Scaling",
                     R"(Offset scaling affects tick lengths and offsets of axis captions and labels.
                        + **None**     No scaling, offsets and lengths are given in world 
                                       coordinates.
                        + **Min**      Relative scaling based on the shortest extent of the bounding
                                       box. Useful when visualizing growing data.
                        + **Max**      Relative scaling based on the longest extent of the bounding
                                       box. Useful when visualizing shrinking data.
                        + **Mean**     Relative scaling basd on the mean bounding box extent.
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
                      .set("Offset between each axis and the data "
                           "considering the Offset Scaling mode"_help)}
    , rangeMode_{"rangeMode", "Axis Range Mode", "Determines axis ranges"_help,
                 rangeOptions(useDimsRange == DimsRangeMode::Yes, getBoundingBox != nullptr)}

    , captionType_("captionType", "Caption Type", captionTypeState())
    , customCaption_("customCaption", "Custom Caption", "{n}{u: [}")
    , labelScale_{"labelScale", "Label Scaling", labelScaleState()}

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
    , xAxis_{"xAxis", "X Axis", "Axis properties for x"_help, AxisProperty::Orientation::Horizontal}
    , yAxis_{"yAxis", "Y Axis", "Axis properties for y"_help, AxisProperty::Orientation::Horizontal}
    , zAxis_{"zAxis", "Z Axis", "Axis properties for y"_help, AxisProperty::Orientation::Horizontal}
    , camera_{"camera", "Camera", getBoundingBox}
    , trackball_{&camera_}
    , axisRenderers_{AxisData{}, AxisData{}, AxisData{}}
    , propertyUpdate_{false}
    , getBoundingBox_{getBoundingBox ? std::optional{getBoundingBox} : std::nullopt} {

    presets_.setSerializationMode(PropertySerializationMode::None);
    presets_.onChange([this]() {
        const NetworkLock lock(&visibility_);
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
    axisStyle_.setCollapsed(true);
    visibility_.setCollapsed(true);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    // initialize axes
    xAxis_.setCaption("x");
    yAxis_.setCaption("y");
    zAxis_.setCaption("z");
    for (auto* property : {&xAxis_, &yAxis_, &zAxis_}) {
        property->majorTicks_.width.set(1.5f);
        property->majorTicks_.style.set(TickData::Style::Outside);

        property->minorTicks_.width.set(1.3f);
        property->minorTicks_.style.set(TickData::Style::Outside);

        // Treat the range as a pure output property.
        property->range_.setInvalidationLevel(InvalidationLevel::Valid);

        property->setCurrentStateAsDefault();
    }
}

void Axis3DProcessorHelper::renderAxes(size2_t outputDims, const SpatialEntity& entity) {
    const auto d2w = getDataToWorldMatrix(entity);
    const auto nD2W = dmat3{glm::transpose(glm::inverse(d2w))};

    const auto scale = scalingFactor(&entity);
    const double offset = axisOffset_.get() * scale;
    const auto autoScale = labelScaleStep(labelScale_.get());

    for (auto [i, ap, ar, range] :
         std::views::zip(std::views::iota(0, 3), std::to_array({&xAxis_, &yAxis_, &zAxis_}),
                         axisRenderers_, axisRanges(entity))) {

        auto [scaledRange, exponent] = scaleRange(range, autoScale);
        if (ap->overrideRange_) {
            scaledRange = ap->customRange_;
            exponent = 0;
        }

        auto& data = ar.getData();
        data.range = scaledRange;
        data.visible = ap->isChecked();
        data.mirrored = ap->mirrored_.get();
        data.color = ap->color_.get();
        data.width = ap->width_.get();
        data.caption = ap->captionSettings_.title_.get();
        ap->captionSettings_.update(data.captionSettings);

        updateLabelPositions(data.majorPositions, data.minorPositions, ap->labelingAlgorithm_.get(),
                             data.range, ap->majorTicks_.numberOfTicks.get(),
                             ap->minorTicks_.frequency.get(), ap->minorTicks_.fillAxis.get());

        ap->labelSettings_.update(data.labelSettings);
        updateLabels(data.labels, data.majorPositions, ap->labelSettings_.title_.get());

        ap->majorTicks_.update(data.major);
        ap->minorTicks_.update(data.minor);

        data.major.length *= scale;
        data.minor.length *= scale;
        data.labelSettings.offset.x *= scale;
        data.captionSettings.offset.x *= scale;

        if (entity.getAxis(i)) {
            data.caption =
                formatAxisCaption(*entity.getAxis(i), captionType_.get(), labelScale_.get(),
                                  customCaption_.get(), exponent, data.caption);
        }

        // Update axis range
        ap->range_.set(scaledRange);
    }

    const auto render = [&](const AxisParams& axis, size_t axisIdx) {
        const dvec3 center{0.5, 0.5, 0.5};
        const auto offsetDir =
            glm::normalize(dmat3{d2w} * glm::normalize(axis.start - center + axis.stop - center));
        const vec3 start{dvec3{d2w * dvec4(axis.start, 1)} + offset * offsetDir};
        const vec3 stop{dvec3{d2w * dvec4(axis.stop, 1)} + offset * offsetDir};
        const vec3 tickDir{nD2W * axis.tickDir};
        axisRenderers_[axisIdx].render(&camera_.get(), outputDims, start, stop, tickDir);
    };

    if (visibility_.isChecked()) {  // automatic selection
        // transform camera to data space since findAxisPositions uses a uniform cube centered at
        // the origin to determine the visible faces
        const mat4 trafo{glm::inverse(d2w)};
        const auto axes =
            findAxisPositions(glm::normalize(vec3(trafo * vec4(camera_.getLookFrom(), 1.0f)) -
                                             vec3(trafo * vec4(camera_.getLookTo(), 1.0f))));
        for (auto&& [i, axis] : util::enumerate(axes)) {
            render(axis, i);
        }
    } else {
        constexpr std::array<AxisParams, 12> axes{{
            // x axis
            {.start = {0., 0., 0.}, .stop = {1., 0., 0.}, .tickDir = {0., 1., 1.}},
            {.start = {0., 1., 0.}, .stop = {1., 1., 0.}, .tickDir = {0., -1., 1.}},
            {.start = {0., 1., 1.}, .stop = {1., 1., 1.}, .tickDir = {0., -1., -1.}},
            {.start = {0., 0., 1.}, .stop = {1., 0., 1.}, .tickDir = {0., 1., -1.}},
            // y axis
            {.start = {0., 0., 0.}, .stop = {0., 1., 0.}, .tickDir = {1., 0., 1.}},
            {.start = {1., 0., 0.}, .stop = {1., 1., 0.}, .tickDir = {-1., 0., 1.}},
            {.start = {1., 0., 1.}, .stop = {1., 1., 1.}, .tickDir = {-1., 0., -1.}},
            {.start = {0., 0., 1.}, .stop = {0., 1., 1.}, .tickDir = {1., 0., -1.}},
            // z axis
            {.start = {0., 0., 0.}, .stop = {0., 0., 1.}, .tickDir = {1., 1., 0.}},
            {.start = {1., 0., 0.}, .stop = {1., 0., 1.}, .tickDir = {-1., 1., 0.}},
            {.start = {1., 1., 0.}, .stop = {1., 1., 1.}, .tickDir = {-1., -1., 0.}},
            {.start = {0., 1., 0.}, .stop = {0., 1., 1.}, .tickDir = {1., -1., 0.}},
        }};
        for (auto&& [i, axis] : util::enumerate(axes)) {
            if (!visibleAxes_[i].get()) continue;
            render(axis, i / 4);
        }
    }
}

float Axis3DProcessorHelper::scalingFactor(const SpatialEntity* entity) const {
    if (entity) {
        const mat4 m{getDataToWorldMatrix(*entity)};
        const auto l = vec3{glm::length(m[0]), glm::length(m[1]), glm::length(m[2])};
        switch (offsetScaling_.get()) {
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
    } else {
        return 1.0f;
    }
}

namespace {
std::array<dvec2, 3> getRangeDims(const SpatialEntity& entity) {
    dvec3 volDims(1.0);
    if (const auto* grid2d = dynamic_cast<const StructuredGridEntity<2>*>(&entity)) {
        volDims = dvec3(grid2d->getDimensions(), 0.0);
    } else if (const auto* grid3d = dynamic_cast<const StructuredGridEntity<3>*>(&entity)) {
        volDims = dvec3(grid3d->getDimensions());
    }
    return {dvec2{0.0, volDims.x}, dvec2{0.0, volDims.y}, dvec2{0.0, volDims.z}};
}

std::array<dvec2, 3> getRangeBasis(const SpatialEntity& entity) {
    dvec3 basisLen(1.0);
    for (size_t i = 0; i < 3; ++i) {
        basisLen[i] = glm::length(entity.getBasis()[i]);
    }
    return {dvec2{0.0, basisLen.x}, dvec2{0.0, basisLen.y}, dvec2{0.0, basisLen.z}};
}

std::array<dvec2, 3> getRangeBasisOffset(const SpatialEntity& entity) {
    const auto offset = dvec3{entity.getOffset()};
    const auto ranges = getRangeBasis(entity);
    return {ranges[0] + offset.x, ranges[1] + offset.y, ranges[2] + offset.z};
}

std::array<dvec2, 3> getRangeWorld(const SpatialEntity& entity) {
    const auto d2w = dmat4{entity.getCoordinateTransformer().getDataToWorldMatrix()};

    return {dvec2{d2w[3].x, d2w[3].x + glm::length(d2w[0])},
            dvec2{d2w[3].y, d2w[3].y + glm::length(d2w[1])},
            dvec2{d2w[3].z, d2w[3].z + glm::length(d2w[2])}};
}

std::array<dvec2, 3> getRangeBoundingBox(CoordinateSpace destSpace, const SpatialEntity& entity,
                                         std::optional<mat4> worldBoundingBox) {
    if (worldBoundingBox) {
        const auto bb =
            dmat4{entity.getCoordinateTransformer().getMatrix(CoordinateSpace::World, destSpace)} *
            dmat4{worldBoundingBox.value()};

        return {dvec2{bb[3].x, bb[3].x + glm::length(bb[0])},
                dvec2{bb[3].y, bb[3].y + glm::length(bb[1])},
                dvec2{bb[3].z, bb[3].z + glm::length(bb[2])}};
    } else {
        return {dvec2{0.0, 1.0}, dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};
    }
}

}  // namespace

std::array<dvec2, 3> Axis3DProcessorHelper::axisRanges(const SpatialEntity& entity) const {
    switch (rangeMode_.get()) {
        case AxisRangeMode::Dims:
            return getRangeDims(entity);
        case AxisRangeMode::Basis:
            return getRangeBasis(entity);
        case AxisRangeMode::BasisOffset:
            return getRangeBasisOffset(entity);
        case AxisRangeMode::World:
            return getRangeWorld(entity);
        case AxisRangeMode::DataBoundingBox:
            return getRangeBoundingBox(
                CoordinateSpace::Data, entity,
                getBoundingBox_.and_then([](auto&& func) { return func(); }));
        case AxisRangeMode::ModelBoundingBox:
            return getRangeBoundingBox(
                CoordinateSpace::Model, entity,
                getBoundingBox_.and_then([](auto&& func) { return func(); }));
        case AxisRangeMode::WorldBoundingBox:
            return getRangeBoundingBox(
                CoordinateSpace::World, entity,
                getBoundingBox_.and_then([](auto&& func) { return func(); }));
    }
    return {dvec2{0.0, 1.0}, dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};
}

/*
ivec3 Axis3DProcessorHelper::adjustRanges(const SpatialEntity* entity,
                                          const std::optional<int>& autoScale) {
    if (entity) {
        std::array<AxisProperty*, 3> axes = {&xAxis_, &yAxis_, &zAxis_};
        for (size_t i = 0; i < 3; ++i) {
            if (const auto* axis = entity->getAxis(i)) {
                util::updateDefaultState(axes[i]->captionSettings_.title_,
                                         fmt::format("{}{: [}", axis->name, axis->unit),
                                         util::OverwriteState::No);
            } else {
                util::updateDefaultState(axes[i]->captionSettings_.title_,
                                         util::defaultAxesNames[i], util::OverwriteState::No);
            }
        }
    }
}

*/

dmat4 Axis3DProcessorHelper::getDataToWorldMatrix(const SpatialEntity& entity) const {
    const auto& coordinateTransformer = entity.getCoordinateTransformer();
    const dmat4 m = coordinateTransformer.getDataToWorldMatrix();
    using enum AxisRangeMode;
    if (util::contains({DataBoundingBox, ModelBoundingBox, WorldBoundingBox}, rangeMode_.get())) {
        return getBoundingBox_.and_then([](auto&& func) { return func(); })
            .transform([](const mat4& bbox) { return dmat4{bbox}; })
            .value_or(m);
    } else {
        return m;
    }
}


}  // namespace inviwo::plot
