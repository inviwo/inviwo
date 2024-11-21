/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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
    throw Exception(IVW_CONTEXT_CUSTOM("enumToStr"), "Found invalid CoordinateSpace enum value '{}'",
                    static_cast<int>(s));
}

std::ostream& operator<<(std::ostream& ss, CoordinateSpace s) { return ss << enumToStr(s); }

glm::vec3 SpatialCoordinateTransformer::transformPosition(const vec3& pos, CoordinateSpace from,
                                                          CoordinateSpace to) const {
    const vec4 result = getMatrix(from, to) * vec4{pos, 1.0f};
    return vec3{result} / result.w;
}

glm::vec4 SpatialCoordinateTransformer::transformPositionHomogeneous(const vec4& pos,
                                                                     CoordinateSpace from,
                                                                     CoordinateSpace to) const {
    return getMatrix(from, to) * pos;
}

glm::vec3 SpatialCoordinateTransformer::transformNormal(const vec3& normal, CoordinateSpace from,
                                                        CoordinateSpace to) const {
    const mat3 m{
        glm::transpose(glm::inverse(mat3{SpatialCoordinateTransformer::getMatrix(from, to)}))};
    return m * normal;
}

#include <warn/push>
#include <warn/ignore/switch-enum>
glm::mat4 SpatialCoordinateTransformer::getMatrix(CoordinateSpace from, CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Data:
                    return glm::mat4(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return glm::mat4(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                default:
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}",
                            from);
    }
}

glm::mat4 StructuredCoordinateTransformer::getMatrix(CoordinateSpace from,
                                                     CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Index:
            switch (to) {
                case CoordinateSpace::Index:
                    return glm::mat4(1.0f);
                case CoordinateSpace::Data:
                    return getIndexToDataMatrix();
                case CoordinateSpace::Model:
                    return getIndexToModelMatrix();
                case CoordinateSpace::World:
                    return getIndexToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Index, to);
            }
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Index:
                    return getDataToIndexMatrix();
                case CoordinateSpace::Data:
                    return glm::mat4(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                default:
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::World, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}",
                            from);
    }
}

glm::mat4 SpatialCameraCoordinateTransformer::getMatrix(CoordinateSpace from,
                                                        CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Data:
                    return glm::mat4(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                case CoordinateSpace::View:
                    return getDataToViewMatrix();
                case CoordinateSpace::Clip:
                    return getDataToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Data, to);
            }
        case CoordinateSpace::Model:
            switch (to) {
                case CoordinateSpace::Data:
                    return getModelToDataMatrix();
                case CoordinateSpace::Model:
                    return glm::mat4(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                case CoordinateSpace::View:
                    return getModelToViewMatrix();
                case CoordinateSpace::Clip:
                    return getModelToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                case CoordinateSpace::View:
                    return getWorldToViewMatrix();
                case CoordinateSpace::Clip:
                    return getWorldToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                case CoordinateSpace::Clip:
                    return getViewToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                default:
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Clip, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}",
                            from);
    }
}

