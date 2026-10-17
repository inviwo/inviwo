/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <modules/base/algorithm/meshutils.h>
#include <modules/base/basemoduledefine.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <utility>
#include <numbers>
#include <ranges>
#include <numeric>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inviwo::meshutil {

namespace detail {

static vec3 orthvec(const vec3& vec) {
    const vec3 u(1.0f, 0.0f, 0.0f);
    const vec3 n = glm::normalize(vec);
    const float p = glm::dot(u, n);
    if (std::abs(p) != 1.0f) {
        return glm::normalize(u - p * n);
    } else {
        return {0.0f, 1.0f, 0.0f};
    }
}

static vec3 tospherical(const vec2& v) {
    return {std::sin(v.x) * std::cos(v.y), std::sin(v.x) * std::sin(v.y), std::cos(v.x)};
}

}  // namespace detail

std::shared_ptr<BasicMesh> ellipse(const vec3& center, const vec3& majorAxis, const vec3& minorAxis,
                                   const vec4& color, const float& radius,
                                   const size_t& nsegments) {
    auto mesh = std::make_shared<BasicMesh>();
    const auto angle = static_cast<float>(std::numbers::pi * 2.0 / static_cast<double>(nsegments));
    const float a = glm::length(majorAxis);
    const float b = glm::length(minorAxis);

    const auto pos = [&](std::uint32_t i) {
        const auto x = glm::cos(static_cast<float>(i) * angle) * a;
        const auto y = glm::sin(static_cast<float>(i) * angle) * b;
        const auto eRadius =
            glm::length(glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y);
        const auto rot = glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y;
        const auto p = center + eRadius * glm::normalize(rot);
        return p;
    };

    auto segments = static_cast<std::uint32_t>(nsegments);
    for (std::uint32_t i = 0; i < segments; ++i) {
        const vec3 p = pos(i);
        const vec3 p1 = pos((i + 1) % segments);

        const vec3 dir = glm::normalize(p1 - p);
        const vec3 odir = detail::orthvec(dir);

        BasicMesh tube{};
        const auto inds = tube.addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
        for (std::uint32_t j = 0; j < segments; ++j) {
            const auto k = static_cast<float>(j);
            const vec3 o = glm::rotate(odir, static_cast<float>(j) * angle, dir);
            tube.addVertex(p + radius * o, o, vec3(k / static_cast<float>(segments), 0.0f, 0.0f),
                           color);
            tube.addVertex(p1 + radius * o, o, vec3(k / static_cast<float>(segments), 1.0f, 0.0f),
                           color);

            inds->add(j * 2 + 1);
            inds->add(j * 2 + 0);
            inds->add(((j + 1) * 2) % (2 * segments) + 0);

            inds->add(j * 2 + 1);
            inds->add(((j + 1) * 2) % (2 * segments) + 0);
            inds->add(((j + 1) * 2) % (2 * segments) + 1);
        }
        mesh->append(&tube);
    }
    return mesh;
}

std::shared_ptr<BasicMesh> disk(const vec3& center, const vec3& normal, const vec4& color,
                                const float& radius, const size_t& segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    const vec3 orth = detail::orthvec(normal);

    mesh->addVertex(center, normal, vec3(0.5f, 0.5f, 0.0f), color);

    const vec3 tc = vec3(0.5f, 0.5f, 0.0f);
    const vec3 tn = vec3(0.0f, 0.0f, 1.0f);
    const vec3 to = vec3(0.5f, 0.0f, 0.0f);
    vec3 p;
    vec3 t;
    const double angle = 2.0 * std::numbers::pi / static_cast<double>(segments);
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        p = center + radius * glm::rotate(orth, static_cast<float>(i * angle), normal);
        t = tc + glm::rotate(to, static_cast<float>(i * angle), tn);
        mesh->addVertex(p, normal, t, color);
        inds->add({0, 1 + i, 1 + ((i + 1) % ns)});
    }
    return mesh;
}

std::shared_ptr<BasicMesh> cone(const vec3& start, const vec3& stop, const vec4& color,
                                const float& radius, const size_t& segments) {
    const vec3 tc(0.5f, 0.5f, 0.0f);

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    const vec3 e1 = glm::normalize(stop - start);
    const vec3 e2 = detail::orthvec(e1);
    const vec3 e3 = glm::cross(e1, e2);
    const mat3 basis(e1, e2, e3);

    const float height = glm::length(stop - start);
    const float ratio = radius / height;

    const double angle = 2.0 * std::numbers::pi / static_cast<double>(segments);
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        // first vertex at base
        double x = cos(i * angle);
        double y = sin(i * angle);
        vec3 normal = basis * glm::normalize(vec3(ratio, x, y));
        vec3 pos = start + basis * vec3(0.0, x * radius, y * radius);
        vec3 texCoord = tc + vec3(0.5 * x, 0.5 * y, 0.0);
        mesh->addVertex(pos, normal, texCoord, color);

        // second vertex at base
        x = cos((i + 1) * angle);
        y = sin((i + 1) * angle);
        normal = basis * glm::normalize(vec3(ratio, x, y));
        pos = start + basis * vec3(0.0, x * radius, y * radius);
        texCoord = tc + vec3(0.5 * x, 0.5 * y, 0.0);
        mesh->addVertex(pos, normal, texCoord, color);

        // third vertex at tip
        x = cos((i + 0.5) * angle);
        y = sin((i + 0.5) * angle);
        normal = basis * glm::normalize(vec3(ratio, x, y));
        mesh->addVertex(stop, normal, tc, color);

        // add indices
        inds->add({i * 3 + 0, i * 3 + 1, i * 3 + 2});
    }

    return mesh;
}

