/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_SPATIALDATA_H
#define IVW_SPATIALDATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>

namespace inviwo {

/** \brief A convenience class to generate transformation matrices between
 *         the different coordinate systems in use.
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
 *      0  0  0 dz
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
 */

template <unsigned int N>
class SpatialEntity {
public:
    SpatialEntity();
    SpatialEntity(const SpatialEntity<N>& rhs);
    SpatialEntity(const Matrix<N + 1, float>& modelMatrix);
    SpatialEntity(const Matrix<N + 1, float>& modelMatrix, const Matrix<N + 1, float>& worldMatrix);

    SpatialEntity<N>& operator=(const SpatialEntity<N>& that);
    virtual SpatialEntity<N>* clone() const = 0;
    virtual ~SpatialEntity();

    Vector<N, float> getOffset() const;
    void setOffset(const Vector<N, float>& offset);

    // Using row vectors in basis
    Matrix<N, float> getBasis() const;
    void setBasis(const Matrix<N, float>& basis);

    Matrix<N + 1, float> getModelMatrix() const;
    void setModelMatrix(const Matrix<N + 1, float>& modelMatrix);

    Matrix<N + 1, float> getWorldMatrix() const;
    void setWorldMatrix(const Matrix<N + 1, float>& worldMatrix);

    virtual const SpatialCoordinateTransformer<N>& getCoordinateTransformer() const;
    virtual const SpatialCameraCoordinateTransformer<N>& getCoordinateTransformer(Camera<N> camera) const;

protected:
    mutable SpatialCoordinateTransformer<N>* transformer_;
    mutable SpatialCameraCoordinateTransformer<N>* cameraTransformer_;

    Matrix<N + 1, float> modelMatrix_;
    Matrix<N + 1, float> worldMatrix_;
};

extern template class SpatialEntity<2>;
extern template class SpatialEntity<3>;

template <unsigned int N>
class StructuredGridEntity : public SpatialEntity<N> {
public:
    StructuredGridEntity();
    StructuredGridEntity(const StructuredGridEntity<N>& rhs);
    StructuredGridEntity(const Vector<N, unsigned int>& dimensions);
    StructuredGridEntity(const Vector<N, unsigned int>& dimensions, const Vector<N, float>& spacing);
    StructuredGridEntity(const Vector<N, unsigned int>& dimensions,
                         const Matrix<N + 1, float>& modelMatrix);
    StructuredGridEntity(const Vector<N, unsigned int>& dimensions,
                         const Matrix<N + 1, float>& modelMatrix,
                         const Matrix<N + 1, float>& worldMatrix);

    StructuredGridEntity<N>& operator=(const StructuredGridEntity<N>& that);
    virtual StructuredGridEntity<N>* clone() const = 0;

    virtual ~StructuredGridEntity() {}

    virtual Vector<N, unsigned int> getDimensions() const;
    virtual void setDimensions(const Vector<N, unsigned int>& dimensions);

    
    /**
     * Returns the matrix transformation mapping from texture coordinates
     * to voxel index coordinates, i.e. from [0,1] to [-0.5, number of voxels-0.5]
     * @note Data is centered on the voxel, see OpenGL specifications, figure 8.3 
     * http://www.opengl.org/registry/doc/glspec43.core.20120806.pdf 
     * or for instance http://bpeers.com/articles/glpixel/
     * @see CoordinateTransformer::getTextureToIndexMatrix
     */
    Matrix<N + 1, float> getIndexMatrix() const;

    virtual const StructuredCoordinateTransformer<N>& getCoordinateTransformer() const;
    virtual const StructuredCameraCoordinateTransformer<N>& getCoordinateTransformer(Camera<N> camera) const;

protected:
    Vector<N, unsigned int> dimensions_;
};


/*---------------------------------------------------------------*
 *  Implementations                                              *
 *  SpatialEntity                                                *
 *---------------------------------------------------------------*/

template <unsigned int N>
SpatialEntity<N>::SpatialEntity()
    : transformer_(nullptr)
    , cameraTransformer_(nullptr)
    , modelMatrix_(1.0f)
    , worldMatrix_(1.0f) {}

template <unsigned int N>
SpatialEntity<N>::SpatialEntity(const SpatialEntity<N>& rhs)
    : transformer_(nullptr)
    , cameraTransformer_(nullptr)
    , modelMatrix_(rhs.modelMatrix_)
    , worldMatrix_(rhs.worldMatrix_) {}

template <unsigned int N>
SpatialEntity<N>::SpatialEntity(const Matrix<N + 1, float>& modelMatrix)
    : transformer_(nullptr)
    , cameraTransformer_(nullptr)
    , modelMatrix_(modelMatrix)
    , worldMatrix_(1.0f) {}

template <unsigned int N>
SpatialEntity<N>::SpatialEntity(const Matrix<N + 1, float>& modelMatrix,
                                const Matrix<N + 1, float>& worldMatrix)
    : transformer_(nullptr)
    , cameraTransformer_(nullptr)
    , modelMatrix_(modelMatrix)
    , worldMatrix_(worldMatrix) {}

template <unsigned int N>
SpatialEntity<N>& SpatialEntity<N>::operator=(const SpatialEntity<N>& that) {
    if (this != &that) {
        modelMatrix_ = that.modelMatrix_;
        worldMatrix_ = that.worldMatrix_;
    }
    return *this;
}

template <unsigned int N>
SpatialEntity<N>::~SpatialEntity() {
    delete transformer_;
    delete cameraTransformer_;
}

template <unsigned int N>
Vector<N, float> SpatialEntity<N>::getOffset() const {
    Vector<N, float> offset(0.0f);

    for (int i = 0; i < N; i++) {
        offset[i] = modelMatrix_[N][i];
    }

    return offset;
}
template <unsigned int N>
void SpatialEntity<N>::setOffset(const Vector<N, float>& offset) {
    for (int i = 0; i < N; i++) {
        modelMatrix_[N][i] = offset[i];
    }
}

template <unsigned int N>
Matrix<N, float> SpatialEntity<N>::getBasis() const {
    Matrix<N, float> basis(1.0f);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            basis[i][j] = modelMatrix_[i][j];
        }
    }
    return basis;
}

