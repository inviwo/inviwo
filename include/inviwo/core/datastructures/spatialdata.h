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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/datastructures/unitsystem.h>

namespace inviwo {

/**
 * \brief A convenience class to generate transformation matrices between
 *        the different coordinate systems in use.
 *
 *  Spatial meta data in Inviwo uses 4 different coordinate systems, they are defined as
 *  - Index - The voxel indices in the data
 *  - Data  - The corresponding texture coordinates of the data.
 *  - Model - Defines a local basis and offset for the data.
 *  - World - Puts the data at a position and angle in the scene.
 *
 *  A matrix is always stored in a 1 dim array, for example a 4x4 matrix would be:
 *  m = (1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)
 *  in c/c++, that uses row major, that translates to a matrix like
 *
 *      m[0][0]=1  m[0][1]=2  m[0][2]=3  m[0][3]=4
 *      m[1][0]=5  m[1][1]=6  m[1][2]=7  m[1][3]=8
 *      m[2][0]=9  m[2][1]=10 m[2][2]=11 m[2][3]=12
 *      m[3][0]=13 m[3][1]=14 m[3][2]=15 m[3][3]=16
 *
 *  here the first index represent the row and the second the column: m[row][column].
 *  On the gpu, that uses column major, the same array would look like:
 *
 *      m[0][0]=1  m[1][0]=5  m[2][0]=9  m[3][0]=13
 *      m[0][1]=2  m[1][1]=6  m[2][1]=10 m[3][1]=14
 *      m[0][2]=3  m[1][2]=7  m[2][2]=11 m[3][2]=15
 *      m[0][3]=4  m[1][3]=8  m[2][3]=12 m[3][3]=16
 *
 *  here the first index is the column and the second the row: m[column][row]
 *
 *  For example to create a translation matrix for on the gpu you want:
 *
 *      1  0  0 dx
 *      0  1  0 dy
 *      0  0  1 dz
 *      0  0  0  1
 *
 *  That means that in c/c++ you would create a transposed matrix like:
 *
 *      1  0  0  0
 *      0  1  0  0
 *      0  0  1  0
 *      dx dy dz 1
 *
 *  GLM also uses column major hence in glm you write m[column][row]
 *  hence you would enter the a translation like:
 *
 *      m[0][0]=1  m[1][0]=0  m[2][0]=0  m[3][0]=dx
 *      m[0][1]=0  m[1][1]=1  m[2][1]=0  m[3][1]=dy
 *      m[0][2]=0  m[1][2]=0  m[2][2]=1  m[3][2]=dz
 *      m[0][3]=0  m[1][3]=0  m[2][3]=0  m[3][3]=1
 *
 *  This means that they have the same representation as on the gpu.
 *
 *  ![](coordinate-spaces.png)
 */

class IVW_CORE_API SpatialEntity {
public:
    SpatialEntity();
    explicit SpatialEntity(const glm::mat4& modelMatrix);
    SpatialEntity(const glm::mat4& modelMatrix, const glm::mat4& worldMatrix);
    SpatialEntity(const SpatialEntity& rhs);
    SpatialEntity& operator=(const SpatialEntity& that);
    virtual SpatialEntity* clone() const = 0;
    virtual ~SpatialEntity();

    glm::vec3 getOffset() const;
    void setOffset(const glm::vec3& offset);

    // Using row vectors in basis
    glm::mat3 getBasis() const;
    void setBasis(const glm::mat3& basis);

    glm::mat4 getModelMatrix() const;
    void setModelMatrix(const glm::mat4& modelMatrix);

    glm::mat4 getWorldMatrix() const;
    void setWorldMatrix(const glm::mat4& worldMatrix);

    virtual const SpatialCoordinateTransformer& getCoordinateTransformer() const;
    virtual const SpatialCameraCoordinateTransformer& getCoordinateTransformer(
        const Camera& camera) const;

    /**
     * returns the axis information corresponding to \p index
     * @return nullptr if there is no axis for \p index
     */
    virtual const Axis* getAxis(size_t index) const = 0;

protected:
    mutable std::unique_ptr<SpatialCoordinateTransformer> transformer_;
    mutable std::unique_ptr<SpatialCameraCoordinateTransformer> cameraTransformer_;