std::shared_ptr<BasicMesh> cylinder(const vec3& start, const vec3& stop, const vec4& color,
                                    const float& radius, const size_t& segments, bool caps,
                                    std::shared_ptr<BasicMesh> mesh) {
    std::uint32_t globalIndexOffset = 0;
    if (!mesh) {
        mesh = std::make_shared<BasicMesh>();
        mesh->setModelMatrix(mat4(1.f));
    } else {
        globalIndexOffset = static_cast<std::uint32_t>(mesh->getVertices()->getSize());
    }
    std::vector<BasicMesh::Vertex> vertices;
    vertices.reserve(segments * 2);

    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(6 * segments);
    const vec3 e1 = glm::normalize(stop - start);
    const vec3 e2 = detail::orthvec(e1);
    const vec3 e3 = glm::cross(e1, e2);
    const mat3 basis(e1, e2, e3);

    const double angle = 2.0 * std::numbers::pi / static_cast<double>(segments);
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        // first vertex at base
        const double x = cos(i * angle);
        const double y = sin(i * angle);
        const vec3 normal = basis * vec3(0.0, x, y);
        const vec3 dir = basis * vec3(0.0, x * radius, y * radius);
        const vec3 texCoord = vec3(static_cast<float>(i) / static_cast<double>(segments), 0.0, 0.0);

        // vertex at base
        vertices.emplace_back(start + dir, normal, texCoord, color);
        // vertex at top
        vertices.emplace_back(stop + dir, normal, texCoord + vec3(0.0, 1.0, 0.0), color);

        // indices for two triangles filling the strip
        inds->add(i * 2 + 1 + globalIndexOffset);
        inds->add(i * 2 + 0 + globalIndexOffset);
        inds->add(((i + 1) * 2) % (2 * ns) + 0 + globalIndexOffset);

        inds->add(i * 2 + 1 + globalIndexOffset);
        inds->add(((i + 1) * 2) % (2 * ns) + 0 + globalIndexOffset);
        inds->add(((i + 1) * 2) % (2 * ns) + 1 + globalIndexOffset);
    }

    mesh->addVertices(vertices);

    // add end caps
    if (caps) {
        const auto startcap = disk(start, -e1, color, radius, segments);
        const auto endcap = disk(stop, e1, color, radius, segments);

        mesh->append(startcap.get());
        mesh->append(endcap.get());
    }

    return mesh;
}

std::shared_ptr<BasicMesh> point(vec3 position, vec4 color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    mesh->addVertex(position, vec3{0.0f}, vec3{0.0f}, color);

    const auto inds = mesh->addIndexBuffer(DrawType::Points, ConnectivityType::None);
    inds->add(0);

    return mesh;
}

std::shared_ptr<BasicMesh> line(vec3 start, vec3 stop, vec4 color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    const vec3 normal = detail::orthvec(stop - start);

    mesh->addVertex(start, normal, vec3{0.0f, 0.0f, 0.0f}, color);
    mesh->addVertex(stop, normal, vec3{1.0f, 0.0f, 0.0f}, color);

    const auto inds = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
    inds->add({0, 1});

    return mesh;
}

std::shared_ptr<BasicMesh> circle(vec3 center, vec3 normal, vec4 color, float radius,
                                  size_t segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    const auto inds = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::Loop);
    const vec3 orth = detail::orthvec(normal);

    vec3 t{0.0f};
    const double angle = 2.0 * std::numbers::pi / static_cast<double>(segments);
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        const vec3 p = center + radius * glm::rotate(orth, static_cast<float>(i * angle), normal);
        t.x = static_cast<float>(i) / static_cast<float>(segments);
        mesh->addVertex(p, normal, t, color);
    }
    inds->append(std::views::iota(static_cast<std::uint32_t>(0), ns) |
                 std::ranges::to<std::vector>());

    return mesh;
}

std::shared_ptr<BasicMesh> line(const vec3& start, const vec3& stop, const vec3& normal,
                                const vec4& color /*= vec4(1.0f, 0.0f, 0.0f, 1.0f)*/,
                                const float& width /*= 1.0f*/, const ivec2& inres /*= ivec2(1)*/) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    const vec3 direction = stop - start;
    const vec3 up = glm::cross(glm::normalize(direction), normal);

    const vec3 startCornerPoint = start - 0.5f * width * up;
    const ivec2 res = inres + ivec2(1);

    for (int j = 0; j < res.y; j++) {
        for (int i = 0; i < res.x; i++) {
            mesh->addVertex(startCornerPoint +
                                static_cast<float>(i) / static_cast<float>(inres.x) * direction +
                                static_cast<float>(j) / static_cast<float>(inres.y) * width * up,
                            normal,
                            vec3(static_cast<float>(i) / static_cast<float>(inres.x),
                                 static_cast<float>(j) / static_cast<float>(inres.y), 0.0f),
                            color);

            if (i != inres.x && j != inres.y) {
                inds->add(i + res.x * j);
                inds->add(i + 1 + res.x * j);
                inds->add(i + res.x * (j + 1));

                inds->add(i + 1 + res.x * j);
                inds->add(i + 1 + res.x * (j + 1));
                inds->add(i + res.x * (j + 1));
            }
        }
    }

    return mesh;
}

