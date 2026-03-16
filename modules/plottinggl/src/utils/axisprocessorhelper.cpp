/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/plottinggl/utils/axisprocessorhelper.h>
#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/spatialdata.h>

#include <glm/gtx/component_wise.hpp>

#include <algorithm>
#include <ranges>
#include <array>
#include <vector>

namespace inviwo::plot {

OptionPropertyState<AxisRangeMode> rangeModeState(bool hasDims, bool hasBoundingBox) {

    using enum AxisRangeMode;
    std::vector<OptionPropertyOption<AxisRangeMode>> rangeOptions;
    if (hasDims) {
        rangeOptions.emplace_back("dims", "Volume Dimensions (voxel)", Dims);
    }
    rangeOptions.emplace_back("basis", "Basis", Basis);
    rangeOptions.emplace_back("basisOffset", "Basis & Offset", BasisOffset);
    rangeOptions.emplace_back("world", "Model & World Transform", World);
    if (hasBoundingBox) {
        rangeOptions.emplace_back("dataBoundingBox", "Data Bounding Box", DataBoundingBox);
        rangeOptions.emplace_back("modelBoundingBox", "Model Bounding Box", ModelBoundingBox);
        rangeOptions.emplace_back("boundingBox", "World Bounding Box", WorldBoundingBox);
    }

    return {.options = rangeOptions, .help = "Determines axis ranges"_help};
}

float calcScaleFactor(const glm::mat4& matrix, OffsetScaling mode) {
    const auto l = vec3{glm::length(matrix[0]), glm::length(matrix[1]), glm::length(matrix[2])};
    switch (mode) {
        case OffsetScaling::MinExtent:
            return glm::compMin(l) / 100.0f;
        case OffsetScaling::MaxExtent:
            return glm::compMax(l) / 100.0f;
        case OffsetScaling::MeanExtent:
            return glm::compAdd(l) / (3.0f * 100.0f);
        case OffsetScaling::Diagonal:
            return glm::length(matrix[0] + matrix[1] + matrix[2]) / 100.0f;
        case OffsetScaling::None:
        default:
            return 1.0f;
    }
}

namespace {
std::array<dvec2, 3> getRangeDims(const SpatialEntity& entity) {
    dvec3 volDims(1.0);
    if (const auto* grid2d = dynamic_cast<const StructuredGridEntity<2>*>(&entity)) {
        volDims = dvec3(grid2d->getDimensions(), 0.0);
    } else if (const auto* grid3d = dynamic_cast<const StructuredGridEntity<3>*>(&entity)) {
        volDims = dvec3(grid3d->getDimensions());
    }
    return {dvec2{0.0, volDims.x}, dvec2{0.0, volDims.y}, dvec2{0.0, volDims.z}};
}

std::array<dvec2, 3> getRangeBasis(const SpatialEntity& entity) {
    dvec3 basisLen(1.0);
    for (size_t i = 0; i < 3; ++i) {
        basisLen[i] = glm::length(entity.getBasis()[i]);
    }
    return {dvec2{0.0, basisLen.x}, dvec2{0.0, basisLen.y}, dvec2{0.0, basisLen.z}};
}

std::array<dvec2, 3> getRangeBasisOffset(const SpatialEntity& entity) {
    const auto offset = dvec3{entity.getOffset()};
    const auto ranges = getRangeBasis(entity);
    return {ranges[0] + offset.x, ranges[1] + offset.y, ranges[2] + offset.z};
}

std::array<dvec2, 3> getRangeWorld(const SpatialEntity& entity) {
    const auto d2w = dmat4{entity.getCoordinateTransformer().getDataToWorldMatrix()};

    return {dvec2{d2w[3].x, d2w[3].x + glm::length(d2w[0])},
            dvec2{d2w[3].y, d2w[3].y + glm::length(d2w[1])},
            dvec2{d2w[3].z, d2w[3].z + glm::length(d2w[2])}};
}

std::array<dvec2, 3> getRangeBoundingBox(CoordinateSpace destSpace, const SpatialEntity& entity,
                                         std::optional<mat4> worldBoundingBox) {
    if (worldBoundingBox) {
        const auto bb =
            dmat4{entity.getCoordinateTransformer().getMatrix(CoordinateSpace::World, destSpace)} *
            dmat4{worldBoundingBox.value()};

        return {dvec2{bb[3].x, bb[3].x + glm::length(bb[0])},
                dvec2{bb[3].y, bb[3].y + glm::length(bb[1])},
                dvec2{bb[3].z, bb[3].z + glm::length(bb[2])}};
    } else {
        return {dvec2{0.0, 1.0}, dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};
    }
}

}  // namespace

std::array<dvec2, 3> calcAxisRanges(const SpatialEntity& entity,
                                    std::optional<mat4> worldBoundingBox, AxisRangeMode mode) {

    switch (mode) {
        case AxisRangeMode::Dims:
            return getRangeDims(entity);
        case AxisRangeMode::Basis:
            return getRangeBasis(entity);
        case AxisRangeMode::BasisOffset:
            return getRangeBasisOffset(entity);
        case AxisRangeMode::World:
            return getRangeWorld(entity);
        case AxisRangeMode::DataBoundingBox:
            return getRangeBoundingBox(CoordinateSpace::Data, entity, worldBoundingBox);
        case AxisRangeMode::ModelBoundingBox:
            return getRangeBoundingBox(CoordinateSpace::Model, entity, worldBoundingBox);
        case AxisRangeMode::WorldBoundingBox:
            return getRangeBoundingBox(CoordinateSpace::World, entity, worldBoundingBox);
    }
    return {dvec2{0.0, 1.0}, dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};
}

dmat4 getTransform(const SpatialEntity& entity, std::optional<mat4> worldBoundingBox,
                   AxisRangeMode mode) {

    const dmat4 m = entity.getCoordinateTransformer().getDataToWorldMatrix();
    using enum AxisRangeMode;
    if (std::ranges::contains(std::to_array({DataBoundingBox, ModelBoundingBox, WorldBoundingBox}),
                              mode)) {
        return worldBoundingBox.transform([](const mat4& bbox) { return dmat4{bbox}; }).value_or(m);
    } else {
        return m;
    }
}

}  // namespace inviwo::plot
