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

#include <modules/plotting/utils/axisutils.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/zip.h>
#include <modules/plotting/algorithm/labeling.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <unordered_map>
#include <ranges>

#include <glm/common.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <fmt/printf.h>

namespace inviwo::plot {

std::unique_ptr<Mesh> generateTicksMesh(const std::vector<double>& tickMarks, dvec2 axisRange,
                                        TickData::Style style) {
    if (tickMarks.empty()) {
        return nullptr;
    }

    // compute tick positions
    constexpr auto axisDir = vec3{1.0f, 0.0f, 0.0f};
    constexpr auto tickDir = vec3{0.0f, 1.0f, 0.0f};
    const auto axisLength = axisRange.y - axisRange.x;
    const vec3 scaling(axisDir / axisLength);

    vec2 tickOffset;
    switch (style) {
        using enum TickData::Style;
        case Inside:
            tickOffset = vec2(0.0f, 1.0f);
            break;
        case Outside:
            tickOffset = vec2(-1.0f, 0.0f);
            break;
        case Both:
        default:
            tickOffset = vec2(-1.0f, 1.0f);
            break;
    }

    const size_t numTicks = tickMarks.size();
    auto posBuffer = std::make_shared<Buffer<vec3>>(numTicks * 2, BufferUsage::Static);
    auto& vertices = posBuffer->getEditableRAMRepresentation()->getDataContainer();

    for (size_t i = 0u; i < numTicks; ++i) {
        const vec3 pos(scaling * static_cast<float>(tickMarks[i] - axisRange.x));
        vertices[2 * i] = pos + tickDir * tickOffset.x;
        vertices[2 * i + 1] = pos + tickDir * tickOffset.y;
    }

    auto mesh = std::make_unique<Mesh>(DrawType::Lines, ConnectivityType::None);
    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);

    std::vector<uint32_t> indices(numTicks * 2);
    std::iota(indices.begin(), indices.end(), 0);

    mesh->addIndices(Mesh::MeshInfo{.dt = DrawType::Lines, .ct = ConnectivityType::None},
                     inviwo::util::makeIndexBuffer(std::move(indices)));

    return mesh;
}