std::shared_ptr<Mesh> rotatingLine(vec3 start, vec3 stop, float endAngle, vec4 color,
                                   size_t segments, float width, bool withAdjacency) {
    auto mesh = std::make_shared<Mesh>();

    const float length = glm::length(stop - start);
    const vec3 direction{1.0f, 0.0f, 0.0f};
    mat4 m = glm::mat4_cast(glm::rotation(direction, glm::normalize(stop - start)));
    m[3] = vec4{start, 1.0f};
    mesh->setModelMatrix(m);

    segments = std::max<size_t>(segments, 1);
    const vec3 step{direction * length / static_cast<float>(segments)};
    const float deltaAngle = glm::radians(endAngle) / static_cast<float>(segments);

    auto positions =
        std::views::iota(size_t{0}, segments + 1) |
        std::views::transform([step](auto i) { return step * static_cast<float>(i); }) |
        std::ranges::to<std::vector>();
    auto normals = std::views::iota(size_t{0}, segments + 1) | std::views::transform([&](auto i) {
                       const float angle = deltaAngle * static_cast<float>(i);
                       return vec3{0.0f, std::cos(angle) * width, std::sin(angle) * width};
                   }) |
                   std::ranges::to<std::vector>();

    mesh->addBuffer(BufferType::PositionAttrib, util::makeBuffer(std::move(positions)));
    mesh->addBuffer(BufferType::NormalAttrib, util::makeBuffer(std::move(normals)));
    mesh->addBuffer(BufferType::ColorAttrib, util::makeBuffer(std::vector{segments + 1, color}));

    const auto ct = withAdjacency ? ConnectivityType::StripAdjacency : ConnectivityType::Strip;

    auto indices = std::views::iota(uint32_t{0}, static_cast<uint32_t>(segments + 1)) |
                   std::ranges::to<std::vector>();

    if (withAdjacency) {
        indices.insert(indices.begin(), 0);
        indices.emplace_back(indices.back());
    }
    mesh->addIndices(Mesh::MeshInfo{.dt = DrawType::Lines, .ct = ct},
                     util::makeIndexBuffer(std::move(indices)));
    return mesh;
}

std::shared_ptr<BasicMesh> arrow(const vec3& start, const vec3& stop, const vec4& color,
                                 const float& radius, const float& arrowfraction,
                                 const float& arrowRadius, const size_t& segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    const vec3 mid = glm::mix(start, stop, 1.0f - arrowfraction);
    const auto cylinderpart = cylinder(start, mid, color, radius, segments);
    mesh->append(cylinderpart.get());

    const auto diskpart = disk(mid, glm::normalize(start - stop), color, arrowRadius, segments);
    mesh->append(diskpart.get());

    const auto conepart = cone(mid, stop, color, arrowRadius, segments);
    mesh->append(conepart.get());

    return mesh;
}

