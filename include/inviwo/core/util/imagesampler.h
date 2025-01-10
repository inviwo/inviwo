/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/indexmapper.h>

#include <inviwo/core/util/spatialsampler.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/util/interpolation.h>

namespace inviwo {

class Image;
class Layer;
class LayerRAM;

/**
 * \brief ImageSampler aids sampling of images using Bi-Linear Interpolation.
 *
 * A helper class to aid sampling of images. Expects the input to be on the range [0 1]. Eg., input
 * of (0,0) will return the color of the bottom left pixel and (1,1) will return then top right
 * pixel. Output values are bi-linear interpolated between the 4 nearest neighbors.
 */
template <typename ReturnType = dvec4>
class ImageSampler : public SpatialSampler<ReturnType> {
public:
    /**
     * Creates a ImageSampler for the given Layer, does not take ownership of \p layer.
     * Use ImageSampler(std::shared_ptr<const Layer>) to ensure that the Layer is available
     * for the lifetime of the ImageSampler
     */
    ImageSampler(const Layer* layer)
        : SpatialSampler<ReturnType>(*layer)
        , layer_(layer->getRepresentation<LayerRAM>())
        , dims_(layer->getDimensions())
        , sharedImage_(nullptr) {}

    ImageSampler(std::shared_ptr<const Layer> layer) : ImageSampler(layer.get()) {
        sharedLayer_ = layer;
    }

    /**
     * Creates a ImageSampler for the given Image, does not take ownership of ram.
     * Use ImageSampler(std::shared_ptr<const Image>) to ensure that the Image is available
     * for the lifetime of the ImageSampler
     */
    ImageSampler(const Image* img) : ImageSampler(img->getColorLayer()) {}

    /**
     * Creates a ImageSampler for the given Image.
     * The shared_ptr will ensure that the Image is available for the lifetime of the
     * ImageSampler
     */
    ImageSampler(std::shared_ptr<const Image> sharedImage)
        : ImageSampler(sharedImage->getColorLayer()) {
        sharedImage_ = sharedImage;
    }

    virtual ~ImageSampler() = default;

    using SpatialSampler<ReturnType>::sample;

    /**
     * Samples the image at the given position using bi-linear interpolation.
     * @param x position to sample at
     * @param y position to sample at
     * @param space in what CoordinateSpace x and y is defined in
     */
    ReturnType sample(double x, double y, CoordinateSpace space) const {
        return sample(dvec3(x, y, 0.0), space);
    }

    /**
     * @see sample(double, double, CoordinateSpace)
     * @param x X coordinate of the position to sample at
     * @param y Y coordinate of the position to sample at
     */
    ReturnType sample(double x, double y) const { return sample(dvec3(x, y, 0.0)); }

protected:
    virtual ReturnType sampleDataSpace(const dvec3& pos) const override {
        dvec2 samplePos = dvec2{pos} * dvec2(dims_ - size2_t(1));
        size2_t indexPos = size2_t(samplePos);
        dvec2 interpolants = samplePos - dvec2(indexPos);

        ReturnType samples[4];
        samples[0] = getPixel(indexPos);
        samples[1] = getPixel(indexPos + size2_t(1, 0));
        samples[2] = getPixel(indexPos + size2_t(0, 1));
        samples[3] = getPixel(indexPos + size2_t(1, 1));

        return Interpolation<ReturnType>::bilinear(samples, interpolants);
    }

    virtual bool withinBoundsDataSpace(const dvec3& pos) const override {
        return !(glm::any(glm::lessThan(dvec2{pos}, dvec2(0.0))) ||
                 glm::any(glm::greaterThan(dvec2{pos}, dvec2(1.0))));
    }

private:
    ReturnType getPixel(const size2_t& pos) const {
        auto p = glm::clamp(pos, size2_t(0), dims_ - size2_t(1));
        return static_cast<ReturnType>(layer_->getAsDVec4(p));
    }
    const LayerRAM* layer_;
    size2_t dims_;

    std::shared_ptr<const Image> sharedImage_;
    std::shared_ptr<const Layer> sharedLayer_;
};

template <size_t N = 4>
using ImageDoubleSampler = ImageSampler<glm::vec<4, double>>;
template <size_t N = 4>
using ImageSpatialSampler = ImageSampler<glm::vec<4, double>>;

/**
 * \brief TemplateImageSampler<T,P> aids sampling of images of a given type (T) using Bi-Linear
 * Interpolation with precision (P).
 *
 * A helper class to aid sampling of images of a given type (T). Expects the input to be on the
 * range [0 1]. Eg., input of (0,0) will return the color of the bottom left pixel and (1,1) will
 * return then top right pixel. Output values are bi-linear interpolated between the 4 nearest
 * neighbors.
 *
 * The template parameter P should be either double or float. When T is either float our double P
 * should be the same.
 */
template <typename ReturnType, typename DataType>
class TemplateImageSampler : public SpatialSampler<ReturnType> {
public:
    TemplateImageSampler(const Layer* layer);
    TemplateImageSampler(const Image* img);
    TemplateImageSampler(std::shared_ptr<const Image> sharedImage);
    virtual ~TemplateImageSampler() = default;

