/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>

#include <numeric>
#include <utility>

namespace inviwo {

namespace util {

mat4 boundingBox(const Mesh &mesh) {
    vec3 worldMin(std::numeric_limits<float>::max());
    vec3 worldMax(std::numeric_limits<float>::lowest());

    const auto &buffers = mesh.getBuffers();
    auto it = std::find_if(buffers.begin(), buffers.end(), [](const auto &buff) {
        return buff.first.type == BufferType::PositionAttrib;
    });
    if (it != buffers.end() && it->second->getSize() > 0) {

        const auto minmax =
            it->second->getRepresentation<BufferRAM>()->dispatch<std::pair<dvec4, dvec4>>(
                [](auto br) {
                    using ValueType = util::PrecisionValueType<decltype(br)>;
                    const auto &data = br->getDataContainer();

                    using Res = std::pair<ValueType, ValueType>;
                    Res minmax{DataFormat<ValueType>::max(), DataFormat<ValueType>::lowest()};
                    minmax =
                        std::accumulate(data.begin(), data.end(), minmax,
                                        [](const Res &mm, const ValueType &v) -> Res {
                                            return {glm::min(mm.first, v), glm::max(mm.second, v)};
                                        });

                    return std::pair<dvec4, dvec4>{util::glm_convert<dvec4>(minmax.first),
                                                   util::glm_convert<dvec4>(minmax.second)};
                });

        mat4 trans = mesh.getCoordinateTransformer().getDataToWorldMatrix();
        worldMin = glm::min(worldMin, vec3(trans * vec4(vec3(minmax.first), 1.f)));
        worldMax = glm::max(worldMax, vec3(trans * vec4(vec3(minmax.second), 1.f)));
    } else {
        // No vertices, use same values for min/max
        worldMin = worldMax = mesh.getOffset();
    }

    auto m = glm::scale(worldMax - worldMin);
    m[3] = vec4(worldMin, 1.0f);
    return m;
}

mat4 boundingBox(const std::vector<std::shared_ptr<const Mesh>> &meshes) {
    if (meshes.empty()) return mat4(0.f);

    vec3 worldMin(std::numeric_limits<float>::max());
    vec3 worldMax(std::numeric_limits<float>::lowest());

    const std::array<vec3, 8> corners = {vec3{0, 0, 0}, vec3{1, 0, 0}, vec3{1, 1, 0},
                                         vec3{0, 1, 0}, vec3{0, 0, 1}, vec3{1, 0, 1},
                                         vec3{1, 1, 1}, vec3{0, 1, 1}};
    for (const auto &mesh : meshes) {
        auto bb = boundingBox(*mesh);
        for (const auto &corner : corners) {
            const auto point = vec3(bb * vec4(corner, 1.f));
            worldMin = glm::min(worldMin, point);
            worldMax = glm::max(worldMax, point);
        }
    }
    auto m = glm::scale(worldMax - worldMin);
    m[3] = vec4(worldMin, 1.0f);
    return m;
}

mat4 boundingBox(const Volume &volume) {
    return volume.getCoordinateTransformer().getDataToWorldMatrix();
}

mat4 boundingBox(const std::vector<std::shared_ptr<Volume>> &volumes) {
    if (volumes.empty()) return mat4(0.f);

    vec3 worldMin(std::numeric_limits<float>::max());
    vec3 worldMax(std::numeric_limits<float>::lowest());

    const std::array<vec3, 8> corners = {vec3{0, 0, 0}, vec3{1, 0, 0}, vec3{1, 1, 0},
                                         vec3{0, 1, 0}, vec3{0, 0, 1}, vec3{1, 0, 1},
                                         vec3{1, 1, 1}, vec3{0, 1, 1}};
    for (const auto &volume : volumes) {
        auto bb = boundingBox(*volume);
        for (const auto &corner : corners) {
            const auto point = vec3(bb * vec4(corner, 1.f));
            worldMin = glm::min(worldMin, point);
            worldMax = glm::max(worldMax, point);
        }
    }
    auto m = glm::scale(worldMax - worldMin);
    m[3] = vec4(worldMin, 1.0f);
    return m;
}

std::function<std::optional<mat4>()> boundingBox(const DataInport<Mesh> &mesh) {
    return [port = &mesh]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<mat4>()> boundingBox(const DataInport<Mesh, 0> &meshes) {
    return [port = &meshes]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(port->getVectorData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<mat4>()> boundingBox(const DataInport<Mesh, 0, true> &meshes) {
    return [port = &meshes]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(port->getVectorData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<mat4>()> boundingBox(const DataOutport<Mesh> &mesh) {
    return [port = &mesh]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}

std::function<std::optional<mat4>()> boundingBox(const DataInport<Volume> &volume) {
    return [port = &volume]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<mat4>()> boundingBox(
    const DataInport<std::vector<std::shared_ptr<Volume>>> &volumes) {
    return [port = &volumes]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}

std::function<std::optional<mat4>()> boundingBox(const DataOutport<Volume> &volume) {
    return [port = &volume]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<mat4>()> boundingBox(
    const DataOutport<std::vector<std::shared_ptr<Volume>>> &volumes) {

    return [port = &volumes]() -> std::optional<mat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}

}  // namespace util

}  // namespace inviwo