std::shared_ptr<BasicMesh> colorsphere(const vec3& center, const float& radius,
                                       std::shared_ptr<BasicMesh> mesh) {
    const static std::vector<vec2> spheremesh = {
        {glm::half_pi<float>(), 0.0f},
        {glm::half_pi<float>(), glm::half_pi<float>()},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {std::atan(6.0f), 0.0f},
        {std::atan(5.0f / 2.0f), 0.0f},
        {std::atan(4.0f / 3.0f), 0.0f},
        {std::atan(3.0f / 4.0f), 0.0f},
        {std::atan(2.0f / 5.0f), 0.0f},
        {std::atan(1.0f / 6.0f), 0.0f},
        {glm::half_pi<float>(), std::atan(1.0f / 6.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)},
        {std::atan(std::numbers::sqrt2_v<float> / 5.0f), glm::quarter_pi<float>()},
        {std::atan(1.0f / 6.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(2.0f / 5.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)},
        {std::atan((2.0f * std::numbers::sqrt2_v<float>) / 3.0f), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)},
        {std::atan(2.0f / 5.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(3.0f / 4.0f)},
        {std::atan(3.0f * std::numbers::sqrt2_v<float>), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)},
        {std::atan(3.0f / 4.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(4.0f / 3.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)},
        {std::atan(4.0f / 3.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(5.0f / 2.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(5.0f)},
        {std::atan(5.0f / 2.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(6.0f)},
        {std::atan(6.0f), glm::half_pi<float>()}};

    const static std::vector<uvec3> sphereind = {
        {0, 10, 4},   {4, 11, 5},   {5, 12, 6},   {6, 13, 7},   {7, 14, 8},   {8, 15, 9},
        {9, 16, 2},   {10, 17, 11}, {11, 18, 12}, {12, 19, 13}, {13, 20, 14}, {14, 21, 15},
        {15, 22, 16}, {17, 23, 18}, {18, 24, 19}, {19, 25, 20}, {20, 26, 21}, {21, 27, 22},
        {23, 28, 24}, {24, 29, 25}, {25, 30, 26}, {26, 31, 27}, {28, 32, 29}, {29, 33, 30},
        {30, 34, 31}, {32, 35, 33}, {33, 36, 34}, {35, 1, 36},  {11, 4, 10},  {12, 5, 11},
        {13, 6, 12},  {14, 7, 13},  {15, 8, 14},  {16, 9, 15},  {18, 11, 17}, {19, 12, 18},
        {20, 13, 19}, {21, 14, 20}, {22, 15, 21}, {24, 18, 23}, {25, 19, 24}, {26, 20, 25},
        {27, 21, 26}, {29, 24, 28}, {30, 25, 29}, {31, 26, 30}, {33, 29, 32}, {34, 30, 33},
        {36, 33, 35}};

    std::uint32_t globalIndexOffset = 0;
    if (!mesh) {
        mesh = std::make_shared<BasicMesh>();
        mesh->setModelMatrix(mat4(1.f));
    } else {
        globalIndexOffset = static_cast<std::uint32_t>(mesh->getVertices()->getSize());
    }

    std::vector<BasicMesh::Vertex> vertices;
    vertices.reserve(spheremesh.size() * 8);
    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(sphereind.size() * 8 * 3);

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                const glm::uint32_t idxOffset =
                    static_cast<std::uint32_t>(vertices.size()) + globalIndexOffset;

                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                const vec4 color((quad + 1.0f) / 2.0f, 1.0f);
                for (const auto& elem : spheremesh) {
                    normal = quad * detail::tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * elem.x, quad.y * elem.y, 0.0f);
                    vertices.emplace_back(vertex, normal, tcoord, color);
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (const auto& elem : sphereind) {
                        inds->add({idxOffset + elem.x, idxOffset + elem.y, idxOffset + elem.z});
                    }
                } else {
                    for (const auto& elem : sphereind) {
                        inds->add({idxOffset + elem.z, idxOffset + elem.y, idxOffset + elem.x});
                    }
                }
            }
        }
    }
    mesh->addVertices(vertices);
    return mesh;
}

std::shared_ptr<BasicMesh> sphere(const vec3& center, const float& radius, const vec4& color,
                                  std::shared_ptr<BasicMesh> mesh) {
    const std::vector<vec2> spheremesh = {
        {glm::half_pi<float>(), 0.0f},
        {glm::half_pi<float>(), glm::half_pi<float>()},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {std::atan(6.0f), 0.0f},
        {std::atan(5.0f / 2.0f), 0.0f},
        {std::atan(4.0f / 3.0f), 0.0f},
        {std::atan(3.0f / 4.0f), 0.0f},
        {std::atan(2.0f / 5.0f), 0.0f},
        {std::atan(1.0f / 6.0f), 0.0f},
        {glm::half_pi<float>(), std::atan(1.0f / 6.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)},
        {std::atan(std::numbers::sqrt2_v<float> / 5.0f), glm::quarter_pi<float>()},
        {std::atan(1.0f / 6.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(2.0f / 5.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)},
        {std::atan((2.0f * std::numbers::sqrt2_v<float>) / 3.0f), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)},
        {std::atan(2.0f / 5.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(3.0f / 4.0f)},
        {std::atan(3.0f * std::numbers::sqrt2_v<float>), glm::quarter_pi<float>()},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)},
        {std::atan(3.0f / 4.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(4.0f / 3.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)},
        {std::atan(4.0f / 3.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(5.0f / 2.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(5.0f)},
        {std::atan(5.0f / 2.0f), glm::half_pi<float>()},
        {glm::half_pi<float>(), std::atan(6.0f)},
        {std::atan(6.0f), glm::half_pi<float>()}};

    const std::vector<uvec3> sphereind = {
        {0, 10, 4},   {4, 11, 5},   {5, 12, 6},   {6, 13, 7},   {7, 14, 8},   {8, 15, 9},
        {9, 16, 2},   {10, 17, 11}, {11, 18, 12}, {12, 19, 13}, {13, 20, 14}, {14, 21, 15},
        {15, 22, 16}, {17, 23, 18}, {18, 24, 19}, {19, 25, 20}, {20, 26, 21}, {21, 27, 22},
        {23, 28, 24}, {24, 29, 25}, {25, 30, 26}, {26, 31, 27}, {28, 32, 29}, {29, 33, 30},
        {30, 34, 31}, {32, 35, 33}, {33, 36, 34}, {35, 1, 36},  {11, 4, 10},  {12, 5, 11},
        {13, 6, 12},  {14, 7, 13},  {15, 8, 14},  {16, 9, 15},  {18, 11, 17}, {19, 12, 18},
        {20, 13, 19}, {21, 14, 20}, {22, 15, 21}, {24, 18, 23}, {25, 19, 24}, {26, 20, 25},
        {27, 21, 26}, {29, 24, 28}, {30, 25, 29}, {31, 26, 30}, {33, 29, 32}, {34, 30, 33},
        {36, 33, 35}};

    std::uint32_t globalIndexOffset = 0;
    if (!mesh) {
        mesh = std::make_shared<BasicMesh>();
        mesh->setModelMatrix(mat4(1.f));
    } else {
        globalIndexOffset = static_cast<std::uint32_t>(mesh->getVertices()->getSize());
    }

    std::vector<BasicMesh::Vertex> vertices;
    vertices.reserve(spheremesh.size() * 8);
    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(sphereind.size() * 8 * 3);

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                const glm::uint32_t idxOffset =
                    static_cast<std::uint32_t>(vertices.size()) + globalIndexOffset;
                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                for (const auto& elem : spheremesh) {
                    normal = quad * detail::tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * elem.x, quad.y * elem.y, 0.0f);
                    vertices.emplace_back(vertex, normal, tcoord, color);
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (const auto& elem : sphereind) {
                        inds->add({idxOffset + elem.x, idxOffset + elem.y, idxOffset + elem.z});
                    }
                } else {
                    for (const auto& elem : sphereind) {
                        inds->add({idxOffset + elem.z, idxOffset + elem.y, idxOffset + elem.x});
                    }
                }
            }
        }
    }
    mesh->addVertices(vertices);
    return mesh;
}

