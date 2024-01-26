/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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
template <typename ReturnType>
class SpatialSampler {
public:
    static const unsigned SpatialDimensions = 3;
    using type = ReturnType;
    SpatialSampler(const SpatialEntity& spatialEntity,
                   CoordinateSpace space = CoordinateSpace::Data);
    virtual ~SpatialSampler() = default;
    

    ReturnType sample(const dvec3& pos) const;
    ReturnType sample(const vec3& pos) const;
    ReturnType sample(const dvec2& pos) const;
    ReturnType sample(const vec2& pos) const;

    ReturnType sample(const dvec3& pos, CoordinateSpace space) const;
    ReturnType sample(const vec3& pos, CoordinateSpace space) const;
    ReturnType sample(const dvec2& pos, CoordinateSpace space) const;
    ReturnType sample(const vec2& pos, CoordinateSpace space) const;

    bool withinBounds(const dvec3& pos) const;
    bool withinBounds(const vec3& pos) const;
    bool withinBounds(const dvec2& pos) const;
    bool withinBounds(const vec2& pos) const;

    bool withinBounds(const dvec3& pos, CoordinateSpace space) const;
    bool withinBounds(const vec3& pos, CoordinateSpace space) const;
    bool withinBounds(const dvec2& pos, CoordinateSpace space) const;
    bool withinBounds(const vec2& pos, CoordinateSpace space) const;

    mat3 getBasis() const;
    mat4 getModelMatrix() const;
    mat4 getWorldMatrix() const;

    const SpatialCoordinateTransformer& getCoordinateTransformer() const;

protected:
    virtual ReturnType sampleDataSpace(const dvec3& pos) const = 0;
    virtual bool withinBoundsDataSpace(const dvec3& pos) const = 0;

    CoordinateSpace space_;
    const SpatialEntity& spatialEntity_;
    dmat4 transform_;
};

template <typename ReturnType>
struct DataTraits<SpatialSampler<ReturnType>> {
    static std::string classIdentifier() {
        return fmt::format("org.inviwo.SpatialSampler.3D.{}", DataFormat<ReturnType>::str());
    }
    static std::string dataName() {
        return fmt::format("SpatialSampler<{}>", DataFormat<ReturnType>::str());
    }
    static uvec3 colorCode() { return uvec3(153, 0, 76); }
    static Document info(const SpatialSampler<ReturnType>&) {
        Document doc;
        doc.append("p", dataName());
        return doc;
    }
};

template <typename ReturnType>
SpatialSampler<ReturnType>::SpatialSampler(const SpatialEntity& spatialEntity,
                                           CoordinateSpace space)
    : space_(space)
    , spatialEntity_(spatialEntity)
    , transform_{
          spatialEntity_.getCoordinateTransformer().getMatrix(space, CoordinateSpace::Data)} {}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const vec3& pos) const -> ReturnType {
    return sample(static_cast<dvec3>(pos));
}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const vec2& pos) const -> ReturnType {
    return sample(dvec3{pos, 0.0});
}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const dvec2& pos) const -> ReturnType {
    return sample(dvec3{pos, 0.0});
}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const dvec3& pos) const -> ReturnType {
    if (space_ != CoordinateSpace::Data) {
        const auto p = transform_ * dvec4(pos, 1.0);
        return sampleDataSpace(dvec3(p) / p.w);
    } else {
        return sampleDataSpace(pos);
    }
}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const vec3& pos, CoordinateSpace space) const
    -> ReturnType {
    return sample(static_cast<dvec3>(pos), space);
}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const vec2& pos, CoordinateSpace space) const
    -> ReturnType {
    return sample(dvec3{pos, 0.0}, space);
}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const dvec2& pos, CoordinateSpace space) const
    -> ReturnType {
    return sample(dvec3{pos, 0.0}, space);
}

template <typename ReturnType>
auto SpatialSampler<ReturnType>::sample(const dvec3& pos, CoordinateSpace space) const
    -> ReturnType {
    if (space != CoordinateSpace::Data) {
        const dmat4 m{
            spatialEntity_.getCoordinateTransformer().getMatrix(space, CoordinateSpace::Data)};
        const auto p = m * dvec4(pos, 1.0);
        return sampleDataSpace(dvec3(p) / p.w);
    } else {
        return sampleDataSpace(pos);
    }
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const vec3& pos) const {
    return withinBounds(static_cast<dvec3>(pos));
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const vec2& pos) const {
    return withinBounds(dvec3{pos, 0.0});
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const dvec2& pos) const {
    return withinBounds(dvec3{pos, 0.0});
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const dvec3& pos) const {
    if (space_ != CoordinateSpace::Data) {
        const auto p = transform_ * dvec4(pos, 1.0);
        return withinBoundsDataSpace(dvec3(p) / p.w);
    } else {
        return withinBoundsDataSpace(pos);
    }
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const vec3& pos, CoordinateSpace space) const {
    return withinBounds(static_cast<dvec3>(pos), space);
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const vec2& pos, CoordinateSpace space) const {
    return withinBounds(dvec3{pos, 0.0}, space);
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const dvec2& pos, CoordinateSpace space) const {
    return withinBounds(dvec3{pos, 0.0}, space);
}

template <typename ReturnType>
bool SpatialSampler<ReturnType>::withinBounds(const dvec3& pos, CoordinateSpace space) const {
    if (space != CoordinateSpace::Data) {
        const dmat4 m{
            spatialEntity_.getCoordinateTransformer().getMatrix(space, CoordinateSpace::Data)};
        const auto p = m * dvec4(pos, 1.0);
        return withinBoundsDataSpace(dvec3(p) / p.w);
    } else {
        return withinBoundsDataSpace(pos);
    }
}

template <typename ReturnType>
const SpatialCoordinateTransformer& SpatialSampler<ReturnType>::getCoordinateTransformer() const {
    return spatialEntity_.getCoordinateTransformer();
}

template <typename ReturnType>
mat4 SpatialSampler<ReturnType>::getWorldMatrix() const {
    return spatialEntity_.getWorldMatrix();
}

template <typename ReturnType>
mat4 SpatialSampler<ReturnType>::getModelMatrix() const {
    return spatialEntity_.getModelMatrix();
}

template <typename ReturnType>
mat3 SpatialSampler<ReturnType>::getBasis() const {
    return spatialEntity_.getBasis();
}

}  // namespace inviwo
