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

#include <inviwo/core/datastructures/buffer/buffer.h>              // for Buffer, makeBuffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>  // for BufferRAMPrecision
#include <inviwo/core/datastructures/geometry/geometrytype.h>      // for BufferType, Conne...
#include <inviwo/core/datastructures/geometry/mesh.h>              // for Mesh, Mesh::MeshInfo
#include <inviwo/core/util/glm.h>                                  // for vec3, vec4, vec2
#include <inviwo/core/util/zip.h>                                  // for make_sequence
#include <modules/plotting/datastructures/axissettings.h>          // for AxisSettings
#include <modules/plotting/datastructures/majorticksettings.h>     // for MajorTickSettings
#include <modules/plotting/datastructures/minorticksettings.h>     // for MinorTickSettings
#include <modules/plotting/datastructures/plottextsettings.h>      // for PlotTextSettings
#include <modules/plotting/algorithm/labeling.h>

#include <algorithm>      // for transform, fill, max
#include <cmath>          // for floor, ceil, abs
#include <cstdint>        // for uint32_t
#include <numeric>        // for iota
#include <unordered_map>  // for unordered_map

#include "glm/gtx/scalar_multiplication.hpp"  // for operator*
#include <glm/common.hpp>                     // for min, max, mix
#include <glm/ext/scalar_constants.hpp>       // for epsilon
#include <glm/geometric.hpp>                  // for normalize, distance
#include <glm/vec2.hpp>                       // for operator*, vec
#include <glm/vec3.hpp>                       // for operator*, vec

#include <fmt/printf.h>

