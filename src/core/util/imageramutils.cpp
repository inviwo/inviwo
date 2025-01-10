/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <algorithm>

namespace inviwo {

namespace util {

void flipLayerVertical(Layer& layer) {
    layer.getEditableRepresentation<LayerRAM>()->dispatch<void>([](auto layerpr) {
        using ValueType = util::PrecisionValueType<decltype(layerpr)>;
        auto data = layerpr->getDataTyped();
        const auto dims = layerpr->getDimensions();
        for (size_t y = 0; y < dims.y / 2; ++y) {
            ValueType* it1 = data + y * dims.x;
            ValueType* it2 = data + (dims.y - 1 - y) * dims.x;
            std::swap_ranges(it1, it1 + dims.x, it2);
        }
    });
}

void flipLayerHorizontal(Layer& layer) {
    layer.getEditableRepresentation<LayerRAM>()->dispatch<void>([](auto layerpr) {
        using ValueType = util::PrecisionValueType<decltype(layerpr)>;
        auto data = layerpr->getDataTyped();
        const auto dims = layerpr->getDimensions();
        for (size_t y = 0; y < dims.y; ++y) {
            ValueType* it = data + y * dims.x;
            std::reverse(it, it + dims.x);
        }
    });
}

void flipImageVertical(Image& img) { img.forEachLayer(flipLayerVertical); }

void flipImageHorizontal(Image& img) { img.forEachLayer(flipLayerHorizontal); }

std::shared_ptr<Image> readImageFromDisk(std::string filename) {
    auto app = InviwoApplication::getPtr();
    auto factory = app->getDataReaderFactory();
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>(filename)) {
        auto outLayer = reader->readData(filename);
        return std::make_shared<Image>(outLayer);
    } else {
        log::error("Failed to read image '{}'", filename);
        return nullptr;
    }
}

size_t detail::getPoolSize() {
    return InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->poolSize_.get();
}

}  // namespace util

}  // namespace inviwo
