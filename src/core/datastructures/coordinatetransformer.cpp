/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/datastructures/spatialdata.h>

#include <ostream>

namespace inviwo {

template class IVW_CORE_TMPL_INST StructuredCoordinateTransformerImpl<2>;
template class IVW_CORE_TMPL_INST StructuredCoordinateTransformerImpl<3>;
template class IVW_CORE_TMPL_INST StructuredCameraCoordinateTransformerImpl<2>;
template class IVW_CORE_TMPL_INST StructuredCameraCoordinateTransformerImpl<3>;

std::string_view enumToStr(CoordinateSpace s) {
    switch (s) {
        case CoordinateSpace::Data:
            return "Data";
        case CoordinateSpace::Model:
            return "Model";
        case CoordinateSpace::World:
            return "World";
        case CoordinateSpace::Index:
            return "Index";
        case CoordinateSpace::Clip:
            return "Clip";
        case CoordinateSpace::View:
            return "View";
    }
    throw Exception(SourceContext{}, "Found invalid CoordinateSpace enum value '{}'",
                    static_cast<int>(s));
}

std::ostream& operator<<(std::ostream& ss, CoordinateSpace s) { return ss << enumToStr(s); }

glm::dvec3 SpatialCoordinateTransformer::transformPosition(const dvec3& pos, CoordinateSpace from,
                                                           CoordinateSpace to) const {
    const dvec4 result = getMatrix(from, to) * dvec4{pos, 1.0};
    return dvec3{result} / result.w;
}

glm::dvec4 SpatialCoordinateTransformer::transformPositionHomogeneous(const dvec4& pos,
                                                                      CoordinateSpace from,
                                                                      CoordinateSpace to) const {
    return getMatrix(from, to) * pos;
}

glm::dvec3 SpatialCoordinateTransformer::transformNormal(const dvec3& normal, CoordinateSpace from,
                                                         CoordinateSpace to) const {
    const dmat3 m{
        glm::transpose(glm::inverse(dmat3{SpatialCoordinateTransformer::getMatrix(from, to)}))};
    return m * normal;
}

glm::dmat3 SpatialCoordinateTransformer::getMetricTensor(dmat3 basis) {
    return {
        glm::dot(basis[0], basis[0]), glm::dot(basis[1], basis[0]), glm::dot(basis[2], basis[0]),
        glm::dot(basis[0], basis[1]), glm::dot(basis[1], basis[1]), glm::dot(basis[2], basis[1]),
        glm::dot(basis[0], basis[2]), glm::dot(basis[1], basis[2]), glm::dot(basis[2], basis[2]),
    };
}

glm::dmat3 SpatialCoordinateTransformer::getMetricTensor() const {
    return getMetricTensor(glm::dmat3(getDataToWorldMatrix()));
}

glm::dmat3 SpatialCoordinateTransformer::getInverseMetricTensor() const {
    return glm::inverse(getMetricTensor());
}

#include <warn/push>
#include <warn/ignore/switch-enum>
glm::dmat4 SpatialCoordinateTransformer::getMatrix(CoordinateSpace from, CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Data:
                    return {1.0};
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return {1.0};
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return {1.0};
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        default:
            throw Exception(SourceContext{}, "getMatrix is not available for the given space: {}",
                            from);
    }
}

glm::dmat4 StructuredCoordinateTransformer::getMatrix(CoordinateSpace from,
                                                      CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Index:
            switch (to) {
                case CoordinateSpace::Index:
                    return {1.0};
                case CoordinateSpace::Data:
                    return getIndexToDataMatrix();
                case CoordinateSpace::Model:
                    return getIndexToModelMatrix();
                case CoordinateSpace::World:
                    return getIndexToWorldMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Index, to);
            }
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Index:
                    return getDataToIndexMatrix();
                case CoordinateSpace::Data:
                    return {1.0};
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Index:
                    return getModelToIndexMatrix();
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return {1.0};
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Index:
                    return getWorldToIndexMatrix();
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return {1.0};
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        default:
            throw Exception(SourceContext{}, "getMatrix is not available for the given space: {}",
                            from);
    }
}

