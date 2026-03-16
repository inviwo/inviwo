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

#include <modules/plottinggl/utils/axis2dprocessorhelper.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glm.h>

#include <fmt/core.h>
#include <algorithm>
#include <ranges>

namespace inviwo {

namespace {

struct AxisParams {
    dvec3 start;
    dvec3 stop;
    dvec3 tickDir;
};

std::array<AxisParams, 2> findAxisPositions(dvec3 viewDirection) {
    constexpr dvec3 center = {0.5, 0.5, 0.0};
    constexpr std::array<dvec3, 4> corners = {{{0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0}}};
    struct AI {
        std::array<size_t, 2> idx;
        dvec3 tickDir;
    };

    constexpr std::array<std::array<AI, 2>, 2> meta = {
        {{
             AI{.idx = {0, 2}, .tickDir = dvec3{0, 1, 0}},
             AI{.idx = {1, 3}, .tickDir = dvec3{0, -1, 0}},
         },
         {
             AI{.idx = {0, 1}, .tickDir = dvec3{1, 0, 0}},
             AI{.idx = {2, 3}, .tickDir = dvec3{-1, 0, 0}},
         }}};

    const auto dist = [&](const AI& edge) {
        return 0.5 * (glm::dot(corners[edge.idx[0]] - center, viewDirection) +
                      glm::dot(corners[edge.idx[1]] - center, viewDirection));
    };

    const auto find = [&](const std::array<AI, 2>& axis) -> AxisParams {
        const auto min = *std::ranges::max_element(
            axis, [&](const AI& a, const AI& b) { return (dist(a) < dist(b)); });

        return {.start = corners[min.idx[0]], .stop = corners[min.idx[1]], .tickDir = min.tickDir};
    };

    return {find(meta[0]), find(meta[1])};
}

}  // namespace

namespace plot {

Axis2DProcessorHelper::Axis2DProcessorHelper(std::function<std::optional<mat4>()> getBoundingBox,
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
    , rangeMode_{"rangeMode", "Axis Range Mode",
                 rangeModeState(useDimsRange == DimsRangeMode::Yes, getBoundingBox != nullptr)}

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
          {"negY", "X -Y", true},
          {"posY", "X +Y", false},
          // y axis
          {"negX", "Y -X", true},
          {"posX", "Y +X", false},
      }}
    , axisStyle_{"axisStyle", "Global Axis Style"}
    , axes_{{{"xAxis", "X Axis", "Axis properties for x"_help},
             {"yAxis", "Y Axis", "Axis properties for y"_help}}}
    , camera_{"camera", "Camera", std::move(getBoundingBox)}
    , trackball_{&camera_}
    , axisRenderers_{}
    , getBoundingBox_{getBoundingBox} {

    presets_.setSerializationMode(PropertySerializationMode::None);
    presets_.onChange([&]() {
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
                visibleAxes_[2].set(true);
            }
        }
    });

    visibility_.getBoolProperty()->setDisplayName("Automatic");
    visibility_.addProperty(presets_);
    for (auto& p : visibleAxes_) {
        visibility_.addProperty(p);
    }

    axisStyle_.setCollapsed(true);
    visibility_.setCollapsed(true);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    // initialize axes
    static constexpr std::array<std::string_view, 2> defaulCaptions{"x", "y"};
    for (auto&& [axis, caption] : std::views::zip(axes_, defaulCaptions)) {
        axis.setCaption(caption);
        axis.majorTicks_.width.set(1.5f);
        axis.majorTicks_.style.set(TickData::Style::Outside);

        axis.minorTicks_.width.set(1.3f);
        axis.minorTicks_.style.set(TickData::Style::Outside);

        // Treat the range as a pure output property.
        axis.range_.setInvalidationLevel(InvalidationLevel::Valid);

        axis.setCurrentStateAsDefault();

        axisStyle_.registerProperties(axis);
    }
}

