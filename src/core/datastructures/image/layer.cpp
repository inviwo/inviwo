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
    , defaultWrapping_{wrapping}
    , histograms_{} {}

Layer::Layer(LayerConfig config)
    : Data<Layer, LayerRepresentation>{}
    , StructuredGridEntity<2>{config.model.value_or(LayerConfig::defaultModel),
                              config.world.value_or(LayerConfig::defaultWorld)}
    , dataMap{config.dataMap()}
    , axes{config.xAxis.value_or(LayerConfig::defaultXAxis),
           config.yAxis.value_or(LayerConfig::defaultYAxis)}
    , defaultLayerType_{config.type.value_or(LayerConfig::defaultType)}
    , defaultDimensions_{config.dimensions.value_or(LayerConfig::defaultDimensions)}
    , defaultDataFormat_{config.format ? config.format : LayerConfig::defaultFormat}
    , defaultSwizzleMask_{config.swizzleMask.value_or(LayerConfig::defaultSwizzleMask)}
    , defaultInterpolation_{config.interpolation.value_or(LayerConfig::defaultInterpolation)}
    , defaultWrapping_{config.wrapping.value_or(LayerConfig::defaultWrapping)}
    , histograms_{} {}

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
    , defaultWrapping_{in->getWrapping()}
    , histograms_{} {

    addRepresentation(in);
}

Layer::Layer(const Layer& rhs, NoData, LayerConfig config)
    : Data<Layer, LayerRepresentation>{}
    , StructuredGridEntity<2>{config.model.value_or(rhs.getModelMatrix()),
                              config.world.value_or(rhs.getWorldMatrix())}
    , dataMap{config.dataMap(rhs.dataMap)}
    , axes{config.xAxis.value_or(rhs.axes[0]), config.yAxis.value_or(rhs.axes[1])}
    , defaultLayerType_{config.type.value_or(rhs.getLayerType())}
    , defaultDimensions_{config.dimensions.value_or(rhs.getDimensions())}
    , defaultDataFormat_{config.format ? config.format : rhs.getDataFormat()}
    , defaultSwizzleMask_{config.swizzleMask.value_or(rhs.getSwizzleMask())}
    , defaultInterpolation_{config.interpolation.value_or(rhs.getInterpolation())}
    , defaultWrapping_{config.wrapping.value_or(rhs.getWrapping())}
    , histograms_{} {}

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

vec2 Layer::getWorldSpaceGradientSpacing() const {
    mat3 textureToWorld = mat3(getCoordinateTransformer().getTextureToWorldMatrix());

    const vec3 extent{glm::length2(textureToWorld[0]), glm::length2(textureToWorld[1]),
                      glm::length2(textureToWorld[2])};

    // basis vectors
    vec3 a;
    vec3 b;
    if (extent[0] < glm::epsilon<float>()) {
        a = textureToWorld[1];
        b = textureToWorld[2];
    } else if (extent[1] < glm::epsilon<float>()) {
        a = textureToWorld[0];
        b = textureToWorld[2];
    } else {
        a = textureToWorld[0];
        b = textureToWorld[1];
    }
    // basis vectors with a length of one texel, may be non-orthogonal
    const auto dimensions = getDimensions();
    a /= static_cast<float>(dimensions[0]);
    b /= static_cast<float>(dimensions[1]);

    // Project the texel basis vectors
    // onto the world space x/y axes,
    // and choose the longest projected vector
    // for each axis.
    // Using the fact that
    // vec2 x{ 1.f, 0 };
    // vec2 y{ 0, 1.f };
    // such that
    // ax' = dot(x, a) = a.x
    // bx' = dot(x, b) = b.x
    // and so on.
    auto signedMax = [](const float& x1, const float& x2) {
        return (std::abs(x1) >= std::abs(x2)) ? x1 : x2;
    };

    vec2 ds{signedMax(a.x, b.x), signedMax(a.y, b.y)};

    // Return the spacing in world space,
    // actually given by:
    // { gradientSpacing.x         0
    //         0             gradientSpacing.y }
    return ds;
}

const Axis* Layer::getAxis(size_t index) const {
    if (index >= 2) {
        return nullptr;
    }
    return &axes[index];
}

LayerConfig Layer::config() const {
    return {.dimensions = getDimensions(),
            .format = getDataFormat(),
            .type = getLayerType(),
            .swizzleMask = getSwizzleMask(),
            .interpolation = getInterpolation(),
            .wrapping = getWrapping(),
            .xAxis = axes[0],
            .yAxis = axes[1],
            .valueAxis = dataMap.valueAxis,
            .dataRange = dataMap.dataRange,
            .valueRange = dataMap.valueRange,
            .model = getModelMatrix(),
            .world = getWorldMatrix()};
}

namespace {

auto histCalc(const Layer& v) {
    return [dataMap = v.dataMap, repr = v.getRepresentationShared<LayerRAM>()]() {
        return repr->dispatch<std::vector<Histogram1D>>(
            [&]<typename T>(const LayerRAMPrecision<T>* rp) {
                return util::calculateHistograms(rp->getView(), dataMap, 2048);
            });
    };
}

}  // namespace

void Layer::discardHistograms() { histograms_.discard(histCalc(*this)); }

HistogramCache::Result Layer::calculateHistograms(
    std::function<void(const std::vector<Histogram1D>&)> whenDone) const {
    return histograms_.calculateHistograms(histCalc(*this), std::move(whenDone));
}

template class IVW_CORE_TMPL_INST DataReaderType<Layer>;
template class IVW_CORE_TMPL_INST DataWriterType<Layer>;

}  // namespace inviwo
