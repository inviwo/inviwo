/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layerconfig.h>

namespace inviwo {

DataMapper LayerConfig::dataMap() const {
    const auto dataFormat = formatId.value_or(LayerConfig::defaultFormatId);
    return DataMapper{dataRange.value_or(defaultDataRange(dataFormat)),
                      valueRange.value_or(defaultValueRange(dataFormat)),
                      valueAxis.value_or(defaultValueAxis)};
}
DataMapper LayerConfig::dataMap(const DataMapper& defaultMapper) const {
    return DataMapper{dataRange.value_or(defaultMapper.dataRange),
                      valueRange.value_or(defaultMapper.valueRange),
                      valueAxis.value_or(defaultMapper.valueAxis)};
}

LayerConfig& LayerConfig::updateFrom(const LayerConfig& config) {
    static constexpr auto update = [](auto& dest, const auto& src) {
        if (src) {
            dest = src.value();
        }
    };
    update(dimensions, config.dimensions);
    update(formatId, config.formatId);
    update(type, config.type);
    update(swizzleMask, config.swizzleMask);
    update(interpolation, config.interpolation);
    update(wrapping, config.wrapping);
    update(xAxis, config.xAxis);
    update(yAxis, config.yAxis);
    update(valueAxis, config.valueAxis);
    update(dataRange, config.dataRange);
    update(valueRange, config.valueRange);
    update(model, config.model);
    update(world, config.world);

    return *this;
}

LayerConfig LayerConfig::orDefault() const {
    return {.dimensions = dimensions.value_or(LayerConfig::defaultDimensions),
            .formatId = formatId.value_or(LayerConfig::defaultFormatId),
            .swizzleMask = swizzleMask.value_or(LayerConfig::defaultSwizzleMask),
            .interpolation = interpolation.value_or(LayerConfig::defaultInterpolation),
            .wrapping = wrapping.value_or(LayerConfig::defaultWrapping),
            .xAxis = xAxis.value_or(LayerConfig::defaultXAxis),
            .yAxis = yAxis.value_or(LayerConfig::defaultYAxis),
            .valueAxis = valueAxis.value_or(LayerConfig::defaultValueAxis),
            .dataRange =
                dataRange.value_or(LayerConfig::defaultDataRange(LayerConfig::defaultFormatId)),
            .valueRange =
                valueRange.value_or(LayerConfig::defaultValueRange(LayerConfig::defaultFormatId)),
            .model = model.value_or(LayerConfig::defaultModel),
            .world = world.value_or(LayerConfig::defaultWorld)};
}

LayerReprConfig LayerConfig::reprConfig() const {
    return {.dimensions = dimensions,
            .formatId = formatId,
            .swizzleMask = swizzleMask,
            .interpolation = interpolation,
            .wrapping = wrapping};
}

glm::dmat4 LayerConfig::aspectPreservingModelMatrixFromDimensions(size2_t dimensions) {
    glm::dmat4 model{1.0};
    if (dimensions.x < dimensions.y) {
        model[0][0] = static_cast<double>(dimensions.x) / static_cast<double>(dimensions.y);
    } else {
        model[1][1] = static_cast<double>(dimensions.y) / static_cast<double>(dimensions.x);
    }
    return model;
}

}  // namespace inviwo
