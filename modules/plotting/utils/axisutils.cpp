/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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
#include <inviwo/core/util/assertion.h>

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

AxisTickRange getMajorTickRange(const AxisProperty& property) {
    const auto& ticks = property.ticks_.majorTicks_;

    // calculate number of major ticks
    double startValue = property.range_.getStart();
    double endValue = property.range_.getEnd();

    if (ticks.tickDelta_.get() <= 0.0) {
        return {startValue, endValue, endValue - startValue, 2u};
    }

    if (!ticks.rangeBasedTicks_.get()) {
        // the major ticks should appear at n * tickDelta and not based at axis range min
        startValue = std::ceil((startValue - glm::epsilon<double>()) / ticks.tickDelta_.get()) *
                     ticks.tickDelta_.get();
        endValue = std::floor((endValue + glm::epsilon<double>()) / ticks.tickDelta_.get()) *
                   ticks.tickDelta_.get();
        endValue = std::max(startValue, endValue);
    }

    const auto axisLength = endValue - startValue;
    // in case tickDelta is larger than the entire axis, adjust it to axis length
    // in order to only draw first and last tick corresponding to axis range
    double tickDelta = ticks.tickDelta_.get();

    size_t numTicks = 0u;
    if ((startValue - property.range_.getEnd() > glm::epsilon<double>()) ||
        (ticks.tickDelta_.get() <= 0.0)) {
        numTicks = 0u;
        endValue = startValue;
    } else if (std::abs(axisLength) < glm::epsilon<double>()) {
        // no difference between start and end tick, place only one tick
        numTicks = 1u;
    } else {
        numTicks = static_cast<size_t>((axisLength + glm::epsilon<double>()) / tickDelta) + 1;
        // adjust end value
        endValue = startValue + (numTicks - 1) * tickDelta;
        tickDelta = glm::min(ticks.tickDelta_.get(), axisLength);
    }

    return {startValue, endValue, tickDelta, numTicks};
}

}  // namespace tickutil

bool isAxisFlipped(const AxisProperty& property) {
    // flip tick placement if axis is either vertical or placement is inside (top/right)
    return (property.orientation_.get() == AxisProperty::Orientation::Vertical) !=
           (property.placement_.get() == AxisProperty::Placement::Inside);
}

std::vector<double> getMajorTickPositions(const AxisProperty& property) {
    const auto& ticks = property.ticks_.majorTicks_;

    if (ticks.style_.get() == TickStyle::None) {
        return {};
    }

    // calculate number of major ticks
    tickutil::AxisTickRange tickRange = tickutil::getMajorTickRange(property);

    if (tickRange.numTicks == 0u) {
        LogWarnCustom("Axis Plotting", "Invalid axis range or tick delta. No major ticks ("
                                           << property.getDisplayName() << ")");
        return {};
    }

    std::vector<double> tickPositions(tickRange.numTicks);
    // compute tick positions
    for (size_t i = 0u; i < tickRange.numTicks; ++i) {
        tickPositions[i] = tickRange.start + tickRange.delta * static_cast<double>(i);
    }
    return tickPositions;
}

