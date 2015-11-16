/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/indexmapper.h>

#include <inviwo/core/util/interpolation.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/image.h>


namespace inviwo {

class Image;
class Layer;
class LayerRAM;

/**
 * \class VolumeSampler
 *
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 *
 * DESCRIBE_THE_CLASS
 */
class IVW_CORE_API ImageSampler {
public:
    ImageSampler(const LayerRAM *ram);
    ImageSampler(const Layer *layer);
    ImageSampler(const Image *img);
    ImageSampler(std::shared_ptr<const Image> img);
    virtual ~ImageSampler();

    dvec4 sample(const dvec2 &pos) const;
    dvec4 sample(double x, double y) const { return sample(dvec2(x, y)); }
    dvec4 sample(const vec2 &pos) const { return sample(dvec2(pos)); }

private:
    dvec4 getPixel(const size2_t &pos)const;
    const LayerRAM *layer_;
    size2_t dims_;

    std::shared_ptr<const Image> sharedImage_;
};

template <typename T,typename P>
class TemplateImageSampler {
public:
    TemplateImageSampler(const LayerRAM *ram)
        : data_(static_cast<const T *>(ram->getData())), dims_(ram->getDimensions()), ic_(dims_), sharedImage_(nullptr) {}
    TemplateImageSampler(const Layer *layer)
        : TemplateImageSampler(layer->getRepresentation<LayerRAM>()) {}

    TemplateImageSampler(const Image *img) : TemplateImageSampler(img->getColorLayer()) {}
    TemplateImageSampler(std::shared_ptr<const Image> sharedImage)
        : TemplateImageSampler(sharedImage->getColorLayer()) {
        sharedImage_ = sharedImage;
    }
    virtual ~TemplateImageSampler(){}

    T sample(P x,P y) { return sample(Vector<2, P>(x,y)); }
    T sample(const Vector<2, P> &pos) {
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

private:
    T getPixel(const size2_t &pos) {
        auto p = glm::clamp(pos, size2_t(0), dims_ - size2_t(1));
        return data_[ic_(p)];;

    }
    const T *data_;
    size2_t dims_;
    util::IndexMapper2D ic_;

    std::shared_ptr<const Image> sharedImage_;
};

}  // namespace

#endif  // IVW_VOLUMESAMPLER_H
