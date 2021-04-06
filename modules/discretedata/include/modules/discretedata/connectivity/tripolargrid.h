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

#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/util/util.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <inviwo/core/datastructures/spatialdata.h>

#include <initializer_list>

namespace inviwo {
namespace discretedata {

class ElementIterator;

/**
 * \brief A tripolar grid used for ocean simulations
 * Extends to nD, N being at least 2 for latitude and longitude.
 */
template <ind N>
class TripolarGrid : public Connectivity {
public:
    /**
     * \brief Create an nD grid
     * @param numVertices Number of vertices in each dimension
     */
    TripolarGrid(const std::array<ind, N>& numVertices);

    /**
     * \brief Create an nD grid
     * @param numVertices Number of vertices in each dimension
     */
    TripolarGrid(std::array<ind, N>&& numVertices);

    /**
     * \brief Create an nD grid
     * @param val0 Required size of first dimension
     * @param valX Further N-1 sizes
     */
    template <typename... IND>
    TripolarGrid(ind val0, IND... valX);

    virtual ~TripolarGrid() = default;

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
        friend class TripolarGrid<N>;

    public:
        Primitive(const TripolarGrid<N>& grid, ind globalIdx);
        Primitive(const TripolarGrid<N>& grid, CoordArray idx, DirArray<P> dirs);
        Primitive(const TripolarGrid<N>& grid, ind perDirIdx, DirArray<P> dirs);
        Primitive(const Primitive<P>& prim);
        Primitive(const Primitive<P>&& prim);

        // Copy assignment by placement new copy construction.
        void operator=(const Primitive& prim) { new (this) Primitive(prim); }
        void operator=(const Primitive&& prim) { new (this) Primitive(prim); }

        const TripolarGrid& Grid;
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

        NumPrimitives(TripolarGrid& grid, const CoordArray& numVerticesPerDimension);

        template <GridPrimitive P>
        constexpr static ind getDirectionsIndex(const DirArray<P>& dirs);

        template <GridPrimitive P>
        constexpr static DirArray<P> getIndexDirections(ind dirIndex);

        template <GridPrimitive P>
        ind globalIndexFromCoordinates(const TripolarGrid& grid, const CoordArray& coords,
                                       const DirArray<P>& dirs) const;

        template <GridPrimitive P>
        std::pair<CoordArray, DirArray<P>> coordinatesFromGlobalIndex(ind globalIdx) const;

        template <GridPrimitive P>
        constexpr const ind* getOffset(ind dirsIdx) const;
        template <GridPrimitive P>
        constexpr const ind* getOffset(const DirArray<P>& dirs) const;

        template <GridPrimitive P>
        ind getSize(ind dirsIdx) const;
        template <GridPrimitive P>
        ind getSize(const DirArray<P>& dirs) const;

        ind operator[](size_t idx) const { return NumVerticesPerDimension[idx]; }

        /** The global index offsets for primitives with the respective direction combination.
         * Starts at 0 at all indices indicated by DirectionOffsets. **/
        std::array<ind, (1 << N)> PerDirectionOffsets;

        /** The number of "normal" cells per dimension, i.e. without the ones wrapping the pole. **/
        std::array<ind, (1 << N)> PerDirectionNumNormalPrimitives;

        /** The offsets into NumPrimitivesSum per primitive type. **/
        static constexpr typename std::array<size_t, N + 1> PrimitiveOffsets =
            dd_util::binomialCoefficientOffsets<N>();

        const TripolarGrid& Grid;

        // std::array<ind, N> NumVerticesRegularGrid;

        /** The number of vertices in each dimension. **/
        std::array<ind, N> NumVerticesPerDimension;
    };
    const NumPrimitives numPrimitives_;

    inline static const std::string GRID_IDENTIFIER = "TripolarGrid" + std::to_string(N) + "D";
    /** Get a unique identifier of this grid type. **/
    virtual const std::string& getIdentifier() const override { return GRID_IDENTIFIER; }
};

}  // namespace discretedata
}  // namespace inviwo

#include "tripolargrid.inl"
