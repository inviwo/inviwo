/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>

namespace inviwo {

SpatialEntity::SpatialEntity() : SpatialEntity{mat4(1.0f), mat4(1.0f)} {}

SpatialEntity::SpatialEntity(const glm::mat4& modelMatrix)
    : SpatialEntity{modelMatrix, mat4(1.0f)} {}

SpatialEntity::SpatialEntity(const glm::mat4& modelMatrix, const glm::mat4& worldMatrix)
    : cameraTransformer_(nullptr), modelMatrix_(modelMatrix), worldMatrix_(worldMatrix) {}

SpatialEntity::SpatialEntity(const SpatialEntity& rhs)
    : modelMatrix_(rhs.modelMatrix_), worldMatrix_(rhs.worldMatrix_) {}

SpatialEntity& SpatialEntity::operator=(const SpatialEntity& that) {
    if (this != &that) {
        modelMatrix_ = that.modelMatrix_;
        worldMatrix_ = that.worldMatrix_;
    }
    return *this;
}

SpatialEntity::~SpatialEntity() = default;

glm::vec3 SpatialEntity::getOffset() const {
    glm::vec3 offset(0.0f);

    for (unsigned int i = 0; i < 3; i++) {
        offset[i] = modelMatrix_[3][i];
    }

    return offset;
}
void SpatialEntity::setOffset(const glm::vec3& offset) {
    for (unsigned int i = 0; i < 3; i++) {
        modelMatrix_[3][i] = offset[i];
    }
}

glm::mat3 SpatialEntity::getBasis() const {
    glm::mat3 basis(1.0f);

    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 3; j++) {
            basis[i][j] = modelMatrix_[i][j];
        }
    }
    return basis;
}

void SpatialEntity::setBasis(const glm::mat3& basis) {
    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 3; j++) {
            modelMatrix_[i][j] = basis[i][j];
        }
    }
}

glm::mat4 SpatialEntity::getModelMatrix() const { return modelMatrix_; }

void SpatialEntity::setModelMatrix(const glm::mat4& modelMatrix) { modelMatrix_ = modelMatrix; }

glm::mat4 SpatialEntity::getWorldMatrix() const { return worldMatrix_; }
void SpatialEntity::setWorldMatrix(const glm::mat4& worldMatrix) { worldMatrix_ = worldMatrix; }

const SpatialCoordinateTransformer& SpatialEntity::getCoordinateTransformer() const {
    if (!transformer_) {
        transformer_ = std::make_unique<SpatialCoordinateTransformerImpl>(*this);
    }
    return *transformer_;
}

const SpatialCameraCoordinateTransformer& SpatialEntity::getCoordinateTransformer(
    const Camera& camera) const {
    if (!cameraTransformer_) {
        cameraTransformer_ =
            std::make_unique<SpatialCameraCoordinateTransformerImpl>(*this, camera);
    }
    static_cast<SpatialCameraCoordinateTransformerImpl*>(cameraTransformer_.get())
        ->setCamera(camera);
    return *cameraTransformer_;
}

SpatialIdentity::SpatialIdentity() : SpatialEntity{mat4(1.0f), mat4(1.0f)} {}

SpatialIdentity::SpatialIdentity(const glm::mat4& modelMatrix)
    : SpatialIdentity{modelMatrix, mat4(1.0f)} {}

SpatialIdentity::SpatialIdentity(const glm::mat4& modelMatrix, const glm::mat4& worldMatrix)
    : SpatialEntity{modelMatrix, worldMatrix} {}

SpatialIdentity::SpatialIdentity(const SpatialIdentity& rhs) : SpatialEntity{rhs} {}

SpatialIdentity& SpatialIdentity::operator=(const SpatialIdentity& that) {
    if (this != &that) {
        modelMatrix_ = that.modelMatrix_;
        worldMatrix_ = that.worldMatrix_;
    }
    return *this;
}

SpatialIdentity* SpatialIdentity::clone() const { return new SpatialIdentity(*this); }

SpatialIdentity::~SpatialIdentity() = default;

template class IVW_CORE_TMPL_INST StructuredGridEntity<2>;
template class IVW_CORE_TMPL_INST StructuredGridEntity<3>;

}  // namespace inviwo