static vec3 V(const mat4& m, const vec3 v) {
    const vec4 V = m * vec4(v, 1);
    return vec3(V) / V.w;
}

static vec3 N(const mat4& m, const vec3 v) {
    const vec4 V = m * vec4(v, 0);
    return {V};
}

std::shared_ptr<BasicMesh> cube(const mat4& m, const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1));

    const auto indices = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    // Front back
    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(0, 0, -1)), vec3(0, 0, 0), color},
                       {V(m, vec3(1, 0, 0)), N(m, vec3(0, 0, -1)), vec3(1, 0, 0), color},
                       {V(m, vec3(1, 1, 0)), N(m, vec3(0, 0, -1)), vec3(1, 1, 0), color},
                       {V(m, vec3(0, 1, 0)), N(m, vec3(0, 0, -1)), vec3(0, 1, 0), color}});

    std::uint32_t o = 0;
    indices->add({0 + o, 1 + o, 2 + o, 0 + o, 2 + o, 3 + o});

    mesh->addVertices({{V(m, vec3(0, 0, 1)), N(m, vec3(0, 0, 1)), vec3(0, 0, 1), color},
                       {V(m, vec3(1, 0, 1)), N(m, vec3(0, 0, 1)), vec3(1, 0, 1), color},
                       {V(m, vec3(1, 1, 1)), N(m, vec3(0, 0, 1)), vec3(1, 1, 1), color},
                       {V(m, vec3(0, 1, 1)), N(m, vec3(0, 0, 1)), vec3(0, 1, 1), color}});

    o += 4;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    // Right left
    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(-1, 0, 0)), vec3(0, 0, 0), color},
                       {V(m, vec3(0, 1, 0)), N(m, vec3(-1, 0, 0)), vec3(0, 1, 0), color},
                       {V(m, vec3(0, 1, 1)), N(m, vec3(-1, 0, 0)), vec3(0, 1, 1), color},
                       {V(m, vec3(0, 0, 1)), N(m, vec3(-1, 0, 0)), vec3(0, 0, 1), color}});
    o += 4;
    indices->add({0 + o, 1 + o, 2 + o, 0 + o, 2 + o, 3 + o});

    mesh->addVertices({{V(m, vec3(1, 0, 0)), N(m, vec3(1, 0, 0)), vec3(1, 0, 0), color},
                       {V(m, vec3(1, 1, 0)), N(m, vec3(1, 0, 0)), vec3(1, 1, 0), color},
                       {V(m, vec3(1, 1, 1)), N(m, vec3(1, 0, 0)), vec3(1, 1, 1), color},
                       {V(m, vec3(1, 0, 1)), N(m, vec3(1, 0, 0)), vec3(1, 0, 1), color}});
    o += 4;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    // top bottom
    mesh->addVertices({{V(m, vec3(0, 1, 0)), N(m, vec3(0, 1, 0)), vec3(0, 1, 0), color},
                       {V(m, vec3(1, 1, 0)), N(m, vec3(0, 1, 0)), vec3(1, 1, 0), color},
                       {V(m, vec3(1, 1, 1)), N(m, vec3(0, 1, 0)), vec3(1, 1, 1), color},
                       {V(m, vec3(0, 1, 1)), N(m, vec3(0, 1, 0)), vec3(0, 1, 1), color}});
    o += 4;
    indices->add({0 + o, 1 + o, 2 + o, 0 + o, 2 + o, 3 + o});

    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(0, -1, 0)), vec3(0, -1, 0), color},
                       {V(m, vec3(1, 0, 0)), N(m, vec3(0, -1, 0)), vec3(1, -1, 0), color},
                       {V(m, vec3(1, 0, 1)), N(m, vec3(0, -1, 0)), vec3(1, -1, 1), color},
                       {V(m, vec3(0, 0, 1)), N(m, vec3(0, -1, 0)), vec3(0, -1, 1), color}});
    o += 4;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    return mesh;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
