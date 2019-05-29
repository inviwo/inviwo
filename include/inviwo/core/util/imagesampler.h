/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_IMAGESAMPLER_H
#define IVW_IMAGESAMPLER_H

#include <inviwo/core/common/inviwo.h>
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
 * \brief ImageSpatialSampler aids sampling of images using Bi-Linear Interpolation.
 *
 * A helper class to aid sampling of images. Expects the input to be on the range [0 1]. Eg., input
 * of (0,0) will return the color of the bottom left pixel and (1,1) will return then top right
 * pixel. Output values are bi-linear interpolated between the 4 nearest neighbors.
 */
template <unsigned int DataDims, typename T = double>
class ImageSpatialSampler : public SpatialSampler<2, DataDims, T> {
public:
    /**
     * Creates a ImageSpatialSampler for the given LayerRAM, does not take ownership of ram.
     * Use ImageSpatialSampler(std::shared_ptr<const Image>) to ensure that the LayerRAM is
     * available for the lifetime of the ImageSpatialSampler
     */
    ImageSpatialSampler(const LayerRAM *ram)
        : SpatialSampler<2, DataDims, T>(*ram->getOwner())
        , layer_(ram)
        , dims_(layer_->getDimensions())
        , sharedImage_(nullptr) {}

    /**
     * Creates a ImageSpatialSampler for the given Layer, does not take ownership of ram.
     * Use ImageSpatialSampler(std::shared_ptr<const Image>) to ensure that the Layer is available
     * for the lifetime of the ImageSpatialSampler
     */
    ImageSpatialSampler(const Layer *layer)
        : ImageSpatialSampler(layer->getRepresentation<LayerRAM>()) {}

    /**
     * Creates a ImageSpatialSampler for the given Image, does not take ownership of ram.
     * Use ImageSpatialSampler(std::shared_ptr<const Image>) to ensure that the Image is available
     * for the lifetime of the ImageSpatialSampler
     */
    ImageSpatialSampler(const Image *img) : ImageSpatialSampler(img->getColorLayer()) {}

    /**
     * Creates a ImageSpatialSampler for the given Image.
     * The shared_ptr will ensure that the Image is available for the lifetime of the
     * ImageSpatialSampler
     */
    ImageSpatialSampler(std::shared_ptr<const Image> sharedImage)
        : ImageSpatialSampler(sharedImage->getColorLayer()) {
        sharedImage_ = sharedImage;
    }

    virtual ~ImageSpatialSampler() {}

    using SpatialSampler<2, DataDims, T>::sample;

    /**
     * Samples the image at the given position using bi-linear interpolation.
     * @param x position to sample at
     * @param y position to sample at
     * @param space in what CoordinateSpace x and y is defined in
     */
    Vector<DataDims, T> sample(double x, double y, CoordinateSpace space) const {
        return SpatialSampler<2, DataDims, T>::sample(dvec2(x, y), space);
    }

    /**
     * @see sample(double, double, CoordinateSpace)
     * @param x X coordinate of the position to sample at
     * @param y Y coordinate of the position to sample at
     */
    Vector<DataDims, T> sample(double x, double y) const {
        return SpatialSampler<2, DataDims, T>::sample(dvec2(x, y));
    }

protected:
    virtual Vector<DataDims, T> sampleDataSpace(const dvec2 &pos) const {
        dvec2 samplePos = pos * dvec2(dims_ - size2_t(1));
        size2_t indexPos = size2_t(samplePos);
        dvec2 interpolants = samplePos - dvec2(indexPos);

        dvec4 samples[4];
        samples[0] = getPixel(indexPos);
        samples[1] = getPixel(indexPos + size2_t(1, 0));
        samples[2] = getPixel(indexPos + size2_t(0, 1));
        samples[3] = getPixel(indexPos + size2_t(1, 1));

        return Interpolation<dvec4>::bilinear(samples, interpolants);
    }

