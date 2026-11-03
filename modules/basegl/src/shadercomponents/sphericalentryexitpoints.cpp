/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/sphericalentryexitpoints.h>

#include <modules/opengl/shader/shaderutils.h>
#include <modules/basegl/shadercomponents/shadercomponentutil.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>

#include <numbers>

namespace inviwo {

namespace eep {
namespace {
Mesh sphere(double outerRadius, double innerRadius, unsigned int numLoops,
            unsigned int segmentsPerLoop) {

    numLoops = std::max(4u, numLoops);
    segmentsPerLoop = std::max(8u, segmentsPerLoop);

    // Create Vertices
    const auto shellSize = (numLoops + 1) * (segmentsPerLoop + 1);
    const auto points = (innerRadius == 0.0 ? 1 : 2) * shellSize;

    std::vector<vec3> vertices;
    vertices.reserve(points);
    std::vector<vec3> normals;
    normals.reserve(points);
    std::vector<vec4> colors;
    colors.reserve(points);

    const unsigned int pointsPerLine = segmentsPerLoop + 1;

    std::array<double, 2> radii{outerRadius, innerRadius};

    for (auto r : radii | std::views::take(innerRadius == 0 ? 1 : 2)) {
        for (unsigned int i : std::views::iota(0u, numLoops + 1u)) {
            for (unsigned int j : std::views::iota(0u, segmentsPerLoop + 1u)) {
                const auto theta = i == numLoops ? std::numbers::pi
                                                 : std::numbers::pi * static_cast<double>(i) /
                                                       static_cast<double>(numLoops);

                const double phi = std::numbers::pi * static_cast<double>(j * 2) /
                                   static_cast<double>(segmentsPerLoop);

                const double sinTheta = std::sin(theta);
                const double sinPhi = std::sin(phi);
                const double cosTheta = std::cos(theta);
                const double cosPhi = std::cos(phi);

                const dvec3 normal{cosPhi * sinTheta, sinPhi * sinTheta, cosTheta};
                const dvec3 vertex{normal * r};
                const dvec3 color{dvec3{1.0 / (2.0 * outerRadius)} * (vertex + dvec3{outerRadius})};

                vertices.emplace_back(vec3{vertex});
                normals.emplace_back(vec3{normal});
                colors.emplace_back(vec4{vec3{color}, 1.0f});
            }
        }
    }

    // compute indices
    std::vector<std::uint32_t> indices;
    indices.reserve(points * 6);

    const auto shellOffsets = std::array<unsigned int, 2>{0, shellSize};

    const auto trig1 = std::array{0u, pointsPerLine, 1u, 1u, pointsPerLine, pointsPerLine + 1u};
    const auto trig2 = std::array{0u, 1u, pointsPerLine, 1u, pointsPerLine + 1u, pointsPerLine};

    for (auto&& [trig, o] : std::views::zip(std::array{trig1, trig2}, shellOffsets) |
                                std::views::take(innerRadius == 0 ? 1 : 2)) {
        for (unsigned int y = 0; y < numLoops; ++y) {
            for (unsigned int x = 0; x < pointsPerLine; ++x) {
                for (auto i : trig) {
                    indices.emplace_back(x + i + y * pointsPerLine + o);
                }
            }
        }
    }

    const auto pos = std::make_shared<Buffer<vec3>>(
        std::make_shared<BufferRAMPrecision<vec3>>(std::move(vertices)));
    const auto norm = std::make_shared<Buffer<vec3>>(
        std::make_shared<BufferRAMPrecision<vec3>>(std::move(normals)));
    const auto col = std::make_shared<Buffer<vec4>>(
        std::make_shared<BufferRAMPrecision<vec4>>(std::move(colors)));

    const auto inds =
        std::make_shared<IndexBuffer>(std::make_shared<IndexBufferRAM>(std::move(indices)));

    return {Mesh::BufferVector{{BufferType::PositionAttrib, pos},
                               {BufferType::NormalAttrib, norm},
                               {BufferType::ColorAttrib, col}},
            Mesh::IndexVector{{Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, inds}}};
}

constexpr std::string_view uniforms = util::trim(R"(
uniform ImageParameters {0}Parameters;
uniform sampler2D {0}Color;
uniform sampler2D {0}Depth;
)");

constexpr std::string_view surfaceNormalUniforms = util::trim(R"(
uniform sampler2D surfaceNormal;
uniform bool useSurfaceNormals;
)");

constexpr std::string_view setup = util::trim(R"(
vec3 entryPoint = texture(entryColor, texCoords).rgb;
vec3 exitPoint = texture(exitColor, texCoords).rgb;
float entryPointDepth = texture(entryDepth, texCoords).x;
float exitPointDepth = texture(exitDepth, texCoords).x;

// The length of the ray in texture space
float rayLength = length(exitPoint - entryPoint);

// The normalized direction of the ray
vec3 rayDirection = normalize(exitPoint - entryPoint);
)");

}  // namespace
}  // namespace eep
SphericalEntryExitPoints::SphericalEntryExitPoints(CameraProperty* camera)
    : ShaderComponent{}
    , basis{"basis", "Basis", util::ordinalMatrix(dmat4{1.0})}
    , outerRadius{"outerRadius", "outerRadius", util::ordinalScale(1.0, 5.0).setInc(0.01)}
    , innerRadius{"innerRadius", "innerRadius", util::ordinalScale(0.9, 5.0).setInc(0.01)}
    , sphereMesh{eep::sphere(outerRadius.get(), innerRadius.get(), 16, 32)}
    , entryPoints{LayerConfig::defaultDimensions, DataVec4UInt16::get()}
    , exitPoints{LayerConfig::defaultDimensions, DataVec4UInt16::get()}
    , eepHelper{}
    , trackball{camera} {}

std::string_view SphericalEntryExitPoints::getName() const { return "Spherical EEP"; }
void SphericalEntryExitPoints::initializeResources(Shader&) {}
void SphericalEntryExitPoints::process(Shader& shader, TextureUnitContainer& cont) {

    utilgl::bindAndSetUniforms(shader, cont, entryPoints, "entry", ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(shader, cont, exitPoints, "exit", ImageType::ColorDepth);
    shader.setUniform("useSurfaceNormals", true);
    shader.setUniform("sphereBasis", basis.get());
    shader.setUniform("outerRadius", outerRadius.get());
    shader.setUniform("innerRadius", innerRadius.get());
}

auto SphericalEntryExitPoints::getSegments() -> std::vector<Segment> {
    using namespace fmt::literals;
    return {{fmt::format(eep::uniforms, "entry"), placeholder::uniform, 100},
            {fmt::format(eep::uniforms, "exit"), placeholder::uniform, 101},
            {"uniform mat4 sphereBasis;", placeholder::uniform, 102},
            {"uniform float outerRadius;", placeholder::uniform, 103},
            {"uniform float innerRadius;", placeholder::uniform, 104},
            {std::string{eep::surfaceNormalUniforms}, placeholder::uniform, 102},
            {std::string{eep::setup}, placeholder::setup, 100}};
}

std::vector<Property*> SphericalEntryExitPoints::getProperties() {
    return {&basis, &outerRadius, &innerRadius, &trackball};
}

void SphericalEntryExitPoints::preprocess(Camera& camera, size2_t dim) {
    entryPoints.setDimensions(dim);
    exitPoints.setDimensions(dim);

    sphereMesh.setModelMatrix(basis.get());

    eepHelper(entryPoints, exitPoints, camera, sphereMesh, algorithm::CapNearClip::Yes,
              algorithm::IncludeNormals::Yes);
}

}  // namespace inviwo