std::shared_ptr<Mesh> cubeIndicator(const mat4& basisAndOffset) {
    constexpr float offset = 0.2f;
    constexpr std::array<float, 4> p{{0.0f, 0.0f + offset, 1.0f - offset, 1.0f}};
    constexpr size_t pointsPerFace = (p.size() - 1) * (p.size() - 1) * 4;
    constexpr size_t indicesPerFace = (p.size() - 1) * (p.size() - 1) * 6;
    constexpr size_t facesPerCube = 6;

    std::vector<vec3> positions;
    positions.reserve(facesPerCube * pointsPerFace);
    std::vector<vec3> normals;
    normals.reserve(positions.capacity());
    std::vector<vec4> colors;
    colors.reserve(positions.capacity());
    std::vector<std::uint32_t> pickIds;
    pickIds.reserve(positions.capacity());

    std::vector<std::uint32_t> indices;
    indices.reserve(facesPerCube * indicesPerFace);

    constexpr auto index = [](float x) -> std::uint32_t {
        if (x < 1.0f / 3.0f) {
            return 0;
        } else if (x > 2.0f / 3.0f) {
            return 2;
        } else {
            return 1;
        }
    };

    constexpr auto xyz = +[](const vec3& v) { return vec3{v.x, v.y, v.z}; };
    constexpr auto zxy = +[](const vec3& v) { return vec3{v.z, v.x, v.y}; };
    constexpr auto yzx = +[](const vec3& v) { return vec3{v.y, v.z, v.x}; };

    // generate a mesh of 3x3 quads on each side of the cube
    for (const auto& swizzle : {xyz, zxy, yzx}) {
        for (const auto x : {0.0f, 1.0f}) {
            for (const auto [y1, y2] : std::views::zip(p, p | std::views::drop(1))) {
                for (const auto [z1, z2] : std::views::zip(p, p | std::views::drop(1))) {
                    const auto c1 = vec3{x, y1, z1};
                    const auto c2 = vec3{x, y2, z1};
                    const auto c3 = vec3{x, y2, z2};
                    const auto c4 = vec3{x, y1, z2};

                    for (const uint32_t i : {0, 1, 2, 0, 2, 3}) {
                        indices.emplace_back(static_cast<uint32_t>(positions.size()) + i);
                    }

                    const auto cs1 = swizzle(c1);
                    const auto cs2 = swizzle(c2);
                    const auto cs3 = swizzle(c3);
                    const auto cs4 = swizzle(c4);

                    positions.emplace_back(cs1);
                    positions.emplace_back(cs2);
                    positions.emplace_back(cs3);
                    positions.emplace_back(cs4);

                    const auto center = (cs1 + cs2 + cs3 + cs4) / 4.0f;
                    const auto pickPos =
                        glm::u32vec3{index(center.x), index(center.y), index(center.z)};

                    const std::uint32_t pickId = pickPos.x + 3 * pickPos.y + 9 * pickPos.z;
                    pickIds.emplace_back(pickId);
                    pickIds.emplace_back(pickId);
                    pickIds.emplace_back(pickId);
                    pickIds.emplace_back(pickId);
                }
            }
        }
    }

    // generate normals for each point
    for (const auto& normal : {vec3{1.f, 0.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec3{0.f, 0.f, 1.f}}) {
        for (const auto side : {1.f, -1.f}) {
            for ([[maybe_unused]] const auto _ : std::views::iota(size_t{0}, pointsPerFace)) {
                normals.emplace_back(normal * side);
            }
        }
    }

    // generate colors for each point
    for (const auto& color : {vec3{1.f, 0.f, 0.f}, vec3{0.f, 1.f, 0.f}, vec3{0.f, 0.f, 1.f}}) {
        for (const auto side : {-0.2f, 0.2f}) {
            for (const auto& xCorner : {vec3{0.1f}, vec3{0.0f}, vec3{0.1f}}) {
                for (const auto& yCorner : {vec3{0.1f}, vec3{0.0f}, vec3{0.1f}}) {
                    for ([[maybe_unused]] const auto _ : std::views::iota(0, 4)) {
                        const auto c = 0.5f * color + side + xCorner + yCorner;
                        colors.emplace_back(c, 1.0);
                    }
                }
            }
        }
    }

    const auto pos = std::make_shared<Buffer<vec3>>(
        std::make_shared<BufferRAMPrecision<vec3>>(std::move(positions)));
    const auto norm = std::make_shared<Buffer<vec3>>(
        std::make_shared<BufferRAMPrecision<vec3>>(std::move(normals)));
    const auto col = std::make_shared<Buffer<vec4>>(
        std::make_shared<BufferRAMPrecision<vec4>>(std::move(colors)));
    const auto pick = std::make_shared<Buffer<std::uint32_t>>(
        std::make_shared<BufferRAMPrecision<std::uint32_t>>(std::move(pickIds)));

    const auto inds =
        std::make_shared<IndexBuffer>(std::make_shared<IndexBufferRAM>(std::move(indices)));

    auto mesh = std::make_shared<Mesh>(
        Mesh::BufferVector{{BufferType::PositionAttrib, pos},
                           {BufferType::NormalAttrib, norm},
                           {BufferType::ColorAttrib, col},
                           {BufferType::PickingAttrib, pick}},
        Mesh::IndexVector{{Mesh::MeshInfo{DrawType::Triangles, ConnectivityType::None}, inds}});

    mesh->setModelMatrix(basisAndOffset);

    return mesh;
}

std::shared_ptr<BasicMesh> coordindicator(const vec3& center, const float& size) {
    const size_t segments = 16;
    const float bsize = size * 1.0f;
    const float fsize = size * 1.2f;
    const float radius = size * 0.08f;
    const float arrowpart = 0.12f;
    const float arrowRadius = 2.0f * radius;
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    const auto xarrow =
        arrow(center - vec3(bsize, 0.0f, 0.0f), center + vec3(fsize, 0.0f, 0.0f),
              vec4(1.0f, 0.0f, 0.0f, 1.0f), radius, arrowpart, arrowRadius, segments);
    const auto yarrow =
        arrow(center - vec3(0.0f, bsize, 0.0f), center + vec3(0.0f, fsize, 0.0f),
              vec4(0.0f, 1.0f, 0.0f, 1.0f), radius, arrowpart, arrowRadius, segments);
    const auto zarrow =
        arrow(center - vec3(0.0f, 0.0f, bsize), center + vec3(0.0f, 0.0f, fsize),
              vec4(0.0f, 0.0f, 1.0f, 1.0f), radius, arrowpart, arrowRadius, segments);

    const auto sphere = colorsphere(center, 0.7f * size);

    mesh->append(sphere.get());
    mesh->append(xarrow.get());
    mesh->append(yarrow.get());
    mesh->append(zarrow.get());

    return mesh;
}

std::shared_ptr<BasicMesh> boundingbox(const mat4& basisAndOffset, const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(basisAndOffset);

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), color},
                       {vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 0.0), color},
                       {vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), color},
                       {vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), color},
                       {vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color},
                       {vec3(1.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color}});

    const auto inds = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
    inds->add({0, 1, 0, 2, 0, 3, 1, 6, 1, 4, 2, 5, 2, 4, 3, 5, 3, 6, 5, 7, 6, 7, 4, 7});

    return mesh;
}

