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

#include <modules/hdf5/datastructures/layerhdf.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/hdf5exception.h>

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmfmt.h>

#include <mutex>

#include <fmt/std.h>

namespace inviwo::hdf5 {

LayerHDF::LayerHDF(Handle handle, std::vector<Selection> selection, const LayerReprConfig& config)
    : LayerRepresentation{config.type.value_or(LayerConfig::defaultType)}
    , handle_{std::move(handle)}
    , selection_{std::move(selection)}
    , dimensions_{}
    , format_{nullptr}
    , swizzleMask_{config.swizzleMask.value_or(LayerConfig::defaultSwizzleMask)}
    , interpolation_{config.interpolation.value_or(LayerConfig::defaultInterpolation)}
    , wrapping_{config.wrapping.value_or(LayerConfig::defaultWrapping)} {

    const std::scoped_lock lock{Handle::globalMutex()};
    const auto resolved =
        getLayerConfig(handle_, selection_, LayerConfig{.formatId = config.formatId});
    if (!resolved.formatId) {
        throw Exception{SourceContext{}, "Could not determine data format for dataset: {}{}",
                        handle_.getFilename(), handle_.getPath().toString()};
    }
    dimensions_ = resolved.dimensions.value_or(LayerConfig::defaultDimensions);
    format_ = DataFormatBase::get(*resolved.formatId);
}

LayerHDF* LayerHDF::clone() const { return new LayerHDF(*this); }

std::type_index LayerHDF::getTypeIndex() const { return {typeid(LayerHDF)}; }

const DataFormatBase* LayerHDF::getDataFormat() const { return format_; }

void LayerHDF::setDimensions(size2_t) {
    throw Exception{SourceContext{},
                    "Can not set the dimensions of a LayerHDF, they are given by the selection"};
}

const size2_t& LayerHDF::getDimensions() const { return dimensions_; }

void LayerHDF::setSwizzleMask(const SwizzleMask& mask) { swizzleMask_ = mask; }

SwizzleMask LayerHDF::getSwizzleMask() const { return swizzleMask_; }

void LayerHDF::setInterpolation(InterpolationType interpolation) { interpolation_ = interpolation; }

InterpolationType LayerHDF::getInterpolation() const { return interpolation_; }

void LayerHDF::setWrapping(const Wrapping2D& wrapping) { wrapping_ = wrapping; }

Wrapping2D LayerHDF::getWrapping() const { return wrapping_; }

bool LayerHDF::copyRepresentationsTo(LayerRepresentation*) const { return false; }

const Handle& LayerHDF::getHandle() const { return handle_; }

const std::vector<Selection>& LayerHDF::getSelection() const { return selection_; }

void LayerHDF::readInto(std::span<std::byte> dst, const DataFormatBase* format) const {
    readLayerData(handle_, selection_, dimensions_, format, dst);
}

void LayerHDF::readInto(std::span<std::byte> dst) const { readInto(dst, format_); }

size_t LayerHDF::getSizeInBytes(const DataFormatBase* format) const {
    return glm::compMul(dimensions_) * format->getSizeInBytes();
}

}  // namespace inviwo::hdf5