    virtual bool withinBoundsDataSpace(const dvec2 &pos) const {
        return !(glm::any(glm::lessThan(pos, dvec2(0.0))) ||
                 glm::any(glm::greaterThan(pos, dvec2(1.0))));
    }

private:
    dvec4 getPixel(const size2_t &pos) const {
        auto p = glm::clamp(pos, size2_t(0), dims_ - size2_t(1));
        return layer_->getAsDVec4(p);
    }
    const LayerRAM *layer_;
    size2_t dims_;

    std::shared_ptr<const Image> sharedImage_;
};

using ImageSampler = ImageSpatialSampler<4, double>;  // For backwards compatibility

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
template <typename T, typename P>
class TemplateImageSampler {
public:
    TemplateImageSampler(const LayerRAM *ram);
    TemplateImageSampler(const Layer *layer);

    TemplateImageSampler(const Image *img);
    TemplateImageSampler(std::shared_ptr<const Image> sharedImage);
    virtual ~TemplateImageSampler() = default;

    /**
     * Samples the image at the given position using bi-linear interpolation.
     * @param pos Position to sample at, expects range [0 1]
     */
    T sample(const Vector<2, P> &pos);

    /**
     * @see sample()
     * @param x X coordinate of the position to sample at, expects range [0 1]
     * @param y Y coordinate of the position to sample at, expects range [0 1]
     */
    T sample(P x, P y);

private:
    T getPixel(const size2_t &pos);
    const T *data_;
    size2_t dims_;
    util::IndexMapper2D ic_;

    std::shared_ptr<const Image> sharedImage_;
};

template <typename T, typename P>
TemplateImageSampler<T, P>::TemplateImageSampler(const LayerRAM *ram)
    : data_(static_cast<const T *>(ram->getData()))
    , dims_(ram->getDimensions())
    , ic_(dims_)
    , sharedImage_(nullptr) {
    if (ram->getDataFormat() != DataFormat<T>::get()) {
        std::ostringstream oss;
        oss << "Type mismatch when trying to initialize TemplateImageSampler. Image is "
            << ram->getDataFormat()->getString() << " but expected "
            << DataFormat<T>::get()->getString();
        throw Exception(oss.str(), IVW_CONTEXT);
    }
}

template <typename T, typename P>
TemplateImageSampler<T, P>::TemplateImageSampler(const Layer *layer)
    : TemplateImageSampler(layer->getRepresentation<LayerRAM>()) {}

template <typename T, typename P>
TemplateImageSampler<T, P>::TemplateImageSampler(const Image *img)
    : TemplateImageSampler(img->getColorLayer()) {}

template <typename T, typename P>
TemplateImageSampler<T, P>::TemplateImageSampler(std::shared_ptr<const Image> sharedImage)
    : TemplateImageSampler(sharedImage->getColorLayer()) {
    sharedImage_ = sharedImage;
}

template <typename T, typename P>
T TemplateImageSampler<T, P>::sample(P x, P y) {
    return sample(Vector<2, P>(x, y));
}

template <typename T, typename P>
T TemplateImageSampler<T, P>::getPixel(const size2_t &pos) {
    auto p = glm::clamp(pos, size2_t(0), dims_ - size2_t(1));
    return data_[ic_(p)];
}

template <typename T, typename P>
T TemplateImageSampler<T, P>::sample(const Vector<2, P> &pos) {
    Vector<2, P> samplePos = pos * Vector<2, P>(dims_ - size2_t(1));
    size2_t indexPos = size2_t(samplePos);
    Vector<2, P> interpolants = samplePos - Vector<2, P>(indexPos);

    T samples[4];
    samples[0] = getPixel(indexPos);
    samples[1] = getPixel(indexPos + size2_t(1, 0));

    samples[2] = getPixel(indexPos + size2_t(0, 1));
    samples[3] = getPixel(indexPos + size2_t(1, 1));

    return Interpolation<T, P>::bilinear(samples, interpolants);
}

}  // namespace inviwo

#endif  // IVW_IMAGESAMPLER_H