//! [Using PosTexColorMesh]
std::shared_ptr<PosTexColorMesh> boundingBoxAdjacency(const mat4& basisAndOffset,
                                                      const vec4& color) {
    auto mesh = std::make_shared<PosTexColorMesh>();
    mesh->setModelMatrix(basisAndOffset);

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), color},
                       {vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), color},
                       {vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), color},
                       {vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 0.0), color},
                       {vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, 1.0), color},
                       {vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color},
                       {vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color}});

    const auto inds1 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds1->add({3, 0, 1, 2, 3, 0, 1});

    const auto inds2 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds2->add({7, 4, 5, 6, 7, 4, 5});

    const auto inds3 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds3->add({3, 0, 4, 7, 3, 0, 4});

    const auto inds4 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds4->add({2, 1, 5, 6, 2, 1, 2});

    return mesh;
}
//! [Using PosTexColorMesh]

std::shared_ptr<BasicMesh> torus(const vec3& center, const vec3& up_, float r1, float r2,
                                 const ivec2& subdivisions, vec4 color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 side;
    const auto up = glm::normalize(up_);
    if (std::abs(std::abs(glm::dot(up, vec3(0, 1, 0))) - 1) <
        std::numeric_limits<float>::epsilon()) {
        side = vec3(1, 0, 0);
    } else {
        side = glm::normalize(glm::cross(up, vec3(0, 1, 0)));
    }

    const auto numVertex = subdivisions.x * subdivisions.y;

    for (int i = 0; i < subdivisions.x; i++) {
        const auto axis =
            glm::rotate(side, static_cast<float>(i * 2 * std::numbers::pi / subdivisions.x), up);
        const auto centerP = center + axis * r1;

        const auto N = glm::normalize(glm::cross(axis, up));

        const int startI = i * subdivisions.y;
        const int startJ = ((i + 1) % subdivisions.x) * subdivisions.y;

        for (int j = 0; j < subdivisions.y; j++) {
            const auto axis2 =
                glm::rotate(axis, static_cast<float>(j * 2 * std::numbers::pi / subdivisions.y), N);
            const auto VP = centerP + axis2 * r2;

            mesh->addVertex(VP, axis2, N, color);

            const int i0 = startI + j;
            const int i1 = startI + j + 1;
            const int j0 = startJ + j;
            const int j1 = startJ + j + 1;

            inds->add(i0 % numVertex);
            inds->add(j0 % numVertex);
            inds->add(i1 % numVertex);

            inds->add(j0 % numVertex);
            inds->add(j1 % numVertex);
            inds->add(i1 % numVertex);
        }
    }

    return mesh;
}

std::shared_ptr<BasicMesh> square(const vec3& center, const vec3& normal, const vec2& extent,
                                  const vec4& color /*= vec4(1,1,1,1)*/,
                                  const ivec2& segments /*= ivec2(1)*/) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    const auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    const vec3 right = detail::orthvec(normal);
    const vec3 up = glm::cross(right, normal);

    const vec3 start = center - 0.5f * extent.x * right - 0.5f * extent.y * up;
    const ivec2 res = segments + ivec2(1);

    for (int j = 0; j < res.y; j++) {
        for (int i = 0; i < res.x; i++) {
            mesh->addVertex(
                start + static_cast<float>(i) / static_cast<float>(segments.x) * extent.x * right +
                    static_cast<float>(j) / static_cast<float>(segments.y) * extent.y * up,
                normal,
                vec3(static_cast<float>(i) / static_cast<float>(segments.x),
                     static_cast<float>(j) / static_cast<float>(segments.y), 0.0f),
                color);

            if (i != segments.x && j != segments.y) {
                inds->add(i + res.x * (j + 1));
                inds->add(i + 1 + res.x * j);
                inds->add(i + res.x * j);

                inds->add(i + res.x * (j + 1));
                inds->add(i + 1 + res.x * (j + 1));
                inds->add(i + 1 + res.x * j);
            }
        }
    }

    return mesh;
}

