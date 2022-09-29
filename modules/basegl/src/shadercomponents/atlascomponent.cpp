/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/atlascomponent.h>

#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/texture2d.h>

#include <modules/basegl/shadercomponents/timecomponent.h>

#include <algorithm>
#include <functional>

namespace inviwo {

namespace {

OrdinalPropertyState<float> ordinalAlpha(
    const float& value, InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput) {
    return {value,
            0.0f,
            ConstraintBehavior::Immutable,
            1.0f,
            ConstraintBehavior::Immutable,
            0.01f,
            invalidationLevel,
            PropertySemantics::Default};
}

void orderTf(TransferFunction& tf, size_t nSegments) {
    while (tf.size() > 2 * (nSegments + 1)) {
        tf.remove(tf.back());
    }
    while (tf.size() < 2 * (nSegments + 1)) {
        tf.add(1.0, vec4{1.0});
    }

    std::vector<TFPrimitive*> primitives;
    for (auto& p : tf) {
        primitives.push_back(&p);
    }

    auto setPos = [&](double pos, size_t i) {
        primitives[i]->setPosition(std::clamp(pos, 0.0, 1.0));
    };
    auto setPosAlmost = [&](double pos, size_t i) {
        setPos(pos - 100.0 * std::numeric_limits<double>::epsilon(), i);
    };

    for (size_t i = 0; i <= nSegments; ++i) {
        const auto start = (-0.5 / nSegments);
        const auto stop = (nSegments + 0.5) / nSegments;
        const auto delta = (stop - start) / (nSegments + 1);

        setPos(start + i * delta, i * 2);
        setPosAlmost(start + (i + 1) * delta, i * 2 + 1);
    }
}

}  // namespace

AtlasComponent::AtlasComponent(Processor* p, std::string_view color, TimeComponent* time)
    : ShaderComponent{}
    , atlas_{"atlas"}
    , brushing_{"brushing"}
    , selectionColor_{"selectionColor", "Selection Color",
                      util::ordinalColor(vec3{1.0f, 0.0f, 0.0f})}
    , selectionAlpha_{"selectionAlpha", "Selection Alpha", ordinalAlpha(1.0f)}
    , selectionMix_{"selectionMix", "Selection Mix", ordinalAlpha(0.0f)}
    , filteredColor_{"filteredColor", "Filtered Color", util::ordinalColor(vec3{0.1f, 0.1f, 0.1f})}
    , filteredAlpha_{"filteredAlpha", "Filtered Alpha", ordinalAlpha(0.1f)}
    , filteredMix_{"filteredMix", "Filtered Mix", ordinalAlpha(0.0f)}
    , tf_{"atlasTF", "Atlas TF", &atlas_}
    , coloringGroup_{"coloringGroup",
                     "Coloring Group",
                     {{"all", "All Segments", ColoringGroup::All},
                      {"selected", "Selected Segments", ColoringGroup::Selected},
                      {"unselected", "Unselected Segments", ColoringGroup::Unselected},
                      {"filtered", "Filtered Segments", ColoringGroup::Filtered},
                      {"unfiltered", "Unfiltered Segments", ColoringGroup::Unfiltered},
                      {"zero", "Non Segment", ColoringGroup::Zero}},
                     0,
                     InvalidationLevel::Valid}
    , coloringColor_{"coloringColor", "Coloring Color",
                     util::ordinalColor(vec3{0.0f, 1.0f, 0.0f}, InvalidationLevel::Valid)}
    , coloringAlpha_{"coloringAlpha", "Coloring Alpha",
                     ordinalAlpha(1.0f, InvalidationLevel::Valid)}
    , coloringScheme_{"coloringScheme", "Coloring Scheme", colorbrewer::getFamilies(), 0,
                      InvalidationLevel::Valid}
    , coloringApply_{"coloringApply",
                     "Apply Coloring",
                     {
                         {"Set Color", std::nullopt, std::nullopt,
                          [&]() { coloringAction_ = ColoringAction::SetColor; }},
                         {"Set Alpha", std::nullopt, std::nullopt,
                          [&]() { coloringAction_ = ColoringAction::SetAlpha; }},
                         {"Set Scheme", std::nullopt, std::nullopt,
                          [&]() { coloringAction_ = ColoringAction::SetScheme; }},
                     }}
    , coloringAction_{ColoringAction::None}
    , colors_{size2_t{64, 2},     DataFormat<vec4>::get(),    LayerType::Color,
              swizzlemasks::rgba, InterpolationType::Nearest, wrapping2d::clampAll}
    , color_{color}
    , picking_{p, 0, [this](PickingEvent* e) { onPickingEvent(e); }}
    , time_{time} {

    auto lrp = static_cast<LayerRAMPrecision<vec4>*>(colors_.getEditableRepresentation<LayerRAM>());
    std::fill_n(lrp->getDataTyped(), glm::compMul(lrp->getDimensions()), vec4(1.0));
}

std::string_view AtlasComponent::getName() const { return atlas_.getIdentifier(); }

void AtlasComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, atlas_);

    auto nSegments = static_cast<uint32_t>(atlas_.getData()->dataMap_.dataRange.y);
    picking_.resize(nSegments + 1);
    shader.setUniform(fmt::format("{0}PickingStart", getName()),
                      static_cast<uint32_t>(picking_.getPickingId(0)));
    shader.setUniform(fmt::format("{0}Size", getName()), nSegments);

    if (coloringAction_ != ColoringAction::None) {
        const auto foreachInGroup = [&]() -> std::function<void(std::function<void(uint32_t)>)> {
            switch (coloringGroup_.get()) {
                default:
                case ColoringGroup::All:
                    return [&](std::function<void(uint32_t)> fun) {
                        for (uint32_t i = 1; i <= nSegments; ++i) {
                            fun(i);
                        }
                    };
                case ColoringGroup::Selected:
                    return [&](std::function<void(uint32_t)> fun) {
                        for (auto i : brushing_.getSelectedIndices()) {
                            fun(i + 1);
                        }
                    };
                case ColoringGroup::Unselected:
                    return [&](std::function<void(uint32_t)> fun) {
                        for (uint32_t i = 1; i <= nSegments; ++i) {
                            if (!brushing_.isSelected(i - 1)) fun(i);
                        }
                    };
                case ColoringGroup::Filtered:
                    return [&](std::function<void(uint32_t)> fun) {
                        for (auto i : brushing_.getFilteredIndices()) {
                            fun(i + 1);
                        }
                    };
                case ColoringGroup::Unfiltered:
                    return [&](std::function<void(uint32_t)> fun) {
                        for (uint32_t i = 1; i <= nSegments; ++i) {
                            if (!brushing_.isFiltered(i - 1)) fun(i);
                        }
                    };
                case ColoringGroup::Zero:
                    return [&](std::function<void(uint32_t)> fun) { fun(0); };
            }
        }();

        auto& tf = tf_.get();
        orderTf(tf, nSegments);

        switch (coloringAction_) {
            case ColoringAction::SetColor: {
                foreachInGroup([&](size_t i) {
                    if (i * 2 < tf.size()) {
                        tf[2 * i].setColor(coloringColor_);
                        tf[2 * i + 1].setColor(coloringColor_);
                    }
                });
                break;
            }
            case ColoringAction::SetAlpha: {
                foreachInGroup([&](size_t i) {
                    if (i * 2 < tf.size()) {
                        tf[2 * i].setAlpha(coloringAlpha_);
                        tf[2 * i + 1].setAlpha(coloringAlpha_);
                    }
                });
                break;
            }
            case ColoringAction::SetScheme: {
                const size_t maxColor = colorbrewer::getMaxNumberOfColorsForFamily(coloringScheme_);
                const size_t minColor = colorbrewer::getMinNumberOfColorsForFamily(coloringScheme_);
                size_t groupSize = 0;
                foreachInGroup([&](size_t) { ++groupSize; });

                const auto& map = colorbrewer::getColormap(
                    coloringScheme_.get(),
                    static_cast<glm::uint8>(std::clamp(groupSize, minColor, maxColor)));

                foreachInGroup([&](size_t i) {
                    if (i * 2 < tf.size()) {
                        tf[2 * i].setColor(vec3{map[i % map.size()]});
                        tf[2 * i + 1].setColor(vec3{map[i % map.size()]});
                    }
                });
                break;
            }
            case ColoringAction::None:
                break;
        }

        coloringAction_ = ColoringAction::None;
    }

    const auto colorProps =
        util::ref<Property>(tf_, selectionColor_, selectionAlpha_, selectionMix_, filteredColor_,
                            filteredAlpha_, filteredMix_);
    if (brushing_.isChanged() || util::any_of(colorProps, &Property::isModified)) {

        if (colors_.getDimensions().x < nSegments + 1) {
            colors_.setDimensions(size2_t{nSegments + 1, 2});
        }

        util::IndexMapper2D im(colors_.getDimensions());

        auto lrp =
            static_cast<LayerRAMPrecision<vec4>*>(colors_.getEditableRepresentation<LayerRAM>())
                ->getDataTyped();

        for (uint32_t i = 0; i <= nSegments; ++i) {
            auto color = tf_->sample(static_cast<double>(i) / nSegments);
            lrp[im(i, 0)] = color;
            lrp[im(i, 1)] = color;
        }
        for (auto i : brushing_.getSelectedIndices()) {
            lrp[im(i + 1, 0)] =
                vec4{glm::mix(selectionColor_.get(), vec3{lrp[im(i + 1, 0)]}, selectionMix_.get()),
                     selectionAlpha_.get()};

            lrp[im(i + 1, 1)] =
                vec4{glm::mix(vec3{1.0, 1.0, 1.0}, vec3{lrp[im(i + 1, 0)]}, selectionMix_.get()),
                     selectionAlpha_.get()};
        }

        for (auto i : brushing_.getHighlightedIndices()) {
            lrp[im(i + 1, 0)] =
                vec4{glm::mix(selectionColor_.get(), vec3{lrp[im(i + 1, 0)]}, selectionMix_.get()),
                     selectionAlpha_.get()};

            lrp[im(i + 1, 1)] =
                vec4{glm::mix(vec3{1.0, 1.0, 1.0}, vec3{lrp[im(i + 1, 0)]}, selectionMix_.get()),
                     selectionAlpha_.get()};
        }

        for (auto i : brushing_.getFilteredIndices()) {
            lrp[im(i + 1, 0)] =
                vec4{glm::mix(filteredColor_.get(), vec3{lrp[im(i + 1, 0)]}, filteredMix_.get()),
                     filteredAlpha_.get()};
            lrp[im(i + 1, 1)] = lrp[im(i + 1, 0)];
        }
    }
    utilgl::bindAndSetUniforms(shader, cont, *colors_.getRepresentation<LayerGL>()->getTexture(),
                               fmt::format("{0}Colors", getName()));

    time_->setRunning(!brushing_.getSelectedIndices().empty() ||
                      !brushing_.getHighlightedIndices().empty());
}