void Axis2DProcessorHelper::renderAxes(size2_t outputDims, const SpatialEntity& entity) {

    const auto maybeBB = std::optional<mat4>{getBoundingBox_ ? getBoundingBox_() : std::nullopt};
    auto d2w = plot::getTransform(entity, maybeBB, rangeMode_.get());
    // ensure non-zero columns in model matrix which are caused by zero-length basis vectors
    // FIXME: coordinate transformer should take care of this for the model matrix
    if (glm::length(d2w[2]) < glm::epsilon<double>()) {
        d2w[2] = dvec4(glm::normalize(glm::cross(dvec3(d2w[0]), dvec3(d2w[1]))), 0.0);
    }

    const auto nD2W = dmat3{glm::transpose(glm::inverse(d2w))};
    const auto scale = plot::calcScaleFactor(d2w, offsetScaling_.get());
    const auto autoScale = plot::labelScaleStep(labelScale_.get());
    const auto axisRanges = plot::calcAxisRanges(entity, maybeBB, rangeMode_.get());

    for (auto&& [i, axis, renderer, range] :
         std::views::zip(std::views::iota(0uz, axes_.size()), axes_, axisRenderers_, axisRanges)) {
        auto [scaledRange, exponent] = scaleRange(range, autoScale);
        if (axis.overrideRange_) {
            scaledRange = axis.customRange_;
            exponent = 0;
        }

        auto& data = renderer.getData();
        data.range = scaledRange;
        data.visible = axis.isChecked();
        data.mirrored = axis.mirrored_.get();
        data.color = axis.color_.get();
        data.width = axis.width_.get();
        data.caption = axis.captionSettings_.title_.get();
        axis.captionSettings_.update(data.captionSettings);

        updateLabelPositions(data.majorPositions, data.minorPositions,
                             axis.labelingAlgorithm_.get(), data.range,
                             axis.majorTicks_.numberOfTicks.get(), axis.minorTicks_.frequency.get(),
                             axis.minorTicks_.fillAxis.get());

        axis.labelSettings_.update(data.labelSettings);
        updateLabels(data.labels, data.majorPositions, axis.labelSettings_.title_.get());

        axis.majorTicks_.update(data.major);
        axis.minorTicks_.update(data.minor);

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
        axis.range_.set(scaledRange);
    }

    const double offset = axisOffset_.get() * scale;

    const auto render = [&](const AxisParams& axis, size_t axisIdx) {
        const dvec3 center{0.5, 0.5, 0.5};
        const auto offsetDir =
            glm::normalize(dmat3{d2w} * glm::normalize(axis.start - center + axis.stop - center));
        const vec3 start{dvec3{d2w * dvec4(axis.start, 1)} + offset * offsetDir};
        const vec3 stop{dvec3{d2w * dvec4(axis.stop, 1)} + offset * offsetDir};
        const vec3 tickDir{nD2W * glm::normalize(axis.tickDir)};
        axisRenderers_[axisIdx].render(&camera_.get(), outputDims, start, stop, tickDir);
    };

    if (visibility_.isChecked()) {  // automatic selection
        // transform camera to data space since findAxisPositions uses a uniform square centered at
        // the origin to determine the visible faces
        const mat4 trafo{glm::inverse(d2w)};
        const auto axes =
            findAxisPositions(glm::normalize(vec3(trafo * vec4(camera_.getLookFrom(), 1.0f)) -
                                             vec3(trafo * vec4(camera_.getLookTo(), 1.0f))));
        for (auto&& [i, axis] : util::enumerate(axes)) {
            render(axis, i);
        }
    } else {
        constexpr std::array<AxisParams, 4> axes{{
            // x axis
            {.start = {0., 0., 0.}, .stop = {1., 0., 0.}, .tickDir = {0., 1., 0.}},
            {.start = {0., 1., 0.}, .stop = {1., 1., 0.}, .tickDir = {0., -1., 0.}},
            // y axis
            {.start = {0., 0., 0.}, .stop = {0., 1., 0.}, .tickDir = {1., 0., 0.}},
            {.start = {1., 0., 0.}, .stop = {1., 1., 0.}, .tickDir = {-1., 0., 0.}},
        }};
        for (auto&& [i, axis] : util::enumerate(axes)) {
            if (!visibleAxes_[i].get()) continue;
            render(axis, i / 2);
        }
    }
}

}  // namespace plot

}  // namespace inviwo
