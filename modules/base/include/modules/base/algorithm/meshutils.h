/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#pragma once

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/buffer/buffer.h>          // for IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferram.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/camera/camera.h>          // for mat4, Camera (ptr only)
#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for ConnectivityType, Connecti...
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh::MeshInfo, Mesh
#include <inviwo/core/datastructures/geometry/typedmesh.h>     // for BasicMesh, ColoredMesh
#include <inviwo/core/util/exception.h>                        // for Exception
#include <inviwo/core/util/glmvec.h>                           // for vec4, vec3, ivec2, vec2

#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t
#include <functional>   // for invoke
#include <memory>       // for shared_ptr, make_shared
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <glm/fwd.hpp>  // for vec3, vec4

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


/**
 * Create a triangle mesh for a cube with 3x3 quads on each face.
 * Each face gets a color (red, green, and blue) brighter on the front face and darker on the
 * back, the edges gets a bit brighter and the corners brighter still.
 * Picking will are assigned to each 3x3x3 "sub cube" where the center one is unused.
 * The use case is for a orientation indicator and rotation tool, see CameraWidget.
 */
IVW_MODULE_BASE_API std::shared_ptr<Mesh> cubeIndicator(const mat4& basisAndOffset);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> coordindicator(const vec3& center,
                                                              const float& size);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> boundingbox(const mat4& basisAndOffset,
                                                           const vec4& color);

IVW_MODULE_BASE_API std::shared_ptr<PosTexColorMesh> boundingBoxAdjacency(
    const mat4& basisAndOffset, const vec4& color);

IVW_MODULE_BASE_API std::shared_ptr<BasicMesh> torus(const vec3& center,
                                                     const vec3& up = vec3(0, 1, 0), float r1 = 1.f,
                                                     float r2 = .3f,
                                                     const ivec2& subdivisions = ivec2(32, 8),
                                                     vec4 color = vec4(1, 1, 1, 1));

IVW_MODULE_BASE_API std::shared_ptr<ColoredMesh> cameraFrustum(
    const Camera& camera, vec4 color,
    std::shared_ptr<ColoredMesh> mesh = std::make_shared<ColoredMesh>());

enum class IncludeNormals { Yes, No };

/**
 * @brief Create parallelepiped mesh
 *
 * The parallelepiped is anchored at origin and spanned by p1, p2, and p2
 *
 * @verbatim
 *
 *        6───────────────────▶7
 *       ▲ ▲                  ▲ ▲
 *      ╱   ╲                ╱   ╲
 *     ╱     ╲              ╱     ╲
 *    ╱       ╲            ╱       ╲
 *   ╱         ╲          ╱         ╲
 *  4───────────────────▶5           ╲
 *   ▲           ╲        ▲           ╲
 *    ╲           ╲        ╲           ╲
 *     ╲           2────────╲──────────▶3
 *      ╲         ▲          ╲         ▲
 *    p3 ╲       ╱            ╲       ╱
 *        ╲     ╱p2            ╲     ╱
 *         ╲   ╱                ╲   ╱
 *          ╲ ╱                  ╲ ╱
 *           0───────────────────▶1
 *          ▲         p1
 *         ╱
 *        ╱
 *       ╱ Origin
 *      ╱
 *     ╱
 *    ╱
 * @endverbatim
 *
 * @param origin position
 * @param p1 basis vector 1
 * @param p2 basis vector 2
 * @param p3 basis vector 3
 * @param color at origin
 * @param c1 color at p1
 * @param c2 color at p2
 * @param c3 color at p3
 * @param includeNormals Add face normals to the mesh
 * @return parallelepiped mesh
 */
IVW_MODULE_BASE_API std::shared_ptr<Mesh> parallelepiped(glm::vec3 origin, glm::vec3 p1,
                                                         glm::vec3 p2, glm::vec3 p3,
                                                         glm::vec4 color, glm::vec4 c1,
                                                         glm::vec4 c2, glm::vec4 c3,
                                                         IncludeNormals includeNormals);

template <typename Callback>
void forEachTriangle(const Mesh::MeshInfo& info, const IndexBuffer& ib, Callback callback) {
    auto& ram = ib.getRAMRepresentation()->getDataContainer();
    if (info.dt != DrawType::Triangles) {
        throw Exception(SourceContext{}, "Only works for triangles, got {}", info.dt);
    }

    if (ram.size() < 3) {
        return;
    }

    if (info.ct == ConnectivityType::None) {
        for (size_t i = 0; i < ram.size(); i += 3) {
            std::invoke(callback, ram[i], ram[i + 1], ram[i + 2]);
        }
    }

    else if (info.ct == ConnectivityType::Strip) {
        for (size_t i = 0; i < ram.size() - 2; ++i) {
            if (1 % 2 == 0) {
                std::invoke(callback, ram[i], ram[i + 1], ram[i + 2]);
            } else {
                std::invoke(callback, ram[i + 1], ram[i], ram[i + 2]);
            }
        }
    }

    else if (info.ct == ConnectivityType::Fan) {
        uint32_t a = static_cast<uint32_t>(ram.front());
        for (size_t i = 1; i < ram.size(); ++i) {
            std::invoke(callback, a, ram[i], ram[i + 1]);
        }
    }

    else if (info.ct == ConnectivityType::Adjacency) {
        if (ram.size() < 6) return;
        for (size_t i = 0; i < ram.size(); i += 6) {
            std::invoke(callback, ram[i], ram[i + 2], ram[i + 4]);
        }

    }

    else if (info.ct == ConnectivityType::StripAdjacency) {
        if (ram.size() < 6) return;
        for (size_t i = 0; i < ram.size(); i += 2) {
            if (1 % 2 == 0) {
                std::invoke(callback, ram[i], ram[i + 2], ram[i + 4]);
            } else {
                std::invoke(callback, ram[i + 2], ram[i], ram[i + 4]);
            }
        }
    }

    else {
        throw Exception(SourceContext{}, "ConnectivityType {} not supported", info.ct);
    }
}