void AtlasComponent::onPickingEvent(PickingEvent* e) {
    const auto id = static_cast<uint32_t>(e->getPickedId());

    if (e->getHoverState() == PickingHoverState::Enter) {
        e->setToolTip(fmt::format("Segment {}", id));
        brushing_.highlight(BitSet(id - 1));
    } else if (e->getHoverState() == PickingHoverState::Exit) {
        e->setToolTip("");
        brushing_.highlight(BitSet());
    }

    if (e->getPressState() == PickingPressState::Release &&
        e->getPressItem() == PickingPressItem::Primary &&
        e->modifiers().contains(KeyModifier::Control)) {

        auto selection = brushing_.getSelectedIndices();
        selection.flip(id - 1);
        brushing_.select(selection);

        e->markAsUsed();
    } else if (e->getPressState() == PickingPressState::Release &&
               e->getPressItem() == PickingPressItem::Primary &&
               e->modifiers().contains(KeyModifier::Shift)) {

        auto filtered = brushing_.getFilteredIndices();
        filtered.flip(id - 1);
        brushing_.filter("atlas", filtered);

        e->markAsUsed();
    }
}

std::vector<std::tuple<Inport*, std::string>> AtlasComponent::getInports() {
    return {{&atlas_, std::string{"default"}}, {&brushing_, "brushing"}};
}

