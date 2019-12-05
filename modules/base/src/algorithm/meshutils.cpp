/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <memory>

namespace inviwo {

namespace meshutil {

namespace detail {

vec3 orthvec(const vec3& vec) {
    vec3 u(1.0f, 0.0f, 0.0f);
    vec3 n = glm::normalize(vec);
    float p = glm::dot(u, n);
    if (std::abs(p) != 1.0f) {
        return glm::normalize(u - p * n);
    } else {
        return vec3(0.0f, 1.0f, 0.0f);
    }
}

vec3 tospherical(const vec2& v) {
    return vec3(std::sin(v.x) * std::cos(v.y), std::sin(v.x) * std::sin(v.y), std::cos(v.x));
}

}  // namespace detail

std::shared_ptr<BasicMesh> ellipse(const vec3& center, const vec3& majorAxis, const vec3& minorAxis,
                                   const vec4& color, const float& radius,
                                   const size_t& nsegments) {
    auto mesh = std::make_shared<BasicMesh>();
    vec3 p, p1;
    float angle = static_cast<float>(M_PI * 2.0 / nsegments);
    float a = glm::length(majorAxis);
    float b = glm::length(minorAxis);
    float eradius, eradius1;
    float x, y;
    vec3 rot, rot1;

    auto segments = static_cast<std::uint32_t>(nsegments);
    for (std::uint32_t i = 0; i < segments; ++i) {
        x = glm::cos(i * angle) * a;
        y = glm::sin(i * angle) * b;
        eradius = glm::length(glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y);
        rot = glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y;

        x = glm::cos(((i + 1) % segments) * angle) * a;
        y = glm::sin(((i + 1) % segments) * angle) * b;

        eradius1 = glm::length(glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y);
        rot1 = glm::normalize(majorAxis) * x + glm::normalize(minorAxis) * y;

        p = center + eradius * glm::normalize(rot);
        p1 = center + eradius1 * glm::normalize(rot1);

        vec3 dir = glm::normalize(p1 - p);
        vec3 odir = detail::orthvec(dir);

        float k;
        vec3 o;
        auto tube = std::make_unique<BasicMesh>();
        auto inds = tube->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
        for (std::uint32_t j = 0; j < segments; ++j) {
            k = static_cast<float>(j);
            o = glm::rotate(odir, static_cast<float>(j * angle), dir);
            tube->addVertex(p + radius * o, o, vec3(k / segments, 0.0f, 0.0f), color);
            tube->addVertex(p1 + radius * o, o, vec3(k / segments, 1.0f, 0.0f), color);

            inds->add(j * 2 + 1);
            inds->add(j * 2 + 0);
            inds->add(((j + 1) * 2) % (2 * segments) + 0);

            inds->add(j * 2 + 1);
            inds->add(((j + 1) * 2) % (2 * segments) + 0);
            inds->add(((j + 1) * 2) % (2 * segments) + 1);
        }

        mesh->append(tube.get());
    }
    return mesh;
}

std::shared_ptr<BasicMesh> disk(const vec3& center, const vec3& normal, const vec4& color,
                                const float& radius, const size_t& segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    vec3 orth = detail::orthvec(normal);

    mesh->addVertex(center, normal, vec3(0.5f, 0.5f, 0.0f), color);

    vec3 tc = vec3(0.5f, 0.5f, 0.0f);
    vec3 tn = vec3(0.0f, 0.0f, 1.0f);
    vec3 to = vec3(0.5f, 0.0f, 0.0f);
    vec3 p;
    vec3 t;
    double angle = 2.0 * M_PI / segments;
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

    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    vec3 e1 = glm::normalize(stop - start);
    vec3 e2 = detail::orthvec(e1);
    vec3 e3 = glm::cross(e1, e2);
    mat3 basis(e1, e2, e3);

    float height = glm::length(stop - start);
    float ratio = radius / height;

    double angle = 2.0 * M_PI / segments;
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

    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(6 * segments);
    vec3 e1 = glm::normalize(stop - start);
    vec3 e2 = detail::orthvec(e1);
    vec3 e3 = glm::cross(e1, e2);
    mat3 basis(e1, e2, e3);

    double angle = 2.0 * M_PI / segments;
    const auto ns = static_cast<std::uint32_t>(segments);
    for (std::uint32_t i = 0; i < ns; ++i) {
        // first vertex at base
        double x = cos(i * angle);
        double y = sin(i * angle);
        vec3 normal = basis * vec3(0.0, x, y);
        vec3 dir = basis * vec3(0.0, x * radius, y * radius);
        vec3 texCoord = vec3(static_cast<float>(i) / segments, 0.0, 0.0);

        // vertex at base
        vertices.push_back({start + dir, normal, texCoord, color});
        // vertex at top
        vertices.push_back({stop + dir, normal, texCoord + vec3(0.0, 1.0, 0.0), color});

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
        auto startcap = disk(start, -e1, color, radius, segments);
        auto endcap = disk(stop, e1, color, radius, segments);

        mesh->append(startcap.get());
        mesh->append(endcap.get());
    }

    return mesh;
}

std::shared_ptr<BasicMesh> line(const vec3& start, const vec3& stop, const vec3& normal,
                                const vec4& color /*= vec4(1.0f, 0.0f, 0.0f, 1.0f)*/,
                                const float& width /*= 1.0f*/, const ivec2& inres /*= ivec2(1)*/) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 direction = stop - start;
    vec3 up = glm::cross(glm::normalize(direction), normal);