glm::mat4 StructuredCameraCoordinateTransformer::getMatrix(CoordinateSpace from,
                                                           CoordinateSpace to) const {
    switch (from) {
        case CoordinateSpace::Index:
            switch (to) {
                case CoordinateSpace::Index:
                    return glm::mat4(1.0f);
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
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Index, to);
            }
        case CoordinateSpace::Data:
            switch (to) {
                case CoordinateSpace::Index:
                    return getDataToIndexMatrix();
                case CoordinateSpace::Data:
                    return glm::mat4(1.0f);
                case CoordinateSpace::Model:
                    return getDataToModelMatrix();
                case CoordinateSpace::World:
                    return getDataToWorldMatrix();
                case CoordinateSpace::View:
                    return getDataToViewMatrix();
                case CoordinateSpace::Clip:
                    return getDataToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                case CoordinateSpace::World:
                    return getModelToWorldMatrix();
                case CoordinateSpace::View:
                    return getModelToViewMatrix();
                case CoordinateSpace::Clip:
                    return getModelToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                case CoordinateSpace::View:
                    return getWorldToViewMatrix();
                case CoordinateSpace::Clip:
                    return getWorldToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                case CoordinateSpace::Clip:
                    return getViewToClipMatrix();
                default:
                    throw Exception(IVW_CONTEXT,
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
                    return glm::mat4(1.0f);
                default:
                    throw Exception(IVW_CONTEXT,
                                    "getMatrix is not available for the given spaces: {} to {}",
                                    CoordinateSpace::Clip, to);
            }
        default:
            throw Exception(IVW_CONTEXT, "getMatrix is not available for the given space: {}",
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

glm::mat4 SpatialCoordinateTransformerImpl::getModelMatrix() const {
    return entity_->getModelMatrix();
}

glm::mat4 SpatialCoordinateTransformerImpl::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}

void SpatialCoordinateTransformerImpl::setEntity(const SpatialEntity& entity) { entity_ = &entity; }

glm::mat4 SpatialCoordinateTransformerImpl::getDataToModelMatrix() const {
    return getModelMatrix();
}

glm::mat4 SpatialCoordinateTransformerImpl::getDataToWorldMatrix() const {
    return getWorldMatrix() * getModelMatrix();
}

glm::mat4 SpatialCoordinateTransformerImpl::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

glm::mat4 SpatialCoordinateTransformerImpl::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

glm::mat4 SpatialCoordinateTransformerImpl::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix() * getModelMatrix());
}

glm::mat4 SpatialCoordinateTransformerImpl::getWorldToModelMatrix() const {
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

glm::mat4 SpatialCameraCoordinateTransformerImpl::getModelMatrix() const {
    return entity_->getModelMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getWorldMatrix() const {
    return entity_->getWorldMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getViewMatrix() const {
    return camera_->getViewMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getProjectionMatrix() const {
    return camera_->getProjectionMatrix();
}

void SpatialCameraCoordinateTransformerImpl::setEntity(const SpatialEntity& entity) {
    entity_ = &entity;
}

void SpatialCameraCoordinateTransformerImpl::setCamera(const Camera& camera) { camera_ = &camera; }

glm::mat4 SpatialCameraCoordinateTransformerImpl::getClipToDataMatrix() const {
    return glm::inverse(getProjectionMatrix() * getViewMatrix() * getWorldMatrix() *
                        getModelMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getClipToModelMatrix() const {
    return glm::inverse(getProjectionMatrix() * getViewMatrix() * getWorldMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getClipToViewMatrix() const {
    return glm::inverse(getProjectionMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getClipToWorldMatrix() const {
    return glm::inverse(getProjectionMatrix() * getViewMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getDataToClipMatrix() const {
    return getProjectionMatrix() * getViewMatrix() * getWorldMatrix() * getModelMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getDataToModelMatrix() const {
    return getModelMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getDataToViewMatrix() const {
    return getViewMatrix() * getWorldMatrix() * getModelMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getDataToWorldMatrix() const {
    return getWorldMatrix() * getModelMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getModelToClipMatrix() const {
    return getProjectionMatrix() * getViewMatrix() * getWorldMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getModelToDataMatrix() const {
    return glm::inverse(getModelMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getModelToViewMatrix() const {
    return getViewMatrix() * getWorldMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getModelToWorldMatrix() const {
    return getWorldMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getViewToClipMatrix() const {
    return getProjectionMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getViewToDataMatrix() const {
    return glm::inverse(getViewMatrix() * getWorldMatrix() * getModelMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getViewToModelMatrix() const {
    return glm::inverse(getViewMatrix() * getWorldMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getViewToWorldMatrix() const {
    return glm::inverse(getViewMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getWorldToClipMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getWorldToDataMatrix() const {
    return glm::inverse(getWorldMatrix() * getModelMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getWorldToModelMatrix() const {
    return glm::inverse(getWorldMatrix());
}

glm::mat4 SpatialCameraCoordinateTransformerImpl::getWorldToViewMatrix() const {
    return getViewMatrix();
}

}  // namespace inviwo
