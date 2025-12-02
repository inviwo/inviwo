/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/2d/hedgehog2d.h>

#include <inviwo/core/datastructures/buffer/bufferramprecision.h>  // for IndexBufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>      // for ConnectivityType
#include <inviwo/core/datastructures/geometry/typedmesh.h>         // for BasicMesh, TypedMesh
#include <inviwo/core/processors/processor.h>                      // for Processor
#include <inviwo/core/processors/processorinfo.h>                  // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                 // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                  // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>                   // for BoolProperty
#include <inviwo/core/properties/optionproperty.h>                 // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                // for FloatProperty
#include <inviwo/core/util/glmmat.h>                               // for dmat2, mat2
#include <inviwo/core/util/glmvec.h>                               // for vec3, vec2, ivec2
#include <inviwo/core/util/imagesampler.h>                         // for ImageSampler

#include <cmath>   // for atan2, cos, sin
#include <memory>  // for unique_ptr, share...
#include <numbers>

#include <glm/geometric.hpp>  // for length
#include <glm/mat2x2.hpp>     // for operator*, mat<>:...
#include <glm/vec2.hpp>       // for operator+, vec<>:...

namespace inviwo {
const ProcessorInfo HedgeHog2D::processorInfo_{
    "org.inviwo.HedgeHog2D",       // Class identifier
    "Hedge Hog 2D",                // Display name
    "Vector Field Visualization",  // Category
    CodeState::Stable,             // Code state
    Tags::GL | Tag{"Mesh"},        // Tags
    R"(Creates a mesh for a hedgehog plot of a 2D vector field using arrow and
    quiver glyphs.

    Example Workspaces:
    * [VectorFieldVisualizationGL/hedgehog2d-arrows.inv](file:~modulePath~/data/workspaces/hedgehog2d-arrows.inv)
    * [VectorFieldVisualizationGL/hedgehog2d-quivers.inv](file:~modulePath~/data/workspaces/hedgehog2d-quivers.inv)

    )"_unindentHelp,
};
const ProcessorInfo& HedgeHog2D::getProcessorInfo() const { return processorInfo_; }

HedgeHog2D::HedgeHog2D()
    : Processor{}
    , inport_{"inport", "2D vector field"_help}
    , mesh_{"mesh"}
    , glyphType_{"glyphType",
                 "Glyph Type",
                 {{"arrow", "Arrow", GlyphType::Arrow}, {"quiver", "Quiver", GlyphType::Quiver}}}
    , pivot_{"pivot",
             "Pivot",
             {{"tail", "Tail", Pivot::Tail},
              {"middle", "Middle", Pivot::Middle},
              {"tip", "Tip", Pivot::Tip}}}
    , color_{"color", "Color", util::ordinalColor(vec4{0.0f, 0.0f, 0.0f, 1.0f})}
    , tfGroup_{"tfGroup", "Magnitude Color Mapping", false}
    , normalized_{"normalized", "Normalized Length",
                  "Use a normalized length for the glyphs. Glyphs are scaled with the "
                  "velocity magnitude otherwise"_help,
                  true}
    , glyphScale_{"glyphScale", "Glyph Scale", 0.9f, 0.0f, 2.0f}
    , transferFunction_{"transferFunction", "Transfer Function",
                        "Defines the transfer function for mapping the velocity magnitude "
                        "to color and opacity"_help,
                        TransferFunction{{{.pos = 0.0, .color = vec4{0.0f, 0.0f, 0.0f, 1.0f}},
                                          {.pos = 1.0, .color = vec4{1.0f, 1.0f, 1.0f, 1.0f}}}}}
    , numberOfGlyphs_{"numberOfGlyphs", "Number of Glyphs", ivec2{30}, ivec2{1}, ivec2{1000}}
    , jitter_{"jitter", "Jitter", false}
    , jitterScale_{"jitterScale", "Jitter Scaling",
                   util::ordinalScale(0.5f, 1.0f)
                       .setInc(0.001f)
                       .setMax(ConstraintBehavior::Immutable)}
    , arrowSettings_{"arrowSettings", "Arrow Settings"}
    , arrowBaseWidth_{"baseWidth", "Base Width", util::ordinalScale(0.1f, 1.0f).setInc(0.001f)}
    , arrowHookWidth_{"hookWidth", "Hook Width", util::ordinalScale(0.1f, 1.0f).setInc(0.001f)}
    , arrowHeadRatio_{"headRatio", "Head Ratio",
                      util::ordinalLength(0.25f, 1.0f)
                          .setInc(0.001f)
                          .setMax(ConstraintBehavior::Immutable)}

    , seed_{"seed", "Seed", 0, 0, std::mt19937::max()}
    , reseed_{"reseed_", "Randomize Seed"}
    , rand_{} {

    addPorts(inport_, mesh_);
    addProperties(glyphType_, pivot_, color_, tfGroup_, normalized_, glyphScale_, numberOfGlyphs_,
                  jitter_, jitterScale_, seed_, reseed_, arrowSettings_);

    tfGroup_.addProperties(transferFunction_);

    arrowSettings_.addProperties(arrowBaseWidth_, arrowHookWidth_, arrowHeadRatio_);
    arrowBaseWidth_.readonlyDependsOn(glyphType_,
                                      [](const auto& p) { return p.get() != GlyphType::Arrow; });

    reseed_.onChange([this]() {
        std::uniform_int_distribution<std::int64_t> dist{0, seed_.getMaxValue()};
        seed_.set(dist(rand_));
    });
}

