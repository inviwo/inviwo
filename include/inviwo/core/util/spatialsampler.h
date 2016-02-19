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

#ifndef IVW_SPATIALSAMPLER_H
#define IVW_SPATIALSAMPLER_H

#include <inviwo/core/common/inviwo.h>
#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>

#include <inviwo/core/datastructures/spatialdata.h>

namespace inviwo {

/**
 * \class SpatialSampler
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
template <unsigned int SpatialDims, unsigned int DataDims, typename T>
class SpatialSampler {
public:
    using Space = typename SpatialCoordinateTransformer<SpatialDims>::Space;

    SpatialSampler(std::shared_ptr<const SpatialEntity<SpatialDims>> spatialEntity)
        : spatialEntity_(spatialEntity) {}
    virtual ~SpatialSampler() {}

    Vector<DataDims, T> sample(const Vector<SpatialDims, double> &pos,
                                       Space space = Space::Data) const {
        auto dataPos = pos;
        if (space != Space::Data) {
            auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, Space::Data);
            auto p = m * Vector<SpatialDims + 1, float>(
                             static_cast<Vector<SpatialDims, float>>(pos), 1.0);
            dataPos = p.xyz() / p.w;
        }

        return sampleDataSpace(dataPos);
    }

    Vector<DataDims, T> sample(const Vector<SpatialDims, float> &pos,
        Space space = Space::Data) const {
        return sample(static_cast<Vector<SpatialDims, double>>(pos), space);
    }

    Matrix<SpatialDims, float> getBasis() const {
        return spatialEntity_->getBasis();
    }

    Matrix<SpatialDims +1, float> getModelMatrix() const {
        return spatialEntity_->getModelMatrix();
    }

    Matrix<SpatialDims +1, float> getWorldMatrix() const {
        return spatialEntity_->getWorldMatrix();
    }

    const SpatialCoordinateTransformer<SpatialDims> &getCoordinateTransformer() const {
        return spatialEntity_->getCoordinateTransformer();
    }

    bool withinBounds(const Vector<SpatialDims, double> &pos,
        Space space = Space::Data) const {
        auto dataPos = pos;
        if (space != Space::Data) {
            auto m = spatialEntity_->getCoordinateTransformer().getMatrix(space, Space::Data);
            auto p = m * Vector<SpatialDims + 1, float>(
                static_cast<Vector<SpatialDims, float>>(pos), 1.0);
            dataPos = p.xyz() / p.w;
        }

        return withinBoundsDataSpace(dataPos);
    }

    bool withinBounds(const Vector<SpatialDims, float> &pos,
        Space space = Space::Data) const {
        return withinBounds(static_cast<Vector<SpatialDims, double>>(pos), space);
    }

protected:
    virtual Vector<DataDims, T> sampleDataSpace(const Vector<SpatialDims, double> &pos) const = 0;
    virtual bool withinBoundsDataSpace(const Vector<SpatialDims, double> &pos) const = 0;


    std::shared_ptr<const SpatialEntity<SpatialDims>> spatialEntity_;

};

}  // namespace

#endif  // IVW_SPATIALSAMPLER_H