std::vector<Property*> AtlasComponent::getProperties() {
    return {&selectionColor_,
            &selectionAlpha_,
            &selectionMix_,
            &filteredColor_,
            &filteredAlpha_,
            &filteredMix_,
            &tf_,
            &coloringGroup_,
            &coloringColor_,
            &coloringAlpha_,
            &coloringScheme_,
            &coloringApply_};
}

namespace {
constexpr std::string_view uniforms = util::trim(R"(
uniform VolumeParameters {atlas}Parameters;
uniform sampler3D {atlas};
uniform sampler2D {atlas}Colors;
uniform uint {atlas}PickingStart;
uniform uint {atlas}Size;
)");

constexpr std::string_view first = util::trim(R"(
float {atlas}Scale = float({atlas}Size) / (textureSize({atlas}Colors, 0).x - 1);
float {atlas}Segment = getNormalizedVoxel({atlas}, {atlas}Parameters, samplePosition).x;

vec4 {atlas}Color1 = texture({atlas}Colors, vec2({atlas}Segment * {atlas}Scale, 0.25));
vec4 {atlas}Color2 = texture({atlas}Colors, vec2({atlas}Segment * {atlas}Scale, 0.75));
{color} = highlight({color}, {atlas}Color1, {atlas}Color2, time);

if (picking.a == 0.0 && {color}.a > 0.0 && {atlas}Segment != 0.0) {{
    uint pid = {atlas}PickingStart + uint({atlas}Size * {atlas}Segment + 0.5);
    picking = vec4(pickingIndexToColor(pid), 1.0);
}}
)");

constexpr std::string_view loop = util::trim(R"(
{atlas}Segment = getNormalizedVoxel({atlas}, {atlas}Parameters, samplePosition).x;
{atlas}Color1 = texture({atlas}Colors, vec2({atlas}Segment * {atlas}Scale, 0.25));
{atlas}Color2 = texture({atlas}Colors, vec2({atlas}Segment * {atlas}Scale, 0.75));
{color} = highlight({color}, {atlas}Color1, {atlas}Color2, time);

if (picking.a == 0.0 && {color}.a > 0.0 && {atlas}Segment != 0.0) {{
    uint pid = {atlas}PickingStart + uint({atlas}Size * {atlas}Segment + 0.5);
    picking = vec4(pickingIndexToColor(pid), 1.0);
}}
)");

}  // namespace

auto AtlasComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {
        {R"(#include "utils/pickingutils.glsl")", placeholder::include, 800},
        {fmt::format(uniforms, "atlas"_a = getName()), placeholder::uniform, 800},
        {fmt::format(first, "atlas"_a = getName(), "color"_a = color_), placeholder::first, 800},
        {fmt::format(loop, "atlas"_a = getName(), "color"_a = color_), placeholder::loop, 800}};
}

}  // namespace inviwo
