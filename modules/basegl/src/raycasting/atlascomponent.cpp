/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/raycasting/atlascomponent.h>

#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/texture2d.h>

#include <modules/basegl/raycasting/timecomponent.h>

#include <algorithm>
#include <functional>

namespace inviwo {

namespace {

OrdinalPropertyState<float> ordinalAlpha(
    const float& value, InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput) {
    return {value,
            {0.0f, ConstraintBehavior::Immutable},
            {1.0f, ConstraintBehavior::Immutable},
            0.01f,
            invalidationLevel,
            PropertySemantics::Default};
}

std::vector<colorbrewer::Family> listFamilies() {
    using Index = std::underlying_type_t<colorbrewer::Family>;
    std::vector<colorbrewer::Family> res;
    for (Index i = 0; i < static_cast<Index>(colorbrewer::Family::NumberOfColormapFamilies); ++i) {
        res.push_back(static_cast<colorbrewer::Family>(i));
    }
    return res;
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

AtlasComponent::AtlasComponent(Processor* p, std::string_view volume, TimeComponent* time)
    : RaycasterComponent{}
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
                      {"notSelected", "Not Selected Segments", ColoringGroup::NotSelected},
                      {"filtered", "Filtered Segments", ColoringGroup::Filtered},
                      {"notFiltered", "Not Filtered Segments", ColoringGroup::NotFiltered},
                      {"zero", "Non Segment", ColoringGroup::Zero}},
                     0,
                     InvalidationLevel::Valid}
    , coloringColor_{"coloringColor", "Coloring Color",
                     util::ordinalColor(vec3{0.0f, 1.0f, 0.0f}, InvalidationLevel::Valid)}
    , coloringAlpha_{"coloringAlpha", "Coloring Alpha",
                     ordinalAlpha(1.0f, InvalidationLevel::Valid)}
    , coloringScheme_{"coloringScheme", "Coloring Scheme", listFamilies(), 0,
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
    , volume_{volume}
    , picking_{p, 0, [this](PickingEvent* e) { onPickingEvent(e); }}
    , time_{time} {

    auto lrp = static_cast<LayerRAMPrecision<vec4>*>(colors_.getEditableRepresentation<LayerRAM>());
    std::fill_n(lrp->getDataTyped(), glm::compMul(lrp->getDimensions()), vec4(1.0));
}

std::string_view AtlasComponent::getName() const { return atlas_.getIdentifier(); }

void AtlasComponent::process(Shader& shader, TextureUnitContainer& cont) {
    utilgl::bindAndSetUniforms(shader, cont, atlas_);

    auto nsegments = static_cast<size_t>(atlas_.getData()->dataMap_.dataRange.y);
    picking_.resize(nsegments + 1);
    shader.setUniform(fmt::format("{0}PickingStart", getName()),
                      static_cast<uint32_t>(picking_.getPickingId(0)));
    shader.setUniform(fmt::format("{0}Size", getName()), static_cast<uint32_t>(nsegments));

    if (coloringAction_ != ColoringAction::None) {
        const auto foreachInGroup = [&]() -> std::function<void(std::function<void(size_t)>)> {
            switch (coloringGroup_.get()) {
                default:
                case ColoringGroup::All:
                    return [&](std::function<void(size_t)> fun) {
                        for (size_t i = 1; i <= nsegments; ++i) {
                            fun(i);
                        }
                    };
                case ColoringGroup::Selected:
                    return [&](std::function<void(size_t)> fun) {
                        for (auto i : brushing_.getSelectedIndices()) {
                            fun(i + 1);
                        }
                    };
                case ColoringGroup::NotSelected:
                    return [&](std::function<void(size_t)> fun) {
                        for (size_t i = 1; i <= nsegments; ++i) {
                            if (!brushing_.isSelected(i - 1)) fun(i);
                        }
                    };
                case ColoringGroup::Filtered:
                    return [&](std::function<void(size_t)> fun) {
                        for (auto i : brushing_.getFilteredIndices()) {
                            fun(i + 1);
                        }
                    };
                case ColoringGroup::NotFiltered:
                    return [&](std::function<void(size_t)> fun) {
                        for (size_t i = 1; i <= nsegments; ++i) {
                            if (!brushing_.isFiltered(i - 1)) fun(i);
                        }
                    };
                case ColoringGroup::Zero:
                    return [&](std::function<void(size_t)> fun) { fun(0); };
            }
        }();

        auto& tf = tf_.get();
        orderTf(tf, nsegments);

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
        }

        coloringAction_ = ColoringAction::None;
    }

    if (tf_.isModified() || brushing_.isChanged() || selectionColor_.isModified() ||
        selectionAlpha_.isModified() || selectionMix_.isModified() || filteredColor_.isModified() ||
        filteredAlpha_.isModified() || filteredMix_.isModified()) {

        if (colors_.getDimensions().x < nsegments + 1) {
            colors_.setDimensions(size2_t{nsegments + 1, 2});
        }

        util::IndexMapper2D im(colors_.getDimensions());

        auto lrp =
            static_cast<LayerRAMPrecision<vec4>*>(colors_.getEditableRepresentation<LayerRAM>())
                ->getDataTyped();

        for (size_t i = 0; i <= nsegments; ++i) {
            auto color = tf_->sample(static_cast<double>(i) / nsegments);
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
        for (auto i : brushing_.getFilteredIndices()) {
            lrp[im(i + 1, 0)] =
                vec4{glm::mix(filteredColor_.get(), vec3{lrp[im(i + 1, 0)]}, filteredMix_.get()),
                     filteredAlpha_.get()};
            lrp[im(i + 1, 1)] = lrp[im(i + 1, 0)];
        }
    }
    utilgl::bindAndSetUniforms(shader, cont, *colors_.getRepresentation<LayerGL>()->getTexture(),
                               fmt::format("{0}Colors", getName()));

    if (brushing_.getSelectedIndices().empty()) {
        time_->timer.stop();
    } else {
        time_->timer.start();
    }
}

void AtlasComponent::onPickingEvent(PickingEvent* e) {
    auto id = e->getPickedId();

    if (e->getHoverState() == PickingHoverState::Enter) {
        e->setToolTip(fmt::format("Segment {}", id));
    } else if (e->getHoverState() == PickingHoverState::Exit) {
        e->setToolTip("");
    }

    if (e->getPressState() == PickingPressState::Release &&
        e->getPressItem() == PickingPressItem::Primary &&
        e->modifiers().contains(KeyModifier::Control)) {

        auto selection = brushing_.getSelectedIndices();

        if (brushing_.isSelected(id - 1)) {
            selection.erase(id - 1);
        } else {
            selection.insert(id - 1);
        }
        brushing_.sendSelectionEvent(selection);

        e->markAsUsed();
    } else if (e->getPressState() == PickingPressState::Release &&
               e->getPressItem() == PickingPressItem::Primary &&
               e->modifiers().contains(KeyModifier::Shift)) {

        auto filtered = brushing_.getFilteredIndices();

        if (brushing_.isFiltered(id - 1)) {
            filtered.erase(id - 1);
        } else {
            filtered.insert(id - 1);
        }
        brushing_.sendFilterEvent(filtered);

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
{volume}Color = highlight({volume}Color, {atlas}Color1, {atlas}Color2, time);

if (picking.a == 0.0 && {volume}Color.a > 0.0 && {atlas}Segment != 0.0) {{
    uint pid = {atlas}PickingStart + uint({atlas}Size * {atlas}Segment + 0.5);
    picking = vec4(pickingIndexToColor(pid), 1.0);
}}
)");

constexpr std::string_view loop = util::trim(R"(
{atlas}Segment = getNormalizedVoxel({atlas}, {atlas}Parameters, samplePosition).x;
{atlas}Color1 = texture({atlas}Colors, vec2({atlas}Segment * {atlas}Scale, 0.25));
{atlas}Color2 = texture({atlas}Colors, vec2({atlas}Segment * {atlas}Scale, 0.75));
{volume}Color = highlight({volume}Color, {atlas}Color1, {atlas}Color2, time);

if (picking.a == 0.0 && {volume}Color.a > 0.0 && {atlas}Segment != 0.0) {{
    uint pid = {atlas}PickingStart + uint({atlas}Size * {atlas}Segment + 0.5);
    picking = vec4(pickingIndexToColor(pid), 1.0);
}}
)");

}  // namespace

auto AtlasComponent::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;

    return {{R"(#include "utils/pickingutils.glsl")", Segment::include, 800},
            {fmt::format(uniforms, "atlas"_a = getName()), Segment::uniform, 800},
            {fmt::format(first, "atlas"_a = getName(), "volume"_a = volume_), Segment::first, 800},
            {fmt::format(loop, "atlas"_a = getName(), "volume"_a = volume_), Segment::loop, 800}};
}

}  // namespace inviwo
