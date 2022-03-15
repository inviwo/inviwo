/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/sampling/datasetsampler.h>
#include <modules/discretedata/connectivity/extrudedgrid.h>
#include <modules/discretedata/channels/extendedchannel.h>
#include <modules/discretedata/util/arrayutil.h>

namespace inviwo {
namespace discretedata {

/**
 * \brief Extending a DataSet Sampler into one additional dimension.
 **/
template <unsigned int SpatialDims>
class ExtrudedDataSetSampler : public DataSetSampler<SpatialDims> {

public:
    ExtrudedDataSetSampler(std::shared_ptr<const DataSetSampler<SpatialDims - 1>> baseSampler,
                           const std::shared_ptr<const DataChannel<double, 1>>& extraDimension,
                           const std::shared_ptr<const ExtrudedGrid>& grid = nullptr);

    virtual ~ExtrudedDataSetSampler() {
        std::cerr << "Deleting extruded data set sampler" << std::endl;
    }
    ExtrudedDataSetSampler(ExtrudedDataSetSampler<SpatialDims>&& other);
    ExtrudedDataSetSampler(const ExtrudedDataSetSampler<SpatialDims>& other);
    // ExtrudedDataSetSampler& operator=(ExtrudedDataSetSampler<SpatialDims>&& other);
    // ExtrudedDataSetSampler& operator=(const ExtrudedDataSetSampler<SpatialDims>& other);
    virtual SpatialEntity<SpatialDims>* clone() const override;

    virtual unsigned int getDimension() const { return SpatialDims; }

