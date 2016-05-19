/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

namespace inviwo {
BasicMesh::BasicMesh() : Mesh() {
    vertices_ = std::make_shared<Buffer<vec3>>();
    texCoords_ = std::make_shared<Buffer<vec3>>();
    colors_ = std::make_shared<Buffer<vec4>>();
    normals_ = std::make_shared<Buffer<vec3>>();

    addBuffer(BufferType::PositionAttrib, vertices_);   // pos 0
    addBuffer(BufferType::TexcoordAttrib, texCoords_);  // pos 1
    addBuffer(BufferType::ColorAttrib, colors_);        // pos 2
    addBuffer(BufferType::NormalAttrib, normals_);      // pos 3
}

BasicMesh* BasicMesh::clone() const { return new BasicMesh(*this); }

uint32_t BasicMesh::addVertex(vec3 pos, vec3 normal, vec3 texCoord, vec4 color) {
    getEditableVerticesRAM()->add(pos);
    getEditableTexCoordsRAM()->add(texCoord);
    getEditableColorsRAM()->add(color);
    getEditableNormalsRAM()->add(normal);
    return static_cast<uint32_t>(getVertices()->getSize() - 1);
}

void BasicMesh::addVertices(const std::vector<Vertex> &data) {
    auto v = getEditableVerticesRAM();
    auto t = getEditableTexCoordsRAM();
    auto c = getEditableColorsRAM();
    auto n = getEditableNormalsRAM();

    for (const auto& elem : data) {
        v->add(elem.pos);
        n->add(elem.normal);
        t->add(elem.tex);
        c->add(elem.color);
    }
}

void BasicMesh::setVertex(size_t index, vec3 pos, vec3 normal, vec3 texCoord, vec4 color) {
    getEditableVerticesRAM()->set(index, pos);
    getEditableTexCoordsRAM()->set(index, texCoord);
    getEditableColorsRAM()->set(index, color);
    getEditableNormalsRAM()->set(index, normal);
}

void BasicMesh::setVertexPosition(size_t index, vec3 pos) {
    getEditableVerticesRAM()->set(index, pos);
}

void BasicMesh::setVertexNormal(size_t index, vec3 normal) {
    getEditableNormalsRAM()->set(index, normal);
}

void BasicMesh::setVertexTexCoord(size_t index, vec3 texCoord) {
    getEditableTexCoordsRAM()->set(index, texCoord);
}

void BasicMesh::setVertexColor(size_t index, vec4 color) {
    getEditableColorsRAM()->set(index, color);
}

IndexBufferRAM* BasicMesh::addIndexBuffer(DrawType dt, ConnectivityType ct) {
    auto indicesRam = std::make_shared<IndexBufferRAM>();
    auto indices_ = std::make_shared<IndexBuffer>(indicesRam);
    addIndicies(Mesh::MeshInfo(dt, ct), indices_);
    return indicesRam.get();
}

void BasicMesh::append(const BasicMesh* mesh) {
    size_t size = buffers_[0].second->getSize();

    getEditableVerticesRAM()->append(mesh->getVerticesRAM()->getDataContainer());
    getEditableTexCoordsRAM()->append(mesh->getTexCoordsRAM()->getDataContainer());
    getEditableColorsRAM()->append(mesh->getColorsRAM()->getDataContainer());
    getEditableNormalsRAM()->append(mesh->getNormalsRAM()->getDataContainer());

    for (auto buffer : mesh->indices_) {
        IndexBufferRAM* ind = addIndexBuffer(buffer.first.dt, buffer.first.ct);

        const std::vector<unsigned int>* newinds =
            static_cast<const IndexBufferRAM*>(buffer.second->getRepresentation<BufferRAM>())
                ->getDataContainer();

        for (const auto& newind : *newinds) {
            ind->add(static_cast<const unsigned int>(newind + size));
        }
    }
}

const Buffer<vec3>* BasicMesh::getVertices() const { return vertices_.get(); }

const Buffer<vec3>* BasicMesh::getTexCoords() const { return texCoords_.get(); }

const Buffer<vec4>* BasicMesh::getColors() const { return colors_.get(); }

const Buffer<vec3>* BasicMesh::getNormals() const { return normals_.get(); }

const Vec3BufferRAM* BasicMesh::getVerticesRAM() const { return vertices_->getRAMRepresentation(); }
const Vec3BufferRAM* BasicMesh::getTexCoordsRAM() const {
    return texCoords_->getRAMRepresentation();
}
const Vec4BufferRAM* BasicMesh::getColorsRAM() const { return colors_->getRAMRepresentation(); }
const Vec3BufferRAM* BasicMesh::getNormalsRAM() const { return normals_->getRAMRepresentation(); }

Vec3BufferRAM* BasicMesh::getEditableVerticesRAM() {
    return vertices_->getEditableRAMRepresentation();
}

Vec3BufferRAM* BasicMesh::getEditableTexCoordsRAM() {
    return texCoords_->getEditableRAMRepresentation();
}

Vec4BufferRAM* BasicMesh::getEditableColorsRAM() { return colors_->getEditableRAMRepresentation(); }
Vec3BufferRAM* BasicMesh::getEditableNormalsRAM() {
    return normals_->getEditableRAMRepresentation();
}

vec3 BasicMesh::orthvec(const vec3& vec) {
    vec3 u(1.0f, 0.0f, 0.0f);
    vec3 n = glm::normalize(vec);
    float p = glm::dot(u, n);
    if (std::abs(p) != 1.0f) {
        return glm::normalize(u - p * n);
    } else {
        return vec3(0.0f, 1.0f, 0.0f);
    }
}

vec3 BasicMesh::tospherical(const vec2& v) {
    return vec3(std::sin(v.x) * std::cos(v.y), std::sin(v.x) * std::sin(v.y), std::cos(v.x));
}

std::shared_ptr<BasicMesh> BasicMesh::disk(const vec3& center, const vec3& normal,
                                           const vec4& color, const float& radius,
                                           const size_t& segments) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    IndexBufferRAM* inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    vec3 orth = orthvec(normal);

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

std::shared_ptr<BasicMesh> BasicMesh::cone(const vec3& start, const vec3& stop, const vec4& color,
                                           const float& radius, const size_t& segments) {
    const vec3 tc(0.5f, 0.5f, 0.0f);
    const vec3 tn(0.0f, 0.0f, 1.0f);
    const vec3 to(0.5f, 0.0f, 0.0f);

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    vec3 e1 = glm::normalize(stop - start);
    vec3 e2 = orthvec(e1);
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

std::shared_ptr<BasicMesh> BasicMesh::cylinder(const vec3& start, const vec3& stop,
                                               const vec4& color, const float& radius,
                                               const size_t& segments, bool caps) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    vec3 e1 = glm::normalize(stop - start);
    vec3 e2 = orthvec(e1);
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
        mesh->addVertex(start + dir, normal, texCoord, color);
        // vertex at top
        mesh->addVertex(stop + dir, normal, texCoord + vec3(0.0, 1.0, 0.0), color);

        // indices for two triangles filling the strip
        inds->add(i * 2 + 1);
        inds->add(i * 2 + 0);
        inds->add(((i + 1) * 2) % (2 * ns) + 0);

        inds->add(i * 2 + 1);
        inds->add(((i + 1) * 2) % (2 * ns) + 0);
        inds->add(((i + 1) * 2) % (2 * ns) + 1);
    }
    // add end caps
    if (caps) {
        auto startcap = disk(start, -e1, color, radius, segments);
        auto endcap = disk(stop, e1, color, radius, segments);

        mesh->append(startcap.get());
        mesh->append(endcap.get());
    }

