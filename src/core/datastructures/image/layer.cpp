/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datawriterexception.h>

namespace inviwo {

Layer::Layer(size2_t defaultDimensions, const DataFormatBase* defaultFormat, LayerType type,
             const SwizzleMask& defaultSwizzleMask, InterpolationType interpolation,
             const Wrapping2D& wrapping)
    : Data<Layer, LayerRepresentation>{}
    , StructuredGridEntity<2>{}
    , dataMap{defaultFormat}
    , axes{util::defaultAxes<2>()}
    , defaultLayerType_{type}
    , defaultDimensions_{defaultDimensions}
    , defaultDataFormat_{defaultFormat}
    , defaultSwizzleMask_{defaultSwizzleMask}
    , defaultInterpolation_{interpolation}
    , defaultWrapping_{wrapping} {}

Layer::Layer(std::shared_ptr<LayerRepresentation> in)
    : Data<Layer, LayerRepresentation>{}
    , StructuredGridEntity<2>{}
    , dataMap{in->getDataFormat()}
    , axes{util::defaultAxes<2>()}
    , defaultLayerType_{in->getLayerType()}
    , defaultDimensions_{in->getDimensions()}
    , defaultDataFormat_{in->getDataFormat()}
    , defaultSwizzleMask_{in->getSwizzleMask()}
    , defaultInterpolation_{in->getInterpolation()}
    , defaultWrapping_{in->getWrapping()} {

    addRepresentation(in);
}

Layer::Layer(const Layer& rhs, NoData, const DataFormatBase* defaultFormat)
    : Data<Layer, LayerRepresentation>{}
    , StructuredGridEntity<2>{rhs}
    , dataMap{defaultFormat ? defaultFormat : rhs.getDataFormat()}
    , axes{rhs.axes}
    , defaultLayerType_{rhs.getLayerType()}
    , defaultDimensions_{rhs.getDimensions()}
    , defaultDataFormat_{defaultFormat ? defaultFormat : rhs.getDataFormat()}
    , defaultSwizzleMask_{rhs.getSwizzleMask()}
    , defaultInterpolation_{rhs.getInterpolation()}
    , defaultWrapping_{rhs.getWrapping()} {}

Layer* Layer::clone() const { return new Layer(*this); }

LayerType Layer::getLayerType() const {
    return getLastOr(&LayerRepresentation::getLayerType, defaultLayerType_);
}

void Layer::setDimensions(const size2_t& dim) {
    defaultDimensions_ = dim;
    setLastAndInvalidateOther(&LayerRepresentation::setDimensions, dim);
}

size2_t Layer::getDimensions() const {
    return getLastOr(&LayerRepresentation::getDimensions, defaultDimensions_);
}

void Layer::setDataFormat(const DataFormatBase* format) { defaultDataFormat_ = format; }

const DataFormatBase* Layer::getDataFormat() const {
    return getLastOr(&LayerRepresentation::getDataFormat, defaultDataFormat_);
}

void Layer::setSwizzleMask(const SwizzleMask& mask) {
    defaultSwizzleMask_ = mask;
    setLastAndInvalidateOther(&LayerRepresentation::setSwizzleMask, mask);
}

SwizzleMask Layer::getSwizzleMask() const {
    return getLastOr(&LayerRepresentation::getSwizzleMask, defaultSwizzleMask_);
}

void Layer::setInterpolation(InterpolationType interpolation) {
    defaultInterpolation_ = interpolation;
    setLastAndInvalidateOther(&LayerRepresentation::setInterpolation, interpolation);
}

InterpolationType Layer::getInterpolation() const {
    return getLastOr(&LayerRepresentation::getInterpolation, defaultInterpolation_);
}

void Layer::setWrapping(const Wrapping2D& wrapping) {
    defaultWrapping_ = wrapping;
    setLastAndInvalidateOther(&LayerRepresentation::setWrapping, wrapping);
}

Wrapping2D Layer::getWrapping() const {
    return getLastOr(&LayerRepresentation::getWrapping, defaultWrapping_);
}

std::unique_ptr<std::vector<unsigned char>> Layer::getAsCodedBuffer(
    const std::string& fileExtension) const {
    if (auto writer = std::shared_ptr<DataWriterType<Layer>>(
            InviwoApplication::getPtr()
                ->getDataWriterFactory()
                ->getWriterForTypeAndExtension<Layer>(fileExtension))) {
        try {
            return writer->writeDataToBuffer(this, fileExtension);
        } catch (DataWriterException const& e) {
            LogError(e.getMessage());
        }
    } else {
        LogError("Could not find a writer for the specified file extension (\"" << fileExtension
                                                                                << "\")");
    }

    return std::unique_ptr<std::vector<unsigned char>>();
}

const Axis* Layer::getAxis(size_t index) const {
    if (index >= 2) {
        return nullptr;
    }
    return &axes[index];
}

template class IVW_CORE_TMPL_INST DataReaderType<Layer>;
template class IVW_CORE_TMPL_INST DataWriterType<Layer>;

}  // namespace inviwo