std::vector<std::pair<double, vec2>> getLabelPositions(const std::vector<double>& positions,
                                                       const AxisData& data, const vec2& startPos,
                                                       const vec2& endPos) {
    if (positions.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x);

    if (data.mirrored) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    // determine position of left-most label
    const vec2 labelOrigin(startPos + normal * data.labelSettings.offset.x +
                           axisDir * data.labelSettings.offset.y);

    // position a label below each tick
    const auto axisRange = data.range;
    const auto screenLength(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    std::vector<std::pair<double, vec2>> labelPositions(positions.size());

    if (axisLength != 0.0) {
        const vec2 scaling(axisDir * static_cast<float>(screenLength / axisLength));
        std::ranges::transform(
            positions, labelPositions.begin(), [&](double pos) -> std::pair<double, vec2> {
                return {pos, labelOrigin + scaling * static_cast<float>(pos - axisRange.x)};
            });

    } else {
        const auto denom = positions.size() > 1 ? static_cast<float>(positions.size()) - 1.0f : 1.f;
        const vec2 scaling{axisDir * screenLength / denom};
        auto seq = util::make_sequence(size_t{0}, positions.size(), size_t{1});
        std::ranges::transform(positions, seq, labelPositions.begin(),
                               [&](double pos, size_t i) -> std::pair<double, vec2> {
                                   return {pos, labelOrigin + scaling * static_cast<double>(i)};
                               });
    }
    return labelPositions;
}

std::vector<std::pair<double, vec3>> getLabelPositions3D(const std::vector<double>& positions,
                                                         const AxisData& data, const vec3& startPos,
                                                         const vec3& endPos,
                                                         const vec3& tickDirection) {
    if (positions.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = -glm::normalize(tickDirection);

    if (data.mirrored) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    // determine position of left-most label
    const vec3 labelOrigin =
        startPos + normal * data.labelSettings.offset.x + axisDir * data.labelSettings.offset.y;

    // position a label below each tick
    const auto axisRange = data.range;
    const auto worldLen(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    const vec3 scaling(axisDir * static_cast<float>(worldLen / axisLength));

    std::vector<std::pair<double, vec3>> labelPositions(positions.size());
    std::ranges::transform(
        positions, labelPositions.begin(), [&](double pos) -> std::pair<double, vec3> {
            return {pos, labelOrigin + scaling * static_cast<float>(pos - axisRange.x)};
        });

    return labelPositions;
}

vec2 getAxisCaptionPosition(const AxisData& data, const vec2& startPos, const vec2& endPos) {
    const vec2 axisPos = glm::mix(startPos, endPos, data.captionSettings.position);
    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x);

    if (data.mirrored) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * data.captionSettings.offset.x +
           axisDir * data.captionSettings.offset.y;
}

vec3 getAxisCaptionPosition3D(const AxisData& data, const vec3& startPos, const vec3& endPos,
                              const vec3& tickDirection) {
    const auto axisPos = glm::mix(startPos, endPos, data.captionSettings.position);
    auto normal = -glm::normalize(tickDirection);

    if (data.mirrored) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * data.captionSettings.offset.x +
           glm::normalize(endPos - startPos) * data.captionSettings.offset.y;
}

std::pair<vec2, vec2> tickBoundingRect(const AxisData& data,
                                       const std::vector<double>& majorPositions,
                                       const std::vector<double>& minorPositions,
                                       const vec2& startPos, const vec2& endPos) {

    vec2 lowerLeft = glm::min(startPos, endPos);
    vec2 upperRight = glm::max(startPos, endPos);

    const auto axisDir = glm::normalize(endPos - startPos);
    const auto tickDirection = vec3(-axisDir.y, axisDir.x, 0.0f);

    const auto axisRange = data.range;

    const auto screenLength(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    const vec2 scaling(axisDir * static_cast<float>(screenLength / axisLength));

    auto getSize = [](vec2 pos, vec2 tickDir, TickData::Style style,
                      bool flip) -> std::pair<vec2, vec2> {
        vec2 tickOffset;
        switch (style) {
            using enum TickData::Style;
            case Inside:
                tickOffset = vec2(0.0f, 1.0f);
                break;
            case Outside:
                tickOffset = vec2(-1.0f, 0.0f);
                break;
            case Both:
            default:
                tickOffset = vec2(-1.0f, 1.0f);
                break;
        }
        if (flip) {
            tickOffset = -tickOffset;
        }
        return {pos + tickDir * tickOffset.x, pos + tickDir * tickOffset.y};
    };

    if (!majorPositions.empty() && data.major.style != TickData::Style::None) {
        const auto pos1 =
            startPos + scaling * static_cast<float>(majorPositions.front() - axisRange.x);
        const vec2 pos2 =
            startPos + scaling * static_cast<float>(majorPositions.back() - axisRange.x);

        const auto [start1, end1] = getSize(pos1, tickDirection, data.major.style, data.mirrored);
        const auto [start2, end2] = getSize(pos2, tickDirection, data.major.style, data.mirrored);

        lowerLeft = glm::min(lowerLeft, start1);
        lowerLeft = glm::min(lowerLeft, end1);
        lowerLeft = glm::min(lowerLeft, start2);
        lowerLeft = glm::min(lowerLeft, end2);

        upperRight = glm::max(upperRight, start1);
        upperRight = glm::max(upperRight, end1);
        upperRight = glm::max(upperRight, start2);
        upperRight = glm::max(upperRight, end2);
    }
    if (!minorPositions.empty() && data.minor.style != TickData::Style::None) {
        const auto pos1 =
            startPos + scaling * static_cast<float>(minorPositions.front() - axisRange.x);
        const vec2 pos2 =
            startPos + scaling * static_cast<float>(minorPositions.back() - axisRange.x);

        const auto [start1, end1] = getSize(pos1, tickDirection, data.minor.style, data.mirrored);
        const auto [start2, end2] = getSize(pos2, tickDirection, data.minor.style, data.mirrored);

        lowerLeft = glm::min(lowerLeft, start1);
        lowerLeft = glm::min(lowerLeft, end1);
        lowerLeft = glm::min(lowerLeft, start2);
        lowerLeft = glm::min(lowerLeft, end2);

        upperRight = glm::max(upperRight, start1);
        upperRight = glm::max(upperRight, end1);
        upperRight = glm::max(upperRight, start2);
        upperRight = glm::max(upperRight, end2);
    }

    return {lowerLeft, upperRight};
}

}  // namespace inviwo::plot
