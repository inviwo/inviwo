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

#pragma once

#include "discretedata/connectivity/connectivity.h"
#include "discretedata/util.h"

namespace inviwo {
namespace dd {

class ElementIterator;

/** \class StructuredGrid
    \brief A curvilinear grid in nD

    @author Anke Friederici and Tino Weinkauf
*/
class IVW_MODULE_DISCRETEDATA_API StructuredGrid : public Connectivity {
    // Construction / Deconstruction
public:
    /** \brief Create an nD grid
    *   @param gridDimension Dimension of grid (not vertices)
    *   @param gridSize Number of cells in each dimension, expect size gridDimension+1
    */
    StructuredGrid(GridPrimitive gridDimension, const std::vector<ind>& numCellsPerDim);
    virtual ~StructuredGrid() = default;

    ind getNumCellsInDimension(ind dim) const;

    virtual double getPrimitiveMeasure(GridPrimitive dim, ind index) const override;

    // Methods
public:
    virtual std::vector<ind> getConnections(ind index, GridPrimitive from, GridPrimitive to) const;

    // Attributes
protected:
    template <typename T>
    double computeHexVolume(ind index) const;

protected:
    std::vector<ind> numCellsPerDimension_;
};

template <typename T>
double StructuredGrid::computeHexVolume(ind index) const {
    // Work with respective type
    std::shared_ptr<const DataChannel<T>> doubleVertices =
        std::dynamic_pointer_cast<const DataChannel<T>, const Channel>(vertices_);
    if (!doubleVertices) return -1;

    // Get all corner points.
    auto corners = getConnections(index, GridPrimitive::Volume, GridPrimitive::Vertex);
    IVW_ASSERT(corners.size() == 8, "Not a hexahedron.");

    // Tetrahedron corners
    static constexpr ind tetrahedra[5][4] = {
        {0, 3, 5, 6}, {1, 0, 3, 5}, {0, 2, 6, 3}, {5, 3, 6, 7}, {4, 5, 6, 0}};

    // Setup variables for measure calculation
    double measure = 0;
    double cornerMatrix[4][3];
    T vertex[3];

    // Calculate measure of 5 tetrahedra
    for (ind tet = 0; tet < 5; tet++) {
        for (ind corner = 0; corner < 4; corner++) {
            ind cornerIndex = corners[tetrahedra[tet][corner]];
            doubleVertices->fill(vertex, cornerIndex);
            for (ind dim = 0; dim < 3; ++dim) {
                cornerMatrix[corner][dim] = double(vertex[dim]);
            }
        }

        // Compute measure and sum
        measure += util::tetrahedronVolume(cornerMatrix);
    }

    return measure;
}

}  // namespace
}