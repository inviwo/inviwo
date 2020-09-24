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
#include <modules/discretedata/channels/channeldispatching.h>

#include <inviwo/core/util/assertion.h>
#include <numeric>

namespace inviwo {
namespace discretedata {

template <ind N>
StructuredGrid<N>::StructuredGrid(const std::array<ind, N>& numVertices)
    : Connectivity(static_cast<GridPrimitive>(N)), numPrimitives_(*this, numVertices) {

    // Calculating number of edges, faces etc.
    calculateSizes();
}

template <ind N>
StructuredGrid<N>::StructuredGrid(std::array<ind, N>&& numVertices)
    : Connectivity(static_cast<GridPrimitive>(N)), numPrimitives_(*this, std::move(numVertices)) {

    // Calculating number of edges, faces etc.
    calculateSizes();
}

template <ind N>
template <typename... IND>
StructuredGrid<N>::StructuredGrid(ind val0, IND... valX)
    : Connectivity(static_cast<GridPrimitive>(N)), numPrimitives_{*this, {val0, valX...}} {

    // Calculating number of edges, faces etc.
    calculateSizes();
}

template <ind N>
std::array<ind, N> StructuredGrid<N>::indexFromLinear(ind idxLin, const std::array<ind, N>& size) {
    std::array<ind, N> index;
    for (ind dim = 0; dim < N; ++dim) {
        index[dim] = idxLin % size[dim];
        idxLin = static_cast<ind>((idxLin) / size[dim]);
    }

    return index;
}

template <ind N>
ind StructuredGrid<N>::indexToLinear(const std::array<ind, N>& idx,
                                     const std::array<ind, N>& size) {
    ind linIdx = 0;
    ind step = 1;

    for (size_t dim = 0; dim < N; ++dim) {
        IVW_ASSERT((idx[dim] < size[dim] && idx[dim] >= 0), "Index not within bounds.");
        linIdx += step * idx[dim];
        step *= size[dim];
    }
    return linIdx;
}

template <ind N>
void StructuredGrid<N>::sameLevelConnection(std::vector<ind>& result, const ind idxLin,
                                            const std::array<ind, N>& size) {
    std::array<ind, N> index = indexFromLinear(idxLin, size);

    ind dimensionProduct = 1;
    for (size_t dim = 0; dim < N; ++dim) {
        if (index[dim] > 0) result.push_back(idxLin - dimensionProduct);
        if (index[dim] < size[dim] - 1) result.push_back(idxLin + dimensionProduct);

        dimensionProduct *= size[dim];
    }
}

namespace dd_util {
template <ind N, ind From>
struct GetConnectionsFromToHelper {
    template <typename Result, ind To>
    Result operator()(const StructuredGrid<N>& grid, std::vector<ind>& result, ind idxLin) {
        std::cout << "Dispatched " << From << " -> " << To << std::endl;
        grid.template getConnectionsDispatched<From, To>(result, idxLin);
    }
};
template <ind N>
struct GetConnectionsFromHelper {
    template <typename Result, ind From>
    Result operator()(const StructuredGrid<N>& grid, std::vector<ind>& result, ind idxLin,
                      GridPrimitive to) {
        GetConnectionsFromToHelper<N, From> dispatcher;
        channeldispatching::dispatchNumber<void, 0, N>(ind(to), dispatcher, grid, result, idxLin);
    }
};
}  // namespace dd_util

template <ind N>
void StructuredGrid<N>::getConnections(std::vector<ind>& result, ind idxLin, GridPrimitive from,
                                       GridPrimitive to, bool) const {

    dd_util::GetConnectionsFromHelper<N> dispatcher;
    std::cout << "Dispatching " << ind(from) << " -> " << ind(to) << std::endl;
    channeldispatching::dispatchNumber<void, 0, N>(ind(from), dispatcher, *this, result, idxLin,
                                                   to);
}

template <ind N>
template <ind From, ind To>
void StructuredGrid<N>::getConnectionsDispatched(std::vector<ind>& result, ind index) const {
    Primitive<GridPrimitive(From)> fromPrim(index);
    fromPrim.getConnections<To>(result);
}

template <ind N>
ind StructuredGrid<N>::getNumVerticesInDimension(ind dim) const {
    IVW_ASSERT(numPrimitives_[dim] >= 2, "Number of elements not known yet.");
    return numPrimitives_[dim];
}

template <ind N>
const std::array<ind, N>& StructuredGrid<N>::getNumVertices() const {
    return numPrimitives_.NumVerticesPerDimension;
}

template <ind N>
const CellStructure* StructuredGrid<N>::getCellType(GridPrimitive dim, ind) const {
    CellType cell;
    switch (dim) {
        case GridPrimitive::Vertex:
            cell = CellType::Vertex;
            break;
        case GridPrimitive::Edge:
            cell = CellType::Line;
            break;
        case GridPrimitive::Face:
            cell = CellType::Quad;
            break;
        case GridPrimitive::Volume:
            cell = CellType::Hexahedron;
            break;
        default:
            cell = CellType::HigherOrderHexahedron;
    }
    return CellStructureByCellType[(int)cell];
}

template <ind N>
void StructuredGrid<N>::calculateSizes() {
#ifdef IVW_DEBUG
    IVW_ASSERT(static_cast<ind>(gridDimension_) > static_cast<ind>(GridPrimitive::Vertex),
               "GridPrimitive need to be at least Edge for a structured grid");
    IVW_ASSERT(N == static_cast<ind>(gridDimension_),
               "Grid dimension should match cell dimension.");
    for (ind size : numPrimitives_)
        IVW_ASSERT(size >= 1, "At least one vertex in each dimension required.");
#endif

    ind numCombinations = ind(1) << N;
    for (ind combo = 0; combo < numCombinations; ++combo) {
        ind numNormal = 0;  // Count number of 1 bits. This is the dimension we add to.
        ind product = 1;    // Get one term, depending on the combo.
        for (ind dim = 0; dim < N; ++dim) {
            if (combo & (ind(1) << dim)) {
                numNormal++;
                product *= numPrimitives_[dim] - 1;
            } else
                product *= numPrimitives_[dim];
        }

        // Add to correct dimension.
        numGridPrimitives_[numNormal] += product;
    }
}

namespace dd_detail {

template <ind N, ind P>
constexpr void writeSize(typename StructuredGrid<N>::NumPrimitives& numPrimitives) {
    auto dirs = dd_util::initNchooseK<P>();
    bool valid = true;
    size_t dirsIdx = 0;
    ind sizeSum = 0;
    while (valid) {
        numPrimitives.PerDirectionOffsets[numPrimitives.PrimitiveOffsets[P] + dirsIdx] = sizeSum;

        // Assemble size of a single direction combination:
        // multiply all sizes up, one less in dimensions our primitives span.
        auto itDir = dirs.begin();
        ind size = 1;
        for (size_t dim = 0; dim < N; ++dim) {
            ind dimSize = numPrimitives.NumVerticesPerDimension[dim];
            if (itDir != dirs.end() && *itDir == dim) {
                dimSize--;
                ++itDir;
            }
            size *= dimSize;
        }
        sizeSum += size;
        valid = dd_util::nextNchooseK(N, dirs);
        dirsIdx++;
    }
}

template <ind N, int... Is>
constexpr void writeAllSizes(typename StructuredGrid<N>::NumPrimitives& numPrimitives,
                             std::integer_sequence<int, Is...> const&) {

    ((writeSize<N, Is>(numPrimitives)), ...);
}

template <ind N, GridPrimitive P>
constexpr std::enable_if_t<P != GridPrimitive::Vertex, ind> directionsIndex(
    const std::array<size_t, size_t(P)>& dirs) {
    auto dirCheck = dd_util::initNchooseK<size_t(P)>();
    bool valid = true;
    ind idx = 0;
    while (valid) {
        if (dirs == dirCheck) return idx;
        idx++;
        valid = dd_util::nextNchooseK(N, dirCheck);
    }
    return -1;
}

template <ind N, GridPrimitive P>
constexpr std::enable_if_t<P == GridPrimitive::Vertex, ind> directionsIndex(
    const std::array<size_t, 0>& dirs) {
    return 0;
}
}  // namespace dd_detail

template <ind N>
StructuredGrid<N>::NumPrimitives::NumPrimitives(
    const StructuredGrid& grid, const std::array<ind, size_t(N)>& numVerticesPerDimension)
    : PerDirectionOffsets({0}), Grid(grid), NumVerticesPerDimension(numVerticesPerDimension) {

    dd_detail::writeAllSizes<N>(*this, std::make_integer_sequence<int, N + 1>{});
}

template <ind N>
template <GridPrimitive P>
constexpr const ind* StructuredGrid<N>::NumPrimitives::getOffset(ind dirsIdx) const {
    if (dirsIdx < 0) return nullptr;
    return &PerDirectionOffsets[PrimitiveOffsets[size_t(P)] + dirsIdx];
}

template <ind N>
template <GridPrimitive P>
constexpr const ind* StructuredGrid<N>::NumPrimitives::getOffset(
    const std::array<size_t, size_t(P)>& dirs) const {
    ind dirsIdx = getDirectionsIndex<P>(dirs);
    return getOffset<P>(dirsIdx);
}

template <ind N>
template <GridPrimitive P>
ind StructuredGrid<N>::NumPrimitives::getSize(ind dirsIdx) const {
    if (dirsIdx < 0) return -1;

    size_t primOffset = PrimitiveOffsets[size_t(P)];
    if (primOffset + dirsIdx >= PerDirectionOffsets.size()) return -1;
    ind upperLimit = (primOffset + 1 == PerDirectionOffsets.size())
                         ? 0
                         : PerDirectionOffsets[primOffset + dirsIdx + 1];
    if (upperLimit == 0) upperLimit = Grid.getNumElements(P);

    return upperLimit - (PerDirectionOffsets[primOffset + dirsIdx]);
}

template <ind N>
template <GridPrimitive P>
ind StructuredGrid<N>::NumPrimitives::getSize(const std::array<size_t, size_t(P)>& dirs) const {
    ind dirIdx = getDirectionsIndex<P>(dirs);
    return getSize<P>(dirIdx);
}

template <ind N>
template <GridPrimitive P>
constexpr ind StructuredGrid<N>::NumPrimitives::getDirectionsIndex(
    const std::array<size_t, size_t(P)>& dirs) {
    return dd_detail::directionsIndex<N, P>(dirs);
}

template <ind N>
template <GridPrimitive P>
ind StructuredGrid<N>::NumPrimitives::globalIndexFromCoordinates(
    const StructuredGrid& grid, const std::array<ind, size_t(N)>& coords,
    const std::array<size_t, size_t(P)>& dirs) const {

    ind idx = grid.numPrimitives_.getOffset(dirs);
    if (idx < 0) return idx;

    size_t mult = 1;
    auto itDir = dirs.begin();
    for (size_t dim = 0; dim < N; ++dim) {
        idx += coords[dim] * mult;
        ind dimSize = grid.numPrimitives_.NumVerticesPerDimension[dim];
        if (itDir != dirs.end() && *itDir == dim) {
            dimSize--;
            ++itDir;
        }
        mult *= dimSize;
    }
    return idx;
}

template <ind N>
template <GridPrimitive P>
std::pair<std::array<ind, size_t(N)>, std::array<size_t, size_t(P)>>
StructuredGrid<N>::NumPrimitives::coordinatesFromGlobalIndex(ind globalIdx) const {
    // Find direction offset.
    ind idx = getOffset(globalIdx);

    return std::make_pair<CoordArray, DirArray>();
}

// template <ind N>
// template <GridPrimitive P>
// StructuredGrid<N>::Primitive<P>::Primitive(const StructuredGrid<N>& grid) : Grid(grid) {}

template <ind N>
template <GridPrimitive P>
StructuredGrid<N>::Primitive<P>::Primitive(const StructuredGrid<N>& grid, ind globalIdx)
    : Grid(grid) {}

template <ind N>
template <GridPrimitive P>
StructuredGrid<N>::Primitive<P>::Primitive(const StructuredGrid<N>& grid,
                                           std::array<ind, size_t(N)> coords,
                                           std::array<size_t, size_t(P)> dirs)
    : Grid(grid)
    , Coords(coords)
    , Directions(dirs)
    , GlobalPrimitiveIndex(globalIndexFromCoordinates(grid, coords, dirs)) {}

template <ind N>
template <GridPrimitive P>
StructuredGrid<N>::Primitive<P>::Primitive(const StructuredGrid<N>& grid, ind perDirIdx,
                                           std::array<size_t, size_t(P)> dirs)
    : Grid(grid)
    , GlobalPrimitiveIndex(grid.numPrimitives_.getOffset(dirs) + perDirIdx)
    , Directions(dirs)
    , Coords(grid.numPrimitives_.coordinatesFromGlobalIndex(GlobalPrimitiveIndex)) {}

template <ind N>
template <GridPrimitive P>
void StructuredGrid<N>::Primitive<P>::getConnections(std::vector<ind>& result,
                                                     GridPrimitive toDim) const {}

template <ind N>
template <GridPrimitive From>
template <GridPrimitive To>
void StructuredGrid<N>::Primitive<From>::getConnections(std::vector<ind>& result) const {

    if constexpr (To > P) {  // Going to a higher dimensional primitive? (e.g., edge to face)

    } else {
        // std::array<ind, N - From> remainingDirections;
        auto dirSelection = dd_util::initNchooseK<To>();
        bool validSelection = 0;
        for () }
    // Coords
}

}  // namespace discretedata
}  // namespace inviwo