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

#include <modules/plotting/utils/axisutils.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/zip.h>

#include <algorithm>
#include <tuple>

namespace inviwo {

namespace plot {

namespace tickutil {

struct AxisTickRange {
    double start;
    double end;
    double delta;

    size_t numTicks;
};

AxisTickRange getMajorTickRange(const MajorTickSettings& ticks, dvec2 range, size_t maxTicks) {
    // calculate number of major ticks
    double startValue = range.x;
    double endValue = range.y;

    if (ticks.getTickDelta() <= 0.0) {
        return {startValue, endValue, endValue - startValue, 2u};
    }

    if (!ticks.getRangeBasedTicks()) {
        // the major ticks should appear at n * tickDelta and not based at axis range min
        startValue = std::ceil((startValue - glm::epsilon<double>()) / ticks.getTickDelta()) *
                     ticks.getTickDelta();
        endValue = std::floor((endValue + glm::epsilon<double>()) / ticks.getTickDelta()) *
                   ticks.getTickDelta();
        endValue = std::max(startValue, endValue);
    }

    const auto axisLength = endValue - startValue;
    // in case tickDelta is larger than the entire axis, adjust it to axis length
    // in order to only draw first and last tick corresponding to axis range
    double tickDelta = ticks.getTickDelta();

    size_t numTicks = 0u;
    if ((startValue - range.y > glm::epsilon<double>()) || (ticks.getTickDelta() <= 0.0)) {
        numTicks = 0u;
        endValue = startValue;
    } else if (std::abs(axisLength) < glm::epsilon<double>()) {
        // no difference between start and end tick, place only one tick
        numTicks = 1u;
    } else {
        numTicks = static_cast<size_t>((axisLength + glm::epsilon<double>()) / tickDelta) + 1;
        // adjust end value
        endValue = startValue + (numTicks - 1) * tickDelta;
        tickDelta = glm::min(ticks.getTickDelta(), axisLength);
    }

    while (numTicks > maxTicks) {
        numTicks /= 2;
        tickDelta *= 2.0f;
    }

    return {startValue, endValue, tickDelta, numTicks};
}

}  // namespace tickutil

std::vector<double> getMajorTickPositions(const MajorTickSettings& ticks, dvec2 range,
                                          size_t maxTicks) {
    if (ticks.getStyle() == TickStyle::None) {
        return {};
    }

    // calculate number of major ticks
    tickutil::AxisTickRange tickRange = tickutil::getMajorTickRange(ticks, range, maxTicks);

    if (tickRange.numTicks == 0u) return {};

    std::vector<double> tickPositions(tickRange.numTicks);
    // compute tick positions
    for (size_t i = 0u; i < tickRange.numTicks; ++i) {
        tickPositions[i] = tickRange.start + tickRange.delta * static_cast<double>(i);
    }
    return tickPositions;
}

std::vector<double> getMinorTickPositions(const MinorTickSettings& ticks,
                                          const MajorTickSettings& majorTicks, dvec2 range,
                                          size_t maxTicks) {
    if ((ticks.getStyle() == TickStyle::None) || (ticks.getTickFrequency() < 2)) {
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

    // calculate number of major ticks
    const auto majorTickRange = tickutil::getMajorTickRange(majorTicks, range, maxTicks);

    const auto minorTickDelta =
        majorTickRange.delta / static_cast<double>(ticks.getTickFrequency());

    double startMinor = majorTickRange.start;
    double stopMinor = majorTickRange.end;

    size_t nextMajorTickIndex;
    if (ticks.getFillAxis()) {
        // need to figure out first and last minor tick positions beyond major ticks

        // first minor tick
        auto n = std::floor((majorTickRange.start - range.x) / minorTickDelta);
        startMinor = majorTickRange.start - n * minorTickDelta;
        // save index for later
        nextMajorTickIndex = static_cast<size_t>(n + 0.5);

        // last minor tick
        n = std::floor((range.y - majorTickRange.end) / minorTickDelta);

        stopMinor = majorTickRange.end + n * minorTickDelta;
    } else {
        // minor ticks are only filled in between major ticks
        startMinor = majorTickRange.start + minorTickDelta;
        stopMinor = majorTickRange.end - minorTickDelta;

        nextMajorTickIndex = ticks.getTickFrequency() - 1u;
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
            nextMajorTickIndex += ticks.getTickFrequency();
            continue;
        }
        tickPositions.push_back(startMinor + minorTickDelta * static_cast<double>(i));
    }

    return tickPositions;
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

    std::fill(colors.begin(), colors.end(), color);

    for (size_t i = 0u; i < numTicks; ++i) {
        const vec3 pos(startPos + scaling * static_cast<float>(tickmarks[i] - axisRange.x));
        vertices[2 * i] = pos + tickDir * tickOffset.x;
        vertices[2 * i + 1] = pos + tickDir * tickOffset.y;
    }

    auto mesh = std::make_unique<Mesh>(DrawType::Lines, ConnectivityType::None);
    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);
    mesh->addBuffer(BufferType::ColorAttrib, colBuffer);