namespace inviwo::plot {

AxisLabels getAxisLabels(LabelingAlgorithm algorithm, dvec2 range, int maxTicks,
                         std::string_view labelFormat, const AxisLabels& customLabels) {
    using enum LabelingAlgorithm;

    // ensure a positive range for the label positioning algorithms, that is range max > range min
    const bool flippedRange = range.x > range.y;
    if (flippedRange) {
        std::swap(range.x, range.y);
    }

    AxisLabels labels{};
    switch (algorithm) {
        case Heckbert:
            labels = labelingHeckbert(range.x, range.y, maxTicks);
            break;
        case Matplotlib:
            labels = labelingMatplotlib(range.x, range.y, maxTicks);
            break;
        case ExtendedWilkinson:
            labels = labelingExtendedWilkinson(range.x, range.y, maxTicks);
            break;
        case Limits:
            labels = labelingLimits(range.x, range.y);
            break;
        case CustomOnly:
        default:
            break;
    }

    if (flippedRange) {
        std::ranges::reverse(labels.positions);
        std::swap(labels.start, labels.stop);
        labels.step *= -1.0;
    }

    // generate labels based on positions
    if (labels.labels.size() != labels.positions.size()) {
        labels.labels.clear();
        labels.labels.reserve(labels.positions.size());
        std::ranges::transform(labels.positions, std::back_inserter(labels.labels),
                               [labelFormat](double pos) {
                                   // TODO(martin.falk): switch from sprintf to fmt::format
                                   return fmt::sprintf(labelFormat, pos);
                               });
    }

    // add custom labels, replace existing ones if possible
    for (const auto&& [pos, label] :
         std::ranges::zip_view(customLabels.positions, customLabels.labels)) {
        if (auto it = std::ranges::find_if(
                labels.positions, [&pos](double val) { return util::almostEqual(pos, val); });
            it != labels.positions.end()) {
            // found position, replace existing label with custom one
            labels.labels[std::distance(labels.positions.begin(), it)] = label;
        } else {
            labels.positions.emplace_back(pos);
            labels.labels.emplace_back(label);
        }
    }

    return labels;
}

AxisLabels getAxisLabels(const AxisSettings& settings) {
    return getAxisLabels(settings.getLabelingAlgorithm(), settings.getRange(),
                         settings.getMajorTicks().getNumberOfTicks(),
                         settings.getLabelFormatString(), settings.getCustomLabels());
}

std::vector<double> getMinorTicks(const MinorTickSettings& tickSettings,
                                  const AxisLabels& axisTicks, dvec2 range) {
    if ((tickSettings.getStyle() == TickStyle::None) || (tickSettings.getTickFrequency() < 2)) {
        // a tick frequency of 1 would draw the minor ticks directly on top of the major ticks
        return {};
    }

    // calculate number of minor ticks by filling each segment between two major ticks with
    // (n - 1) ticks where n is the frequency. Minor ticks will only fill the ranges in between
    // major ticks.
    //
    // With a tick frequency of 5, each major segment is split into 5 parts:
    //
    //    :  |....+....+....+....+....|....+....+....+....+....|  :
    //
    // '|' representing a major tick, '+' a minor tick, and ':' the axis range
    //
    // If MinorTicks.fillAxis is true, minor ticks will cover the entire axis.
    //
    //    :..+....|....+....+....+....+....|....+....+...:
    //

    const auto minorTickDelta =
        axisTicks.step / static_cast<double>(tickSettings.getTickFrequency());

    double startMinor = axisTicks.start;
    double stopMinor = axisTicks.stop;

    size_t nextMajorTickIndex = 0;
    if (tickSettings.getFillAxis()) {
        // need to figure out first and last minor tick positions beyond major ticks

        // first minor tick
        auto n = std::floor((axisTicks.start - range.x) / minorTickDelta);
        startMinor = axisTicks.start - n * minorTickDelta;
        // save index for later
        nextMajorTickIndex = static_cast<size_t>(n + 0.5);
        // last minor tick
        n = std::floor((range.y - axisTicks.stop) / minorTickDelta);

        stopMinor = axisTicks.stop + n * minorTickDelta;
    } else {
        // minor ticks are only filled in between major ticks
        startMinor = axisTicks.start + minorTickDelta;
        stopMinor = axisTicks.stop - minorTickDelta;

        nextMajorTickIndex = static_cast<size_t>(tickSettings.getTickFrequency()) - 1u;
    }

    // Minor ticks fill the entire axis, except where a major tick is supposed to be.
    const size_t totalTicks =
        minorTickDelta > 0 ? static_cast<size_t>((stopMinor - startMinor + glm::epsilon<double>()) /
                                                 minorTickDelta) +
                                 1
                           : 0;

    std::vector<double> tickPositions;
    tickPositions.reserve(totalTicks);

    // compute tick positions
    for (size_t i = 0u; i < totalTicks; ++i) {
        if (i == nextMajorTickIndex) {
            // skip major tick positions
            nextMajorTickIndex += static_cast<size_t>(tickSettings.getTickFrequency());
            continue;
        }
        tickPositions.push_back(startMinor + minorTickDelta * static_cast<double>(i));
    }

    return tickPositions;
}

std::vector<double> getMinorTicks(const AxisSettings& settings) {
    return getMinorTicks(settings.getMinorTicks(), getAxisLabels(settings), settings.getRange());
}

std::unique_ptr<Mesh> generateTicksMesh(const std::vector<double>& tickmarks, dvec2 axisRange,
                                        const vec3& startPos, const vec3& endPos,
                                        const vec3& tickDirection, float tickLength,
                                        TickStyle style, const vec4& color, bool flip) {
    if (tickmarks.empty()) {
        return nullptr;
    }

    // compute tick positions
    const auto axisDir = glm::normalize(endPos - startPos);
    const auto tickDir = glm::normalize(tickDirection) * tickLength;

    const auto screenLength(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    const vec3 scaling(axisDir * static_cast<float>(screenLength / axisLength));

    vec2 tickOffset;
    switch (style) {
        case TickStyle::Inside:
            tickOffset = vec2(0.0f, 1.0f);
            break;
        case TickStyle::Outside:
            tickOffset = vec2(-1.0f, 0.0f);
            break;
        case TickStyle::Both:
        default:
            tickOffset = vec2(-1.0f, 1.0f);
            break;
    }
    if (flip) {
        tickOffset = -tickOffset;
    }

    const size_t numTicks = tickmarks.size();
    auto posBuffer = std::make_shared<Buffer<vec3>>(numTicks * 2, BufferUsage::Static);
    auto colBuffer = std::make_shared<Buffer<vec4>>(numTicks * 2, BufferUsage::Static);
    auto& vertices = posBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto& colors = colBuffer->getEditableRAMRepresentation()->getDataContainer();

    std::ranges::fill(colors, color);

    for (size_t i = 0u; i < numTicks; ++i) {
        const vec3 pos(startPos + scaling * static_cast<float>(tickmarks[i] - axisRange.x));
        vertices[2 * i] = pos + tickDir * tickOffset.x;
        vertices[2 * i + 1] = pos + tickDir * tickOffset.y;
    }

    auto mesh = std::make_unique<Mesh>(DrawType::Lines, ConnectivityType::None);
    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);
    mesh->addBuffer(BufferType::ColorAttrib, colBuffer);

    std::vector<uint32_t> indices(numTicks * 2);
    std::ranges::iota(indices, 0);

    mesh->addIndices(Mesh::MeshInfo{.dt = DrawType::Lines, .ct = ConnectivityType::None},
                     inviwo::util::makeIndexBuffer(std::move(indices)));

    return mesh;
}

std::pair<vec2, vec2> tickBoundingRect(const AxisSettings& settings, const AxisLabels& ticks,
                                       const vec2& startPos, const vec2& endPos) {
    vec2 lowerLeft = glm::min(startPos, endPos);
    vec2 upperRight = glm::max(startPos, endPos);

    const auto axisDir = glm::normalize(endPos - startPos);
    const auto tickDirection = vec3(-axisDir.y, axisDir.x, 0.0f);

    const auto axisRange = settings.getRange();

    const auto screenLength(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    const vec2 scaling(axisDir * static_cast<float>(screenLength / axisLength));

    const auto& majorTicks = settings.getMajorTicks();
    const auto& minorTicks = settings.getMinorTicks();

    auto getSize = [](vec2 pos, vec2 tickDir, TickStyle style, bool flip) -> std::pair<vec2, vec2> {
        vec2 tickOffset;
        switch (style) {
            case TickStyle::Inside:
                tickOffset = vec2(0.0f, 1.0f);
                break;
            case TickStyle::Outside:
                tickOffset = vec2(-1.0f, 0.0f);
                break;
            case TickStyle::Both:
            default:
                tickOffset = vec2(-1.0f, 1.0f);
                break;
        }
        if (flip) {
            tickOffset = -tickOffset;
        }
        return {pos + tickDir * tickOffset.x, pos + tickDir * tickOffset.y};
    };

    {
        const auto& tickPositions = ticks.positions;
        if (!tickPositions.empty()) {
            const auto pos1 =
                startPos + scaling * static_cast<float>(tickPositions.front() - axisRange.x);
            const vec2 pos2 =
                startPos + scaling * static_cast<float>(tickPositions.back() - axisRange.x);

            const auto [start1, end1] =
                getSize(pos1, tickDirection, majorTicks.getStyle(), settings.getMirrored());
            const auto [start2, end2] =
                getSize(pos2, tickDirection, majorTicks.getStyle(), settings.getMirrored());

            lowerLeft = glm::min(lowerLeft, start1);
            lowerLeft = glm::min(lowerLeft, end1);
            lowerLeft = glm::min(lowerLeft, start2);
            lowerLeft = glm::min(lowerLeft, end2);

            upperRight = glm::max(upperRight, start1);
            upperRight = glm::max(upperRight, end1);
            upperRight = glm::max(upperRight, start2);
            upperRight = glm::max(upperRight, end2);
        }
    }

    {
        const auto tickPositions =
            getMinorTicks(settings.getMinorTicks(), ticks, settings.getRange());
        if (!tickPositions.empty()) {
            const auto pos1 =
                startPos + scaling * static_cast<float>(tickPositions.front() - axisRange.x);
            const vec2 pos2 =
                startPos + scaling * static_cast<float>(tickPositions.back() - axisRange.x);

            const auto [start1, end1] =
                getSize(pos1, tickDirection, minorTicks.getStyle(), settings.getMirrored());
            const auto [start2, end2] =
                getSize(pos2, tickDirection, minorTicks.getStyle(), settings.getMirrored());

            lowerLeft = glm::min(lowerLeft, start1);
            lowerLeft = glm::min(lowerLeft, end1);
            lowerLeft = glm::min(lowerLeft, start2);
            lowerLeft = glm::min(lowerLeft, end2);

            upperRight = glm::max(upperRight, start1);
            upperRight = glm::max(upperRight, end1);
            upperRight = glm::max(upperRight, start2);
            upperRight = glm::max(upperRight, end2);
        }
    }

    return {lowerLeft, upperRight};
}

std::unique_ptr<Mesh> generateAxisMesh(const vec2& startPos, const vec2& endPos, const vec4& color,
                                       const size_t& pickingId) {
    return generateAxisMesh3D(vec3(startPos, 0.0f), vec3(endPos, 0.0f), color, pickingId);
}

std::unique_ptr<Mesh> generateAxisMesh3D(const vec3& startPos, const vec3& endPos,
                                         const vec4& color, const size_t& pickingId) {

    auto verticesBuffer = util::makeBuffer<vec3>({startPos, endPos});
    auto colorBuffer = util::makeBuffer<vec4>({color, color});

    auto m = std::make_unique<Mesh>();
    m->addBuffer(BufferType::PositionAttrib, verticesBuffer);
    m->addBuffer(BufferType::ColorAttrib, colorBuffer);

    if (pickingId != std::numeric_limits<size_t>::max()) {
        const auto id = static_cast<uint32_t>(pickingId);
        auto pickingBuffer = util::makeBuffer<uint32_t>({id, id});
        m->addBuffer(BufferType::PickingAttrib, pickingBuffer);
    }
    m->addIndices(Mesh::MeshInfo{.dt = DrawType::Lines, .ct = ConnectivityType::None},
                  util::makeIndexBuffer({0, 1}));

    return m;
}

vec2 getAxisCaptionPosition(const AxisSettings& settings, const vec2& startPos,
                            const vec2& endPos) {

    const vec2 axisPos = glm::mix(startPos, endPos, settings.getCaptionSettings().getPosition());

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x) * settings.getScalingFactor();

    if (settings.getMirrored()) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * settings.getCaptionSettings().getOffset().x +
           axisDir * settings.getCaptionSettings().getOffset().y;
}

std::vector<std::pair<double, vec2>> getLabelPositions(const AxisLabels& ticks,
                                                       const AxisSettings& settings,
                                                       const vec2& startPos, const vec2& endPos) {
    if (ticks.positions.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x) * settings.getScalingFactor();

    if (settings.getMirrored()) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    // determine position of left-most label
    const vec2 labelOrigin(startPos + normal * settings.getLabelSettings().getOffset().x +
                           axisDir * settings.getLabelSettings().getOffset().y);

    // position a label below each tick
    const auto axisRange = settings.getRange();
    const auto screenLength(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    std::vector<std::pair<double, vec2>> labelPositions(ticks.positions.size());

    if (axisLength != 0.0) {
        const vec2 scaling(axisDir * static_cast<float>(screenLength / axisLength));
        std::ranges::transform(
            ticks.positions, labelPositions.begin(), [&](double pos) -> std::pair<double, vec2> {
                return {pos, labelOrigin + scaling * static_cast<float>(pos - axisRange.x)};
            });

    } else {
        const auto denom = ticks.positions.size() > 1 ? ticks.positions.size() - 1 : 1.f;
        const vec2 scaling(axisDir * static_cast<float>(screenLength / denom));
        auto seq = util::make_sequence(size_t{0}, ticks.positions.size(), size_t{1});
        std::ranges::transform(ticks.positions, seq, labelPositions.begin(),
                               [&](double pos, size_t i) -> std::pair<double, vec2> {
                                   return {pos, labelOrigin + scaling * static_cast<double>(i)};
                               });
    }
    return labelPositions;
}

vec3 getAxisCaptionPosition3D(const AxisSettings& settings, const vec3& startPos,
                              const vec3& endPos, const vec3& tickDirection) {
    const vec3 axisPos(glm::mix(startPos, endPos, settings.getCaptionSettings().getPosition()));
    auto normal = -glm::normalize(tickDirection) * settings.getScalingFactor();

    if (settings.getMirrored()) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * settings.getCaptionSettings().getOffset().x +
           glm::normalize(endPos - startPos) * settings.getCaptionSettings().getOffset().y;
}

std::vector<std::pair<double, vec3>> getLabelPositions3D(const AxisLabels& ticks,
                                                         const AxisSettings& settings,
                                                         const vec3& startPos, const vec3& endPos,
                                                         const vec3& tickDirection) {
    if (ticks.positions.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = -glm::normalize(tickDirection) * settings.getScalingFactor();

    if (settings.getMirrored()) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    // determine position of left-most label
    const vec3 labelOrigin(startPos + normal * settings.getLabelSettings().getOffset().x +
                           axisDir * settings.getLabelSettings().getOffset().y);

    // position a label below each tick
    const auto axisRange = settings.getRange();
    const auto worldLen(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    const vec3 scaling(axisDir * static_cast<float>(worldLen / axisLength));

    std::vector<std::pair<double, vec3>> labelPositions(ticks.positions.size());
    std::ranges::transform(
        ticks.positions, labelPositions.begin(), [&](double pos) -> std::pair<double, vec3> {
            return {pos, labelOrigin + scaling * static_cast<float>(pos - axisRange.x)};
        });

    return labelPositions;
}

}  // namespace inviwo::plot