glm::dmat4 SpatialCameraCoordinateTransformer::getMatrix(CoordinateSpace from,
                                                         CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Data:
                    return {1.0};
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                case CoordinateSpace::View:
                    return getDataToViewMatrix();
                case CoordinateSpace::Clip:
                    return getDataToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return {1.0};
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                case CoordinateSpace::View:
                    return getModelToViewMatrix();
                case CoordinateSpace::Clip:
                    return getModelToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return {1.0};
                case CoordinateSpace::View:
                    return getWorldToViewMatrix();
                case CoordinateSpace::Clip:
                    return getWorldToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        case CoordinateSpace::View:
            switch (to) {
                case CoordinateSpace::Data:
                    return getViewToDataMatrix();
                case CoordinateSpace::Model:
                    return getViewToModelMatrix();
                case CoordinateSpace::World:
                    return getViewToWorldMatrix();
                case CoordinateSpace::View:
                    return {1.0};
                case CoordinateSpace::Clip:
                    return getViewToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::View, to);
            }
        case CoordinateSpace::Clip:
            switch (to) {
                case CoordinateSpace::Data:
                    return getClipToDataMatrix();
                case CoordinateSpace::Model:
                    return getClipToModelMatrix();
                case CoordinateSpace::World:
                    return getClipToWorldMatrix();
                case CoordinateSpace::View:
                    return getClipToViewMatrix();
                case CoordinateSpace::Clip:
                    return {1.0};
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Clip, to);
            }
        default:
            throw Exception(SourceContext{}, "getMatrix is not available for the given space: {}",
                            from);
    }
}

glm::dmat4 StructuredCameraCoordinateTransformer::getMatrix(CoordinateSpace from,
                                                            CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Index:
            switch (to) {
                case CoordinateSpace::Index:
                    return {1.0};
                case CoordinateSpace::Data:
                    return getIndexToDataMatrix();
                case CoordinateSpace::Model:
                    return getIndexToModelMatrix();
                case CoordinateSpace::World:
                    return getIndexToWorldMatrix();
                case CoordinateSpace::View:
                    return getIndexToViewMatrix();
                case CoordinateSpace::Clip:
                    return getIndexToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Index, to);
            }
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Index:
                    return getDataToIndexMatrix();
                case CoordinateSpace::Data:
                    return {1.0};
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                case CoordinateSpace::View:
                    return getDataToViewMatrix();
                case CoordinateSpace::Clip:
                    return getDataToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Index:
                    return getModelToIndexMatrix();
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return {1.0};
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                case CoordinateSpace::View:
                    return getModelToViewMatrix();
                case CoordinateSpace::Clip:
                    return getModelToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Model, to);
            }
        case CoordinateSpace::World:
            switch (to) {
                case CoordinateSpace::Index:
                    return getWorldToIndexMatrix();
                case CoordinateSpace::Data:
                    return getWorldToDataMatrix();
                case CoordinateSpace::Model:
                    return getWorldToModelMatrix();
                case CoordinateSpace::World:
                    return {1.0};
                case CoordinateSpace::View:
                    return getWorldToViewMatrix();
                case CoordinateSpace::Clip:
                    return getWorldToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        case CoordinateSpace::View:
            switch (to) {
                case CoordinateSpace::Index:
                    return getViewToIndexMatrix();
                case CoordinateSpace::Data:
                    return getViewToDataMatrix();
                case CoordinateSpace::Model:
                    return getViewToModelMatrix();
                case CoordinateSpace::World:
                    return getViewToWorldMatrix();
                case CoordinateSpace::View:
                    return {1.0};
                case CoordinateSpace::Clip:
                    return getViewToClipMatrix();
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::View, to);
            }
        case CoordinateSpace::Clip:
            switch (to) {
                case CoordinateSpace::Index:
                    return getClipToIndexMatrix();
                case CoordinateSpace::Data:
                    return getClipToDataMatrix();
                case CoordinateSpace::Model:
                    return getClipToModelMatrix();
                case CoordinateSpace::World:
                    return getClipToWorldMatrix();
                case CoordinateSpace::View:
                    return getClipToViewMatrix();
                case CoordinateSpace::Clip:
                    return {1.0};
                default:
                    throw Exception(SourceContext{},
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Clip, to);
            }
        default:
            throw Exception(SourceContext{}, "getMatrix is not available for the given space: {}",
                            from);
    }
}

