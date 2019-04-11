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

#include <modules/discretedata/connectivity/structuredgrid.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/connectivity/elementiterator.h>

#include <inviwo/core/util/assertion.h>
#include <numeric>

namespace inviwo {
namespace discretedata {

StructuredGrid::StructuredGrid(GridPrimitive gridDimension, const std::vector<ind>& numCellsPerDim)
    : Connectivity(gridDimension), numCellsPerDimension_(numCellsPerDim) {
    IVW_ASSERT(static_cast<ind>(gridDimension) > static_cast<ind>(GridPrimitive::Vertex),
               "GridPrimitive need to be at least Edge for a structured grid");
    IVW_ASSERT(static_cast<ind>(numCellsPerDim.size()) == static_cast<ind>(gridDimension),
               "Grid dimension should match cell dimension.");

    numCellsPerDimension_ = std::vector<ind>(numCellsPerDim);

    ind maxDim = static_cast<ind>(gridDimension);
    ind numCombinations = ind(1) << maxDim;
    for (ind combo = 0; combo < numCombinations; ++combo) {
        ind numNormal = 0;  // Count number of 1 bits. This is the dimension we add to.
        ind product = 1;    // Get one term, depending on the combo.
        for (ind dim = 0; dim < maxDim; ++dim) {
            if (combo & (ind(1) << dim)) {
                numNormal++;
                product *= numCellsPerDimension_[dim];
            } else
                product *= numCellsPerDimension_[dim] + 1;
        }

        // Add to correct dimension.
        numGridPrimitives_[numNormal] += product;
    }
}

std::vector<ind> StructuredGrid::indexFromLinear(ind idxLin, const std::vector<ind>& size) {
    std::vector<ind> index(size.size());
    for (ind dim = 0; dim < static_cast<ind>(size.size()); ++dim) {
        index[dim] = idxLin % size[dim];
        idxLin = static_cast<ind>((idxLin) / size[dim]);
    }

    return index;
}

ind StructuredGrid::indexToLinear(const std::vector<ind>& idx, const std::vector<ind>& size) {
    assert(idx.size() == size.size() && "Different dimensionalities.");
    ind linIdx = 0;
    ind step = 1;

    for (size_t dim = 0; dim < size.size(); ++dim) {
        assert(idx[dim] < size[dim] && idx[dim] >= 0 && "Index not within bounds.");
        linIdx += step * idx[dim];
        step *= size[dim];
    }
    return linIdx;
}

void StructuredGrid::sameLevelConnection(std::vector<ind>& result, const ind idxLin,
                                         const std::vector<ind>& size) {
    std::vector<ind> index = indexFromLinear(idxLin, size);

    ind dimensionProduct = 1;
    for (size_t dim = 0; dim < size.size(); ++dim) {
        if (index[dim] > 0) result.push_back(idxLin - dimensionProduct);
        if (index[dim] < size[dim] - 1) result.push_back(idxLin + dimensionProduct);

        dimensionProduct *= size[dim];
    }
}

void StructuredGrid::getConnections(std::vector<ind>& result, ind idxLin, GridPrimitive from,
                                    GridPrimitive to, bool) const {
    result.clear();

    if (from == to && from == gridDimension_) {
        // Linear Index to nD Cell Index.
        sameLevelConnection(result, idxLin, numCellsPerDimension_);
        return;
    }

    std::vector<ind> vertDims;
    for (ind dim : numCellsPerDimension_) {
        vertDims.push_back(dim + 1);
    }

    if (from == to && from == GridPrimitive::Vertex) {
        sameLevelConnection(result, idxLin, vertDims);
        return;
    }
    const ind numDimensions = numCellsPerDimension_.size();
    const ind Dmax = static_cast<ind>(gridDimension_);
    const GridPrimitive Dminus = static_cast<GridPrimitive>(Dmax - 1);

    // From D-1 to D (face->voxel, edge->face etc).
    if (from == Dminus && to == gridDimension_) {

        ind idxLeft = idxLin;
        auto numElements = numCellsPerDimension_;
        std::vector<ind> idxVec;
        idxVec.reserve(Dmax);

        // Test which direction we lay in.
        for (ind dim = 0; dim < Dmax; ++dim) {
            numElements[dim]++;
            ind numElemProd = std::accumulate(numElements.begin(), numElements.end(), 1,
                                              [](auto a, auto b) { return a * b; });
            // Not in this direction, reset state and carry on.
            if (idxLeft >= numElemProd) {
                idxLeft -= numElemProd;
                numElements[dim]--;
                continue;
            }

            // Correct dimension. Get vectorial index.
            idxVec = indexFromLinear(idxLeft, numElements);
            if (idxVec[dim] < numCellsPerDimension_[dim])
                result.push_back(indexToLinear(idxVec, numCellsPerDimension_));
            if (idxVec[dim] > 0) {
                idxVec[dim]--;
                result.push_back(indexToLinear(idxVec, numCellsPerDimension_));
            }
            return;
        }
        assert(false && "No correct dimension found.");
    }

    // From D to D-1 (face<-voxel, edge<-face etc).
    if (from == gridDimension_ && to == Dminus) {

        auto numElements = numCellsPerDimension_;
        std::vector<ind> idxVec = indexFromLinear(idxLin, numCellsPerDimension_);
        // idxVec.reserve(Dmax);
        ind dimOffset = 0;
        for (ind dim = 0; dim < Dmax; ++dim) {
            numElements[dim]++;
            ind numElemProd = std::accumulate(numElements.begin(), numElements.end(), 1,
                                              [](auto a, auto b) { return a * b; });

            // Add the two neighbors in this direction.
            result.push_back(dimOffset + indexToLinear(idxVec, numElements));
            idxVec[dim]++;
            result.push_back(dimOffset + indexToLinear(idxVec, numElements));
            idxVec[dim]--;

            dimOffset += numElemProd;
        }
        return;
    }

    // From D-1 to 0 (face->vertex, edge->vertex etc).
    if (from == Dminus && to == GridPrimitive::Vertex) {

        ind idxLeft = idxLin;
        auto numElements = numCellsPerDimension_;
        std::vector<ind> idxVec;

        // Test which direction we lay in.
        for (ind dim = 0; dim < Dmax; ++dim) {
            numElements[dim]++;
            ind numElemProd = std::accumulate(numElements.begin(), numElements.end(), 1,
                                              [](auto a, auto b) { return a * b; });
            // Not in this direction, reset state and carry on.
            if (idxLeft >= numElemProd) {
                idxLeft -= numElemProd;
                numElements[dim]--;
                continue;
            }

            idxVec = indexFromLinear(idxLin, numElements);

            std::vector<ind> vertIdx;
            ind numCombos = ind(1) << Dmax;
            for (ind combo = 0; combo < numCombos; ++combo) {
                if (combo & (ind(1) << dim)) continue;

                vertIdx = idxVec;
                for (ind d = 0; d < Dmax; ++d) {
                    if (combo & (ind(1) << d)) vertIdx[d]++;
                }
                result.push_back(indexToLinear(vertIdx, vertDims));
            }
            return;
        }
        assert(false && "Did not find correct direction.");
    }

    if (from == gridDimension_ && to == GridPrimitive::Vertex) {
        // Prepare corners
        const ind numCorners = ind(1) << numDimensions;
        result.resize(numCorners);

        // Vertex Strides - how much to add to the linear index to go forward by 1 in each dimension
        std::vector<ind> vStrides(numDimensions);
        ind dimProduct(1);
        for (ind dim(0); dim < numDimensions; dim++) {
            vStrides[dim] = dimProduct;
            dimProduct *= (numCellsPerDimension_[dim] + 1);
        }

        // Linear Index to nD Cell Index.
        ind idxRemainder = idxLin;
        std::vector<ind> cellIndex(numDimensions, -1);
        for (ind dim(0); dim < numDimensions; dim++) {
            cellIndex[dim] = idxRemainder % numCellsPerDimension_[dim];
            idxRemainder = (ind)(idxRemainder / numCellsPerDimension_[dim]);
        }

        // The given cell index is also the index of its lower-left-front corner vertex
        // Let's compute the linear index for this vertex
        ind lowerLeftFrontVertexLinearIndex = cellIndex[0];
        for (ind dim(1); dim < numDimensions; dim++) {
            lowerLeftFrontVertexLinearIndex += cellIndex[dim] * vStrides[dim];
        }

        result[0] = lowerLeftFrontVertexLinearIndex;
        for (ind i(1); i < numCorners; i++) {
            // Base is the lower-left-front corner.
            result[i] = lowerLeftFrontVertexLinearIndex;

            // Add strides to the lower-left-front corner.
            for (ind d(0); d < numDimensions; d++) {
                if (i & (ind(1) << d)) result[i] += vStrides[d];
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

        const ind numDimensions = vertDims.size();

        // Linear Index to nD Vertex Index.
        std::vector<ind> vertexIndex = indexFromLinear(idxLin, vertDims);

        // Prepare neighbors
        const ind maxNeighbors = ind(1) << numDimensions;
        result.reserve(maxNeighbors);

        // Compute neighbors
        std::vector<ind> currentNeighbor;
        for (ind i(0); i < maxNeighbors; i++) {
            // Base index is the vertex index.
            // The same cell index is the upper-right one of the neighbors.
            currentNeighbor = vertexIndex;

            // Generate new neighbor index
            for (ind d(0); d < numDimensions; d++) {
                if (i & (ind(1) << d)) currentNeighbor[d]--;
            }

            // Is it in the allowed range? And compute linear index while checking.
            bool bOk(true);
            ind currentNeighborLinearIndex(0);
            ind dimensionProduct(1);
            for (ind d(0); bOk && d < numDimensions; d++) {
                if (currentNeighbor[d] < 0 || currentNeighbor[d] >= numCellsPerDimension_[d]) {
                    bOk = false;
                }

                currentNeighborLinearIndex += currentNeighbor[d] * dimensionProduct;
                dimensionProduct *= numCellsPerDimension_[d];
            }

            // If so, let's add it.
            if (bOk) result.push_back(currentNeighborLinearIndex);
        }

        return;
    }

    assert(false && "Not implemented yet.");
}

ind StructuredGrid::getNumCellsInDimension(ind dim) const {
    assert(numCellsPerDimension_[dim] >= 0 && "Number of elements not known yet.");
    return numCellsPerDimension_[dim];
}

void StructuredGrid::getNumCells(std::vector<ind>& result) const {
    result.clear();
    for (ind numCells : numCellsPerDimension_) {
        //if (numCells > 0)
        result.push_back(numCells);
        /*else {
            result.clear();
            return;
        }*/
    }
}

CellType StructuredGrid::getCellType(GridPrimitive dim, ind) const {
    switch (dim) {
        case GridPrimitive::Vertex:
            return CellType::Vertex;
        case GridPrimitive::Edge:
            return CellType::Line;
        case GridPrimitive::Face:
            return CellType::Quad;
        case GridPrimitive::Volume:
            return CellType::Hexahedron;
        default:
            return CellType::HigherOrderHexahedron;
    }
}

}  // namespace discretedata
}  // namespace inviwo
