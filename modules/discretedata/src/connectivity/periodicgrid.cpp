/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/connectivity/periodicgrid.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/connectivity/elementiterator.h>
#include <inviwo/core/util/formatdispatching.h>

namespace inviwo {
namespace discretedata {

PeriodicGrid::PeriodicGrid(GridPrimitive gridDimension, const std::vector<ind>& numCellsPerDim,
                           const std::vector<bool>& isDimPeriodic)
    : StructuredGrid(gridDimension, numCellsPerDim), isDimPeriodic_(isDimPeriodic) {
    assert(numCellsPerDim.size() == static_cast<size_t>(gridDimension) &&
           "Grid dimension should match cell dimension.");
    numCellsPerDimension_ = std::vector<ind>(numCellsPerDim);

    ind numCells = 1;
    ind numVerts = 1;
    for (ind dim = (ind)GridPrimitive::Vertex; dim < (ind)gridDimension; ++dim) {
        numCells *= numCellsPerDimension_[dim];
        numVerts *= numCellsPerDimension_[dim] + 1;
    }

    numGridPrimitives_[(ind)gridDimension] = numCells;
    numGridPrimitives_[(ind)GridPrimitive::Vertex] = numVerts;
}

void PeriodicGrid::getConnections(std::vector<ind>& result, ind idxLin, GridPrimitive from,
                                  GridPrimitive to, bool isPosition) const {
    result.clear();

    if (isPosition) {
        // Position-wise, there is no difference from a StructuredGrid.
        StructuredGrid::getConnections(result, idxLin, from, to, isPosition);
        return;
    }
    if (from == to && from == gridDimension_) {
        // In this variant, the last cell is the same as the first within each dimension.
        sameLevelConnection(result, idxLin, numCellsPerDimension_);
        return;
    }

    if (from == to && from == GridPrimitive::Vertex) {
        std::vector<ind> vertDims;
        for (ind dim : numCellsPerDimension_) {
            vertDims.push_back(dim + 1);
        }

        // In this variant, the last vertex is the same as the first within each dimension.
        sameLevelConnection(result, idxLin, vertDims);
        return;
    }

    if (from == gridDimension_ && to == GridPrimitive::Vertex) {
        // Prepare corners
        const ind numDimensions = numCellsPerDimension_.size();
        const ind numCorners = ind(1) << numDimensions;
        result.resize(numCorners);

        // Vertex Strides - how much to add to the linear index to go forward by 1 in each dimension
        std::vector<ind> VStrides(numDimensions);
        ind dimProduct(1);
        for (ind dim(0); dim < numDimensions; dim++) {
            VStrides[dim] = dimProduct;
            dimProduct *= (numCellsPerDimension_[dim] + 1);
        }
        // Linear Index to nD Cell Index.
        std::vector<ind> cellIndex = StructuredGrid::indexFromLinear(idxLin, numCellsPerDimension_);

        // The given cell index is also the index of its lower-left-front corner vertex
        // Let's compute the linear index for this vertex
        ind lowerLeftFrontVertexLinearIndex = cellIndex[0];
        for (ind dim(1); dim < numDimensions; dim++) {
            lowerLeftFrontVertexLinearIndex += cellIndex[dim] * VStrides[dim];
        }

        result[0] = lowerLeftFrontVertexLinearIndex;
        for (ind i(1); i < numCorners; i++) {
            // Base is the lower-left-front corner.
            result[i] = lowerLeftFrontVertexLinearIndex;
            // Add strides to the lower-left-front corner.
            for (ind d(0); d < numDimensions; d++) {
                if (i & (ind(1) << d)) {
                    // Last cell in a periodic dimension does not connect to the last vertex, but
                    // the first one.
                    if (isPeriodic(d) && cellIndex[d] == numCellsPerDimension_[d])
                        result[i] -= VStrides[d] * (numCellsPerDimension_[d]);
                    else
                        result[i] += VStrides[d];
                }
            }
        }

        return;
    }

    if (from == GridPrimitive::Vertex && to == gridDimension_) {
        // Compute dimensions for vertices
        std::vector<ind> vertDims;
        for (ind dim : numCellsPerDimension_) {
            vertDims.push_back(dim + 1);
        }

        const ind NumDimensions = vertDims.size();

        // Linear Index to nD Vertex Index.
        std::vector<ind> VertexIndex = indexFromLinear(idxLin, vertDims);

        // Prepare neighbors.
        const ind MaxNeighbors = ind(1) << NumDimensions;
        result.reserve(MaxNeighbors);

        // Compute neighbors.
        std::vector<ind> CurrentNeighbor;
        for (ind i(0); i < MaxNeighbors; i++) {
            // Base index is the vertex index. The same cell index is the upper-right one of the
            // neighbors.
            CurrentNeighbor = VertexIndex;

            // Generate new neighbor index
            for (ind d(0); d < NumDimensions; d++) {
                if (i & (ind(1) << d)) CurrentNeighbor[d]--;
            }

            // Is it in the allowed range? And compute linear index while checking.
            bool bOk(true);
            ind CurrentNeighborLinearIndex(0);
            ind DimensionProduct(1);
            for (ind d(0); bOk && d < NumDimensions; d++) {
                if (CurrentNeighbor[d] < 0) {
                    if (isPeriodic(d)) {
                        // Map to last cell in dimension, the one going over the boundary.
                        CurrentNeighbor[d] = numCellsPerDimension_[d] - 2;
                    } else {
                        bOk = false;
                    }
                }
                if (CurrentNeighbor[d] == numCellsPerDimension_[d]) {
                    if (isPeriodic(d)) {
                        // Map to first cell in dimension.
                        CurrentNeighbor[d] = 0;
                    } else {
                        bOk = false;
                    }
                }

                CurrentNeighborLinearIndex += CurrentNeighbor[d] * DimensionProduct;
                DimensionProduct *= numCellsPerDimension_[d];
            }

            // If so, let's add it.
            if (bOk) result.push_back(CurrentNeighborLinearIndex);
        }

        return;
    }

    assert(false && "Not implemented yet.");
}

ind PeriodicGrid::getNumCellsInDimension(ind dim) const {
    assert(numCellsPerDimension_[dim] >= 0 && "Number of elements not known yet.");
    return numCellsPerDimension_[dim];
}

void PeriodicGrid::sameLevelConnection(std::vector<ind>& result, ind idxLin,
                                       const std::vector<ind>& size) const {

    result.clear();
    ind dimensionProduct = 1;
    ind index = idxLin;
    for (ind dim = 0; dim < (ind)size.size(); ++dim) {
        ind nextDimProd = dimensionProduct * size[dim];
        ind locIdx = index % size[dim];
        index = index / size[dim];

        if (locIdx > 0)
            result.push_back(idxLin - dimensionProduct);
        else if (isPeriodic(dim))
            result.push_back(idxLin + nextDimProd - 2 * dimensionProduct);

        if (isPeriodic(dim) && locIdx == size[dim] - 2)
            result.push_back(idxLin - nextDimProd + 2 * dimensionProduct);
        else if (locIdx < size[dim] - 1)
            result.push_back(idxLin + dimensionProduct);

        dimensionProduct = nextDimProd;
    }
}

}  // namespace discretedata
}  // namespace inviwo