HedgeHog2D::~HedgeHog2D() = default;

namespace {

using ArrowMesh = TypedMesh<buffertraits::PositionsBuffer2D, buffertraits::ColorsBuffer>;

struct ArrowConfig {
    HedgeHog2D::Pivot pivot = HedgeHog2D::Pivot::Tail;
    float baseWidth = 0.1f;
    float hookWidth = 0.1f;
    float headRatio = 0.2f;
    float scaling = 1.0f;
};

vec2 getOffset(HedgeHog2D::Pivot pivot) {
    using enum HedgeHog2D::Pivot;
    switch (pivot) {
        case Middle:
            return {-0.5f, 0.0f};
        case Tip:
            return {-1.0f, 0.0f};
        case Tail:
        default:
            return {0.0f, 0.0f};
    }
}

void addArrow(ArrowMesh& mesh, IndexBufferRAM& indexBuffer, const ArrowConfig& config,
              const vec2& pos, const vec2& velocity, float length, const vec4& color) {

    if (length == 0.0f) return;

    const auto dir = glm::normalize(velocity);
    const auto r = std::atan2f(dir.y, dir.x);
    const auto cr = std::cosf(r);
    const auto cs = std::sinf(r);

    const vec2 scaling{vec2{length, 1.0f} * config.scaling};
    const mat2 m{vec2{cr, cs} * scaling.x, vec2{-cs, cr} * scaling.y};
    const auto offset = getOffset(config.pivot);

    // ensure that the tip length stays the same irrespective of the length.
    std::array<vec2, 4> arrowBase{
        vec2{0.0f, -config.baseWidth / 2.0f} + offset,
        vec2{0.0f, config.baseWidth / 2.0f} + offset,
        vec2{1.0f - config.headRatio / length, -config.baseWidth / 2.0f} + offset,
        vec2{1.0f - config.headRatio / length, config.baseWidth / 2.0f} + offset,
    };
    // base of the arrow
    if (config.headRatio / length < 1.0f) {
        auto i0 = mesh.addVertex(m * arrowBase[0] + pos, color);
        auto i1 = mesh.addVertex(m * arrowBase[1] + pos, color);
        auto i2 = mesh.addVertex(m * arrowBase[2] + pos, color);
        auto i3 = mesh.addVertex(m * arrowBase[3] + pos, color);
        indexBuffer.append({i0, i1, i2, i1, i2, i3});
    }

    // arrow head
    if (config.headRatio > 0.0f) {
        std::array<vec2, 3> arrowTip{
            vec2{std::max(arrowBase[2].x, offset.x), arrowBase[0].y - config.hookWidth},
            vec2{std::max(arrowBase[2].x, offset.x), arrowBase[1].y + config.hookWidth},
            vec2{1.0f, 0.0f} + offset,
        };
        const auto widthAdjustment = std::min(length / config.headRatio, 1.0f);
        arrowTip[0].y *= widthAdjustment;
        arrowTip[1].y *= widthAdjustment;

        auto i0 = mesh.addVertex(m * arrowTip[0] + pos, color);
        auto i1 = mesh.addVertex(m * arrowTip[1] + pos, color);
        auto i2 = mesh.addVertex(m * arrowTip[2] + pos, color);
        indexBuffer.append({i0, i1, i2});
    }
}

void addQuiver(ArrowMesh& mesh, IndexBufferRAM& indexBuffer, const ArrowConfig& config,
               const vec2& pos, const vec2& velocity, float length, const vec4& color) {

    if (length == 0.0f) return;

    const auto dir = glm::normalize(velocity);
    const auto r = std::atan2f(dir.y, dir.x);
    const auto cr = std::cosf(r);
    const auto cs = std::sinf(r);

    const vec2 scaling{vec2{length, 1.0f} * config.scaling};
    const mat2 m{vec2{cr, cs} * scaling.x, vec2{-cs, cr} * scaling.y};
    const auto offset = getOffset(config.pivot);

    // ensure that the tip length stays the same irrespective of the length.
    const auto hookWidth = std::min(length / config.headRatio, 1.0f) * config.hookWidth;

    std::array<vec2, 4> quiver{
        vec2{0.0f, 0.0f} + offset,
        vec2{1.0f, 0.0f} + offset,
        vec2{std::max(1.0f - config.headRatio / length, 0.0f) + offset.x, -hookWidth},
        vec2{std::max(1.0f - config.headRatio / length, 0.0f) + offset.x, hookWidth},
    };

    auto i0 = mesh.addVertex(m * quiver[0] + pos, color);
    auto i1 = mesh.addVertex(m * quiver[1] + pos, color);
    indexBuffer.append({i0, i1});
    if (config.headRatio > 0.0f && hookWidth > 0.0f) {
        auto i2 = mesh.addVertex(m * quiver[2] + pos, color);
        auto i3 = mesh.addVertex(m * quiver[3] + pos, color);
        indexBuffer.append({i1, i2, i1, i3});
    }
}

}  // namespace

