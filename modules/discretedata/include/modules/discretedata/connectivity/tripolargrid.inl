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

#include <modules/discretedata/connectivity/tripolargrid.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/connectivity/elementiterator.h>
#include <modules/discretedata/channels/channeldispatching.h>

#include <inviwo/core/util/assertion.h>
#include <numeric>

namespace inviwo {
namespace discretedata {
// template <ind N>
// static const TripolarGrid<N>::std::string GRID_IDENTIFIER;

template <ind N>
TripolarGrid<N>::TripolarGrid(const std::array<ind, N>& numVertices)
    : Connectivity(static_cast<GridPrimitive>(N)), numPrimitives_(*this, numVertices) {
    // Constructing numPrimitives_ also writes total sizes to numGridPrimitives_.
}

template <ind N>
TripolarGrid<N>::TripolarGrid(std::array<ind, N>&& numVertices)
    : Connectivity(static_cast<GridPrimitive>(N)), numPrimitives_(*this, std::move(numVertices)) {
    // Constructing numPrimitives_ also writes total sizes to numGridPrimitives_.
}

template <ind N>
template <typename... IND>
TripolarGrid<N>::TripolarGrid(ind val0, IND... valX)
    : Connectivity(static_cast<GridPrimitive>(N)), numPrimitives_{*this, {val0, valX...}} {
    // Constructing numPrimitives_ also writes total sizes to numGridPrimitives_.
}
namespace dd_detail_tripolar {
template <ind N, ind From>
struct GetConnectionsFromToHelper {
    template <typename Result, ind To>
    Result operator()(const TripolarGrid<N>& grid, std::vector<ind>& result, ind idxLin,
                      bool render) {

        grid.template getConnectionsDispatched<From, To>(result, idxLin, render);
    }
};
template <ind N>
struct GetConnectionsFromHelper {
    template <typename Result, ind From>
    Result operator()(const TripolarGrid<N>& grid, std::vector<ind>& result, ind idxLin,
                      GridPrimitive to, bool render) {
        GetConnectionsFromToHelper<N, From> dispatcher;
        channeldispatching::dispatchNumber<void, 0, N>(ind(to), dispatcher, grid, result, idxLin,
                                                       render);
    }
};
}  // namespace dd_detail_tripolar

template <ind N>
void TripolarGrid<N>::getConnections(std::vector<ind>& result, ind idxLin, GridPrimitive from,
                                     GridPrimitive to, bool render) const {

    dd_detail_tripolar::GetConnectionsFromHelper<N> dispatcher;

    channeldispatching::dispatchNumber<void, 0, N>(ind(from), dispatcher, *this, result, idxLin, to,
                                                   render);
}

template <ind N>
template <ind From, ind To>
void TripolarGrid<N>::getConnectionsDispatched(std::vector<ind>& result, ind index,
                                               bool cutAtBorder) const {

    using FromPrimitive = Primitive<GridPrimitive(From)>;
    using ToPrimitive = Primitive<GridPrimitive(To)>;
    FromPrimitive fromPrim(*this, index);

    if constexpr (To > From) {  // Going to a higher dimensional primitive? (e.g., edge to face).
        throw Exception("Not implemented yet.");

    } else if constexpr (N == From && To == From) {  // Going to a neighbor.
                                                     // Connecting within the maximal dimension?
        for (ind dim = 0; dim < From; ++dim) {
            for (int sign : {-1, 1}) {
                std::array<ind, size_t(N)> neighCoords = fromPrim.getCoordinates();
                std::array<size_t, size_t(To)> neighDirs = fromPrim.getDirections();
                neighCoords[dim] += sign;

                // Wrapping along equator.
                if (dim == 1 && neighCoords[1] == -1) {
                    neighCoords[1] = numPrimitives_.NumVerticesPerDimension[1] - 2;
                }
                if (dim == 1 && neighCoords[1] == numPrimitives_.NumVerticesPerDimension[1] - 1) {
                    neighCoords[dim] = 0;
                }

                // TODOOO TODOOOOO TODODODODODODODODODODODODODODODODODODODODODODO!!!!!!!
                // // Welding together north pole.
                // // Are we at either one of the connecting cells or wrapping around?
                // if (dim == 0 && neighCoords[0] == numPrimitives_.NumVerticesPerDimension[0] - 1)
                // {
                //     ind halfSize = (numPrimitives_.NumVerticesPerDimension[1] - 1) / 2;
                //     // bool isActualCell = neighCoords[1] < halfSize;
                //     // This might be a valid cell.
                //     if (neighCoords[1] < halfSize) {

                //     }
                // }
                // TODOOO TODOOOOO TODODODODODODODODODODODODODODODODODODODODODODO!!!!!!!

                ToPrimitive neighPrim(fromPrim.Grid, std::move(neighCoords), std::move(neighDirs));
                bool inGrid = (neighPrim.getCoordinates()[dim] >= 0 &&
                               (neighPrim.getCoordinates()[dim] <
                                    numPrimitives_.NumVerticesPerDimension[dim] - 1 ||
                                dim == 1));
                if (inGrid) {
                    result.push_back(neighPrim.GlobalPrimitiveIndex);
                }
            }
        }
    } else if (To == From) {
        throw Exception("Not implemented yet.");
    } else {  // Going to an element of the original primitive.

        std::array<ind, size_t(N)> neighCoords;
        std::array<size_t, size_t(To)> neighDirs;
        std::array<size_t, size_t(From) - size_t(To)> offsetDirections;

        // For iterating through all possible offset combinations.
        std::bitset<size_t(From) - size_t(To)> posOffset;

        // Cycle through direction combinations of the neighbor.
        auto dirSelection = dd_util::initNchooseK<size_t(To)>();
        bool validDirSelection = true;
        while (validDirSelection) {
            for (size_t idx = 0; idx < size_t(To); ++idx) {
                neighDirs[idx] = fromPrim.getDirections()[dirSelection[idx]];
            }

            // Assemble the directions the primitive will not point to, but be offset by.
            auto itOffsetDirs = offsetDirections.begin();
            auto itDirSelection = dirSelection.begin();
            for (size_t dirIdx = 0; dirIdx < size_t(From); ++dirIdx) {
                if (itDirSelection != dirSelection.end() && *itDirSelection == dirIdx) {
                    itDirSelection++;
                } else {
                    *itOffsetDirs = fromPrim.getDirections()[dirIdx];
                    itOffsetDirs++;
                }
            }

            // Offset the coordinates into some directions.
            bool validBitConfig = true;

            while (validBitConfig) {
                neighCoords = fromPrim.getCoordinates();

                for (size_t bit = 0; bit < posOffset.size(); ++bit) {
                    if (!posOffset[bit]) continue;
                    size_t offsetDir = offsetDirections[bit];
                    neighCoords[offsetDir]++;

                    if (neighCoords[offsetDir] >=
                        numPrimitives_.NumVerticesPerDimension[offsetDir]) {
                        if (offsetDir == 1) {
                            neighCoords[offsetDir] = 0;
                        } else {
                            validBitConfig = false;
                            break;
                        }
                    }
                }
                if (validBitConfig) {
                    ToPrimitive neighPrim(numPrimitives_.Grid, neighCoords, neighDirs);
                    result.push_back(neighPrim.GlobalPrimitiveIndex);
                }
                validBitConfig = dd_util::nextBitset(posOffset);
            }
            validDirSelection = dd_util::nextNchooseK(ind(From), dirSelection);
        }
    }
}

template <ind N>
ind TripolarGrid<N>::getNumVerticesInDimension(ind dim) const {
    IVW_ASSERT(numPrimitives_[dim] >= 2, "Number of elements not known yet.");
    return numPrimitives_[dim];
}

template <ind N>
const std::array<ind, N>& TripolarGrid<N>::getNumVertices() const {
    return numPrimitives_.NumVerticesPerDimension;
}

template <ind N>
const CellStructure* TripolarGrid<N>::getCellType(GridPrimitive dim, ind) const {
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

namespace dd_detail_tripolar {

template <ind N, ind P>
constexpr void writeSize(typename TripolarGrid<N>::NumPrimitives& numPrimitives,
                         std::vector<ind>& numGridPrimitives) {
    auto dirs = dd_util::initNchooseK<P>();
    bool valid = true;
    size_t dirsIdx = 0;
    ind sizeSum = 0;
    while (valid) {
        numPrimitives.PerDirectionOffsets[numPrimitives.PrimitiveOffsets[P] + dirsIdx] = sizeSum;

        // Assemble size of a single direction combination:
        // multiply all sizes up, one less in dimensions our primitives span.

        auto itDir = dirs.rbegin();
        ind size = 1;
        bool wrapsTop = P > 0 && dirs[0] == 0;
        // bool wrapsSide = (P > 0 && dirs[0] == 1) || (P > 1 && dirs[1] == 1);
        for (size_t dim = N - 1; dim > 1; --dim) {
            ind dimSize = numPrimitives.NumVerticesPerDimension[dim];
            if (itDir != dirs.rend() && *itDir == dim) {
                dimSize--;
                ++itDir;
            }
            size *= dimSize;
        }

        size *= numPrimitives.NumVerticesPerDimension[1];
        ind dimSize0 = numPrimitives.NumVerticesPerDimension[0];
        if (wrapsTop) {
            sizeSum += size / 2;
            size *= dimSize0 - 1;
        } else {
            size *= dimSize0;
        }

        // auto itDir = dirs.begin();
        // ind size = 1;
        // bool wrapsTop = P > 0 && dirs[0] == 0;
        // bool wrapsSide = (P > 0 && dirs[0] == 1) || (P > 1 && dirs[1] == 1);
        // for (size_t dim = 0; dim < N; ++dim) {
        //     ind dimSize = numPrimitives.NumVerticesPerDimension[dim];
        //     if (itDir != dirs.end() && *itDir == dim) {
        //         dimSize--;
        //         ++itDir;
        //     }
        //     // When this primitive spans the longitudinal direction, there is one more layer of
        //     it. if (wrapsSide && dim == 1) {
        //         dimSize++;
        //     }
        //     size *= dimSize;
        // }

        // if (wrapsTop) {
        //     sizeSum += (sizeSum / (numPrimitives.NumVerticesPerDimension[0] - 1)) / 2;
        // }
        sizeSum += size;
        valid = dd_util::nextNchooseK(N, dirs);
        dirsIdx++;
    }
    numGridPrimitives[P] = sizeSum;
}

template <ind N, int... Is>
constexpr void writeAllSizes(typename TripolarGrid<N>::NumPrimitives& numPrimitives,
                             std::vector<ind>& numGridPrimitives,
                             std::integer_sequence<int, Is...> const&) {

    ((writeSize<N, Is>(numPrimitives, numGridPrimitives)), ...);
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
}  // namespace dd_detail_tripolar

template <ind N>
TripolarGrid<N>::NumPrimitives::NumPrimitives(
    TripolarGrid& grid, const std::array<ind, size_t(N)>& numVerticesPerDimension)
    : PerDirectionOffsets({0}), Grid(grid), NumVerticesPerDimension(numVerticesPerDimension) {

    dd_detail_tripolar::writeAllSizes<N>(*this, grid.numGridPrimitives_,
                                         std::make_integer_sequence<int, N + 1>{});
}

template <ind N>
template <GridPrimitive P>
constexpr const ind* TripolarGrid<N>::NumPrimitives::getOffset(ind dirsIdx) const {
    if (dirsIdx < 0) return nullptr;
    return &PerDirectionOffsets[PrimitiveOffsets[size_t(P)] + dirsIdx];
}

template <ind N>
template <GridPrimitive P>
constexpr const ind* TripolarGrid<N>::NumPrimitives::getOffset(
    const std::array<size_t, size_t(P)>& dirs) const {
    ind dirsIdx = getDirectionsIndex<P>(dirs);
    return getOffset<P>(dirsIdx);
}

template <ind N>
template <GridPrimitive P>
ind TripolarGrid<N>::NumPrimitives::getSize(ind dirsIdx) const {
    if (dirsIdx < 0) return -1;

    size_t primOffset = PrimitiveOffsets[size_t(P)];
    if (primOffset + dirsIdx >= PerDirectionOffsets.size()) return -1;
    // If we are looking at the last ever index range or the last of a primitive type, we will get a
    // 0 as offset. We then change it to the known number of total elements of that type.
    ind upperLimit = (primOffset + 1 == PerDirectionOffsets.size())
                         ? 0
                         : PerDirectionOffsets[primOffset + dirsIdx + 1];
    if (upperLimit == 0) upperLimit = Grid.getNumElements(P);

    return upperLimit - (PerDirectionOffsets[primOffset + dirsIdx]);
}

template <ind N>
template <GridPrimitive P>
ind TripolarGrid<N>::NumPrimitives::getSize(const std::array<size_t, size_t(P)>& dirs) const {
    ind dirIdx = getDirectionsIndex<P>(dirs);
    return getSize<P>(dirIdx);
}

template <ind N>
template <GridPrimitive P>
constexpr ind TripolarGrid<N>::NumPrimitives::getDirectionsIndex(
    const std::array<size_t, size_t(P)>& dirs) {
    return dd_detail_tripolar::directionsIndex<N, P>(dirs);
}

template <ind N>
template <GridPrimitive P>
constexpr std::array<size_t, size_t(P)> TripolarGrid<N>::NumPrimitives::getIndexDirections(
    ind dirIndex) {
    auto dirs = dd_util::initNchooseK<size_t(P)>();
    for (ind d = 0; d < dirIndex; ++d) {
        bool valid = dd_util::nextNchooseK(N, dirs);
        if (!valid)
            throw RangeException("Direction index higher than number of direction combinations.");
    }
    return dirs;
}

template <ind N>
template <GridPrimitive P>
ind TripolarGrid<N>::NumPrimitives::globalIndexFromCoordinates(
    const TripolarGrid& grid, const std::array<ind, size_t(N)>& coords,
    const std::array<size_t, size_t(P)>& dirs) const {

    ind idx = *grid.numPrimitives_.getOffset<P>(dirs);
    if (idx < 0) return idx;

    size_t mult = 1;
    auto itDir = dirs.begin();
    for (size_t dim = 0; dim < N; ++dim) {
        idx += coords[dim] * mult;
        ind dimSize = grid.numPrimitives_.NumVerticesPerDimension[dim];
        if (itDir != dirs.end() && *itDir == dim) {
            if (dim != 1) dimSize--;
            ++itDir;
        }
        mult *= dimSize;
    }
    return idx;
}

template <ind N>
template <GridPrimitive P>
std::pair<std::array<ind, size_t(N)>, std::array<size_t, size_t(P)>>
TripolarGrid<N>::NumPrimitives::coordinatesFromGlobalIndex(ind globalIdx) const {
    if (globalIdx < 0 || globalIdx >= Grid.getNumElements(P))
        throw RangeException("Index out of range for this primitive type.");

    // Find the range the index is in.
    auto dirBegin = PerDirectionOffsets.begin() + PrimitiveOffsets[size_t(P)];
    auto dirEnd = (ind(P) == N) ? PerDirectionOffsets.end()
                                : PerDirectionOffsets.begin() + PrimitiveOffsets[size_t(P) + 1];
    auto dirPtr = std::upper_bound(dirBegin, dirEnd, globalIdx);
    dirPtr--;
    size_t dirIdx = dirPtr - dirBegin;
    ind localIndex = globalIdx - *dirPtr;

    DirArray<P> dirs = getIndexDirections<P>(dirIdx);

    CoordArray coords;
    // size_t mult = 1;
    auto itDir = dirs.begin();
    for (size_t dim = 0; dim < N; ++dim) {
        // idx += coords[dim] * mult;
        ind dimSize = Grid.numPrimitives_.NumVerticesPerDimension[dim];
        if (itDir != dirs.end() && *itDir == dim) {
            if (dim != 1) dimSize--;
            ++itDir;
        }
        // mult *= dimSize;
        coords[dim] = localIndex % dimSize;
        localIndex /= dimSize;
    }
    return std::make_pair(std::move(coords), std::move(dirs));
}

// template <ind N>
// template <GridPrimitive P>
// TripolarGrid<N>::Primitive<P>::Primitive(const TripolarGrid<N>& grid) : Grid(grid) {}

template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P>::Primitive(const TripolarGrid<N>& grid, ind globalIdx)
    : Grid(grid), GlobalPrimitiveIndex(globalIdx) {
    auto coordDirPair = grid.numPrimitives_.template coordinatesFromGlobalIndex<P>(globalIdx);
    Coords = coordDirPair.first;
    Directions = coordDirPair.second;
}

template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P>::Primitive(const TripolarGrid<N>& grid,
                                         std::array<ind, size_t(N)> coords,
                                         std::array<size_t, size_t(P)> dirs)
    : Grid(grid)
    , GlobalPrimitiveIndex(
          grid.numPrimitives_.template globalIndexFromCoordinates<P>(grid, coords, dirs))
    , Coords(coords)
    , Directions(dirs) {}

// template <ind N>
// template <GridPrimitive P>
// TripolarGrid<N>::Primitive<P>::Primitive(const TripolarGrid<N>& grid,
//                                                std::array<ind, size_t(N)>&& coords,
//                                                std::array<size_t, size_t(P)>&& dirs)
//     : Grid(grid)
//     , GlobalPrimitiveIndex(
//           grid.numPrimitives_.template globalIndexFromCoordinates<P>(grid, coords, dirs))
//     , Coords(std::move(coords))
//     , Directions(std::move(dirs)) {}

template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P>::Primitive(const TripolarGrid<N>& grid, ind perDirIdx,
                                         std::array<size_t, size_t(P)> dirs)
    : Grid(grid)
    , GlobalPrimitiveIndex(grid.numPrimitives_.getOffset(dirs) + perDirIdx)
    , Coords(grid.numPrimitives_.template coordinatesFromGlobalIndex<P>(GlobalPrimitiveIndex))
    , Directions(dirs) {}

template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P>::Primitive(const Primitive<P>& prim)
    : Grid(prim.Grid)
    , GlobalPrimitiveIndex(prim.GlobalPrimitiveIndex)
    , Coords(prim.getCoordinates())
    , Directions(prim.getDirections()) {}

template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P>::Primitive(const Primitive<P>&& prim)
    : Grid(prim.Grid)
    , GlobalPrimitiveIndex(prim.GlobalPrimitiveIndex)
    , Coords(std::move(prim.getCoordinates()))
    , Directions(std::move(prim.getDirections())) {}

template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P> TripolarGrid<N>::getPrimitive(ind globalIdx) const {
    return Primitive<P>(*this, globalIdx);
}
template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P> TripolarGrid<N>::getPrimitive(
    std::array<ind, size_t(N)> coords, std::array<size_t, size_t(P)> dirs) const {
    return Primitive<P>(*this, coords, dirs);
}

// template <ind N>
// template <GridPrimitive P>
// TripolarGrid<N>::Primitive<P> TripolarGrid<N>::getPrimitive(
//     std::array<ind, size_t(N)>&& coords, std::array<size_t, size_t(P)>&& dirs) const {
//     return Primitive<P>(*this, std::move(coords), std::move(dirs));
// }

template <ind N>
template <GridPrimitive P>
TripolarGrid<N>::Primitive<P> TripolarGrid<N>::getPrimitive(
    ind perDirIdx, std::array<size_t, size_t(P)> dirs) const {
    return Primitive<P>(*this, perDirIdx, dirs);
}

// template <ind N>
// template <GridPrimitive To>
// bool StructuredGridConnections<N>::handleBorder(
//     typename TripolarGrid<N, StructuredGridConnections<N>>::template Primitive<To>& prim,
//     bool render) {
//     // using ToPrimitive =
//     //     typename TripolarGrid<N, StructuredGridConnections<N>>::template Primitive<To>;
//     return false;
// }

}  // namespace discretedata
}  // namespace inviwo