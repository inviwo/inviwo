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

#include <modules/basegl/shadercomponents/analyticentryexitcomponent.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/opengl/shader/shader.h>

#include <cmath>

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>

namespace inviwo {

namespace {

constexpr std::string_view prefix = "entryExit";

}  // namespace

AnalyticEntryExitComponent::AnalyticEntryExitComponent(
    std::function<glm::dmat4(CoordinateSpace from, CoordinateSpace to)> getTransform,
    std::string_view cameraName)
    : ShaderComponent()
    , geometry{"geometry", "Entry Exit Geometry",
               "The geometry used to analytically compute the ray entry and exit points"_help}
    , shape{"shape", "Shape",
            OptionPropertyState<Shape>{
                .options = {{"parallelepiped", "Parallelepiped", Shape::Parallelepiped},
                            {"sphere", "Sphere", Shape::Sphere}}}
                .set("The shape of the geometry. A parallelepiped is given by an origin and "
                     "three edge vectors, a sphere by a center and a radius"_help)
                .set(InvalidationLevel::InvalidResources)}
    , space{"space", "Space",
            OptionPropertyState<CoordinateSpace>{
                .options = {{"data", "Data", CoordinateSpace::Data},
                            {"model", "Model", CoordinateSpace::Model},
                            {"world", "World", CoordinateSpace::World}}}
                .set("The coordinate space of the volume in which the geometry is defined. In "
                     "Data space the volume occupies the unit cube. Note that a sphere in Data "
                     "or Model space might be an ellipsoid in World space"_help)}
    , clipToVolume{"clipToVolume", "Clip to Volume",
                   "Clip the geometry against the bounding box of the volume"_help, true}
    , origin{"origin", "Origin",
             util::ordinalSymmetricVector(vec3{0.0f}, 1.0f)
                 .set("Origin, i.e. one corner, of the parallelepiped"_help)}
    , edge1{"edge1", "Edge 1",
            util::ordinalSymmetricVector(vec3{1.0f, 0.0f, 0.0f}, 1.0f)
                .set("First edge vector of the parallelepiped"_help)}
    , edge2{"edge2", "Edge 2",
            util::ordinalSymmetricVector(vec3{0.0f, 1.0f, 0.0f}, 1.0f)
                .set("Second edge vector of the parallelepiped"_help)}
    , edge3{"edge3", "Edge 3",
            util::ordinalSymmetricVector(vec3{0.0f, 0.0f, 1.0f}, 1.0f)
                .set("Third edge vector of the parallelepiped"_help)}
    , center{"center", "Center",
             util::ordinalSymmetricVector(vec3{0.5f}, 1.0f).set("Center of the sphere"_help)}
    , radius{"radius", "Radius", util::ordinalLength(0.5f, 1.0f).set("Radius of the sphere"_help)}
    , getTransform_{std::move(getTransform)}
    , cameraName_{cameraName} {

    geometry.addProperties(shape, space, clipToVolume, origin, edge1, edge2, edge3, center, radius);

    const auto isParallelepiped = [](const OptionProperty<Shape>& p) {
        return p.getSelectedValue() == Shape::Parallelepiped;
    };
    const auto isSphere = [](const OptionProperty<Shape>& p) {
        return p.getSelectedValue() == Shape::Sphere;
    };
    origin.visibilityDependsOn(shape, isParallelepiped);
    edge1.visibilityDependsOn(shape, isParallelepiped);
    edge2.visibilityDependsOn(shape, isParallelepiped);
    edge3.visibilityDependsOn(shape, isParallelepiped);
    center.visibilityDependsOn(shape, isSphere);
    radius.visibilityDependsOn(shape, isSphere);
}

AnalyticEntryExitComponent::AnalyticEntryExitComponent(VolumeInport& volume,
                                                       std::string_view cameraName)
    : AnalyticEntryExitComponent(
          [vp = &volume](CoordinateSpace from, CoordinateSpace to) -> glm::dmat4 {
              const auto volume = vp->getData();
              if (!volume) glm::dmat4{1.0};
              const auto& ct = volume->getCoordinateTransformer();
              return ct.getMatrix(from, to);
          },
          cameraName) {}

std::string_view AnalyticEntryExitComponent::getName() const { return "analyticentryexit"; }

void AnalyticEntryExitComponent::process(Shader& shader, TextureUnitContainer&) {

    // Transformation from the canonical shape, i.e. the unit cube [0,1]^3 or the unit sphere
    // centered at the origin, to the space selected for the geometry.
    const dmat4 shapeToGeometry = [&]() {
        switch (shape.getSelectedValue()) {
            case Shape::Sphere: {
                const auto r = static_cast<double>(radius.get());
                return glm::scale(glm::translate(dmat4{1.0}, dvec3{center.get()}), dvec3{r});
            }
            case Shape::Parallelepiped:
            default:
                return dmat4{dvec4{dvec3{edge1.get()}, 0.0}, dvec4{dvec3{edge2.get()}, 0.0},
                             dvec4{dvec3{edge3.get()}, 0.0}, dvec4{dvec3{origin.get()}, 1.0}};
        }
    }();

    const dmat4 shapeToWorld =
        getTransform_(space.getSelectedValue(), CoordinateSpace::World) * shapeToGeometry;

    const double det = glm::determinant(shapeToWorld);
    const bool valid = det != 0.0 && std::isfinite(det);

    shader.setUniform(fmt::format("{}WorldToShape", prefix),
                      valid ? mat4{glm::inverse(shapeToWorld)} : mat4{1.0f});

    shader.setUniform(fmt::format("{}WorldToTexture", prefix),
                      mat4{getTransform_(CoordinateSpace::World, CoordinateSpace::Data)});
    shader.setUniform(fmt::format("{}ValidShape", prefix), valid);
    shader.setUniform(fmt::format("{}ClipToVolume", prefix), clipToVolume.get());
}

std::vector<Property*> AnalyticEntryExitComponent::getProperties() { return {&geometry}; }

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform mat4 {0}WorldToShape;
uniform mat4 {0}WorldToTexture;
uniform bool {0}ValidShape;
uniform bool {0}ClipToVolume;

// Surface normals are not supported, declared for compatibility with other components
uniform sampler2D surfaceNormal;
const bool useSurfaceNormals = false;

// Intersect the ray o + t * d with the unit cube [0,1]^3.
// Returns the parameter interval [t0, t1] inside the cube, t0 >= t1 means no intersection.
vec2 {0}IntersectParallelepiped(in vec3 o, in vec3 d) {{
    vec3 invD = 1.0 / mix(d, vec3(1.0e-20), equal(d, vec3(0.0)));
    vec3 ta = -o * invD;
    vec3 tb = (vec3(1.0) - o) * invD;
    vec3 tMin = min(ta, tb);
    vec3 tMax = max(ta, tb);
    return vec2(max(max(tMin.x, tMin.y), tMin.z), min(min(tMax.x, tMax.y), tMax.z));
}}

// Intersect the ray o + t * d with the unit sphere centered at the origin.
// Returns the parameter interval [t0, t1] inside the sphere, t0 >= t1 means no intersection.
vec2 {0}IntersectSphere(in vec3 o, in vec3 d) {{
    // Solve a t^2 + 2 b t + c = 0 using a numerically stable formulation
    float a = dot(d, d);
    float b = dot(o, d);
    float c = dot(o, o) - 1.0;
    float disc = b * b - a * c;
    if (disc <= 0.0) return vec2(1.0, 0.0);
    float q = -(b + (b < 0.0 ? -1.0 : 1.0) * sqrt(disc));
    if (q == 0.0) return vec2(1.0, 0.0);
    float t0 = q / a;
    float t1 = c / q;
    return vec2(min(t0, t1), max(t0, t1));
}}
)");

