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

#ifndef IVW_MESHUTILS_H
#define IVW_MESHUTILS_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>

#include <memory>

namespace inviwo {

namespace meshutil {
IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> ellipse(
    const vec3& center, const vec3& majorAxis, const vec3& minorAxis,
    const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f), const float& radius = 0.001f,
    const size_t& segments = 16);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> disk(
    const vec3& center, const vec3& normal, const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
    const float& radius = 1.0f, const size_t& segments = 16);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> cone(
    const vec3& start, const vec3& stop, const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
    const float& radius = 1.0f, const size_t& segments = 16);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> cylinder(
    const vec3& start, const vec3& stop, const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
    const float& radius = 1.0f, const size_t& segments = 16, bool caps = true,
    std::shared_ptr<BasicMesh> mesh = nullptr);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> line(
    const vec3& start, const vec3& stop, const vec3& normal,
    const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f), const float& width = 1.0f,
    const ivec2& res = ivec2(1));

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> arrow(
    const vec3& start, const vec3& stop, const vec4& color = vec4(1.0f, 0.0f, 0.0f, 1.0f),
    const float& radius = 1.0f, const float& arrowfraction = 0.15f, const float& arrowRadius = 2.0f,
    const size_t& segments = 16);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> colorsphere(
    const vec3& center, const float& radius, std::shared_ptr<BasicMesh> mesh = nullptr);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> sphere(const vec3& center, const float& radius,
                                                      const vec4& color,
                                                      std::shared_ptr<BasicMesh> mesh = nullptr);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> square(const vec3& center, const vec3& normal,
                                                      const vec2& extent,
                                                      const vec4& color = vec4(1, 1, 1, 1),
                                                      const ivec2& segments = ivec2(1));

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> cube(const mat4& orientation,
                                                    const vec4& color = vec4(1, 1, 1, 1));

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> coordindicator(const vec3& center,
                                                              const float& size);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> boundingbox(const mat4& basisandoffset,
                                                           const vec4& color);

IVW_MODULE_BASE_API std::shared_ptr<PosTexColorMesh> boundingBoxAdjacency(
    const mat4& basisandoffset, const vec4& color);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> torus(const vec3& center,
                                                     const vec3& up = vec3(0, 1, 0), float r1 = 1.f,
                                                     float r2 = .3f,
                                                     const ivec2& subdivisions = ivec2(32, 8),
                                                     vec4 color = vec4(1, 1, 1, 1));

IVW_MODULE_BASE_API std::shared_ptr<ColoredMesh> cameraFrustum(
    const Camera& camera, vec4 color,
    std::shared_ptr<ColoredMesh> mesh = std::make_shared<ColoredMesh>());

template <typename Callback>
void forEachTriangle(const Mesh::MeshInfo& info, const IndexBuffer& ib, Callback callback) {
    auto& ram = ib.getRAMRepresentation()->getDataContainer();
    if (info.dt != DrawType::Triangles) {
        std::ostringstream errMsg;
        errMsg << "Only works for triangles, got " << info.dt;
        throw inviwo::Exception(errMsg.str(), IVW_CONTEXT_CUSTOM("meshutil::forEachTriangle"));
    }

    if (ram.size() < 3) {
        return;
    }

    if (info.ct == ConnectivityType::None) {
        for (size_t i = 0; i < ram.size(); i += 3) {
            callback(ram[i], ram[i + 1], ram[i + 2]);
        }
    }

    else if (info.ct == ConnectivityType::Strip) {
        for (size_t i = 0; i < ram.size() - 2; i++) {
            callback(ram[i], ram[i + 1], ram[i + 2]);
        }
    }

    else if (info.ct == ConnectivityType::Fan) {
        uint32_t a = static_cast<uint32_t>(ram.front());
        for (size_t i = 1; i < ram.size(); i++) {
            callback(a, ram[i], ram[i + 1]);
        }
    }

    else {
        std::ostringstream errMsg;
        errMsg << "ConnectivityType " << info.ct << " not supported";
        throw inviwo::Exception(errMsg.str(), IVW_CONTEXT_CUSTOM("meshutil::forEachTriangle"));
    }
}

template <typename Callback>
void forEachLineSegment(const Mesh::MeshInfo& info, const IndexBuffer& ib, Callback callback) {
    auto& ram = ib.getRAMRepresentation()->getDataContainer();
    if (info.dt != DrawType::Lines) {
        std::ostringstream errMsg;
        errMsg << "Only works for lines, got " << info.dt;
        throw inviwo::Exception(errMsg.str(), IVW_CONTEXT_CUSTOM("meshutil::forEachLineSegment"));
    }

    if (ram.size() < 2) {
        return;
    }

    if (info.ct == ConnectivityType::None) {
        for (size_t i = 0; i < ram.size(); i += 2) {
            callback(ram[i], ram[i + 1]);
        }
    }

    else if (info.ct == ConnectivityType::Strip) {
        for (size_t i = 0; i < ram.size() - 1; i++) {
            callback(ram[i], ram[i + 1]);
        }
    }

    else if (info.ct == ConnectivityType::Loop) {
        for (size_t i = 0; i < ram.size() - 1; i++) {
            callback(ram[i], ram[i + 1]);
        }
        callback(ram.back(), ram.front());
    }

    else {
        std::ostringstream errMsg;
        errMsg << "ConnectivityType " << info.ct << " not supported";
        throw inviwo::Exception(errMsg.str(), IVW_CONTEXT_CUSTOM("meshutil::forEachLineSegment"));
    }
}

}  // namespace meshutil

}  // namespace inviwo

#endif  // IVW_MESHUTILS_H