    vec3 startCornerPoint = start - 0.5f * width * up;
    ivec2 res = inres + ivec2(1);

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

std::shared_ptr<BasicMesh> arrow(const vec3& start, const vec3& stop, const vec4& color,
                                 const float& radius, const float& arrowfraction,
                                 const float& arrowRadius, const size_t& segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    vec3 mid = start + (1 - arrowfraction) * (stop - start);
    auto cylinderpart = cylinder(start, mid, color, radius, segments);
    mesh->append(cylinderpart.get());

    auto diskpart = disk(mid, start - mid, color, arrowRadius, segments);
    mesh->append(diskpart.get());

    auto conepart = cone(mid, stop, color, arrowRadius, segments);
    mesh->append(conepart.get());

    return mesh;
}

std::shared_ptr<BasicMesh> colorsphere(const vec3& center, const float& radius,
                                       std::shared_ptr<BasicMesh> mesh) {
    const static std::vector<vec2> spheremesh = {
        {M_PI_2, 0.0f},
        {M_PI_2, M_PI_2},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {std::atan(6.0f), 0.0f},
        {std::atan(5.0f / 2.0f), 0.0f},
        {std::atan(4.0f / 3.0f), 0.0f},
        {std::atan(3.0f / 4.0f), 0.0f},
        {std::atan(2.0f / 5.0f), 0.0f},
        {std::atan(1.0f / 6.0f), 0.0f},
        {M_PI_2, std::atan(1.0f / 6.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(2.0f) / 5.0f), M_PI_4},
        {std::atan(1.0f / 6.0f), M_PI_2},
        {M_PI_2, std::atan(2.0f / 5.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)},
        {std::atan((2.0f * std::sqrt(2.0f)) / 3.0f), M_PI_4},
        {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)},
        {std::atan(2.0f / 5.0f), M_PI_2},
        {M_PI_2, std::atan(3.0f / 4.0f)},
        {std::atan(3.0f * std::sqrt(2.0f)), M_PI_4},
        {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)},
        {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)},
        {std::atan(3.0f / 4.0f), M_PI_2},
        {M_PI_2, std::atan(4.0f / 3.0f)},
        {std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)},
        {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)},
        {std::atan(4.0f / 3.0f), M_PI_2},
        {M_PI_2, std::atan(5.0f / 2.0f)},
        {std::atan(std::sqrt(26.0f)), std::atan(5.0f)},
        {std::atan(5.0f / 2.0f), M_PI_2},
        {M_PI_2, std::atan(6.0f)},
        {std::atan(6.0f), M_PI_2}};

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
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(sphereind.size() * 8 * 3);

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                glm::uint32_t idxOffset =
                    static_cast<std::uint32_t>(vertices.size()) + globalIndexOffset;

                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                vec4 color((quad + 1.0f) / 2.0f, 1.0f);
                for (auto& elem : spheremesh) {
                    normal = quad * detail::tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * (elem).x, quad.y * (elem).y, 0.0f);
                    vertices.push_back({vertex, normal, tcoord, color});
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (auto& elem : sphereind) {
                        inds->add({idxOffset + elem.x, idxOffset + elem.y, idxOffset + elem.z});
                    }
                } else {
                    for (auto& elem : sphereind) {
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
    std::vector<vec2> spheremesh = {{M_PI_2, 0.0f},
                                    {M_PI_2, M_PI_2},
                                    {0.0f, 0.0f},
                                    {0.0f, 0.0f},
                                    {std::atan(6.0f), 0.0f},
                                    {std::atan(5.0f / 2.0f), 0.0f},
                                    {std::atan(4.0f / 3.0f), 0.0f},
                                    {std::atan(3.0f / 4.0f), 0.0f},
                                    {std::atan(2.0f / 5.0f), 0.0f},
                                    {std::atan(1.0f / 6.0f), 0.0f},
                                    {M_PI_2, std::atan(1.0f / 6.0f)},
                                    {std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)},
                                    {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)},
                                    {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)},
                                    {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)},
                                    {std::atan(std::sqrt(2.0f) / 5.0f), M_PI_4},
                                    {std::atan(1.0f / 6.0f), M_PI_2},
                                    {M_PI_2, std::atan(2.0f / 5.0f)},
                                    {std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)},
                                    {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)},
                                    {std::atan((2.0f * std::sqrt(2.0f)) / 3.0f), M_PI_4},
                                    {std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)},
                                    {std::atan(2.0f / 5.0f), M_PI_2},
                                    {M_PI_2, std::atan(3.0f / 4.0f)},
                                    {std::atan(3.0f * std::sqrt(2.0f)), M_PI_4},
                                    {std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)},
                                    {std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)},
                                    {std::atan(3.0f / 4.0f), M_PI_2},
                                    {M_PI_2, std::atan(4.0f / 3.0f)},
                                    {std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)},
                                    {std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)},
                                    {std::atan(4.0f / 3.0f), M_PI_2},
                                    {M_PI_2, std::atan(5.0f / 2.0f)},
                                    {std::atan(std::sqrt(26.0f)), std::atan(5.0f)},
                                    {std::atan(5.0f / 2.0f), M_PI_2},
                                    {M_PI_2, std::atan(6.0f)},
                                    {std::atan(6.0f), M_PI_2}};

    std::vector<uvec3> sphereind = {
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
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    inds->getDataContainer().reserve(sphereind.size() * 8 * 3);

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                glm::uint32_t idxOffset =
                    static_cast<std::uint32_t>(vertices.size()) + globalIndexOffset;
                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                for (auto& elem : spheremesh) {
                    normal = quad * detail::tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * (elem).x, quad.y * (elem).y, 0.0f);
                    vertices.push_back({vertex, normal, tcoord, color});
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (auto& elem : sphereind) {
                        inds->add({idxOffset + elem.x, idxOffset + elem.y, idxOffset + elem.z});
                    }
                } else {
                    for (auto& elem : sphereind) {
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
    vec4 V = m * vec4(v, 1);
    return vec3(V) / V.w;
}

static vec3 N(const mat4& m, const vec3 v) {
    vec4 V = m * vec4(v, 0);
    return vec3(V);
}

std::shared_ptr<BasicMesh> cube(const mat4& m, const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1));

    auto indices = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

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

