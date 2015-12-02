/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "hedgehog2d.h"
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/imagesampler.h>

namespace inviwo {
const ProcessorInfo HedgeHog2D::processorInfo_{
    "org.inviwo.HedgeHog2D",       // Class identifier
    "Hedge Hog 2D",                // Display name
    "Vector Field Visualization",  // Category
    CodeState::Experimental,       // Code state
    Tags::GL,                      // Tags
};
const ProcessorInfo HedgeHog2D::getProcessorInfo() const {
    return processorInfo_;
}

HedgeHog2D::HedgeHog2D()
    : Processor()
    , vectorFeild_("vectorFeild")
    , mesh_("mesh")
    , glyphScale_("glyphScale", "Glyph Scale", 0.9f, 0.0f, 2.0f)
    , numberOfGlyphs_("numberOfGlyphs", "Number of Glyphs", ivec2(30), ivec2(1), ivec2(1000))
    , jitter_("jitter", "Jitter", false)
    , color_("color", "Color", vec4(0.0f, 0.0f, 0.0f, 1.0f))

    , glyphType_("glyphType", "Glyph Type")

    , arrowSettings_("arrowSettings", "Arrow Settings")
    , arrowBaseWidth_("baseWidth_", "Base Width", 0.1f, 0.001f, 1.0f, 0.001f)
    , arrowHookWidth_("hookWidth", "Hook Width", 0.1f, 0.001f, 1.0f, 0.001f)
    , arrowHeadRatio_("headRatio", "Head Ratio", 0.25f, 0.001f, 1.0f, 0.001f)

    , quiverSettings_("quiverSettings", "Quiver Settings")
    , quiverHookWidth_("hookWidth", "Hook Width", 0.2f, 0.001f, 1.0f, 0.001f)
    , quiverHeadRatio_("headRatio", "Head Ratio", 0.2f, 0.001f, 1.0f, 0.001f)

    , rd_()
    , mt_(rd_()) {
    addPort(vectorFeild_);
    addPort(mesh_);

    addProperty(color_);
    color_.setSemantics(PropertySemantics::Color);
    color_.setCurrentStateAsDefault();

    addProperty(glyphType_);

    glyphType_.addOption("arrow", "Arrow", GlyphType::Arrow);
    glyphType_.addOption("quiver", "Quiver", GlyphType::Quiver);

    addProperty(glyphScale_);
    addProperty(numberOfGlyphs_);
    addProperty(jitter_);

    addProperty(arrowSettings_);
    arrowSettings_.addProperty(arrowBaseWidth_);
    arrowSettings_.addProperty(arrowHookWidth_);
    arrowSettings_.addProperty(arrowHeadRatio_);

    addProperty(quiverSettings_);
    quiverSettings_.addProperty(quiverHookWidth_);
    quiverSettings_.addProperty(quiverHeadRatio_);

    glyphType_.onChange([this]() { this->adjustVisibilites(); });

    adjustVisibilites();
    setAllPropertiesCurrentStateAsDefault();
}

HedgeHog2D::~HedgeHog2D() {}

void HedgeHog2D::process() {
    auto mesh = std::make_shared<BasicMesh>();
    auto indexTriangles = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    auto indexLines = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);

    float dx = 1.0f / numberOfGlyphs_.get().x;
    float dy = 1.0f / numberOfGlyphs_.get().y;

    ImageSampler sampler(vectorFeild_.getData().get());

    std::uniform_real_distribution<float> jitterx(-dx / 2, dx / 2);
    std::uniform_real_distribution<float> jittery(-dy / 2, dy / 2);

    for (int j = 0; j < numberOfGlyphs_.get().y; j++) {
        float y = dy * j;
        for (int i = 0; i < numberOfGlyphs_.get().x; i++) {
            float x = dx * i;
            float jx = 0;
            float jy = 0;
            if (jitter_.get()) {
                jx = jitterx(mt_);
                jy = jittery(mt_);
            }
            auto v = sampler.sample(x + jx + dx / 2, y + jy + dy / 2).xy();
            auto t = glyphType_.get();
            switch (t) {
                case GlyphType::Arrow:
                    createArrow(*(mesh.get()), *indexTriangles, x + jx, y + jy, dx, dy, v);
                    break;
                case GlyphType::Quiver:
                    createQuiver(*(mesh.get()), *indexLines, x + jx, y + jy, dx, dy, v);
                    break;
            }
        }
    }

    mesh_.setData(mesh);
}

void HedgeHog2D::adjustVisibilites() {
    auto glyph = glyphType_.get();
    arrowSettings_.setVisible(glyph == GlyphType::Arrow);
    quiverSettings_.setVisible(glyph == GlyphType::Quiver);
}

inviwo::vec4 HedgeHog2D::getColor(const dvec2 &velocity) { return color_; }