    std::vector<uint32_t> indices(numTicks * 2);
    std::iota(indices.begin(), indices.end(), 0);

    mesh->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None),
                     inviwo::util::makeIndexBuffer(std::move(indices)));

    return mesh;
}

std::pair<vec2, vec2> tickBoundingRect(const AxisSettings& settings, const vec2& startPos,
                                       const vec2& endPos) {
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
        const auto tickPositions = getMajorTickPositions(majorTicks, settings.getRange());
        if (!tickPositions.empty()) {
            const auto pos1 =
                startPos + scaling * static_cast<float>(tickPositions.front() - axisRange.x);
            const vec2 pos2 =
                startPos + scaling * static_cast<float>(tickPositions.back() - axisRange.x);

            const auto ext1 =
                getSize(pos1, tickDirection, majorTicks.getStyle(), settings.getFlipped());
            const auto ext2 =
                getSize(pos2, tickDirection, majorTicks.getStyle(), settings.getFlipped());

            lowerLeft = glm::min(lowerLeft, ext1.first);
            lowerLeft = glm::min(lowerLeft, ext1.second);
            lowerLeft = glm::min(lowerLeft, ext2.first);
            lowerLeft = glm::min(lowerLeft, ext2.second);

            upperRight = glm::max(upperRight, ext1.first);
            upperRight = glm::max(upperRight, ext1.second);
            upperRight = glm::max(upperRight, ext2.first);
            upperRight = glm::max(upperRight, ext2.second);
        }
    }

    {
        const auto tickPositions =
            getMinorTickPositions(minorTicks, majorTicks, settings.getRange());
        if (!tickPositions.empty()) {
            const auto pos1 =
                startPos + scaling * static_cast<float>(tickPositions.front() - axisRange.x);
            const vec2 pos2 =
                startPos + scaling * static_cast<float>(tickPositions.back() - axisRange.x);

            const auto ext1 =
                getSize(pos1, tickDirection, minorTicks.getStyle(), settings.getFlipped());
            const auto ext2 =
                getSize(pos2, tickDirection, minorTicks.getStyle(), settings.getFlipped());

            lowerLeft = glm::min(lowerLeft, ext1.first);
            lowerLeft = glm::min(lowerLeft, ext1.second);
            lowerLeft = glm::min(lowerLeft, ext2.first);
            lowerLeft = glm::min(lowerLeft, ext2.second);

            upperRight = glm::max(upperRight, ext1.first);
            upperRight = glm::max(upperRight, ext1.second);
            upperRight = glm::max(upperRight, ext2.first);
            upperRight = glm::max(upperRight, ext2.second);
        }
    }

    return {lowerLeft, upperRight};
}

std::unique_ptr<Mesh> generateMajorTicksMesh(const AxisSettings& settings, const vec2& startPos,
                                             const vec2& endPos) {
    const auto& ticks = settings.getMajorTicks();
    const auto tickPositions = getMajorTickPositions(ticks, settings.getRange());

    const auto axisDir = glm::normalize(endPos - startPos);
    const auto tickDir = vec3(-axisDir.y, axisDir.x, 0.0f);

    return generateTicksMesh(tickPositions, settings.getRange(), vec3(startPos, 0.0f),
                             vec3(endPos, 0.0f), tickDir, ticks.getTickLength(), ticks.getStyle(),
                             ticks.getColor(), settings.getFlipped());
}

std::unique_ptr<Mesh> generateMinorTicksMesh(const AxisSettings& settings, const vec2& startPos,
                                             const vec2& endPos) {
    const auto& ticks = settings.getMinorTicks();
    const auto tickPositions =
        getMinorTickPositions(ticks, settings.getMajorTicks(), settings.getRange());

    const auto axisDir = glm::normalize(endPos - startPos);
    const auto tickDir = vec3(-axisDir.y, axisDir.x, 0.0f);

    return generateTicksMesh(tickPositions, settings.getRange(), vec3(startPos, 0.0f),
                             vec3(endPos, 0.0f), tickDir, ticks.getTickLength(), ticks.getStyle(),
                             ticks.getColor(), settings.getFlipped());
}

