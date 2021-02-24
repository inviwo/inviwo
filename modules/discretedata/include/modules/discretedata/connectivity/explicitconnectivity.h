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

#pragma once

#include <modules/discretedata/discretedatamoduledefine.h>
#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/util/util.h>

namespace inviwo {
namespace discretedata {

using ConnectionMap = std::shared_ptr<dd_util::PrefixSumVector<ind>>;

/** \class ExplicitConnectivity
 *   \brief A connectivity computing connection maps on demand
 */
class IVW_MODULE_DISCRETEDATA_API ExplicitConnectivity : Connectivity {
public:
    ExplicitConnectivity(GridPrimitive gridDimension) : Connectivity(gridDimension){};

    /**
     * \brief Return the number of elements of the given type
     * @param elementType Type to get number of
     * @return Number of elements
     */
    virtual ind getNumElements(GridPrimitive elementType = GridPrimitive::Vertex) const override;

    /**
     * \brief Get the map from one element to another
     * E.g. cell to its vertices, vertex to its neighbors, vertex to connected faces
     * @param result All connected indices in dimension 'to'
     * @param index Index of element in dimension 'from'
     * @param from Dimension the index lives in
     * @param to Dimension the result lives in
     * @param isPosition Answer might depend on whether the undelying data is a position
     */
    virtual void getConnections(std::vector<ind>& result, ind index, GridPrimitive from,
                                GridPrimitive to, bool isPosition = false) const override;

    ConnectionMap createConnectionMap(GridPrimitive from, GridPrimitive to) const;

    // void getCellConnectionByVertexIndex(dd_util::PrefixSumVector<ind>, ind index,
    //                                     GridPrimitive from, GridPrimitive to);

    void clearMemory() { connectionMaps_.clear(); }

private:
    void createConnectionMapToAndFromVertex(GridPrimitive primitive) const;
    void createInverseConnectionMap(GridPrimitive from, GridPrimitive to) const;

    // Attributes
protected:
    struct PrimitivePairCompare {
        bool operator()(const std::pair<GridPrimitive, GridPrimitive>& u,
                        const std::pair<GridPrimitive, GridPrimitive>& v) const {
            return (u.first < v.first) || (u.first == v.first && u.second < v.second);
        }
    };

    // Map of maps from one primitive type to another
    mutable std::map<std::pair<GridPrimitive, GridPrimitive>, ConnectionMap, PrimitivePairCompare>
        connectionMaps_;

public:
    inline static const std::string GRID_IDENTIFIER = "ExplicitConnectivity";
    /** Get a unique identifier of this grid type. **/
    virtual const std::string& getIdentifier() const override { return GRID_IDENTIFIER; }
};

}  // namespace discretedata
}  // namespace inviwo
