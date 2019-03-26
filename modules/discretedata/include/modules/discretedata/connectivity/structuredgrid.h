/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/util.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <inviwo/core/datastructures/spatialdata.h>

namespace inviwo {
namespace discretedata {

class ElementIterator;

/**
 * \brief A curvilinear grid in nD
 * @author Anke Friederici and Tino Weinkauf
 */
class IVW_MODULE_DISCRETEDATA_API StructuredGrid : public Connectivity {
public:
    /**
     * \brief Create an nD grid
     * @param gridDimension Dimension of grid (not vertices)
     * @param numCellsPerDim Number of cells in each dimension, expect size gridDimension+1
     */
    StructuredGrid(GridPrimitive gridDimension, const std::vector<ind>& numCellsPerDim);
    virtual ~StructuredGrid() = default;

    virtual ind getNumCellsInDimension(ind dim) const;

    void getNumCells(std::vector<ind>& result) const;

    virtual CellType getCellType(GridPrimitive dim, ind index) const override;

    virtual void getConnections(std::vector<ind>& result, ind index, GridPrimitive from,
                                GridPrimitive to, bool positions = false) const override;

    static void sameLevelConnection(std::vector<ind>& result, ind idxLin,
                                    const std::vector<ind>& size);

    static std::vector<ind> indexFromLinear(ind idxLin, const std::vector<ind>& size);

    static ind indexToLinear(const std::vector<ind>& idx, const std::vector<ind>& size);

protected:
    std::vector<ind> numCellsPerDimension_;
};

// Making use of Matrix<N + 1, float> StructuredGridEntity<N>::getIndexMatrix() const
template <typename T, ind N, typename Vec = glm::vec<N, T>>
struct IVW_MODULE_DISCRETEDATA_API CurvilinearPositions {  //  : AnalyticChannel<T, N, Vec>
    using VecTN = glm::vec<N, T>;
    using VecIN = glm::vec<N, ind>;
    using VecTNp = glm::vec<N + 1, ind>;
    using MatTNp = Matrix<N + 1, T>;

    CurvilinearPositions(const MatTNp& baseMatrix, const VecIN& size)
        : baseMatrix_(baseMatrix), size_(size) {}

    CurvilinearPositions(const StructuredGridEntity<N>& grid)
        : baseMatrix_(grid.getIndexMatrix()), size_(grid.getDimensions()) {}

    void operator()(Vec& val, ind idx) const {
        // Get vectorial index.
        VecTNp idxVec;
        for (ind dim = 0; dim < N; ++dim) {
            idxVec[dim] = idx % size_[dim];
            idx /= size_[dim];
        }

        // Homogeneous multiply.
        idxVec[N] = 1;
        VecTNp pos = baseMatrix_ * idxVec;
        pos /= pos[N];

        for (ind dim = 0; dim < N; ++dim) val[dim] = pos[dim];
    }

    const MatTNp baseMatrix_;
    const VecIN size_;
};

}  // namespace discretedata
}  // namespace inviwo
