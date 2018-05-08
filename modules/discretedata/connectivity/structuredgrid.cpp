/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2012-2018 Inviwo Foundation
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

#include "structuredgrid.h"
#include "analyticchannel.h"
#include "elementiterator.h"

namespace inviwo {
namespace dd {

StructuredGrid::StructuredGrid(GridPrimitive gridDimension, const std::vector<ind>& numCellsPerDim)
    : Connectivity(gridDimension), numCellsPerDimension_(numCellsPerDim) {
    ivwAssert(numCellsPerDim.size() == gridDimension,
              "Grid dimension should match cell dimension.");
    numCellsPerDimension_ = std::vector<ind>(numCellsPerDim);

    ind numCells = 1;
    ind numVerts = 1;
    for (ind dim = GridPrimitive::Vertex; dim < gridDimension; ++dim) {
        numCells *= numCellsPerDimension_[dim];
        numVerts *= numCellsPerDimension_[dim] + 1;
    }

    numGridPrimitives_[gridDimension] = numCells;
    numGridPrimitives_[GridPrimitive::Vertex] = numVerts;
}

namespace {
std::vector<ind> sameLevelConnection(const ind idxLin, const std::vector<ind>& index,
                                     const std::vector<ind>& size) {
    ivwAssert(index.size() == size.size(), "Dimensions of input not matching.");

    std::vector<ind> neighbors;
    ind dimensionProduct = 1;
    for (int dim = 0; dim < size.size(); ++dim) {
        if (index[dim] > 0) neighbors.push_back(idxLin - dimensionProduct);
        if (index[dim] < size[dim] - 1) neighbors.push_back(idxLin + dimensionProduct);

        dimensionProduct *= size[dim];
    }

    return neighbors;
}
}

std::vector<ind> StructuredGrid::getConnections(ind idxLin, GridPrimitive from,
                                                GridPrimitive to) const {
    if (from == to && from == gridDimension_) {
        // Linear Index to nD Cell Index.
        ind idxCutoff = idxLin;
        std::vector<ind> index(numCellsPerDimension_.size(), -1);
        for (ind dim = 0; dim < (ind)numCellsPerDimension_.size(); ++dim) {
            ind dimSize = numCellsPerDimension_[dim];
            index[dim] = idxCutoff % dimSize;
            idxCutoff = (ind)(idxCutoff / dimSize);
        }
        return sameLevelConnection(idxLin, index, numCellsPerDimension_);
    }

    if (from == to && from == GridPrimitive::Vertex) {
        std::vector<ind> vertDims;
        for (ind dim : numCellsPerDimension_) {
            vertDims.push_back(dim + 1);
        }

        // Linear Index to nD Vertex Index.
        ind idxCutoff = idxLin;
        std::vector<ind> index(vertDims.size(), -1);
        for (ind dim = 0; dim < (ind)vertDims.size(); ++dim) {
            ind dimSize = vertDims[dim];
            index[dim] = idxCutoff % dimSize;
            idxCutoff = (ind)(idxCutoff / dimSize);
        }

        return sameLevelConnection(idxLin, index, vertDims);
    }

    if (from == gridDimension_ && to == GridPrimitive::Vertex) {
        // Prepare corners
        const ind NumDimensions = numCellsPerDimension_.size();
        const ind NumCorners = 1i64 << NumDimensions;
        std::vector<ind> VertexCorners(NumCorners);

        // Vertex Strides - how much to add to the linear index to go forward by 1 in each dimension
        std::vector<ind> VStrides(NumDimensions);
        ind DimProduct(1);
        for (ind dim(0); dim < NumDimensions; dim++) {
            VStrides[dim] = DimProduct;
            DimProduct *= (numCellsPerDimension_[dim] + 1);
        }

        // Linear Index to nD Cell Index.
        ind IdxRemainder = idxLin;
        std::vector<ind> CellIndex(NumDimensions, -1);
        for (ind dim(0); dim < NumDimensions; dim++) {
            CellIndex[dim] = IdxRemainder % numCellsPerDimension_[dim];
            IdxRemainder = (ind)(IdxRemainder / numCellsPerDimension_[dim]);
        }

        // The given cell index is also the index of its lower-left-front corner vertex
        // Let's compute the linear index for this vertex
        ind LowerLeftFrontVertexLinearIndex = CellIndex[0];
        for (ind dim(1); dim < NumDimensions; dim++) {
            LowerLeftFrontVertexLinearIndex += CellIndex[dim] * VStrides[dim];
        }

        VertexCorners[0] = LowerLeftFrontVertexLinearIndex;
        for (ind i(1); i < NumCorners; i++) {
            // Base is the lower-left-front corner.
            VertexCorners[i] = LowerLeftFrontVertexLinearIndex;

            // Add strides to the lower-left-front corner.
            for (ind d(0); d < NumDimensions; d++) {
                if (i & (1i64 << d)) VertexCorners[i] += VStrides[d];
            }
        }

        return VertexCorners;
    }

    if (from == GridPrimitive::Vertex && to == gridDimension_) {
        // Compute dimensions for vertices
        std::vector<ind> vertDims;
        for (ind dim : numCellsPerDimension_) {
            vertDims.push_back(dim + 1);
        }

        const ind NumDimensions = vertDims.size();

        // Linear Index to nD Vertex Index.
        ind idxCutoff = idxLin;
        std::vector<ind> VertexIndex(vertDims.size(), -1);
        for (ind dim(0); dim < NumDimensions; dim++) {
            ind dimSize = vertDims[dim];
            VertexIndex[dim] = idxCutoff % dimSize;
            idxCutoff = (ind)(idxCutoff / dimSize);
        }

        // Prepare neighbors
        std::vector<ind> CellNeighbors;
        const ind MaxNeighbors = 1i64 << NumDimensions;
        CellNeighbors.reserve(MaxNeighbors);

        // Compute neighbors
        std::vector<ind> CurrentNeighbor;
        for (ind i(0); i < MaxNeighbors; i++) {
            // Base index is the vertex index. The same cell index is the upper-right one of the
            // neighbors.
            CurrentNeighbor = VertexIndex;

            // Generate new neighbor index
            for (ind d(0); d < NumDimensions; d++) {
                if (i & (1i64 << d)) CurrentNeighbor[d]--;
            }

            // Is it in the allowed range? And compute linear index while checking.
            bool bOk(true);
            ind CurrentNeighborLinearIndex(0);
            ind DimensionProduct(1);
            for (ind d(0); bOk && d < NumDimensions; d++) {
                if (CurrentNeighbor[d] < 0 || CurrentNeighbor[d] >= numCellsPerDimension_[d]) {
                    bOk = false;
                }

                CurrentNeighborLinearIndex += CurrentNeighbor[d] * DimensionProduct;
                DimensionProduct *= numCellsPerDimension_[d];
            }

            // If so, let's add it.
            if (bOk) CellNeighbors.push_back(CurrentNeighborLinearIndex);
        }

        return CellNeighbors;
    }

    ivwAssert(false, "Not implemented yet.");
    return std::vector<ind>();
}

ind StructuredGrid::getNumCellsInDimension(ind dim) const {
    IVW_ASSERT(numCellsPerDimension_[dim] >= 0, "Number of elements not known yet.");
    return numCellsPerDimension_[dim];
}

double StructuredGrid::getPrimitiveMeasure(GridPrimitive dim, ind index) const {
    if (!this->vertices_) return -1;

    // Only implemented 3D bodies so far.
    if (dim != GridPrimitive::Volume) return -1;

    double measure = -1;

    // TODO: Make this kind of code obsolete or at least compact.
    switch (vertices_->getDataFormatId()) {
        case DataFormatId::Float16:
            measure = computeHexVolume<float>(index);
            break;
        case DataFormatId::Float32:
            measure = computeHexVolume<glm::f32>(index);
            break;
        case DataFormatId::Float64:
            measure = computeHexVolume<glm::f64>(index);
            break;
        case DataFormatId::Int8:
            measure = computeHexVolume<glm::i8>(index);
            break;
        case DataFormatId::Int16:
            measure = computeHexVolume<glm::i16>(index);
            break;
        case DataFormatId::Int32:
            measure = computeHexVolume<glm::i32>(index);
            break;
        case DataFormatId::Int64:
            measure = computeHexVolume<glm::i64>(index);
            break;
        case DataFormatId::UInt8:
            measure = computeHexVolume<glm::u8>(index);
            break;
        case DataFormatId::UInt16:
            measure = computeHexVolume<glm::u16>(index);
            break;
        case DataFormatId::UInt32:
            measure = computeHexVolume<glm::u32>(index);
            break;
        case DataFormatId::UInt64:
            measure = computeHexVolume<glm::u64>(index);
            break;

        default:
            LogWarn("Data type not supported. Edit " << __FILE__);
            break;
    }

    return measure;
}

}  // namespace
}