std::shared_ptr<BasicMesh> coordindicator(const vec3& center, const float& size) {
    size_t segments = 16;
    float bsize = size * 1.0f;
    float fsize = size * 1.2f;
    float radius = size * 0.08f;
    float arrowpart = 0.12f;
    float arrowRadius = 2.0f * radius;
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    auto xarrow = arrow(center - vec3(bsize, 0.0f, 0.0f), center + vec3(fsize, 0.0f, 0.0f),
                        vec4(1.0f, 0.0f, 0.0f, 1.0f), radius, arrowpart, arrowRadius, segments);
    auto yarrow = arrow(center - vec3(0.0f, bsize, 0.0f), center + vec3(0.0f, fsize, 0.0f),
                        vec4(0.0f, 1.0f, 0.0f, 1.0f), radius, arrowpart, arrowRadius, segments);
    auto zarrow = arrow(center - vec3(0.0f, 0.0f, bsize), center + vec3(0.0f, 0.0f, fsize),
                        vec4(0.0f, 0.0f, 1.0f, 1.0f), radius, arrowpart, arrowRadius, segments);

    auto sphere = colorsphere(center, 0.7f * size);

    mesh->append(sphere.get());
    mesh->append(xarrow.get());
    mesh->append(yarrow.get());
    mesh->append(zarrow.get());

    return mesh;
}