void HedgeHog2D::createArrow(BasicMesh &mesh, IndexBufferRAM &index, float x, float y, float dx,
                             float dy, const dvec2 &velocity) {
    dmat2 m1(1);
    dmat2 m2(1);

    auto s = glm::length(velocity);
    auto dir = velocity / s;
    auto r = std::atan2(dir.y, dir.x);
    auto cr = std::cos(r);
    auto cs = std::sin(r);

    m1[0][0] *= cr;
    m1[1][0] = -cs;
    m1[0][1] = cs;
    m1[1][1] *= cr;

    m2[0][0] *= dx * glyphScale_.get();
    m2[1][1] *= dy * glyphScale_.get();

    auto m = static_cast<mat2>(m1 * m2);

    vec2 offset(x + dx * 0.5, y + dy * 0.5);

    auto w = arrowBaseWidth_.get();
    auto c = getColor(velocity);
    if (arrowHeadRatio_.get() != 1) {
        // Draw base
        vec2 p0(0, 0.5 - w / 2);
        vec2 p1(0, 0.5 + w / 2);
        vec2 p2 = p0;
        vec2 p3 = p1;
        p3.x = p2.x = 1 - arrowHeadRatio_.get();

        p0 -= 0.5;
        p1 -= 0.5;
        p2 -= 0.5;
        p3 -= 0.5;

        auto i0 = mesh.addVertex(vec3((m * p0) + offset, 0), vec3(p0, 0), vec3(velocity, 0), c);
        auto i1 = mesh.addVertex(vec3((m * p1) + offset, 0), vec3(p1, 0), vec3(velocity, 0), c);
        auto i2 = mesh.addVertex(vec3((m * p2) + offset, 0), vec3(p2, 0), vec3(velocity, 0), c);
        auto i3 = mesh.addVertex(vec3((m * p3) + offset, 0), vec3(p3, 0), vec3(velocity, 0), c);

        index.add(i0);
        index.add(i1);
        index.add(i2);

        index.add(i1);
        index.add(i2);
        index.add(i3);
    }
    if (arrowHeadRatio_.get() != 0) {
        // Draw head
        w += arrowHookWidth_.get() * 2;
        vec2 p0(0, 0.5 - w / 2);
        vec2 p1(0, 0.5 + w / 2);
        vec2 p2(1, 0.5);

        p0.x = p1.x = 1 - arrowHeadRatio_.get();

        p0 -= 0.5;
        p1 -= 0.5;
        p2 -= 0.5;

        auto i0 = mesh.addVertex(vec3((m * p0) + offset, 0), vec3(p0, 0), vec3(velocity, 0), c);
        auto i1 = mesh.addVertex(vec3((m * p1) + offset, 0), vec3(p1, 0), vec3(velocity, 0), c);
        auto i2 = mesh.addVertex(vec3((m * p2) + offset, 0), vec3(p2, 0), vec3(velocity, 0), c);

        index.add(i0);
        index.add(i1);
        index.add(i2);
    }
}

void HedgeHog2D::createQuiver(BasicMesh &mesh, IndexBufferRAM &index, float x, float y, float dx,
                              float dy, const dvec2 &velocity) {
    dmat2 m1(1);
    dmat2 m2(1);

    auto s = glm::length(velocity);
    auto dir = velocity / s;
    auto r = std::atan2(dir.y, dir.x);
    auto cr = std::cos(r);
    auto cs = std::sin(r);

    m1[0][0] *= cr;
    m1[1][0] = -cs;
    m1[0][1] = cs;
    m1[1][1] *= cr;

    m2[0][0] *= dx * glyphScale_.get();
    m2[1][1] *= dy * glyphScale_.get();

    auto m = static_cast<mat2>(m1 * m2);

    vec2 offset(x + dx * 0.5, y + dy * 0.5);
    auto c = getColor(velocity);

    vec2 p0(0, 0.5);
    vec2 p1(1, 0.5);
    vec2 p2(1 - quiverHeadRatio_.get(), 0.5 + quiverHookWidth_.get() / 2);
    vec2 p3(1 - quiverHeadRatio_.get(), 0.5 - quiverHookWidth_.get() / 2);

    auto i0 = mesh.addVertex(vec3((m * p0) + offset, 0.0f), vec3(p0, 0.0f), vec3(velocity, 0.0f), c);
    auto i1 = mesh.addVertex(vec3((m * p1) + offset, 0.0f), vec3(p1, 0.0f), vec3(velocity, 0.0f), c);
    auto i2 = mesh.addVertex(vec3((m * p2) + offset, 0.0f), vec3(p2, 0.0f), vec3(velocity, 0.0f), c);
    auto i3 = mesh.addVertex(vec3((m * p3) + offset, 0.0f), vec3(p3, 0.0f), vec3(velocity, 0.0f), c);

    index.add(i0);
    index.add(i1);
    index.add(i1);
    index.add(i2);
    index.add(i1);
    index.add(i3);
}

}  // namespace

