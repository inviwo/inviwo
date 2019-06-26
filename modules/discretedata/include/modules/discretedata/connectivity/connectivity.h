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
#include <inviwo/core/common/inviwo.h>

#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/connectivity/cell.h>
#include <modules/discretedata/connectivity/elementiterator.h>

namespace inviwo {
namespace discretedata {

/** \class Connectivity
 *   \brief Basis interface of all connectivity types.
 *
 *   The connectivity between coordinates creates a mesh,
 *   a graph, a cell complex, or some other structure.
 *   It joins coordinates to a larger entity
 *   and puts them in a neighborhood relation.
 *
 *   A connectivity can define several neighborhood relations.
 *   For example, a uniform grid would have 0D vertices,
 *   1D grid edges, 2D grid faces, and 3D grid cells/voxels.
 *   The connectivity defines these neighborhood relations
 *   and allows iterating over them.
 *
 *   This refers to the 'topological space'
 *   in <em>Vector-Bundle Classes Form Powerful Tool for Scientific Visualization</em>,
 *   Butler and Bryson, 1992.
 *
 *  Connectivity and Coordinates jointly define the manifold
 *   in the above paper.
 *
 *   @author Tino Weinkauf and Anke Friederici
 */
class IVW_MODULE_DISCRETEDATA_API Connectivity {
public:
    Connectivity(GridPrimitive gridDimension)
        : gridDimension_(gridDimension)
        , numGridPrimitives_(static_cast<ind>(gridDimension) + 1, -1) {
    }  // Initialize sizes with -1. Overwrite when known.
    virtual ~Connectivity() = default;

    //! Returns the maximal dimension of the grid
    GridPrimitive getDimension() const { return gridDimension_; }

    /**
     * \brief Return the number of elements of the given type
     * @param elementType Type to get number of
     * @return Number of elements
     */
    virtual ind getNumElements(GridPrimitive elementType) const;

    /**
     * \brief Get the map from one element to another
     * E.g. cell to its vertices, vertex to its neighbors, vertex to connected faces
     * @param result All connected indices in dimension 'to'
     * @param index Index of element in dimension 'from'
     * @param from Dimension the index lives in
     * @param to Dimension the result lives in
     * @param isPosition
     */
    virtual void getConnections(std::vector<ind>& result, ind index, GridPrimitive from,
                                GridPrimitive to, bool isPosition = false) const = 0;

    /**
     * \brief Range of all elements to iterate over
     * @param dim Dimension to return the elements of
     */
    ElementRange all(GridPrimitive dim) const;

    /**
     * \brief Get the cell type (i.e. triangle, quad...; tetrahedron, voxel...)
     * @param dim Dimension of element (edge, face, volume...)
     * @param index Index of respective element type
     */
    virtual CellType getCellType(GridPrimitive dim, ind index) const;

    /**
     * \brief Get the cell type (i.e. triangle, quad...; tetrahedron, voxel...)
     * @param element Element to get measure of
     */
    virtual CellType getCellType(ElementIterator& element) const;

    // Attributes
protected:
    //! Highest dimension of GridPrimitives
    GridPrimitive gridDimension_;

    //! Saves the known number of primitves
    mutable std::vector<ind> numGridPrimitives_;
};

}  // namespace discretedata
}  // namespace inviwo