template <unsigned int N>
void SpatialEntity<N>::setBasis(const Matrix<N, float>& basis) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            modelMatrix_[i][j] = basis[i][j];
        }
    }
}

template <unsigned int N>
Matrix<N + 1, float> SpatialEntity<N>::getModelMatrix() const {
    return modelMatrix_;
}

template <unsigned int N>
void SpatialEntity<N>::setModelMatrix(const Matrix<N + 1, float>& modelMatrix) {
    modelMatrix_ = modelMatrix;
}

template <unsigned int N>
Matrix<N + 1, float> SpatialEntity<N>::getWorldMatrix() const {
    return worldMatrix_;
}
template <unsigned int N>
void SpatialEntity<N>::setWorldMatrix(const Matrix<N + 1, float>& worldMatrix) {
    worldMatrix_ = worldMatrix;
}

template <unsigned int N>
const SpatialCoordinateTransformer<N>& SpatialEntity<N>::getCoordinateTransformer() const {
    if (!transformer_) transformer_ = new SpatialCoordinateTransformerImpl<N>(this);
    return *transformer_;
}

template <unsigned int N>
const SpatialCameraCoordinateTransformer<N>& inviwo::SpatialEntity<N>::getCoordinateTransformer(
    Camera<N> camera) const {
    if (!cameraTransformer_)
        cameraTransformer_ = new SpatialCameraCoordinateTransformerImpl<N>(this, camera);
    static_cast<SpatialCameraCoordinateTransformerImpl<N>*>(cameraTransformer_)->setCamera(camera);
    return *cameraTransformer_;
}

/*---------------------------------------------------------------*
 *  Implementations                                              *
 *  StructuredGridEntity                                         *
 *---------------------------------------------------------------*/

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity()
    : SpatialEntity<N>(), dimensions_(1) {}

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const StructuredGridEntity<N>& rhs)
    : SpatialEntity<N>(rhs), dimensions_(rhs.dimensions_) {}

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const Vector<N, unsigned int>& dimensions)
    : SpatialEntity<N>(), dimensions_(dimensions) {}

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const Vector<N, unsigned int>& dimensions,
                                              const Vector<N, float>& spacing)
    : SpatialEntity<N>(), dimensions_(dimensions) {
    Matrix<N, float> basis(1.0f);
    for (int i = 0; i < N; ++i) {
        basis[i][i] = dimensions[i] * spacing[i];
    }
    
    setBasis(basis);
    Vector<N, float> offset(0.0f);
    for (int i = 0; i < N; ++i) {
        offset += basis[i];
    }
    setOffset(-0.5f * offset);
}

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const Vector<N, unsigned int>& dimensions,
                                              const Matrix<N + 1, float>& modelMatrix)
    : SpatialEntity<N>(modelMatrix)
    , dimensions_(dimensions) {}

template <unsigned int N>
StructuredGridEntity<N>::StructuredGridEntity(const Vector<N, unsigned int>& dimensions,
                                              const Matrix<N + 1, float>& modelMatrix,
                                              const Matrix<N + 1, float>& worldMatrix)
    : SpatialEntity<N>(modelMatrix, worldMatrix)
    , dimensions_(dimensions) {}

template <unsigned int N>
StructuredGridEntity<N>& StructuredGridEntity<N>::operator=(const StructuredGridEntity<N>& that) {
    if (this != &that) {
        SpatialEntity<N>::operator=(that);
        dimensions_ = that.dimensions_;
    }
    return *this;
}

template <unsigned int N>
Vector<N, unsigned int> StructuredGridEntity<N>::getDimensions() const {
    return dimensions_;
}
template <unsigned int N>
void StructuredGridEntity<N>::setDimensions(const Vector<N, unsigned int>& dimensions) {
    dimensions_ = dimensions;
}

template <unsigned int N>
Matrix<N + 1, float> StructuredGridEntity<N>::getIndexMatrix() const {
    Matrix<N + 1, float> indexMatrix(1.0f);
    for (int i = 0; i < N; ++i) {
        indexMatrix[i][i] = dimensions_[i];
    }

    // Offset to coordinates to center them in the middle of the texel/voxel.
    for (int i = 0; i < N; i++) {
        indexMatrix[N][i] = -0.5f;
    }
    return indexMatrix;
}

template <unsigned int N>
const StructuredCoordinateTransformer<N>& StructuredGridEntity<N>::getCoordinateTransformer() const {
    if (!this->transformer_) this->transformer_ = new StructuredCoordinateTransformerImpl<N>(this);
    return *static_cast<StructuredCoordinateTransformer<N>*>(this->transformer_);
}

template <unsigned int N>
const StructuredCameraCoordinateTransformer<N>& inviwo::StructuredGridEntity<N>::getCoordinateTransformer(
    Camera<N> camera) const {
    if (!this->cameraTransformer_)
        this->cameraTransformer_ = new StructuredCameraCoordinateTransformerImpl<N>(this, camera);
    static_cast<StructuredCameraCoordinateTransformerImpl<N>*>(this->cameraTransformer_)->setCamera(camera);
    return *static_cast<StructuredCameraCoordinateTransformer<N>*>(this->cameraTransformer_);
}

}  // namespace

#endif  // IVW_SPATIALDATA_H