std::shared_ptr<BasicMesh> boundingbox(const mat4& basisandoffset, const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(basisandoffset);

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), color},
                       {vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 0.0), color},
                       {vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), color},
                       {vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), color},
                       {vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color},
                       {vec3(1.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color}});

    auto inds = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
    inds->add({0, 1, 0, 2, 0, 3, 1, 6, 1, 4, 2, 5, 2, 4, 3, 5, 3, 6, 5, 7, 6, 7, 4, 7});

    return mesh;
}

//! [Using PosTexColorMesh]
std::shared_ptr<PosTexColorMesh> boundingBoxAdjacency(const mat4& basisandoffset,
                                                      const vec4& color) {
    auto mesh = std::make_shared<PosTexColorMesh>();
    mesh->setModelMatrix(basisandoffset);

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), color},
                       {vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), color},
                       {vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), color},
                       {vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 0.0), color},
                       {vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, 1.0), color},
                       {vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color},
                       {vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color}});

    auto inds1 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds1->add({3, 0, 1, 2, 3, 0, 1});

    auto inds2 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds2->add({7, 4, 5, 6, 7, 4, 5});

    auto inds3 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds3->add({3, 0, 4, 7, 3, 0, 4});

    auto inds4 = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    inds4->add({2, 1, 5, 6, 2, 1, 2});

    return mesh;
}
//! [Using PosTexColorMesh]

std::shared_ptr<BasicMesh> torus(const vec3& center, const vec3& up_, float r1, float r2,
                                 const ivec2& subdivisions, vec4 color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 side;
    auto up = glm::normalize(up_);
    if (std::abs(std::abs(glm::dot(up, vec3(0, 1, 0))) - 1) <
        std::numeric_limits<float>::epsilon()) {
        side = vec3(1, 0, 0);
    } else {
        side = glm::normalize(glm::cross(up, vec3(0, 1, 0)));
    }

    auto numVertex = subdivisions.x * subdivisions.y;

    for (int i = 0; i < subdivisions.x; i++) {
        auto axis = glm::rotate(side, static_cast<float>(i * 2 * M_PI / subdivisions.x), up);
        auto centerP = center + axis * r1;

        auto N = glm::normalize(glm::cross(axis, up));

        int startI = i * subdivisions.y;
        int startJ = ((i + 1) % subdivisions.x) * subdivisions.y;

        for (int j = 0; j < subdivisions.y; j++) {
            auto axis2 = glm::rotate(axis, static_cast<float>(j * 2 * M_PI / subdivisions.y), N);
            auto VP = centerP + axis2 * r2;

            mesh->addVertex(VP, axis2, N, color);

            int i0 = startI + j;
            int i1 = startI + j + 1;
            int j0 = startJ + j;
            int j1 = startJ + j + 1;

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
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 right = detail::orthvec(normal);
    vec3 up = glm::cross(right, normal);

    vec3 start = center - 0.5f * extent.x * right - 0.5f * extent.y * up;
    ivec2 res = segments + ivec2(1);

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

//! [Using Colored Mesh]
std::shared_ptr<ColoredMesh> cameraFrustum(const Camera& camera, vec4 color,
                                           std::shared_ptr<ColoredMesh> mesh) {
    const static std::vector<vec3> vertices{vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(1, -1, -1),
                                            vec3(1, 1, -1),   vec3(-1, -1, 1), vec3(-1, 1, 1),
                                            vec3(1, -1, 1),   vec3(1, 1, 1)};

    auto& vertVector = mesh->getTypedDataContainer<buffertraits::PositionsBuffer>();
    auto& colorVector = mesh->getTypedDataContainer<buffertraits::ColorsBuffer>();

    auto off = static_cast<unsigned int>(vertVector.size());
    vertVector.insert(vertVector.end(), vertices.begin(), vertices.end());
    colorVector.insert(colorVector.end(), 8, color);

    mesh->setModelMatrix(glm::inverse(camera.getProjectionMatrix() * camera.getViewMatrix()));

    auto ib = std::make_shared<IndexBufferRAM>();
    auto indices = std::make_shared<IndexBuffer>(ib);
    ib->add({off + 0, off + 1, off + 1, off + 3, off + 3, off + 2, off + 2, off + 0});  // front
    ib->add({off + 4, off + 5, off + 5, off + 7, off + 7, off + 6, off + 6, off + 4});  // back
    ib->add({off + 0, off + 4, off + 1, off + 5, off + 2, off + 6, off + 3, off + 7});  // sides

    mesh->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None), indices);

    return mesh;
}
//! [Using Colored Mesh]

}  // namespace meshutil

}  // namespace inviwo
