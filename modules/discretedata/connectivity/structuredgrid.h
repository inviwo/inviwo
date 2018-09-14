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

    virtual ind getNumCellsInDimension(ind dim) const;

    void getNumCells(std::vector<ind>& result) const;

    virtual double getPrimitiveMeasure(GridPrimitive dim, ind index) const override;

    // Methods
public:
    virtual void getConnections(std::vector<ind>& result, ind index, GridPrimitive from,
                                GridPrimitive to) const;

    static void sameLevelConnection(std::vector<ind>& result, ind idxLin,
                                    const std::vector<ind>& size);

    static std::vector<ind> indexFromLinear(ind idxLin, const std::vector<ind>& size);

    // Attributes
protected:
    struct HexVolumeComputer {
        HexVolumeComputer(const StructuredGrid* p) : parent_(p) {}

        template <typename R, typename T>
        double operator()(ind index) const;

        const StructuredGrid* parent_;
    };

    template <typename T>
    double ComputeHexVolume(const std::vector<ind>& connections) const { return -1.0; }

protected:
    std::vector<ind> numCellsPerDimension_;
};

template <typename R, typename T>
double StructuredGrid::HexVolumeComputer::operator()(ind index) const {

    // Get all corner points.
    std::vector<ind> corners;
    parent_->getConnections(corners, index, GridPrimitive::Volume, GridPrimitive::Vertex);
    return parent_->ComputeHexVolume<T>(corners);
}

}  // namespace dd
}  // namespace inviwo