//! [Using Colored Mesh]
std::shared_ptr<ColoredMesh> cameraFrustum(const Camera& camera, vec4 color,
                                           std::shared_ptr<ColoredMesh> mesh) {
    const static std::vector<vec3> vertices{vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(1, -1, -1),
                                            vec3(1, 1, -1),   vec3(-1, -1, 1), vec3(-1, 1, 1),
                                            vec3(1, -1, 1),   vec3(1, 1, 1)};

    auto& vertVector = mesh->getTypedDataContainer<buffertraits::PositionsBuffer>();
    auto& colorVector = mesh->getTypedDataContainer<buffertraits::ColorsBuffer>();

    const auto off = static_cast<unsigned int>(vertVector.size());
    vertVector.insert(vertVector.end(), vertices.begin(), vertices.end());
    colorVector.insert(colorVector.end(), 8, color);

    mesh->setModelMatrix(glm::inverse(camera.getProjectionMatrix() * camera.getViewMatrix()));

    const auto ib = std::make_shared<IndexBufferRAM>();
    const auto indices = std::make_shared<IndexBuffer>(ib);
    ib->add({off + 0, off + 1, off + 1, off + 3, off + 3, off + 2, off + 2, off + 0});  // front
    ib->add({off + 4, off + 5, off + 5, off + 7, off + 7, off + 6, off + 6, off + 4});  // back
    ib->add({off + 0, off + 4, off + 1, off + 5, off + 2, off + 6, off + 3, off + 7});  // sides

    mesh->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None), indices);

    return mesh;
}
//! [Using Colored Mesh]

std::shared_ptr<Mesh> parallelepiped(glm::vec3 origin, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                                     glm::vec4 color, glm::vec4 c1, glm::vec4 c2, glm::vec4 c3,
                                     IncludeNormals includeNormals) {

    // create a mesh with positions (vec3), normals (vec3), textures(vec3),
    auto mesh = std::make_shared<Mesh>();

    // Set identity matrix
    mesh->setModelMatrix(mat4(1.f));

    const auto corners = [](const auto& origin, const auto& v1, const auto& v2, const auto& v3) {
        return std::array{
            origin,      origin + v1,      origin + v2,      origin + v1 + v2,
            origin + v3, origin + v1 + v3, origin + v2 + v3, origin + v1 + v2 + v3,
        };
    };

    const auto positions = corners(origin, p1, p2, p3);
    const auto colors = corners(color, c1, c2, c3);

    constexpr std::array<std::array<size_t, 4>, 6> faces = {
        {{0, 2, 3, 1}, {4, 5, 7, 6}, {0, 1, 5, 4}, {1, 3, 7, 5}, {3, 2, 6, 7}, {0, 4, 6, 2}}};

    const auto calcNnormal = [&](const std::array<size_t, 4>& inds) {
        const auto a = positions[inds[1]] - positions[inds[0]];
        const auto b = positions[inds[3]] - positions[inds[0]];
        return glm::normalize(glm::cross(a, b));
    };

    const auto indexBuffer = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    auto& indices = indexBuffer->getDataContainer();

    std::vector<vec3> positionVertices;
    std::vector<vec4> colorVertices;
    std::vector<vec3> normalVertices;

    positionVertices.reserve(faces.size() * faces[0].size());
    colorVertices.reserve(faces.size() * faces[0].size());
    normalVertices.reserve(faces.size() * faces[0].size());

    for (const auto& face : faces) {
        const auto normal = calcNnormal(face);
        const auto ind = static_cast<uint32_t>(positionVertices.size());
        for (const auto i : face) {
            positionVertices.emplace_back(positions[i]);
            colorVertices.emplace_back(colors[i]);
            normalVertices.emplace_back(normal);
        }

        indices.push_back(ind + 0);
        indices.push_back(ind + 1);
        indices.push_back(ind + 2);

        indices.push_back(ind + 0);
        indices.push_back(ind + 2);
        indices.push_back(ind + 3);
    }

    mesh->addBuffer(BufferType::PositionAttrib,
                    std::make_shared<Buffer<vec3>>(
                        std::make_shared<BufferRAMPrecision<vec3>>(std::move(positionVertices))));

    mesh->addBuffer(BufferType::ColorAttrib,
                    std::make_shared<Buffer<vec4>>(
                        std::make_shared<BufferRAMPrecision<vec4>>(std::move(colorVertices))));

    if (includeNormals == IncludeNormals::Yes) {
        mesh->addBuffer(BufferType::NormalAttrib,
                        std::make_shared<Buffer<vec3>>(
                            std::make_shared<BufferRAMPrecision<vec3>>(std::move(normalVertices))));
    }

    return mesh;
}

}  // namespace inviwo::meshutil