    glm::mat4 modelMatrix_;
    glm::mat4 worldMatrix_;
};

class IVW_CORE_API SpatialIdentity : public SpatialEntity {
public:
    SpatialIdentity();
    explicit SpatialIdentity(const glm::mat4& modelMatrix);
    SpatialIdentity(const glm::mat4& modelMatrix, const glm::mat4& worldMatrix);
    SpatialIdentity(const SpatialIdentity& rhs);
    SpatialIdentity& operator=(const SpatialIdentity& that);
    virtual SpatialIdentity* clone() const override;
    virtual ~SpatialIdentity();

    virtual const Axis* getAxis([[maybe_unused]] size_t index) const override { return nullptr; }
};

template <unsigned int N>
class StructuredGridEntity : public SpatialEntity {
public:
    StructuredGridEntity() = default;
    StructuredGridEntity(const StructuredGridEntity<N>& rhs) = default;
    StructuredGridEntity(const glm::vec<N, size_t>& dimensions, const glm::vec<N, float>& spacing);
    explicit StructuredGridEntity(const mat4& modelMatrix);
    StructuredGridEntity(const mat4& modelMatrix, const mat4& worldMatrix);

    StructuredGridEntity<N>& operator=(const StructuredGridEntity<N>& that) = default;
    virtual StructuredGridEntity<N>* clone() const override = 0;

    virtual ~StructuredGridEntity() = default;

    virtual glm::vec<N, size_t> getDimensions() const = 0;

    /**
     * Returns the matrix transformation mapping from texture coordinates
     * to voxel index coordinates, i.e. from [0,1] to [-0.5, number of voxels-0.5]
     * @note Data is centered on the voxel, see OpenGL specifications, figure 8.3
     * http://www.opengl.org/registry/doc/glspec43.core.20120806.pdf
     * or for instance http://bpeers.com/articles/glpixel/
     * @see CoordinateTransformer::getTextureToIndexMatrix
     */
    glm::mat4 getIndexMatrix() const;

    virtual const StructuredCoordinateTransformer& getCoordinateTransformer() const override;
    virtual const StructuredCameraCoordinateTransformer& getCoordinateTransformer(
        const Camera& camera) const override;
};

/*
 *  Implementations StructuredGridEntity
 */
template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const glm::vec<N, size_t>& dimensions,
                                              const glm::vec<N, float>& spacing)
    : SpatialEntity() {
    glm::mat3 basis(0.0f);
    for (unsigned int i = 0; i < N; ++i) {
        basis[i][i] = dimensions[i] * spacing[i];
    }
    setBasis(basis);

    glm::vec3 offset(0.0f);
    for (unsigned int i = 0; i < N; ++i) {
        offset += basis[i];
    }
    setOffset(-0.5f * offset);
}

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const mat4& modelMatrix)
    : SpatialEntity(modelMatrix) {}

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const mat4& modelMatrix, const mat4& worldMatrix)
    : SpatialEntity(modelMatrix, worldMatrix) {}

template <unsigned int N>
glm::mat4 StructuredGridEntity<N>::getIndexMatrix() const {
    const auto dimensions = getDimensions();
    glm::mat4 indexMatrix(0.0f);
    for (unsigned int i = 0; i < N; ++i) {
        indexMatrix[i][i] = static_cast<float>(dimensions[i]);
    }
    indexMatrix[3][3] = 1.0f;

    // Offset to coordinates to center them in the middle of the texel/voxel.
    for (unsigned int i = 0; i < N; i++) {
        indexMatrix[N][i] = -0.5f;
    }
    return indexMatrix;
}

template <unsigned int N>
const StructuredCoordinateTransformer& StructuredGridEntity<N>::getCoordinateTransformer() const {
    if (!transformer_) {
        transformer_ = std::make_unique<StructuredCoordinateTransformerImpl<N>>(*this);
    }
    return *static_cast<StructuredCoordinateTransformer*>(transformer_.get());
}

template <unsigned int N>
const StructuredCameraCoordinateTransformer& StructuredGridEntity<N>::getCoordinateTransformer(
    const Camera& camera) const {
    if (!cameraTransformer_) {
        cameraTransformer_ =
            std::make_unique<StructuredCameraCoordinateTransformerImpl<N>>(*this, camera);
    }
    static_cast<StructuredCameraCoordinateTransformerImpl<N>*>(cameraTransformer_.get())
        ->setCamera(camera);
    return *static_cast<StructuredCameraCoordinateTransformer*>(cameraTransformer_.get());
}

extern template class IVW_CORE_TMPL_EXP StructuredGridEntity<2>;
extern template class IVW_CORE_TMPL_EXP StructuredGridEntity<3>;

}  // namespace inviwo