std::unique_ptr<Mesh> generateAxisMesh(const vec2& startPos, const vec2& endPos, const vec4& color,
                                       const size_t& pickingId) {
    return generateAxisMesh3D(vec3(startPos, 0.0f), vec3(endPos, 0.0f), color, pickingId);
}

std::unique_ptr<Mesh> generateMajorTicksMesh3D(const AxisSettings& settings, const vec3& startPos,
                                               const vec3& endPos, const vec3& tickDirection) {
    const auto& ticks = settings.getMajorTicks();
    const auto tickPositions = getMajorTickPositions(ticks, settings.getRange());

    return generateTicksMesh(tickPositions, settings.getRange(), startPos, endPos, tickDirection,
                             ticks.getTickLength(), ticks.getStyle(), ticks.getColor(),
                             settings.getFlipped());
}

std::unique_ptr<Mesh> generateMinorTicksMesh3D(const AxisSettings& settings, const vec3& startPos,
                                               const vec3& endPos, const vec3& tickDirection) {
    const auto& ticks = settings.getMinorTicks();
    const auto tickPositions =
        getMinorTickPositions(ticks, settings.getMajorTicks(), settings.getRange());

    return generateTicksMesh(tickPositions, settings.getRange(), startPos, endPos, tickDirection,
                             ticks.getTickLength(), ticks.getStyle(), ticks.getColor(),
                             settings.getFlipped());
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
    m->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None),
                  util::makeIndexBuffer({0, 1}));

    return m;
}

vec2 getAxisCaptionPosition(const AxisSettings& settings, const vec2& startPos,
                            const vec2& endPos) {

    const vec2 axisPos = glm::mix(startPos, endPos, settings.getCaptionSettings().getPosition());

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x);

    if (settings.getFlipped()) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * settings.getCaptionSettings().getOffset().x +
           axisDir * settings.getCaptionSettings().getOffset().y;
}

std::vector<std::pair<double, vec2>> getLabelPositions(const AxisSettings& settings,
                                                       const vec2& startPos, const vec2& endPos) {
    const auto tickmarks = getMajorTickPositions(settings);

    if (tickmarks.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x);

    if (settings.getFlipped()) {
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
    std::vector<std::pair<double, vec2>> labelPositions(tickmarks.size());

    if (axisLength != 0.0) {
        const vec2 scaling(axisDir * static_cast<float>(screenLength / axisLength));
        std::transform(
            tickmarks.begin(), tickmarks.end(), labelPositions.begin(),
            [&](double pos) -> std::pair<double, vec2> {
                return {pos, labelOrigin + scaling * static_cast<float>(pos - axisRange.x)};
            });

    } else {
        const vec2 scaling(axisDir * static_cast<float>(screenLength / (tickmarks.size() - 1)));
        auto seq = util::make_sequence(size_t{0}, tickmarks.size(), size_t{1});
        std::transform(tickmarks.begin(), tickmarks.end(), seq.begin(), labelPositions.begin(),
                       [&](double pos, size_t i) -> std::pair<double, vec2> {
                           return {pos, labelOrigin + scaling * static_cast<double>(i)};
                       });
    }
    return labelPositions;
}

vec3 getAxisCaptionPosition3D(const AxisSettings& settings, const vec3& startPos,
                              const vec3& endPos, const vec3& tickDirection) {
    const vec3 axisPos(glm::mix(startPos, endPos, settings.getCaptionSettings().getPosition()));
    auto normal = -glm::normalize(tickDirection);

    if (settings.getFlipped()) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * settings.getCaptionSettings().getOffset().x +
           glm::normalize(endPos - startPos) * settings.getCaptionSettings().getOffset().y;
}

std::vector<std::pair<double, vec3>> getLabelPositions3D(const AxisSettings& settings,
                                                         const vec3& startPos, const vec3& endPos,
                                                         const vec3& tickDirection) {
    const auto tickmarks = getMajorTickPositions(settings);

    if (tickmarks.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = -glm::normalize(tickDirection);

    if (settings.getFlipped()) {
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

    std::vector<std::pair<double, vec3>> labelPositions(tickmarks.size());
    std::transform(tickmarks.begin(), tickmarks.end(), labelPositions.begin(),
                   [&](double pos) -> std::pair<double, vec3> {
                       return {pos, labelOrigin + scaling * static_cast<float>(pos - axisRange.x)};
                   });

    return labelPositions;
}

}  // namespace plot

}  // namespace inviwo
