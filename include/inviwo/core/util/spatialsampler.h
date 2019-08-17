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

#ifndef IVW_SPATIALSAMPLER_H
#define IVW_SPATIALSAMPLER_H

#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/datatraits.h>

namespace inviwo {

/**
 * \class SpatialSampler
 */
template <unsigned int SpatialDims, unsigned int DataDims, typename T>
class SpatialSampler {
public:
    static const unsigned SpatialDimensions = SpatialDims;
    static const unsigned DataDimensions = DataDims;
    using Space = CoordinateSpace;
    using ReturnType = Vector<DataDims, T>;

    SpatialSampler(const SpatialEntity<SpatialDims> &spatialEntity, Space space = Space::Data);
    virtual ~SpatialSampler() = default;

    virtual Vector<DataDims, T> sample(const Vector<SpatialDims, double> &pos) const;
    virtual Vector<DataDims, T> sample(const Vector<SpatialDims, float> &pos) const;

    virtual Vector<DataDims, T> sample(const Vector<SpatialDims, double> &pos, Space space) const;
    virtual Vector<DataDims, T> sample(const Vector<SpatialDims, float> &pos, Space space) const;

    virtual bool withinBounds(const Vector<SpatialDims, double> &pos) const;
    virtual bool withinBounds(const Vector<SpatialDims, float> &pos) const;

    virtual bool withinBounds(const Vector<SpatialDims, double> &pos, Space space) const;
    virtual bool withinBounds(const Vector<SpatialDims, float> &pos, Space space) const;

    Matrix<SpatialDims, float> getBasis() const;
    Matrix<SpatialDims + 1, float> getModelMatrix() const;
    Matrix<SpatialDims + 1, float> getWorldMatrix() const;

    const SpatialCoordinateTransformer<SpatialDims> &getCoordinateTransformer() const;

protected:
    virtual Vector<DataDims, T> sampleDataSpace(const Vector<SpatialDims, double> &pos) const = 0;
    virtual bool withinBoundsDataSpace(const Vector<SpatialDims, double> &pos) const = 0;

    Space space_;
    const SpatialEntity<SpatialDims> &spatialEntity_;
    Matrix<SpatialDims + 1, double> transform_;
};

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
struct DataTraits<SpatialSampler<SpatialDims, DataDims, T>> {
    static std::string classIdentifier() {
        return "org.inviwo.SpatialSampler." + toString(SpatialDims) + "D." +
               DataFormat<Vector<DataDims, T>>::str();
    }
    static std::string dataName() {
        return "SpatialSampler<" + toString(SpatialDims) + "D" +
               DataFormat<Vector<DataDims, T>>::str() + ">";
    }
    static uvec3 colorCode() { return uvec3(153, 0, 76); }
    static Document info(const SpatialSampler<SpatialDims, DataDims, T> &) {
        Document doc;
        doc.append("p", dataName());
        return doc;
    }
};

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
SpatialSampler<SpatialDims, DataDims, T>::SpatialSampler(
    const SpatialEntity<SpatialDims> &spatialEntity, Space space)
    : space_(space)
    , spatialEntity_(spatialEntity)
    , transform_{spatialEntity_.getCoordinateTransformer().getMatrix(space, Space::Data)} {}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<SpatialDims, DataDims, T>::sample(
    const Vector<SpatialDims, float> &pos) const {
    return sample(static_cast<Vector<SpatialDims, double>>(pos));
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<SpatialDims, DataDims, T>::sample(
    const Vector<SpatialDims, double> &pos) const {
    if (space_ != Space::Data) {
        const auto p = transform_ * Vector<SpatialDims + 1, double>(pos, 1.0);
        return sampleDataSpace(Vector<SpatialDims, double>(p) / p[SpatialDims]);
    } else {
        return sampleDataSpace(pos);
    }
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<SpatialDims, DataDims, T>::sample(
    const Vector<SpatialDims, float> &pos, Space space) const {
    return sample(static_cast<Vector<SpatialDims, double>>(pos), space);
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Vector<DataDims, T> SpatialSampler<SpatialDims, DataDims, T>::sample(
    const Vector<SpatialDims, double> &pos, Space space) const {
    if (space != Space::Data) {
        const Matrix<SpatialDims + 1, double> m{
            spatialEntity_.getCoordinateTransformer().getMatrix(space, Space::Data)};
        const auto p = m * Vector<SpatialDims + 1, double>(pos, 1.0);
        return sampleDataSpace(Vector<SpatialDims, double>(p) / p[SpatialDims]);
    } else {
        return sampleDataSpace(pos);
    }
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
bool SpatialSampler<SpatialDims, DataDims, T>::withinBounds(
    const Vector<SpatialDims, float> &pos) const {
    return withinBounds(static_cast<Vector<SpatialDims, double>>(pos));
}
template <unsigned int SpatialDims, unsigned int DataDims, typename T>
bool SpatialSampler<SpatialDims, DataDims, T>::withinBounds(
    const Vector<SpatialDims, double> &pos) const {
    if (space_ != Space::Data) {
        const auto p = transform_ * Vector<SpatialDims + 1, double>(pos, 1.0);
        return withinBoundsDataSpace(Vector<SpatialDims, double>(p) / p[SpatialDims]);
    } else {
        return withinBoundsDataSpace(pos);
    }
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
bool SpatialSampler<SpatialDims, DataDims, T>::withinBounds(const Vector<SpatialDims, float> &pos,
                                                            Space space) const {
    return withinBounds(static_cast<Vector<SpatialDims, double>>(pos), space);
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
bool SpatialSampler<SpatialDims, DataDims, T>::withinBounds(const Vector<SpatialDims, double> &pos,
                                                            Space space) const {
    if (space != Space::Data) {
        const Matrix<SpatialDims + 1, double> m{
            spatialEntity_.getCoordinateTransformer().getMatrix(space, Space::Data)};
        const auto p = m * Vector<SpatialDims + 1, double>(pos, 1.0);
        return withinBoundsDataSpace(Vector<SpatialDims, double>(p) / p[SpatialDims]);
    } else {
        return withinBoundsDataSpace(pos);
    }
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
const SpatialCoordinateTransformer<SpatialDims>
    &SpatialSampler<SpatialDims, DataDims, T>::getCoordinateTransformer() const {
    return spatialEntity_.getCoordinateTransformer();
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Matrix<SpatialDims + 1, float> SpatialSampler<SpatialDims, DataDims, T>::getWorldMatrix() const {
    return spatialEntity_.getWorldMatrix();
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Matrix<SpatialDims + 1, float> SpatialSampler<SpatialDims, DataDims, T>::getModelMatrix() const {
    return spatialEntity_.getModelMatrix();
}

template <unsigned int SpatialDims, unsigned int DataDims, typename T>
Matrix<SpatialDims, float> SpatialSampler<SpatialDims, DataDims, T>::getBasis() const {
    return spatialEntity_.getBasis();
}

}  // namespace inviwo

#endif  // IVW_SPATIALSAMPLER_H