std::vector<double> getMinorTickPositions(const AxisProperty& property) {
    const auto& ticks = property.ticks_.minorTicks_;

    if ((ticks.style_.get() == TickStyle::None) || (ticks.tickFrequency_.get() < 2)) {
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
    tickutil::AxisTickRange majorTickRange = tickutil::getMajorTickRange(property);

    const auto minorTickDelta =
        majorTickRange.delta / static_cast<double>(ticks.tickFrequency_.get());

    double startMinor = majorTickRange.start;
    double stopMinor = majorTickRange.end;

    size_t nextMajorTickIndex;
    if (ticks.fillAxis_.get()) {
        // need to figure out first and last minor tick positions beyond major ticks

        // first minor tick
        auto n = std::floor((majorTickRange.start - property.range_.getStart()) / minorTickDelta);
        startMinor = majorTickRange.start - n * minorTickDelta;
        // save index for later
        nextMajorTickIndex = static_cast<size_t>(n + 0.5);

        // last minor tick
        n = std::floor((property.range_.getEnd() - majorTickRange.end) / minorTickDelta);

        stopMinor = majorTickRange.end + n * minorTickDelta;
    } else {
        // minor ticks are only filled in between major ticks
        startMinor = majorTickRange.start + minorTickDelta;
        stopMinor = majorTickRange.end - minorTickDelta;

        nextMajorTickIndex = ticks.tickFrequency_.get() - 1u;
    }

    // Minor ticks fill the entire axis, except where a major tick is supposed to be.
    const size_t totalTicks =
        static_cast<size_t>((stopMinor - startMinor + glm::epsilon<double>()) / minorTickDelta) + 1;

    std::vector<double> tickPositions;
    tickPositions.reserve(totalTicks);

    // compute tick positions
    for (size_t i = 0u; i < totalTicks; ++i) {
        if (i == nextMajorTickIndex) {
            // skip major tick positions
            nextMajorTickIndex += ticks.tickFrequency_.get();
            continue;
        }
        tickPositions.push_back(startMinor + minorTickDelta * static_cast<double>(i));
    }

    return tickPositions;
}

std::shared_ptr<Mesh> generateTicksMesh(const std::vector<double> tickmarks, dvec2 axisRange,
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

    auto mesh = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::None);
    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);
    mesh->addBuffer(BufferType::ColorAttrib, colBuffer);

    std::vector<uint32_t> indices(numTicks * 2);
    std::iota(indices.begin(), indices.end(), 0);

    mesh->addIndicies(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None),
                      inviwo::util::makeIndexBuffer(std::move(indices)));

    return mesh;
}

std::shared_ptr<Mesh> generateMajorTicksMesh(const AxisProperty& property, const vec2& startPos,
                                             const vec2& endPos) {
    const auto& ticks = property.ticks_.majorTicks_;
    const auto tickPositions = getMajorTickPositions(property);

    const auto axisDir = glm::normalize(endPos - startPos);
    const auto tickDir = vec3(-axisDir.y, axisDir.x, 0.0f);

    return generateTicksMesh(tickPositions, property.range_.get(), vec3(startPos, 0.0f),
                             vec3(endPos, 0.0f), tickDir, ticks.tickLength_.get(),
                             ticks.style_.get(), ticks.color_.get(), isAxisFlipped(property));
}

std::shared_ptr<Mesh> generateMinorTicksMesh(const AxisProperty& property, const vec2& startPos,
                                             const vec2& endPos) {
    const auto& ticks = property.ticks_.minorTicks_;
    const auto tickPositions = getMinorTickPositions(property);

    const auto axisDir = glm::normalize(endPos - startPos);
    const auto tickDir = vec3(-axisDir.y, axisDir.x, 0.0f);

    return generateTicksMesh(tickPositions, property.range_.get(), vec3(startPos, 0.0f),
                             vec3(endPos, 0.0f), tickDir, ticks.tickLength_.get(),
                             ticks.style_.get(), ticks.color_.get(), isAxisFlipped(property));
}

std::shared_ptr<Mesh> generateAxisMesh(const AxisProperty& property, const vec2& startPos,
                                       const vec2& endPos) {
    return generateAxisMesh3D(property, vec3(startPos, 0.0f), vec3(endPos, 0.0f));
}

std::shared_ptr<Mesh> generateMajorTicksMesh3D(const AxisProperty& property, const vec3& startPos,
                                               const vec3& endPos, const vec3& tickDirection) {
    const auto& ticks = property.ticks_.majorTicks_;
    const auto tickPositions = getMajorTickPositions(property);

    return generateTicksMesh(tickPositions, property.range_.get(), startPos, endPos, tickDirection,
                             ticks.tickLength_.get(), ticks.style_.get(), ticks.color_.get(),
                             isAxisFlipped(property));
}