void HedgeHog2D::process() {
    rand_.seed(static_cast<std::mt19937::result_type>(seed_.get()));

    const DataMapper& dataMap = inport_.getData()->dataMap;

    const double max = glm::compMax(glm::abs(dataMap.dataRange));
    const double conservativeMax = std::numbers::sqrt2 * max;
    auto getColor = [this](double t) {
        if (tfGroup_.isChecked()) {
            return transferFunction_.get().sample(t);
        }
        return color_.get();
    };

    auto mesh = std::make_shared<ArrowMesh>();
    const auto dt = glyphType_.get() == GlyphType::Arrow ? DrawType::Triangles : DrawType::Lines;
    auto indexBuffer = mesh->addIndexBuffer(dt, ConnectivityType::None);

    const vec2 delta{1.0f / vec2{numberOfGlyphs_.get()}};
    const vec2 deltaHalf{delta * 0.5f};

    ImageSampler sampler(inport_.getData());

    std::uniform_real_distribution<float> jitterx{-deltaHalf.x, deltaHalf.x};
    std::uniform_real_distribution<float> jittery{-deltaHalf.y, deltaHalf.y};

    const ArrowConfig config{
        .pivot = pivot_,
        .baseWidth = arrowBaseWidth_,
        .hookWidth = arrowHookWidth_,
        .headRatio = arrowHeadRatio_,
        .scaling = glyphScale_ * glm::compMin(delta),
    };

    for (int j = 0; j < numberOfGlyphs_.get().y; j++) {
        vec2 center{0.0f, static_cast<float>(j) * delta.y + deltaHalf.y};
        for (int i = 0; i < numberOfGlyphs_.get().x; i++) {
            center.x = static_cast<float>(i) * delta.x + deltaHalf.x;

            vec2 pos{center};
            if (jitter_) {
                pos += vec2{jitterx(rand_), jittery(rand_)} * jitterScale_.get();
            }

            dvec2 velocity{sampler.sample(pos)};
            velocity = dataMap.mapFromDataToValue(velocity);

            const auto normalizedMagnitude = glm::length(velocity) / conservativeMax;
            const vec4 color = getColor(normalizedMagnitude);
            const auto length = normalized_ ? 1.0f : static_cast<float>(normalizedMagnitude);

            switch (glyphType_) {
                case GlyphType::Quiver:
                    addQuiver(*mesh, *indexBuffer, config, pos, velocity, length, color);
                    break;
                case GlyphType::Arrow:
                default:
                    addArrow(*mesh, *indexBuffer, config, pos, velocity, length, color);
                    break;
            }
        }
    }

    mesh->setModelMatrix(inport_.getData()->getModelMatrix());
    mesh->setWorldMatrix(inport_.getData()->getWorldMatrix());

    mesh_.setData(mesh);
}

}  // namespace inviwo
