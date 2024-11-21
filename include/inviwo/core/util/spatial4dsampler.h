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

template <typename ReturnType>
class Spatial4DSampler : public Spatial4DSamplerBase {
public:
    static const unsigned SpatialDimensions = 4;
    using type = ReturnType;

    Spatial4DSampler(std::shared_ptr<const SpatialEntity> spatialEntity);
    virtual ~Spatial4DSampler() = default;

    virtual ReturnType sample(const dvec4& pos,
                              CoordinateSpace space = CoordinateSpace::Data) const;
    virtual ReturnType sample(const vec4& pos, CoordinateSpace space = CoordinateSpace::Data) const;

    virtual bool withinBounds(const dvec4& pos,
                              CoordinateSpace space = CoordinateSpace::Data) const;
    virtual bool withinBounds(const vec4& pos, CoordinateSpace space = CoordinateSpace::Data) const;

    const SpatialCoordinateTransformer& getCoordinateTransformer() const;
    mat4 getModelMatrix() const;
    mat4 getWorldMatrix() const;

protected:
    virtual ReturnType sampleDataSpace(const dvec4& pos) const = 0;
    virtual bool withinBoundsDataSpace(const dvec4& pos) const = 0;

    std::shared_ptr<const SpatialEntity> spatialEntity_;
};

extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<double>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<float>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<dvec2>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<vec2>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<dvec3>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<vec3>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<dvec4>;
extern template class IVW_CORE_TMPL_EXP Spatial4DSampler<vec4>;

template <typename ReturnType>
struct DataTraits<Spatial4DSampler<ReturnType>> {
    static constexpr std::string_view classIdentifier() {
        static constexpr auto cid = []() {
            constexpr auto tName = DataFormat<ReturnType>::str();
            return "org.inviwo.Spatial4DSampler." + StaticString<tName.size()>(tName);
        }();

        return cid;
    }
    static constexpr std::string_view dataName() {
        static constexpr auto name = []() {
            constexpr auto tName = DataFormat<ReturnType>::str();
            return "Spatial4DSampler<" + StaticString<tName.size()>(tName) + ">";
        }();
        return name;
    }
    static constexpr uvec3 colorCode() { return uvec3(153, 0, 76); }
    static Document info(const Spatial4DSampler<ReturnType>&) {
        Document doc;
        doc.append("p", dataName());
        return doc;
    }
};

template <typename ReturnType>
Spatial4DSampler<ReturnType>::Spatial4DSampler(std::shared_ptr<const SpatialEntity> spatialEntity)
    : spatialEntity_(spatialEntity) {}

template <typename ReturnType>
ReturnType Spatial4DSampler<ReturnType>::sample(const dvec4& pos, CoordinateSpace space) const {
    auto dataPos = dvec3(pos);
    if (space != CoordinateSpace::Data) {
        auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, CoordinateSpace::Data);
        auto p = m * vec4(static_cast<vec3>(pos), 1.0);
        dataPos = vec3(p) / p.w;
    }

    return sampleDataSpace(dvec4(dataPos, pos.w));
}

template <typename ReturnType>
ReturnType Spatial4DSampler<ReturnType>::sample(const vec4& pos, CoordinateSpace space) const {
    return sample(static_cast<dvec4>(pos), space);
}

template <typename ReturnType>
bool Spatial4DSampler<ReturnType>::withinBounds(const dvec4& pos, CoordinateSpace space) const {
    auto dataPos = dvec3(pos);
    if (space != CoordinateSpace::Data) {
        auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, CoordinateSpace::Data);
        auto p = m * vec4(static_cast<vec3>(dataPos), 1.0f);
        dataPos = vec3(p) / p.w;
    }

    return withinBoundsDataSpace(dvec4(dataPos, pos.w));
}

template <typename ReturnType>
bool Spatial4DSampler<ReturnType>::withinBounds(const vec4& pos, CoordinateSpace space) const {
    return withinBounds(static_cast<dvec4>(pos), space);
}

template <typename ReturnType>
const SpatialCoordinateTransformer& Spatial4DSampler<ReturnType>::getCoordinateTransformer() const {
    return spatialEntity_->getCoordinateTransformer();
}

template <typename ReturnType>
mat4 Spatial4DSampler<ReturnType>::getModelMatrix() const {
    return spatialEntity_->getModelMatrix();
}

template <typename ReturnType>
mat4 Spatial4DSampler<ReturnType>::getWorldMatrix() const {
    return spatialEntity_->getWorldMatrix();
}

}  // namespace inviwo
