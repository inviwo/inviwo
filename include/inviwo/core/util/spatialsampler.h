/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/datatraits.h>

namespace inviwo {

/**
 * \class SpatialSampler
 */
template <unsigned int DataDims, typename T>
class SpatialSampler {
public:
    static const unsigned SpatialDimensions = 3;
    static const unsigned DataDimensions = DataDims;
    using Space = CoordinateSpace;
    using ReturnType = glm::vec<DataDims, T>;

    SpatialSampler(const SpatialEntity& spatialEntity, Space space = Space::Data);
    virtual ~SpatialSampler() = default;

    Vector<DataDims, T> sample(const dvec3& pos) const;
    Vector<DataDims, T> sample(const vec3& pos) const;
    Vector<DataDims, T> sample(const dvec2& pos) const;
    Vector<DataDims, T> sample(const vec2& pos) const;

    Vector<DataDims, T> sample(const dvec3& pos, Space space) const;
    Vector<DataDims, T> sample(const vec3& pos, Space space) const;
    Vector<DataDims, T> sample(const dvec2& pos, Space space) const;
    Vector<DataDims, T> sample(const vec2& pos, Space space) const;

    bool withinBounds(const dvec3& pos) const;
    bool withinBounds(const vec3& pos) const;
    bool withinBounds(const dvec2& pos) const;
    bool withinBounds(const vec2& pos) const;

    bool withinBounds(const dvec3& pos, Space space) const;
    bool withinBounds(const vec3& pos, Space space) const;
    bool withinBounds(const dvec2& pos, Space space) const;
    bool withinBounds(const vec2& pos, Space space) const;

    mat3 getBasis() const;
    mat4 getModelMatrix() const;
    mat4 getWorldMatrix() const;

    const SpatialCoordinateTransformer& getCoordinateTransformer() const;

protected:
    virtual Vector<DataDims, T> sampleDataSpace(const dvec3& pos) const = 0;
    virtual bool withinBoundsDataSpace(const dvec3& pos) const = 0;

    Space space_;
    const SpatialEntity& spatialEntity_;
    dmat4 transform_;
};

template <unsigned int DataDims, typename T>
struct DataTraits<SpatialSampler<DataDims, T>> {
    static std::string classIdentifier() {
        return fmt::format("org.inviwo.SpatialSampler.3D.{}",
                           DataFormat<Vector<DataDims, T>>::str());
    }
    static std::string dataName() {
        return fmt::format("SpatialSampler<3D{}>", DataFormat<Vector<DataDims, T>>::str());
    }
    static uvec3 colorCode() { return uvec3(153, 0, 76); }
    static Document info(const SpatialSampler<DataDims, T>&) {
        Document doc;
        doc.append("p", dataName());
        return doc;
    }
};

template <unsigned int DataDims, typename T>
SpatialSampler<DataDims, T>::SpatialSampler(const SpatialEntity& spatialEntity, Space space)
    : space_(space)
    , spatialEntity_(spatialEntity)
    , transform_{spatialEntity_.getCoordinateTransformer().getMatrix(space, Space::Data)} {}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const vec3& pos) const {
    return sample(static_cast<dvec3>(pos));
}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const vec2& pos) const {
    return sample(dvec3{pos, 0.0});
}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const dvec2& pos) const {
    return sample(dvec3{pos, 0.0});
}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const dvec3& pos) const {
    if (space_ != Space::Data) {
        const auto p = transform_ * dvec4(pos, 1.0);
        return sampleDataSpace(dvec3(p) / p.w);
    } else {
        return sampleDataSpace(pos);
    }
}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const vec3& pos, Space space) const {
    return sample(static_cast<dvec3>(pos), space);
}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const vec2& pos, Space space) const {
    return sample(dvec3{pos, 0.0}, space);
}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const dvec2& pos, Space space) const {
    return sample(dvec3{pos, 0.0}, space);
}

template <unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<DataDims, T>::sample(const dvec3& pos, Space space) const {
    if (space != Space::Data) {
        const dmat4 m{spatialEntity_.getCoordinateTransformer().getMatrix(space, Space::Data)};
        const auto p = m * dvec4(pos, 1.0);
        return sampleDataSpace(dvec3(p) / p.w);
    } else {
        return sampleDataSpace(pos);
    }
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const vec3& pos) const {
    return withinBounds(static_cast<dvec3>(pos));
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const vec2& pos) const {
    return withinBounds(dvec3{pos, 0.0});
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const dvec2& pos) const {
    return withinBounds(dvec3{pos, 0.0});
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const dvec3& pos) const {
    if (space_ != Space::Data) {
        const auto p = transform_ * dvec4(pos, 1.0);
        return withinBoundsDataSpace(dvec3(p) / p.w);
    } else {
        return withinBoundsDataSpace(pos);
    }
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const vec3& pos, Space space) const {
    return withinBounds(static_cast<dvec3>(pos), space);
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const vec2& pos, Space space) const {
    return withinBounds(dvec3{pos, 0.0}, space);
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const dvec2& pos, Space space) const {
    return withinBounds(dvec3{pos, 0.0}, space);
}

template <unsigned int DataDims, typename T>
bool SpatialSampler<DataDims, T>::withinBounds(const dvec3& pos, Space space) const {
    if (space != Space::Data) {
        const dmat4 m{spatialEntity_.getCoordinateTransformer().getMatrix(space, Space::Data)};
        const auto p = m * dvec4(pos, 1.0);
        return withinBoundsDataSpace(dvec3(p) / p.w);
    } else {
        return withinBoundsDataSpace(pos);
    }
}

template <unsigned int DataDims, typename T>
const SpatialCoordinateTransformer& SpatialSampler<DataDims, T>::getCoordinateTransformer() const {
    return spatialEntity_.getCoordinateTransformer();
}

template <unsigned int DataDims, typename T>
mat4 SpatialSampler<DataDims, T>::getWorldMatrix() const {
    return spatialEntity_.getWorldMatrix();
}

template <unsigned int DataDims, typename T>
mat4 SpatialSampler<DataDims, T>::getModelMatrix() const {
    return spatialEntity_.getModelMatrix();
}

template <unsigned int DataDims, typename T>
mat3 SpatialSampler<DataDims, T>::getBasis() const {
    return spatialEntity_.getBasis();
}

}  // namespace inviwo