template <typename Callback>
void forEachLineSegment(const Mesh::MeshInfo& info, const IndexBuffer& ib, Callback&& callback) {
    auto& ram = ib.getRAMRepresentation()->getDataContainer();
    if (info.dt != DrawType::Lines) {
        throw Exception(SourceContext{}, "Only works for lines, got {}", info.dt);
    }

    if (ram.size() < 2) {
        return;
    }

    if (info.ct == ConnectivityType::None) {
        for (size_t i = 0; i < ram.size(); i += 2) {
            std::invoke(callback, ram[i], ram[i + 1]);
        }
    }

    else if (info.ct == ConnectivityType::Strip) {
        for (size_t i = 0; i < ram.size() - 1; ++i) {
            std::invoke(callback, ram[i], ram[i + 1]);
        }
    }

    else if (info.ct == ConnectivityType::Loop) {
        for (size_t i = 0; i < ram.size() - 1; ++i) {
            std::invoke(callback, ram[i], ram[i + 1]);
        }
        std::invoke(callback, ram.back(), ram.front());
    }

    if (info.ct == ConnectivityType::Adjacency) {
        if (ram.size() < 4) return;
        for (size_t i = 0; i < ram.size(); i += 4) {
            std::invoke(callback, ram[i + 1], ram[i + 2]);
        }
    }

    if (info.ct == ConnectivityType::StripAdjacency) {
        if (ram.size() < 4) return;
        for (size_t i = 1; i < ram.size() - 2; ++i) {
            std::invoke(callback, ram[i], ram[i + 1]);
        }
    }

    else {
        throw Exception(SourceContext{}, "ConnectivityType {} not supported", info.ct);
    }
}

template <typename LineStartCallback, typename LinePointCallback, typename LineEndCallback>
void forEachLine(const Mesh::MeshInfo& info, const IndexBuffer& ib,
                 LineStartCallback&& lineStartCallback, LinePointCallback&& linePointCallback,
                 LineEndCallback&& lineEndCallback) {
    auto& ram = ib.getRAMRepresentation()->getDataContainer();
    if (info.dt != DrawType::Lines) {
        throw Exception(SourceContext{}, "Only works for lines, got {}", info.dt);
    }

    if (ram.size() < 2) {
        return;
    }

    if (info.ct == ConnectivityType::None) {
        for (size_t i = 0; i < ram.size(); i += 2) {
            std::invoke(lineStartCallback);
            std::invoke(linePointCallback, ram[i]);
            std::invoke(linePointCallback, ram[i + 1]);
            std::invoke(lineEndCallback);
        }
    }

    else if (info.ct == ConnectivityType::Strip) {
        std::invoke(lineStartCallback);
        for (size_t i = 0; i < ram.size(); ++i) {
            std::invoke(linePointCallback, ram[i]);
        }
        std::invoke(lineEndCallback);
    }

    else if (info.ct == ConnectivityType::Loop) {
        std::invoke(lineStartCallback);
        for (size_t i = 0; i < ram.size(); ++i) {
            std::invoke(linePointCallback, ram[i]);
        }
        std::invoke(linePointCallback, ram.front());
        std::invoke(lineEndCallback);
    }

    if (info.ct == ConnectivityType::Adjacency) {
        if (ram.size() < 4) return;

        for (size_t i = 0; i < ram.size(); i += 4) {
            std::invoke(lineStartCallback);
            std::invoke(linePointCallback, ram[i + 1]);
            std::invoke(linePointCallback, ram[i + 2]);
            std::invoke(lineEndCallback);
        }
    }

    if (info.ct == ConnectivityType::StripAdjacency) {
        if (ram.size() < 4) return;
        std::invoke(lineStartCallback);
        for (size_t i = 1; i < ram.size() - 2; ++i) {
            std::invoke(linePointCallback, ram[i]);
            std::invoke(linePointCallback, ram[i + 1]);
        }
        std::invoke(lineEndCallback);
    }

    else {
        throw Exception(SourceContext{}, "ConnectivityType {} not supported", info.ct);
    }
}

}  // namespace meshutil

}  // namespace inviwo