    return mesh;
}

std::shared_ptr<BasicMesh> BasicMesh::line(const vec3& start, const vec3& stop, const vec3& normal,
                                           const vec4& color /*= vec4(1.0f, 0.0f, 0.0f, 1.0f)*/,
                                           const float& width /*= 1.0f*/,
                                           const ivec2& inres /*= ivec2(1)*/) {
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
                            normal, vec3(static_cast<float>(i) / static_cast<float>(inres.x),
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

std::shared_ptr<BasicMesh> BasicMesh::arrow(const vec3& start, const vec3& stop, const vec4& color,
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

std::shared_ptr<BasicMesh> BasicMesh::colorsphere(const vec3& center, const float& radius) {
    std::vector<vec2> spheremesh;
    spheremesh.push_back(vec2(M_PI_2, 0.0f));
    spheremesh.push_back(vec2(M_PI_2, M_PI_2));
    spheremesh.push_back(vec2(0.0f, 0.0f));
    spheremesh.push_back(vec2(0.0f, 0.0f));
    spheremesh.push_back(vec2(std::atan(6.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(5.0f / 2.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(4.0f / 3.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(3.0f / 4.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(2.0f / 5.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(1.0f / 6.0f), 0.0f));
    spheremesh.push_back(vec2(M_PI_2, std::atan(1.0f / 6.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(2.0f) / 5.0f), M_PI_4));
    spheremesh.push_back(vec2(std::atan(1.0f / 6.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(2.0f / 5.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan((2.0f * std::sqrt(2.0f)) / 3.0f), M_PI_4));
    spheremesh.push_back(vec2(std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f / 5.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(3.0f / 4.0f)));
    spheremesh.push_back(vec2(std::atan(3.0f * std::sqrt(2.0f)), M_PI_4));
    spheremesh.push_back(vec2(std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)));
    spheremesh.push_back(vec2(std::atan(3.0f / 4.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(4.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)));
    spheremesh.push_back(vec2(std::atan(4.0f / 3.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(5.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(26.0f)), std::atan(5.0f)));
    spheremesh.push_back(vec2(std::atan(5.0f / 2.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(6.0f)));
    spheremesh.push_back(vec2(std::atan(6.0f), M_PI_2));

    std::vector<uvec3> sphereind;
    sphereind.push_back(uvec3(0, 10, 4));
    sphereind.push_back(uvec3(4, 11, 5));
    sphereind.push_back(uvec3(5, 12, 6));
    sphereind.push_back(uvec3(6, 13, 7));
    sphereind.push_back(uvec3(7, 14, 8));
    sphereind.push_back(uvec3(8, 15, 9));
    sphereind.push_back(uvec3(9, 16, 2));
    sphereind.push_back(uvec3(10, 17, 11));
    sphereind.push_back(uvec3(11, 18, 12));
    sphereind.push_back(uvec3(12, 19, 13));
    sphereind.push_back(uvec3(13, 20, 14));
    sphereind.push_back(uvec3(14, 21, 15));
    sphereind.push_back(uvec3(15, 22, 16));
    sphereind.push_back(uvec3(17, 23, 18));
    sphereind.push_back(uvec3(18, 24, 19));
    sphereind.push_back(uvec3(19, 25, 20));
    sphereind.push_back(uvec3(20, 26, 21));
    sphereind.push_back(uvec3(21, 27, 22));
    sphereind.push_back(uvec3(23, 28, 24));
    sphereind.push_back(uvec3(24, 29, 25));
    sphereind.push_back(uvec3(25, 30, 26));
    sphereind.push_back(uvec3(26, 31, 27));
    sphereind.push_back(uvec3(28, 32, 29));
    sphereind.push_back(uvec3(29, 33, 30));
    sphereind.push_back(uvec3(30, 34, 31));
    sphereind.push_back(uvec3(32, 35, 33));
    sphereind.push_back(uvec3(33, 36, 34));
    sphereind.push_back(uvec3(35, 1, 36));
    sphereind.push_back(uvec3(11, 4, 10));
    sphereind.push_back(uvec3(12, 5, 11));
    sphereind.push_back(uvec3(13, 6, 12));
    sphereind.push_back(uvec3(14, 7, 13));
    sphereind.push_back(uvec3(15, 8, 14));
    sphereind.push_back(uvec3(16, 9, 15));
    sphereind.push_back(uvec3(18, 11, 17));
    sphereind.push_back(uvec3(19, 12, 18));
    sphereind.push_back(uvec3(20, 13, 19));
    sphereind.push_back(uvec3(21, 14, 20));
    sphereind.push_back(uvec3(22, 15, 21));
    sphereind.push_back(uvec3(24, 18, 23));
    sphereind.push_back(uvec3(25, 19, 24));
    sphereind.push_back(uvec3(26, 20, 25));
    sphereind.push_back(uvec3(27, 21, 26));
    sphereind.push_back(uvec3(29, 24, 28));
    sphereind.push_back(uvec3(30, 25, 29));
    sphereind.push_back(uvec3(31, 26, 30));
    sphereind.push_back(uvec3(33, 29, 32));
    sphereind.push_back(uvec3(34, 30, 33));
    sphereind.push_back(uvec3(36, 33, 35));

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                BasicMesh temp;
                auto inds = temp.addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                vec4 color((quad + 1.0f) / 2.0f, 1.0f);
                for (auto& elem : spheremesh) {
                    normal = quad * tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * (elem).x, quad.y * (elem).y, 0.0f);
                    temp.addVertex(vertex, normal, tcoord, color);
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (auto& elem : sphereind) {
                        inds->add({ elem.x, elem.y, elem.z });
                    }
                }
                else {
                    for (auto& elem : sphereind) {
                        inds->add({ elem.z, elem.y, elem.x });
                    }
                }
                mesh->append(&temp);
            }
        }
    }
    return mesh;
}

std::shared_ptr<BasicMesh> BasicMesh::sphere(const vec3& center, const float& radius , const vec4 &color) {
    std::vector<vec2> spheremesh;
    spheremesh.push_back(vec2(M_PI_2, 0.0f));
    spheremesh.push_back(vec2(M_PI_2, M_PI_2));
    spheremesh.push_back(vec2(0.0f, 0.0f));
    spheremesh.push_back(vec2(0.0f, 0.0f));
    spheremesh.push_back(vec2(std::atan(6.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(5.0f / 2.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(4.0f / 3.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(3.0f / 4.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(2.0f / 5.0f), 0.0f));
    spheremesh.push_back(vec2(std::atan(1.0f / 6.0f), 0.0f));
    spheremesh.push_back(vec2(M_PI_2, std::atan(1.0f / 6.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(26.0f)), std::atan(1.0f / 5.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(17.0f) / 2.0f), std::atan(1.0f / 4.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(10.0f) / 3.0f), std::atan(1.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(5.0f) / 4.0f), std::atan(1.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(2.0f) / 5.0f), M_PI_4));
    spheremesh.push_back(vec2(std::atan(1.0f / 6.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(2.0f / 5.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f * std::sqrt(5.0f)), std::atan(1.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(13.0f) / 2.0f), std::atan(2.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan((2.0f * std::sqrt(2.0f)) / 3.0f), M_PI_4));
    spheremesh.push_back(vec2(std::atan(std::sqrt(5.0f) / 4.0f), std::atan(2.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f / 5.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(3.0f / 4.0f)));
    spheremesh.push_back(vec2(std::atan(3.0f * std::sqrt(2.0f)), M_PI_4));
    spheremesh.push_back(vec2(std::atan(std::sqrt(13.0f) / 2.0f), std::atan(3.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(10.0f) / 3.0f), std::atan(3.0f)));
    spheremesh.push_back(vec2(std::atan(3.0f / 4.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(4.0f / 3.0f)));
    spheremesh.push_back(vec2(std::atan(2.0f * std::sqrt(5.0f)), std::atan(2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(17.0f) / 2.0f), std::atan(4.0f)));
    spheremesh.push_back(vec2(std::atan(4.0f / 3.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(5.0f / 2.0f)));
    spheremesh.push_back(vec2(std::atan(std::sqrt(26.0f)), std::atan(5.0f)));
    spheremesh.push_back(vec2(std::atan(5.0f / 2.0f), M_PI_2));
    spheremesh.push_back(vec2(M_PI_2, std::atan(6.0f)));
    spheremesh.push_back(vec2(std::atan(6.0f), M_PI_2));

    std::vector<uvec3> sphereind;
    sphereind.push_back(uvec3(0, 10, 4));
    sphereind.push_back(uvec3(4, 11, 5));
    sphereind.push_back(uvec3(5, 12, 6));
    sphereind.push_back(uvec3(6, 13, 7));
    sphereind.push_back(uvec3(7, 14, 8));
    sphereind.push_back(uvec3(8, 15, 9));
    sphereind.push_back(uvec3(9, 16, 2));
    sphereind.push_back(uvec3(10, 17, 11));
    sphereind.push_back(uvec3(11, 18, 12));
    sphereind.push_back(uvec3(12, 19, 13));
    sphereind.push_back(uvec3(13, 20, 14));
    sphereind.push_back(uvec3(14, 21, 15));
    sphereind.push_back(uvec3(15, 22, 16));
    sphereind.push_back(uvec3(17, 23, 18));
    sphereind.push_back(uvec3(18, 24, 19));
    sphereind.push_back(uvec3(19, 25, 20));
    sphereind.push_back(uvec3(20, 26, 21));
    sphereind.push_back(uvec3(21, 27, 22));
    sphereind.push_back(uvec3(23, 28, 24));
    sphereind.push_back(uvec3(24, 29, 25));
    sphereind.push_back(uvec3(25, 30, 26));
    sphereind.push_back(uvec3(26, 31, 27));
    sphereind.push_back(uvec3(28, 32, 29));
    sphereind.push_back(uvec3(29, 33, 30));
    sphereind.push_back(uvec3(30, 34, 31));
    sphereind.push_back(uvec3(32, 35, 33));
    sphereind.push_back(uvec3(33, 36, 34));
    sphereind.push_back(uvec3(35, 1, 36));
    sphereind.push_back(uvec3(11, 4, 10));
    sphereind.push_back(uvec3(12, 5, 11));
    sphereind.push_back(uvec3(13, 6, 12));
    sphereind.push_back(uvec3(14, 7, 13));
    sphereind.push_back(uvec3(15, 8, 14));
    sphereind.push_back(uvec3(16, 9, 15));
    sphereind.push_back(uvec3(18, 11, 17));
    sphereind.push_back(uvec3(19, 12, 18));
    sphereind.push_back(uvec3(20, 13, 19));
    sphereind.push_back(uvec3(21, 14, 20));
    sphereind.push_back(uvec3(22, 15, 21));
    sphereind.push_back(uvec3(24, 18, 23));
    sphereind.push_back(uvec3(25, 19, 24));
    sphereind.push_back(uvec3(26, 20, 25));
    sphereind.push_back(uvec3(27, 21, 26));
    sphereind.push_back(uvec3(29, 24, 28));
    sphereind.push_back(uvec3(30, 25, 29));
    sphereind.push_back(uvec3(31, 26, 30));
    sphereind.push_back(uvec3(33, 29, 32));
    sphereind.push_back(uvec3(34, 30, 33));
    sphereind.push_back(uvec3(36, 33, 35));

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));

    vec3 quad(0);
    for (quad.x = -1.0f; quad.x <= 1.0f; quad.x += 2.0f) {
        for (quad.y = -1.0f; quad.y <= 1.0f; quad.y += 2.0f) {
            for (quad.z = -1.0f; quad.z <= 1.0f; quad.z += 2.0f) {
                BasicMesh temp;
                auto inds = temp.addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

                vec3 normal;
                vec3 vertex;
                vec3 tcoord;
                for (auto& elem : spheremesh) {
                    normal = quad * tospherical(elem);
                    vertex = center + radius * normal;
                    tcoord = vec3(quad.x * (elem).x, quad.y * (elem).y, 0.0f);
                    temp.addVertex(vertex, normal, tcoord, color);
                }

                if (quad.x * quad.y * quad.z > 0) {
                    for (auto& elem : sphereind) {
                        inds->add({ elem.x, elem.y, elem.z });
                    }
                }
                else {
                    for (auto& elem : sphereind) {
                        inds->add({ elem.z, elem.y, elem.x });
                    }
                }
                mesh->append(&temp);
            }
        }
    }
    return mesh;
}

static vec3 V(const mat4& m, const vec3 v) {
    vec4 V = m * vec4(v, 1);
    return V.xyz() / V.w;
}

static vec3 N(const mat4& m, const vec3 v) {
    vec4 V = m * vec4(v, 0);
    return V.xyz();
}

std::shared_ptr<BasicMesh> BasicMesh::cube(const mat4& m, const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1));

    auto indices = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    // Front back
    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(0, 0, -1)), vec3(0, 0, 0), color},
                       {V(m, vec3(1, 0, 0)), N(m, vec3(0, 0, -1)), vec3(1, 0, 0), color},
                       {V(m, vec3(1, 1, 0)), N(m, vec3(0, 0, -1)), vec3(1, 1, 0), color},
                       {V(m, vec3(0, 1, 0)), N(m, vec3(0, 0, -1)), vec3(0, 1, 0), color}});

    std::uint32_t o = 0;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

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
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

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
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    mesh->addVertices({{V(m, vec3(0, 0, 0)), N(m, vec3(0, -1, 0)), vec3(0, -1, 0), color},
                       {V(m, vec3(1, 0, 0)), N(m, vec3(0, -1, 0)), vec3(1, -1, 0), color},
                       {V(m, vec3(1, 0, 1)), N(m, vec3(0, -1, 0)), vec3(1, -1, 1), color},
                       {V(m, vec3(0, 0, 1)), N(m, vec3(0, -1, 0)), vec3(0, -1, 1), color}});
    o += 4;
    indices->add({0 + o, 2 + o, 1 + o, 0 + o, 3 + o, 2 + o});

    return mesh;
}

std::shared_ptr<BasicMesh> BasicMesh::coordindicator(const vec3& center, const float& size) {
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

std::shared_ptr<BasicMesh> BasicMesh::boundingbox(const mat4& basisandoffset, const vec4& color) {
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

std::shared_ptr<BasicMesh> BasicMesh::boundingBoxAdjacency(const mat4& basisandoffset,
                                                           const vec4& color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(basisandoffset);

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), color},
                       {vec3(1.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 0.0), color},
                       {vec3(1.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), color},
                       {vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), color},
                       {vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 1.0), color},
                       {vec3(1.0, 0.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), color},
                       {vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), color},
                       {vec3(0.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), color}});

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

std::shared_ptr<BasicMesh> BasicMesh::torus(const vec3& center,const vec3 &up_, float r1, float r2, const ivec2 &subdivisions, vec4 color) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 side;
    auto up = glm::normalize(up_);
    if (std::abs(std::abs(glm::dot(up, vec3(0, 1, 0)))-1) < std::numeric_limits<float>::epsilon()) {
        side = vec3(1, 0, 0);
    }
    else {
        side = glm::normalize(glm::cross(up, vec3(0, 1, 0)));
    }

    auto numVertex = subdivisions.x*subdivisions.y;

    for (int i = 0; i < subdivisions.x; i++) {
        auto axis = glm::rotate(side, static_cast<float>(i * 2 * M_PI / subdivisions.x), up);
        auto centerP = center + axis*r1;
        
        auto N = glm::normalize(glm::cross(axis, up));

        int startI = i * subdivisions.y;
        int startJ = ((i+1)% subdivisions.x) * subdivisions.y;

        for (int j = 0; j < subdivisions.y; j++) {
            auto axis2 = glm::rotate(axis, static_cast<float>(j * 2 * M_PI / subdivisions.y), N);
            auto VP = centerP + axis2*r2;
            
            mesh->addVertex(VP, axis2, VP, color);

            int i0 = startI + j;
            int i1 = startI + j + 1;
            int j0 = startJ + j;
            int j1 = startJ + j + 1;
            
            inds->add(i0%numVertex);
            inds->add(i1%numVertex);
            inds->add(j0%numVertex);

            inds->add(j0%numVertex);
            inds->add(j1%numVertex);
            inds->add(i1%numVertex);



        }
    }


    return mesh;

}

std::shared_ptr<BasicMesh> BasicMesh::square(const vec3& pos, const vec3& normal,
                                             const glm::vec2& extent,
                                             const vec4& color /*= vec4(1,1,1,1)*/,
                                             const ivec2& inres /*= ivec2(1)*/) {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(mat4(1.f));
    auto inds = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);

    vec3 right = orthvec(normal);
    vec3 up = glm::cross(right, normal);

    vec3 start = pos - 0.5f * extent.x * right - 0.5f * extent.y * up;
    ivec2 res = inres + ivec2(1);

    for (int j = 0; j < res.y; j++) {
        for (int i = 0; i < res.x; i++) {
            mesh->addVertex(
                start + static_cast<float>(i) / static_cast<float>(inres.x) * extent.x * right +
                    static_cast<float>(j) / static_cast<float>(inres.y) * extent.y * up,
                normal, vec3(static_cast<float>(i) / static_cast<float>(inres.x),
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

}  // namespace
