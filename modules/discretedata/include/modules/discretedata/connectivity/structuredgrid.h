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
#include <modules/discretedata/util/util.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <inviwo/core/datastructures/spatialdata.h>

#include <initializer_list>

namespace inviwo {
namespace discretedata {

class ElementIterator;

/**
 * \brief A curvilinear grid in nD
 * N needs to be positive.
 */
template <ind N, typename GetConnections>
class CurvilinearGrid : public Connectivity {
public:
    /**
     * \brief Create an nD grid
     * @param numVertices Number of vertices in each dimension
     */
    CurvilinearGrid(const std::array<ind, N>& numVertices);

    /**
     * \brief Create an nD grid
     * @param numVertices Number of vertices in each dimension
     */
    CurvilinearGrid(std::array<ind, N>&& numVertices);

    /**
     * \brief Create an nD grid
     * @param val0 Required size of first dimension
     * @param valX Further N-1 sizes
     */
    template <typename... IND>
    CurvilinearGrid(ind val0, IND... valX);

    virtual ~CurvilinearGrid() = default;

    virtual ind getNumVerticesInDimension(ind dim) const;

    const std::array<ind, N>& getNumVertices() const;

    virtual const CellStructure* getCellType(GridPrimitive dim, ind index) const override;

    /** Append the indices of all primitves connected to the given index. **/
    virtual void getConnections(std::vector<ind>& result, ind indexLinear, GridPrimitive from,
                                GridPrimitive to, bool cutAtBorder = false) const override;

    /** Append the indices of all primitves connected to the given index. Templated. **/
    template <ind From, ind To>
    void getConnectionsDispatched(std::vector<ind>& result, ind indexLinear,
                                  bool cutAtBorder) const;

    // static void sameLevelConnection(std::vector<ind>& result, ind idxLin,
    //                                 const std::array<ind, N>& size);

    // static std::array<ind, N> indexFromLinear(ind idxLin, const std::array<ind, N>& size);

    // static ind indexToLinear(const std::array<ind, N>& idx, const std::array<ind, N>& size);

private:
    void calculateSizes();

public:
    using CoordArray = std::array<ind, size_t(N)>;
    template <GridPrimitive P>
    using DirArray = std::array<size_t, size_t(P)>;

    template <GridPrimitive P>
    struct Primitive {
        friend class CurvilinearGrid<N, GetConnections>;

        // private:
        //     Primitive(const CurvilinearGrid<N>& grid);

    public:
        Primitive(const CurvilinearGrid<N, GetConnections>& grid, ind globalIdx);
        Primitive(const CurvilinearGrid<N, GetConnections>& grid, CoordArray idx, DirArray<P> dirs);
        Primitive(const CurvilinearGrid<N, GetConnections>& grid, ind perDirIdx, DirArray<P> dirs);
        Primitive(const Primitive<P>& prim);
        Primitive(const Primitive<P>&& prim);

        // Copy assignment by placement new copy construction.
        void operator=(const Primitive& prim) { new (this) Primitive(prim); }
        void operator=(const Primitive&& prim) { new (this) Primitive(prim); }

        const CurvilinearGrid& Grid;
        const ind GlobalPrimitiveIndex;

    private:
        CoordArray Coords;
        DirArray<P> Directions;

    public:
        const CoordArray& getCoordinates() const { return Coords; }
        const DirArray<P>& getDirections() const { return Directions; }
    };

    template <GridPrimitive P>
    Primitive<P> getPrimitive(ind globalIdx) const;
    template <GridPrimitive P>
    Primitive<P> getPrimitive(CoordArray coords, DirArray<P> dirs) const;
    // template <GridPrimitive P>
    // Primitive<P> getPrimitive(CoordArray&& coords, DirArray<P>&& dirs) const;
    template <GridPrimitive P>
    Primitive<P> getPrimitive(ind perDirIdx, DirArray<P> dirs) const;

public:
    struct NumPrimitives {

        NumPrimitives(const CurvilinearGrid& grid, const CoordArray& numVerticesPerDimension);

        template <GridPrimitive P>
        constexpr static ind getDirectionsIndex(const DirArray<P>& dirs);