constexpr std::string_view setup = util::trim(R"(
// Unproject the fragment onto the near and far planes, the ray is parameterized by t in [0,1]
// from the near to the far plane, in all spaces since the transformations are affine.
vec2 {0}Ndc = 2.0 * texCoords - 1.0;
vec4 {0}NearH = {1}.clipToWorld * vec4({0}Ndc, -1.0, 1.0);
vec4 {0}FarH = {1}.clipToWorld * vec4({0}Ndc, 1.0, 1.0);
vec3 {0}Near = {0}NearH.xyz / {0}NearH.w;
vec3 {0}Far = {0}FarH.xyz / {0}FarH.w;

vec3 {0}ShapeNear = ({0}WorldToShape * vec4({0}Near, 1.0)).xyz;
vec3 {0}ShapeFar = ({0}WorldToShape * vec4({0}Far, 1.0)).xyz;
vec2 {0}Interval = {0}Intersect{2}({0}ShapeNear, {0}ShapeFar - {0}ShapeNear);

vec3 {0}TexNear = ({0}WorldToTexture * vec4({0}Near, 1.0)).xyz;
vec3 {0}TexFar = ({0}WorldToTexture * vec4({0}Far, 1.0)).xyz;
if ({0}ClipToVolume) {{
    vec2 {0}Box = {0}IntersectParallelepiped({0}TexNear, {0}TexFar - {0}TexNear);
    {0}Interval = vec2(max({0}Interval.x, {0}Box.x), min({0}Interval.y, {0}Box.y));
}}
{0}Interval = clamp({0}Interval, 0.0, 1.0);

vec3 entryPoint = vec3(0.0);
vec3 exitPoint = vec3(0.0);
float entryPointDepth = 1.0;
float exitPointDepth = 1.0;
if ({0}ValidShape && {0}Interval.x < {0}Interval.y) {{
    entryPoint = mix({0}TexNear, {0}TexFar, {0}Interval.x);
    exitPoint = mix({0}TexNear, {0}TexFar, {0}Interval.y);

    vec4 {0}EntryClip = {1}.worldToClip * vec4(mix({0}Near, {0}Far, {0}Interval.x), 1.0);
    vec4 {0}ExitClip = {1}.worldToClip * vec4(mix({0}Near, {0}Far, {0}Interval.y), 1.0);
    entryPointDepth = clamp(0.5 * {0}EntryClip.z / {0}EntryClip.w + 0.5, 0.0, 1.0);
    exitPointDepth = clamp(0.5 * {0}ExitClip.z / {0}ExitClip.w + 0.5, 0.0, 1.0);
}}

// The length of the ray in texture space
float rayLength = length(exitPoint - entryPoint);

// The normalized direction of the ray
vec3 rayDirection = rayLength > 0.0 ? (exitPoint - entryPoint) / rayLength : vec3(0.0);
)");

}  // namespace

auto AnalyticEntryExitComponent::getSegments() -> std::vector<Segment> {
    const std::string_view shapeName =
        shape.getSelectedValue() == Shape::Sphere ? "Sphere" : "Parallelepiped";

    return {{.snippet = fmt::format(uniforms, prefix),
             .placeholder = placeholder::uniform,
             .priority = 100},
            {.snippet = fmt::format(setup, prefix, cameraName_, shapeName),
             .placeholder = placeholder::setup,
             .priority = 100}};
}

}  // namespace inviwo