    /**
     * Samples the image at the given position using bi-linear interpolation.
     * @param pos Position to sample at, expects range [0 1]
     */
    ReturnType sample2D(const dvec2& pos) const;

    /**
     * @see sample()
     * @param x X coordinate of the position to sample at, expects range [0 1]
     * @param y Y coordinate of the position to sample at, expects range [0 1]
     */
    ReturnType sample(double x, double y) const;

    using SpatialSampler<ReturnType>::sample;

protected:
    virtual ReturnType sampleDataSpace(const dvec3& pos) const override;
    virtual bool withinBoundsDataSpace(const dvec3& pos) const override;

    ReturnType getPixel(const size2_t& pos) const;
    const DataType* data_;
    size2_t dims_;
    util::IndexMapper2D ic_;

    std::shared_ptr<const Image> sharedImage_;
};

template <typename ReturnType, typename DataType>
TemplateImageSampler<ReturnType, DataType>::TemplateImageSampler(const Layer* layer)
    : SpatialSampler<ReturnType>(*layer)
    , data_(static_cast<const DataType*>(layer->getRepresentation<LayerRAM>()->getData()))
    , dims_(layer->getDimensions())
    , ic_(dims_)
    , sharedImage_(nullptr) {
    if (layer->getDataFormat() != DataFormat<DataType>::get()) {
        throw Exception(IVW_CONTEXT,
                        "Type mismatch when trying to initialize TemplateImageSampler. Image is {} "
                        "but expected {}",
                        layer->getDataFormat()->getString(),
                        DataFormat<DataType>::get()->getString());
    }
}

template <typename ReturnType, typename DataType>
TemplateImageSampler<ReturnType, DataType>::TemplateImageSampler(const Image* img)
    : TemplateImageSampler(img->getColorLayer()) {}

template <typename ReturnType, typename DataType>
TemplateImageSampler<ReturnType, DataType>::TemplateImageSampler(
    std::shared_ptr<const Image> sharedImage)
    : TemplateImageSampler(sharedImage->getColorLayer()) {
    sharedImage_ = sharedImage;
}

template <typename ReturnType, typename DataType>
ReturnType TemplateImageSampler<ReturnType, DataType>::sample(double x, double y) const {
    return sample(dvec2{x, y});
}
template <typename ReturnType, typename DataType>
ReturnType TemplateImageSampler<ReturnType, DataType>::sample2D(const dvec2& pos) const {
    return sample(pos);
}

template <typename ReturnType, typename DataType>
bool TemplateImageSampler<ReturnType, DataType>::withinBoundsDataSpace(const dvec3& pos) const {
    if (glm::any(glm::lessThan(pos, dvec3(0.0)))) {
        return false;
    }
    if (glm::any(glm::greaterThan(pos, dvec3(1.0)))) {
        return false;
    }
    return true;
}

template <typename ReturnType, typename DataType>
auto TemplateImageSampler<ReturnType, DataType>::sampleDataSpace(const dvec3& pos) const
    -> ReturnType {
    if (!withinBoundsDataSpace(pos)) {
        return ReturnType{0};
    }
    dvec2 samplePos = dvec2{pos} * dvec2(dims_ - size2_t(1));
    size2_t indexPos = size2_t(samplePos);
    dvec2 interpolants = samplePos - dvec2(indexPos);

    ReturnType samples[4];
    samples[0] = getPixel(indexPos);
    samples[1] = getPixel(indexPos + size2_t(1, 0));
    samples[2] = getPixel(indexPos + size2_t(0, 1));
    samples[3] = getPixel(indexPos + size2_t(1, 1));

    return Interpolation<ReturnType, double>::bilinear(samples, interpolants);
}

template <typename ReturnType, typename DataType>
ReturnType TemplateImageSampler<ReturnType, DataType>::getPixel(const size2_t& pos) const {
    auto p = glm::clamp(pos, size2_t(0), dims_ - size2_t(1));
    return data_[ic_(p)];
}

}  // namespace inviwo
