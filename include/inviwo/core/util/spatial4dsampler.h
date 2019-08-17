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

#ifndef IVW_SPATIAL4DSAMPLER_H
#define IVW_SPATIAL4DSAMPLER_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/datastructures/datatraits.h>

namespace inviwo {

class IVW_CORE_API Spatial4DSamplerBase {
public:
    Spatial4DSamplerBase() = default;
    virtual ~Spatial4DSamplerBase() = default;
};

template <unsigned DataDims, typename T>
class Spatial4DSampler : public Spatial4DSamplerBase {
public:
    static const unsigned SpatialDimensions = 4;
    static const unsigned DataDimensions = DataDims;
    using Space = CoordinateSpace;
    using ReturnType = Vector<DataDims, T>;

    Spatial4DSampler(std::shared_ptr<const SpatialEntity<3>> spatialEntity);
    virtual ~Spatial4DSampler() = default;

    virtual Vector<DataDims, T> sample(const dvec4 &pos, Space space = Space::Data) const;
    virtual Vector<DataDims, T> sample(const vec4 &pos, Space space = Space::Data) const;

    virtual bool withinBounds(const dvec4 &pos, Space space = Space::Data) const;
    virtual bool withinBounds(const vec4 &pos, Space space = Space::Data) const;

    const SpatialCoordinateTransformer<3> &getCoordinateTransformer() const;
    mat4 getModelMatrix() const;
    mat4 getWorldMatrix() const;

protected:
    virtual Vector<DataDims, T> sampleDataSpace(const dvec4 &pos) const = 0;
    virtual bool withinBoundsDataSpace(const dvec4 &pos) const = 0;

    std::shared_ptr<const SpatialEntity<3>> spatialEntity_;
};

extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<1, double>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<1, float>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<2, double>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<2, float>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<3, double>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<3, float>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<4, double>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<4, float>;

template <unsigned DataDims, typename T>
struct DataTraits<Spatial4DSampler<DataDims, T>> {
    static std::string classIdentifier() {
        return "org.inviwo.Spatial4DSampler." + DataFormat<Vector<DataDims, T>>::str();
    }
    static std::string dataName() {
        return "Spatial4DSampler<" + DataFormat<Vector<DataDims, T>>::str() + ">";
    }
    static uvec3 colorCode() { return uvec3(153, 0, 76); }
    static Document info(const Spatial4DSampler<DataDims, T> &) {
        Document doc;
        doc.append("p", dataName());
        return doc;
    }
};

template <unsigned DataDims, typename T>
Spatial4DSampler<DataDims, T>::Spatial4DSampler(
    std::shared_ptr<const SpatialEntity<3>> spatialEntity)
    : spatialEntity_(spatialEntity) {}

template <unsigned DataDims, typename T>
Vector<DataDims, T> Spatial4DSampler<DataDims, T>::sample(const dvec4 &pos, Space space) const {
    auto dataPos = dvec3(pos);
    if (space != Space::Data) {
        auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, Space::Data);
        auto p = m * vec4(static_cast<vec3>(pos), 1.0);
        dataPos = vec3(p) / p.w;
    }

    return sampleDataSpace(dvec4(dataPos, pos.w));
}

template <unsigned DataDims, typename T>
Vector<DataDims, T> Spatial4DSampler<DataDims, T>::sample(const vec4 &pos, Space space) const {
    return sample(static_cast<dvec4>(pos), space);
}

template <unsigned DataDims, typename T>
bool Spatial4DSampler<DataDims, T>::withinBounds(const dvec4 &pos, Space space) const {
    auto dataPos = dvec3(pos);
    if (space != Space::Data) {
        auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, Space::Data);
        auto p = m * vec4(static_cast<vec3>(dataPos), 1.0f);
        dataPos = vec3(p) / p.w;
    }

    return withinBoundsDataSpace(dvec4(dataPos, pos.w));
}

template <unsigned DataDims, typename T>
bool Spatial4DSampler<DataDims, T>::withinBounds(const vec4 &pos, Space space) const {
    return withinBounds(static_cast<dvec4>(pos), space);
}

template <unsigned DataDims, typename T>
const SpatialCoordinateTransformer<3> &Spatial4DSampler<DataDims, T>::getCoordinateTransformer()
    const {
    return spatialEntity_->getCoordinateTransformer();
}

template <unsigned DataDims, typename T>
mat4 Spatial4DSampler<DataDims, T>::getModelMatrix() const {
    return spatialEntity_->getModelMatrix();
}

template <unsigned DataDims, typename T>
mat4 Spatial4DSampler<DataDims, T>::getWorldMatrix() const {
    return spatialEntity_->getWorldMatrix();
}

}  // namespace inviwo

#endif  // IVW_SPATIALSAMPLER_H
