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

#include <algorithm>
#include <cmath>
#include <numbers>

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

namespace inviwo {

namespace {

constexpr std::string_view prefix = "entryExit";

constexpr double twoPi = 2.0 * std::numbers::pi;

// Does [start, end] contain the angle modulo 2 pi
bool containsAngle(double angle, double start, double end) {
    return angle + std::ceil((start - angle) / twoPi) * twoPi <= end;
}

dvec2 cosRange(double start, double end) {
    const auto a = std::cos(start);
    const auto b = std::cos(end);
    return {containsAngle(std::numbers::pi, start, end) ? -1.0 : std::min(a, b),
            containsAngle(0.0, start, end) ? 1.0 : std::max(a, b)};
}

dvec2 sinRange(double start, double end) {
    const auto a = std::sin(start);
    const auto b = std::sin(end);
    return {containsAngle(-0.5 * std::numbers::pi, start, end) ? -1.0 : std::min(a, b),
            containsAngle(0.5 * std::numbers::pi, start, end) ? 1.0 : std::max(a, b)};
}

// Range of a * b for a in [a.x, a.y] and b in [b.x, b.y]
dvec2 productRange(dvec2 a, dvec2 b) {
    const auto [lo, hi] = std::minmax({a.x * b.x, a.x * b.y, a.y * b.x, a.y * b.y});
    return {lo, hi};
}

// Range of r * g for r in [rMin, 1] and g in [g.x, g.y]
dvec2 radialRange(dvec2 g, double rMin) {
    return {g.x < 0.0 ? g.x : rMin * g.x, g.y > 0.0 ? g.y : rMin * g.y};
}

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
    , innerRadius{"innerRadius", "Inner Radius",
                  util::ordinalLength(0.0f, 1.0f)
                      .setMax(ConstraintBehavior::Immutable)
                      .setInc(0.01f)
                      .set("Radius of the clipped away center of the sphere, relative to the "
                           "radius. Zero means no clipping"_help)}
    , longitude{"longitude", "Longitude",
                MinMaxPropertyState<float>{.valueMin = 0.0f,
                                           .valueMax = 360.0f,
                                           .rangeMin = -360.0f,
                                           .rangeMax = 360.0f,
                                           .increment = 1.0f}
                    .set("Longitude range in degrees of the sphere to keep. The longitude is "
                         "the angle around the z axis, measured from the x axis"_help)}
    , latitude{"latitude", "Latitude",
               MinMaxPropertyState<float>{.valueMin = -90.0f,
                                          .valueMax = 90.0f,
                                          .rangeMin = -90.0f,
                                          .rangeMax = 90.0f,
                                          .increment = 1.0f}
                   .set("Latitude range in degrees of the sphere to keep. The latitude is the "
                        "angle above the xy plane, i.e. -90 is the south and 90 the north "
                        "pole"_help)}
    , getTransform_{std::move(getTransform)}
    , cameraName_{cameraName} {

    geometry.addProperties(shape, space, clipToVolume, origin, edge1, edge2, edge3, center, radius,
                           innerRadius, longitude, latitude);

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
    innerRadius.visibilityDependsOn(shape, isSphere);
    longitude.visibilityDependsOn(shape, isSphere);
    latitude.visibilityDependsOn(shape, isSphere);
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

glm::dmat4 AnalyticEntryExitComponent::boundingBox() const {
    switch (shape.getSelectedValue()) {
        case Shape::Sphere: {
            const auto lon = glm::radians(dvec2{longitude.get()});
            const auto lat = glm::radians(dvec2{latitude.get()});
            const double rMin = innerRadius.get();

            // Tight bounds of the cut unit sphere, x, y, z = r (cos lat cos lon, cos lat sin lon,
            // sin lat)
            const dvec2 cosLat{std::min(std::cos(lat.x), std::cos(lat.y)),
                               std::cos(std::clamp(0.0, lat.x, lat.y))};
            const auto x = radialRange(productRange(cosLat, cosRange(lon.x, lon.y)), rMin);
            const auto y = radialRange(productRange(cosLat, sinRange(lon.x, lon.y)), rMin);
            const auto z = radialRange(dvec2{std::sin(lat.x), std::sin(lat.y)}, rMin);

            const dmat4 box{dvec4{x.y - x.x, 0.0, 0.0, 0.0}, dvec4{0.0, y.y - y.x, 0.0, 0.0},
                            dvec4{0.0, 0.0, z.y - z.x, 0.0}, dvec4{x.x, y.x, z.x, 1.0}};

            const auto r = static_cast<double>(radius.get());
            return glm::scale(glm::translate(dmat4{1.0}, dvec3{center.get()}), dvec3{r}) * box;
        }
        case Shape::Parallelepiped:
        default:
            return dmat4{dvec4{dvec3{edge1.get()}, 0.0}, dvec4{dvec3{edge2.get()}, 0.0},
                         dvec4{dvec3{edge3.get()}, 0.0}, dvec4{dvec3{origin.get()}, 1.0}};
    }
}

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

    constexpr float eps = 1.0e-3f;
    const bool clipLon = longitude.getEnd() - longitude.getStart() < 360.0f - eps;
    const bool clipLat = latitude.getStart() > -90.0f + eps || latitude.getEnd() < 90.0f - eps;
    const bool clipSphere = shape.getSelectedValue() == Shape::Sphere &&
                            (innerRadius.get() > 0.0f || clipLon || clipLat);

    shader.setUniform(fmt::format("{}ClipSphere", prefix), clipSphere);
    shader.setUniform(fmt::format("{}InnerRadius", prefix), innerRadius.get());
    shader.setUniform(fmt::format("{}ClipLon", prefix), clipLon);
    shader.setUniform(fmt::format("{}LonRange", prefix),
                      glm::radians(vec2{longitude.getStart(), longitude.getEnd()}));
    shader.setUniform(fmt::format("{}ClipLat", prefix), clipLat);
    shader.setUniform(fmt::format("{}LatRange", prefix),
                      glm::radians(vec2{latitude.getStart(), latitude.getEnd()}));
}

std::vector<Property*> AnalyticEntryExitComponent::getProperties() { return {&geometry}; }

namespace {

constexpr std::string_view uniforms = util::trim(R"(
uniform mat4 {0}WorldToShape;
uniform mat4 {0}WorldToTexture;
uniform bool {0}ValidShape;
uniform bool {0}ClipToVolume;

uniform bool {0}ClipSphere;
uniform float {0}InnerRadius;
uniform bool {0}ClipLon;
uniform vec2 {0}LonRange;
uniform bool {0}ClipLat;
uniform vec2 {0}LatRange;

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

// Intersect the ray o + t * d with a sphere of the given radius centered at the origin.
bool {0}SphereRoots(in vec3 o, in vec3 d, in float radius, out vec2 t) {{
    // Solve a t^2 + 2 b t + c = 0 using a numerically stable formulation
    t = vec2(1.0, 0.0);
    float a = dot(d, d);
    float b = dot(o, d);
    float c = dot(o, o) - radius * radius;
    float disc = b * b - a * c;
    if (disc <= 0.0) return false;
    float q = -(b + (b < 0.0 ? -1.0 : 1.0) * sqrt(disc));
    if (q == 0.0) return false;
    float t0 = q / a;
    float t1 = c / q;
    t = vec2(min(t0, t1), max(t0, t1));
    return true;
}}

// Is the point, given in the space of the unit sphere, inside the clipped sphere (with tolerance)
bool {0}InClippedSphere(in vec3 p) {{
    const float eps = 1.0e-3;
    float r = length(p);
    if (r > 1.0 + eps || r < {0}InnerRadius - eps) return false;
    if ({0}ClipLon && length(p.xy) > 1.0e-6) {{
        float lon = atan(p.y, p.x);
        bool inside = false;
        for (int i = -1; i <= 1; ++i) {{
            float l = lon + 6.28318530718 * float(i);
            inside = inside || (l >= {0}LonRange.x - eps && l <= {0}LonRange.y + eps);
        }}
        if (!inside) return false;
    }}
    if ({0}ClipLat && r > 1.0e-6) {{
        float lat = asin(clamp(p.z / r, -1.0, 1.0));
        if (lat < {0}LatRange.x - eps || lat > {0}LatRange.y + eps) return false;
    }}
    return true;
}}

// Extend the hull [t0, t1] of the ray parameters inside the clipped sphere with t
void {0}Candidate(in vec3 o, in vec3 d, in float t, inout vec2 hull) {{
    if (t >= 0.0 && t <= 1.0 && {0}InClippedSphere(o + t * d)) {{
        hull = vec2(min(hull.x, t), max(hull.y, t));
    }}
}}

// Plane through the z axis with the given normal
void {0}PlaneCandidate(in vec3 o, in vec3 d, in vec3 n, inout vec2 hull) {{
    float nd = dot(n, d);
    if (nd != 0.0) {0}Candidate(o, d, -dot(n, o) / nd, hull);
}}

// Cone with the apex at the origin around the z axis, points at the given latitude
void {0}ConeCandidates(in vec3 o, in vec3 d, in float lat, inout vec2 hull) {{
    float s = sin(lat);
    float c = cos(lat);
    if (abs(c) < 1.0e-4) return; // the poles are a single point on the z axis
    if (abs(s) < 1.0e-6) {{
        if (d.z != 0.0) {0}Candidate(o, d, -o.z / d.z, hull);
        return;
    }}
    float c2 = c * c;
    float s2 = s * s;
    float a = c2 * d.z * d.z - s2 * dot(d.xy, d.xy);
    float b = c2 * o.z * d.z - s2 * dot(o.xy, d.xy);
    float cc = c2 * o.z * o.z - s2 * dot(o.xy, o.xy);
    if (abs(a) < 1.0e-12) {{
        if (b != 0.0) {0}Candidate(o, d, -cc / (2.0 * b), hull);
        return;
    }}
    float disc = b * b - a * cc;
    if (disc < 0.0) return;
    float sq = sqrt(disc);
    {0}Candidate(o, d, (-b - sq) / a, hull);
    {0}Candidate(o, d, (-b + sq) / a, hull);
}}

// Intersect the ray o + t * d with the unit sphere centered at the origin, optionally clipped by
// an inner radius and a longitude and latitude range.
// Returns the parameter interval [t0, t1] inside the sphere, t0 >= t1 means no intersection.
// For a clipped sphere the interval spans from the first to the last point inside the shape.
vec2 {0}IntersectSphere(in vec3 o, in vec3 d) {{
    vec2 outer;
    if (!{0}SphereRoots(o, d, 1.0, outer)) return vec2(1.0, 0.0);
    if (!{0}ClipSphere) return outer;

    // The extremes of the ray inside the shape are either at the ends of the ray or on its boundary
    vec2 hull = vec2(1.0, 0.0);
    {0}Candidate(o, d, 0.0, hull);
    {0}Candidate(o, d, 1.0, hull);
    {0}Candidate(o, d, outer.x, hull);
    {0}Candidate(o, d, outer.y, hull);
    if ({0}InnerRadius > 0.0) {{
        vec2 inner;
        if ({0}SphereRoots(o, d, {0}InnerRadius, inner)) {{
            {0}Candidate(o, d, inner.x, hull);
            {0}Candidate(o, d, inner.y, hull);
        }}
    }}
    if ({0}ClipLon) {{
        {0}PlaneCandidate(o, d, vec3(-sin({0}LonRange.x), cos({0}LonRange.x), 0.0), hull);
        {0}PlaneCandidate(o, d, vec3(-sin({0}LonRange.y), cos({0}LonRange.y), 0.0), hull);
    }}
    if ({0}ClipLat) {{
        {0}ConeCandidates(o, d, {0}LatRange.x, hull);
        {0}ConeCandidates(o, d, {0}LatRange.y, hull);
    }}
    return hull;
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