std::shared_ptr<Mesh> generateMinorTicksMesh3D(const AxisProperty& property, const vec3& startPos,
                                               const vec3& endPos, const vec3& tickDirection) {
    const auto& ticks = property.ticks_.minorTicks_;
    const auto tickPositions = getMinorTickPositions(property);

    return generateTicksMesh(tickPositions, property.range_.get(), startPos, endPos, tickDirection,
                             ticks.tickLength_.get(), ticks.style_.get(), ticks.color_.get(),
                             isAxisFlipped(property));
}

std::shared_ptr<Mesh> generateAxisMesh3D(const AxisProperty& property, const vec3& startPos,
                                         const vec3& endPos) {
    auto mesh = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::None);

    auto posBuffer = std::make_shared<Buffer<vec3>>(2u, BufferUsage::Static);
    auto colBuffer = std::make_shared<Buffer<vec4>>(2u, BufferUsage::Static);
    auto& vertices = posBuffer->getEditableRAMRepresentation()->getDataContainer();
    auto& colors = colBuffer->getEditableRAMRepresentation()->getDataContainer();

    std::fill(colors.begin(), colors.end(), property.color_.get());

    vertices[0] = startPos;
    vertices[1] = endPos;

    mesh->addBuffer(BufferType::PositionAttrib, posBuffer);
    mesh->addBuffer(BufferType::ColorAttrib, colBuffer);

    mesh->addIndicies(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None),
                      inviwo::util::makeIndexBuffer({0, 1}));

    return mesh;
}

vec2 getAxisCaptionPosition(const AxisProperty& property, const vec2& startPos,
                            const vec2& endPos) {
    const vec2 axisPos((startPos + endPos) * property.caption_.position_.get());

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x);

    if (isAxisFlipped(property)) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * property.caption_.offset_.get();
}

std::vector<std::pair<double, vec2>> getLabelPositions(const AxisProperty& property,
                                                       const vec2& startPos, const vec2& endPos) {
    const auto tickmarks = getMajorTickPositions(property);

    if (tickmarks.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = vec2(axisDir.y, -axisDir.x);

    if (isAxisFlipped(property)) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    // determine position of left-most label
    const vec2 labelOrigin(startPos + normal * property.labels_.offset_.get());

    // position a label below each tick
    const auto axisRange = property.range_.get();
    const auto screenLength(glm::distance(endPos, startPos));
    const auto axisLength = axisRange.y - axisRange.x;
    const vec2 scaling(axisDir * static_cast<float>(screenLength / axisLength));

    std::vector<std::pair<double, vec2>> labelPositions(tickmarks.size());
    std::transform(tickmarks.begin(), tickmarks.end(), labelPositions.begin(),
                   [&](double pos) -> std::pair<double, vec2> {
                       return {pos, labelOrigin + scaling * static_cast<float>(pos - axisRange.x)};
                   });

    return labelPositions;
}

vec3 getAxisCaptionPosition3D(const AxisProperty& property, const vec3& startPos,
                              const vec3& endPos, const vec3& tickDirection) {
    const vec3 axisPos(glm::mix(startPos, endPos, property.caption_.position_.get()));
    auto normal = -glm::normalize(tickDirection);

    if (isAxisFlipped(property)) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    return axisPos + normal * property.caption_.offset_.get();
}

std::vector<std::pair<double, vec3>> getLabelPositions3D(const AxisProperty& property,
                                                         const vec3& startPos, const vec3& endPos,
                                                         const vec3& tickDirection) {
    const auto tickmarks = getMajorTickPositions(property);

    if (tickmarks.empty()) {
        return {};
    }

    const auto axisDir = glm::normalize(endPos - startPos);
    auto normal = -glm::normalize(tickDirection);

    if (isAxisFlipped(property)) {
        // reverse normal as labels are supposed to be on the other side of the axis
        normal = -normal;
    }

    // determine position of left-most label
    const vec3 labelOrigin(startPos + normal * property.labels_.offset_.get());

    // position a label below each tick
    const auto axisRange = property.range_.get();
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

}  // namespace plotting

}  // namespace inviwo