        template <GridPrimitive P>
        constexpr static DirArray<P> getIndexDirections(ind dirIndex);

        // template <>
        // constexpr static ind getDirectionsIndex<GridPrimitive::Vertex>(
        //     const DirArray<GridPrimitive::Vertex>&) {
        //     return 0;
        // }

        template <GridPrimitive P>
        ind globalIndexFromCoordinates(const CurvilinearGrid& grid, const CoordArray& coords,
                                       const DirArray<P>& dirs) const;

        template <GridPrimitive P>
        std::pair<CoordArray, DirArray<P>> coordinatesFromGlobalIndex(ind globalIdx) const;

        // template <GridPrimitive P>
        // constexpr ind getPrimitiveOffset() constexpr;
        template <GridPrimitive P>
        constexpr const ind* getOffset(ind dirsIdx) const;
        template <GridPrimitive P>
        constexpr const ind* getOffset(const DirArray<P>& dirs) const;

        template <GridPrimitive P>
        ind getSize(ind dirsIdx) const;
        template <GridPrimitive P>
        ind getSize(const DirArray<P>& dirs) const;

        ind operator[](size_t idx) const { return NumVerticesPerDimension[idx]; }

        // private:
        // /** The global index offsets for primitives with the respective direction combination.
        //  * Starts at 0 at all indices indicated by DirectionOffsets. **/
        std::array<ind, (1 << N)> PerDirectionOffsets;

        /** The offsets into NumPrimitivesSum per primitive type. **/
        static constexpr typename std::array<size_t, N + 1> PrimitiveOffsets =
            dd_util::binomialCoefficientOffsets<N>();

        const CurvilinearGrid& Grid;

        /** The number of vertices in each dimension. **/
        std::array<ind, N> NumVerticesPerDimension;
    };
    NumPrimitives const numPrimitives_;
};

// // Template type deduction guides.
// template <ind N, typename GetConnections, GridPrimitive P>
// CurvilinearGrid<N, GetConnections>::Primitive(const CurvilinearGrid<N, GetConnections>&,
// CoordArray,
//                                               DirArray<P>)
//     ->Primitive<static_cast<GridPrimitive>(DirArray<P>::N)>;

// template <ind N, typename GetConnections>
// template <GridPrimitive P>
// Primitive(const CurvilinearGrid<N, GetConnections>&, ind, DirArray<P>)
//     -> Primitive<static_cast<GridPrimitive>(DirArray<P>::N)>;

template <ind N>
struct StructuredGridConnections {
    static

        template <GridPrimitive To>
        static bool handleBorder(
            typename CurvilinearGrid<N, StructuredGridConnections<N>>::template Primitive<To>&
                prim) {
        // return false;
        auto coords = prim.getCoordinates();
        auto dirs = prim.getDirections();
        auto dirBits = dd_util::indicesToBitset<size_t(N), size_t(To)>(dirs);

        ind ySize = prim.Grid.numPrimitives_.NumVerticesPerDimension[1] - (dirBits[1] ? 1 : 0);
        if (coords[1] < 0) {
            coords[1] = ySize = 1;
        } else if (coords[1] >= ySize) {
            coords[1] = 0;
        } else {
            return false;
        }
        prim = typename CurvilinearGrid<N, StructuredGridConnections<N>>::template Primitive<To>(
            prim.Grid, coords, dirs);
        return true;
    }
};

template <ind N>
using StructuredGrid = CurvilinearGrid<N, StructuredGridConnections<N>>;

// Making use of Matrix<N + 1, float> StructuredGridEntity<N>::getIndexMatrix() const.
template <typename T, ind N, typename Vec = glm::vec<N, T>>
struct IVW_MODULE_DISCRETEDATA_API CurvilinearPositions {
    using VecTN = glm::vec<N, T>;
    using VecIN = glm::vec<N, ind>;
    using VecTNp = glm::vec<N + 1, ind>;
    using MatTNp = Matrix<N + 1, T>;
    static_assert(N <= 3 && "GLM types only support up till 4 dimensions.");

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

#include "structuredgrid.inl"