#include <warn/pop>

/*
 * Implementations SpatialCoordinateTransformerImpl
 */
SpatialCoordinateTransformerImpl::SpatialCoordinateTransformerImpl(const SpatialEntity& entity)
    : SpatialCoordinateTransformer(), entity_{&entity} {}

SpatialCoordinateTransformerImpl* SpatialCoordinateTransformerImpl::clone() const {
    return new SpatialCoordinateTransformerImpl(*this);
}

glm::dmat4 SpatialCoordinateTransformerImpl::getModelMatrix() const {
    return entity_->getModelMatrix();
}

glm::dmat4 SpatialCoordinateTransformerImpl::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}

void SpatialCoordinateTransformerImpl::setEntity(const SpatialEntity& entity) { entity_ = &entity; }

glm::dmat4 SpatialCoordinateTransformerImpl::getDataToModelMatrix() const {
    return getModelMatrix();
}

glm::dmat4 SpatialCoordinateTransformerImpl::getDataToWorldMatrix() const {
    return getWorldMatrix() * getModelMatrix();
}

glm::dmat4 SpatialCoordinateTransformerImpl::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

glm::dmat4 SpatialCoordinateTransformerImpl::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

glm::dmat4 SpatialCoordinateTransformerImpl::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix() * getModelMatrix());
}

glm::dmat4 SpatialCoordinateTransformerImpl::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

/*
 * Implementations SpatialCameraCoordinateTransformerImpl
 */
SpatialCameraCoordinateTransformerImpl::SpatialCameraCoordinateTransformerImpl(
    const SpatialEntity& entity, const Camera& camera)
    : SpatialCameraCoordinateTransformer(), entity_{&entity}, camera_{&camera} {}

SpatialCameraCoordinateTransformerImpl* SpatialCameraCoordinateTransformerImpl::clone() const {
    return new SpatialCameraCoordinateTransformerImpl(*this);
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getModelMatrix() const {
    return entity_->getModelMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getViewMatrix() const {
    return camera_->getViewMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getProjectionMatrix() const {
    return camera_->getProjectionMatrix();
}

void SpatialCameraCoordinateTransformerImpl::setEntity(const SpatialEntity& entity) {
    entity_ = &entity;
}

void SpatialCameraCoordinateTransformerImpl::setCamera(const Camera& camera) { camera_ = &camera; }

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getClipToDataMatrix() const {
    return glm::inverse(getProjectionMatrix() * getViewMatrix() * getWorldMatrix() *
                        getModelMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getClipToModelMatrix() const {
    return glm::inverse(getProjectionMatrix() * getViewMatrix() * getWorldMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getClipToViewMatrix() const {
    return glm::inverse(getProjectionMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getClipToWorldMatrix() const {
    return glm::inverse(getProjectionMatrix() * getViewMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getDataToClipMatrix() const {
    return getProjectionMatrix() * getViewMatrix() * getWorldMatrix() * getModelMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getDataToModelMatrix() const {
    return getModelMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getDataToViewMatrix() const {
    return getViewMatrix() * getWorldMatrix() * getModelMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getDataToWorldMatrix() const {
    return getWorldMatrix() * getModelMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getModelToClipMatrix() const {
    return getProjectionMatrix() * getViewMatrix() * getWorldMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getModelToViewMatrix() const {
    return getViewMatrix() * getWorldMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getViewToClipMatrix() const {
    return getProjectionMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getViewToDataMatrix() const {
    return glm::inverse(getViewMatrix() * getWorldMatrix() * getModelMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getViewToModelMatrix() const {
    return glm::inverse(getViewMatrix() * getWorldMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getViewToWorldMatrix() const {
    return glm::inverse(getViewMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getWorldToClipMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix() * getModelMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

glm::dmat4 SpatialCameraCoordinateTransformerImpl::getWorldToViewMatrix() const {
    return getViewMatrix();
}

}  // namespace inviwo
