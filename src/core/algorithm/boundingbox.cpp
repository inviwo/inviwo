/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layer.h>

#include <numeric>
#include <utility>
#include <ranges>

namespace inviwo::util {

namespace {
dvec3 orthvec(const dvec3& vec) {
    const dvec3 u(1.0, 0.0, 0.0);
    const dvec3 n = glm::normalize(vec);
    const double p = glm::dot(u, n);
    if (std::abs(p) != 1.0) {
        return glm::normalize(u - p * n);
    } else {
        return {0.0, 1.0, 0.0};
    }
}

}  // namespace

dmat4 minExtentBoundingBox(dmat4 boundingBox) {
    const double maxAspectRatio = 1000.0;
    dvec3 extent{
        glm::length(dvec3{boundingBox[0]}),
        glm::length(dvec3{boundingBox[1]}),
        glm::length(dvec3{boundingBox[2]}),
    };
    const double maxExtent = glm::compMax(extent);
    const double minExtent = maxExtent / maxAspectRatio;

    if (util::almostEqual(maxExtent, 0.0)) {
        // bounding box with zero extent
        dmat4 m{1.0};
        if (glm::length2(dvec3{boundingBox[3]}) > 0.0) {
            m[3] = boundingBox[3];
        } else {
            m[3] = dvec4{-0.5, -0.5, -0.5, 1.0};
        }
        return m;
    }

    if (util::almostEqual(extent[0], 0.0)) {
        if (util::almostEqual(extent[1], 0.0)) {
            boundingBox[1] = dvec4{orthvec(dvec3{boundingBox[2]}) * minExtent, 0.0};
            extent[1] = minExtent;
        } else if (util::almostEqual(extent[2], 0.0)) {
            boundingBox[2] = dvec4{orthvec(dvec3{boundingBox[1]}) * minExtent, 0.0};
            extent[2] = minExtent;
        }
        const dvec3 v =
            glm::cross(dvec3{boundingBox[1]} / extent[1], dvec3{boundingBox[2]} / extent[2]) *
            minExtent;
        boundingBox[0] = dvec4{v, 0.0};
    } else if (util::almostEqual(extent[1], 0.0)) {
        if (util::almostEqual(extent[2], 0.0)) {
            boundingBox[2] = dvec4{orthvec(dvec3{boundingBox[0]}) * minExtent, 0.0};
            extent[2] = minExtent;
        }
        const dvec3 v =
            glm::cross(dvec3{boundingBox[2]} / extent[2], dvec3{boundingBox[0]} / extent[0]) *
            minExtent;
        boundingBox[1] = dvec4{v, 0.0};
    } else if (util::almostEqual(extent[2], 0.0)) {
        const dvec3 v =
            glm::cross(dvec3{boundingBox[0]} / extent[0], dvec3{boundingBox[1]} / extent[1]) *
            minExtent;
        boundingBox[2] = dvec4{v, 0.0};
    }
    return boundingBox;
}

std::optional<dmat4> boundingBoxUnion(const std::optional<dmat4>& a, const std::optional<dmat4>& b) {
    if (!a && !b) return std::nullopt;
    if (a && !b) return a;
    if (b && !a) return b;

    dvec3 worldMin(std::numeric_limits<double>::max());
    dvec3 worldMax(std::numeric_limits<double>::lowest());

    const std::array<dvec3, 8> corners = {dvec3{0, 0, 0}, dvec3{1, 0, 0}, dvec3{1, 1, 0},
                                          dvec3{0, 1, 0}, dvec3{0, 0, 1}, dvec3{1, 0, 1},
                                          dvec3{1, 1, 1}, dvec3{0, 1, 1}};
    for (const auto& bb : {a, b}) {
        for (const auto& corner : corners) {
            const auto point = dvec3(*bb * dvec4(corner, 1.0));
            worldMin = glm::min(worldMin, point);
            worldMax = glm::max(worldMax, point);
        }
    }
    auto m = glm::scale(worldMax - worldMin);
    m[3] = dvec4(worldMin, 1.0);
    return m;
}

dmat4 boundingBox(const Layer& layer) {
    auto m = layer.getCoordinateTransformer().getDataToModelMatrix();
    const auto z = glm::cross(dvec3{m[0]}, dvec3{m[1]});
    m[2] = dvec4{z * 0.0001, 0.0};

    return layer.getCoordinateTransformer().getModelToWorldMatrix() * m;
}

dmat4 boundingBox(const std::vector<std::shared_ptr<Layer>>& layers) {
    if (layers.empty()) return dmat4(0.0);

    dvec3 worldMin(std::numeric_limits<double>::max());
    dvec3 worldMax(std::numeric_limits<double>::lowest());

    const std::array<dvec3, 8> corners = {dvec3{0, 0, 0}, dvec3{1, 0, 0}, dvec3{1, 1, 0},
                                          dvec3{0, 1, 0}, dvec3{0, 0, 1}, dvec3{1, 0, 1},
                                          dvec3{1, 1, 1}, dvec3{0, 1, 1}};
    for (const auto& layer : layers) {
        auto bb = boundingBox(*layer);
        for (const auto& corner : corners) {
            const auto point = dvec3(bb * dvec4(corner, 1.0));
            worldMin = glm::min(worldMin, point);
            worldMax = glm::max(worldMax, point);
        }
    }
    auto m = glm::scale(worldMax - worldMin);
    m[3] = dvec4(worldMin, 1.0);
    return m;
}

dmat4 boundingBox(const Mesh& mesh) {
    const auto& buffers = mesh.getBuffers();
    auto it = std::ranges::find_if(
        buffers, [](const auto& buff) { return buff.first.type == BufferType::PositionAttrib; });
    if (it != buffers.end() && it->second->getSize() > 0) {
        const auto minmax =
            it->second->getRepresentation<BufferRAM>()->dispatch<std::pair<dvec4, dvec4>>(
                [](auto br) {
                    using ValueType = util::PrecisionValueType<decltype(br)>;
                    const auto& data = br->getDataContainer();

                    using Res = std::pair<ValueType, ValueType>;
                    Res minmax{DataFormat<ValueType>::max(), DataFormat<ValueType>::lowest()};
                    minmax =
                        std::accumulate(data.begin(), data.end(), minmax,
                                        [](const Res& mm, const ValueType& v) -> Res {
                                            return {glm::min(mm.first, v), glm::max(mm.second, v)};
                                        });

                    return std::pair<dvec4, dvec4>{util::glm_convert<dvec4>(minmax.first),
                                                   util::glm_convert<dvec4>(minmax.second)};
                });

        const dvec3 dataMin = dvec3(minmax.first);
        const dvec3 dataMax = dvec3(minmax.second);
        auto m = glm::scale(dataMax - dataMin);
        m[3] = dvec4(dataMin, 1.0);
        return mesh.getCoordinateTransformer().getDataToWorldMatrix() * m;

    } else {
        dmat4 m{0.0};
        m[3] = dvec4(dvec3(mesh.getOffset()), 1.0);
        return m;
    }
}

dmat4 boundingBox(const std::vector<std::shared_ptr<const Mesh>>& meshes) {
    if (meshes.empty()) return dmat4(0.0);

    dvec3 worldMin(std::numeric_limits<double>::max());
    dvec3 worldMax(std::numeric_limits<double>::lowest());

    const std::array<dvec3, 8> corners = {dvec3{0, 0, 0}, dvec3{1, 0, 0}, dvec3{1, 1, 0},
                                          dvec3{0, 1, 0}, dvec3{0, 0, 1}, dvec3{1, 0, 1},
                                          dvec3{1, 1, 1}, dvec3{0, 1, 1}};
    for (const auto& mesh : meshes) {
        auto bb = boundingBox(*mesh);
        for (const auto& corner : corners) {
            const auto point = dvec3(bb * dvec4(corner, 1.0));
            worldMin = glm::min(worldMin, point);
            worldMax = glm::max(worldMax, point);
        }
    }
    auto m = glm::scale(worldMax - worldMin);
    m[3] = dvec4(worldMin, 1.0);
    return m;
}

dmat4 boundingBox(const Volume& volume) {
    return volume.getCoordinateTransformer().getDataToWorldMatrix();
}

dmat4 boundingBox(const std::vector<std::shared_ptr<Volume>>& volumes) {
    if (volumes.empty()) return dmat4(0.0);

    dvec3 worldMin(std::numeric_limits<double>::max());
    dvec3 worldMax(std::numeric_limits<double>::lowest());

    const std::array<dvec3, 8> corners = {dvec3{0, 0, 0}, dvec3{1, 0, 0}, dvec3{1, 1, 0},
                                          dvec3{0, 1, 0}, dvec3{0, 0, 1}, dvec3{1, 0, 1},
                                          dvec3{1, 1, 1}, dvec3{0, 1, 1}};
    for (const auto& volume : volumes) {
        auto bb = boundingBox(*volume);
        for (const auto& corner : corners) {
            const auto point = dvec3(bb * dvec4(corner, 1.0));
            worldMin = glm::min(worldMin, point);
            worldMax = glm::max(worldMax, point);
        }
    }
    auto m = glm::scale(worldMax - worldMin);
    m[3] = dvec4(worldMin, 1.0);
    return m;
}

std::function<std::optional<dmat4>()> boundingBox(const DataInport<Layer>& layer) {
    return [port = &layer]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(const DataInport<Layer, 0>& layers) {
    return [port = &layers]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(const DataInport<Layer, 0, true>& layers) {
    return [port = &layers]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(const DataOutport<Layer>& layer) {
    return [port = &layer]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}

std::function<std::optional<dmat4>()> boundingBox(const DataInport<Mesh>& mesh) {
    return [port = &mesh]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(const DataInport<Mesh, 0>& meshes) {
    return [port = &meshes]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(port->getVectorData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(const DataInport<Mesh, 0, true>& meshes) {
    return [port = &meshes]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(port->getVectorData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(const DataOutport<Mesh>& mesh) {
    return [port = &mesh]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}

std::function<std::optional<dmat4>()> boundingBox(const DataInport<Volume>& volume) {
    return [port = &volume]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(
    const DataInport<std::vector<std::shared_ptr<Volume>>>& volumes) {
    return [port = &volumes]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}

std::function<std::optional<dmat4>()> boundingBox(const DataOutport<Volume>& volume) {
    return [port = &volume]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}
std::function<std::optional<dmat4>()> boundingBox(
    const DataOutport<std::vector<std::shared_ptr<Volume>>>& volumes) {

    return [port = &volumes]() -> std::optional<dmat4> {
        if (port->hasData()) {
            return boundingBox(*port->getData());
        } else {
            return std::nullopt;
        }
    };
}

}  // namespace inviwo::util