    virtual ind locateAndSampleCell(
        const std::array<float, SpatialDims>& pos, std::vector<double>& returnWeights,
        std::vector<ind>& returnVertices,
        InterpolationType interpolationType = InterpolationType::Ignore) const override;
    virtual std::string getIdentifier() const override;

protected:
    const std::shared_ptr<const DataSetSampler<SpatialDims - 1>> baseSampler_;
    const std::shared_ptr<const DataChannel<double, 1>> extraDimension_;
};

template <unsigned int SpatialDims>
ExtrudedDataSetSampler<SpatialDims>::ExtrudedDataSetSampler(
    std::shared_ptr<const DataSetSampler<SpatialDims - 1>> baseSampler,
    const std::shared_ptr<const DataChannel<double, 1>>& extraDimension,
    const std::shared_ptr<const ExtrudedGrid>& grid)
    : DataSetSampler<SpatialDims>(
          grid ? grid : std::make_shared<ExtrudedGrid>(baseSampler->grid_, extraDimension->size()),
          std::make_shared<ExtendedChannel<double, SpatialDims>>(
              std::dynamic_pointer_cast<const DataChannel<double, SpatialDims - 1>>(
                  baseSampler->coordinates_),
              extraDimension,  //"tmptmp"),
              fmt::format("{}_x_{}", baseSampler->coordinates_->getName(),
                          extraDimension->getName())),
          ExtendedInterpolant<SpatialDims>(baseSampler->getInterpolant()),
          dd_util::arrAppend(baseSampler->getMin(), float(*extraDimension->begin())),
          dd_util::arrAppend(baseSampler->getMax(), float(*(extraDimension->end() - 1))))
    , baseSampler_(baseSampler)
    , extraDimension_(extraDimension) {
    if (grid) {
        ivwAssert(grid->getDimension() == (GridPrimitive)SpatialDims,
                  "The grid has the wrong dimension.");
        ivwAssert(grid->numExtrudedVertices_ == extraDimension->size(),
                  "Given extruded grid does not fit extruding dimensions size.");
    }
}

template <unsigned int SpatialDims>
ind ExtrudedDataSetSampler<SpatialDims>::locateAndSampleCell(
    const std::array<float, SpatialDims>& pos, std::vector<double>& returnWeights,
    std::vector<ind>& returnVertices, InterpolationType interpolationType) const {
    float extendedPos = pos[SpatialDims - 1];
    if (extendedPos < this->coordsMin_[SpatialDims - 1] ||
        extendedPos > this->coordsMax_[SpatialDims - 1]) {
        return -1;
    }
    auto basePos = dd_util::arrRemoveLast(pos);
    ind baseCell = baseSampler_->locateAndSampleCell(basePos, returnWeights, returnVertices,
                                                     interpolationType);
    if (baseCell < 0) return baseCell;

    auto higherIt = std::lower_bound(extraDimension_->begin(), extraDimension_->end(), extendedPos);
    auto lowerIt = higherIt;  // ConstChannelIterator<DataChannel<double, 1>>

    if (higherIt == extraDimension_->begin()) {
        higherIt++;
    } else {
        lowerIt--;
    }

    size_t baseSize = baseSampler_->coordinates_->size();
    ind lowerExtendedIndex = lowerIt - extraDimension_->begin();
    ind cellIndex = baseCell + lowerExtendedIndex * baseSampler_->grid_->getNumElements(
                                                        GridPrimitive(SpatialDims - 1));

    if (interpolationType == InterpolationType::Ignore) return cellIndex;

    size_t numBaseVertices = returnVertices.size();
    returnVertices.resize(numBaseVertices * 2);
    for (size_t vert = 0; vert < numBaseVertices; ++vert) {
        returnVertices[vert] += lowerExtendedIndex * baseSize;
        returnVertices[numBaseVertices + vert] = returnVertices[vert] + baseSize;
    }

    // Depending on the interpolation type, take the nearest layer or interpolate linearly between
    // two layers. Technically, this gives wrong values for the Shepard interpolation,
    // but we don't need to touch the extended grid this way.
    returnWeights.resize(numBaseVertices * 2);
    float interp;
    switch (interpolationType) {
        case InterpolationType::Nearest:
            if (extendedPos - *lowerIt < *higherIt - extendedPos) {
                std::fill_n(returnWeights.begin() + numBaseVertices, numBaseVertices, 0);
            } else {
                std::copy_n(returnWeights.begin(), numBaseVertices,
                            returnWeights.begin() + numBaseVertices);
                std::fill_n(returnWeights.begin(), numBaseVertices, 0);
            }
            break;

        case InterpolationType::Linear:
            [[fallthrough]];
        case InterpolationType::SquaredDistance:
            interp = (extendedPos - *lowerIt) / (*higherIt - *lowerIt);
            for (size_t w = 0; w < numBaseVertices; ++w) {
                returnWeights[numBaseVertices + w] = returnWeights[w] * interp;
                returnWeights[w] *= 1.0 - interp;
            }
            break;

        // Unknown interpolation, just snap to lower layer.
        default:
            std::fill_n(returnWeights.begin() + numBaseVertices, numBaseVertices, 0);
    }

    return cellIndex;
}

template <unsigned int SpatialDims>
std::string ExtrudedDataSetSampler<SpatialDims>::getIdentifier() const {
    return fmt::format("Extruding_\"{}\"", baseSampler_->getIdentifier());
}

template <unsigned int SpatialDims>
ExtrudedDataSetSampler<SpatialDims>::ExtrudedDataSetSampler(
    ExtrudedDataSetSampler<SpatialDims>&& other)
    : DataSetSampler<SpatialDims>(std::move(other))
    , baseSampler_(std::move(other.baseSampler_))
    , extraDimension_(std::move(other.extraDimension_)) {}

template <unsigned int SpatialDims>
ExtrudedDataSetSampler<SpatialDims>::ExtrudedDataSetSampler(
    const ExtrudedDataSetSampler<SpatialDims>& other)
    : DataSetSampler<SpatialDims>(
          other.grid_,
          std::dynamic_pointer_cast<const DataChannel<double, SpatialDims>>(other.coordinates_),
          *other.interpolant_, other.coordsMin_, other.coordsMax_)
    , baseSampler_(other.baseSampler_)
    , extraDimension_(other.extraDimension_) {}

template <unsigned int SpatialDims>
SpatialEntity<SpatialDims>* ExtrudedDataSetSampler<SpatialDims>::clone() const {
    return new ExtrudedDataSetSampler(*this);
}

namespace detail {
struct CreateExtrudedDataSetSamplerHelper {
    template <typename Result, ind N>
    Result operator()(std::shared_ptr<const DataSetSamplerBase> baseSampler,
                      const std::shared_ptr<const DataChannel<double, 1>>& extraDimension,
                      const std::shared_ptr<const ExtrudedGrid>& grid) {
        return std::make_shared<ExtrudedDataSetSampler<N + 1>>(
            std::dynamic_pointer_cast<const DataSetSampler<N>>(baseSampler), extraDimension, grid);
    }
};
}  // namespace detail

std::shared_ptr<DataSetSamplerBase> IVW_MODULE_DISCRETEDATA_API
createExtrudedDataSetSampler(std::shared_ptr<const DataSetSamplerBase> baseSampler,
                             const std::shared_ptr<const DataChannel<double, 1>>& extraDimension,
                             const std::shared_ptr<const ExtrudedGrid>& grid = nullptr);

}  // namespace discretedata
}  // namespace inviwo

#include "extrudeddatasetsampler.inl"
