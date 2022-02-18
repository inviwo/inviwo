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

/**
 * \brief Extruding a grid into a new dimension
 */
class ExtrudedGrid : public Connectivity {
public:
    /**
     * \brief Create an nD grid
     * @param numVertices Number of vertices in each dimension
     */
    ExtrudedGrid(const std::shared_ptr<const Connectivity>& baseGrid, ind numExtrudedVertices)
        : Connectivity(GridPrimitive(int(baseGrid->getDimension()) + 1))
        , baseGrid_(baseGrid)
        , numExtrudedVertices_(numExtrudedVertices) {
        numGridPrimitives_[0] =
            baseGrid_->getNumElements(GridPrimitive::Vertex) * numExtrudedVertices_;
        for (int dim = 1; dim <= int(baseGrid_->getDimension()); ++dim) {
            GridPrimitive dimPrim{dim};
            numGridPrimitives_[dim] =
                baseGrid_->getNumElements(dimPrim) * numExtrudedVertices_ +
                baseGrid_->getNumElements(GridPrimitive(dim - 1)) * (numExtrudedVertices_ - 1);
        }
        numGridPrimitives_[int(gridDimension_)] =
            baseGrid_->getNumElements(GridPrimitive(int(gridDimension_) - 1)) *
            (numExtrudedVertices_ - 1);
    }

    virtual ~ExtrudedGrid() = default;

    // virtual ind getNumVerticesInDimension(ind dim) const override {

    // }

    virtual const CellStructure* getCellType(GridPrimitive dim, ind index) const override {
        return nullptr;
    }

    /** Append the indices of all primitves connected to the given index. **/
    virtual void getConnections(std::vector<ind>& result, ind indexLinear, GridPrimitive fromDim,
                                GridPrimitive toDim, bool cutAtBorder = false) const override {

        size_t resultSizeIn = result.size();
        int from = int(fromDim);
        int to = int(toDim);

        if (indexLinear < 0 || indexLinear >= numGridPrimitives_[from]) return;

        // Given a primitive in a base grid layer, get the extended primitive.
        auto addExtendedPrimitives = [&](ind baseGridIdx, ind extendedLayer, ind fromDimIndex) {
            ind baseGridElementsOffset =
                baseGrid_->getNumElements(GridPrimitive{fromDimIndex + 1}) * numExtrudedVertices_;
            ind numElementsFrom = baseGrid_->getNumElements(GridPrimitive{fromDimIndex});
            if (extendedLayer > 0)
                result.push_back(baseGridElementsOffset + numElementsFrom * (extendedLayer - 1) +
                                 baseGridIdx);
            if (extendedLayer < numExtrudedVertices_ - 1)
                result.push_back(baseGridElementsOffset + numElementsFrom * extendedLayer +
                                 baseGridIdx);
        };

        ind numBaseGridPrimitivesFrom = baseGrid_->getNumElements(fromDim);
        // (fromDim == gridDimensions_) ? 0 :
        ind numBaseGridPrimitivesTo = baseGrid_->getNumElements(toDim);
        bool isBaseGridPrimitive = indexLinear < numBaseGridPrimitivesFrom * numExtrudedVertices_;

        // Coming from an element of the base grid?
        if (isBaseGridPrimitive) {
            ind baseGridIndex = indexLinear % numBaseGridPrimitivesFrom;
            ind extendedLayer = indexLinear / numBaseGridPrimitivesFrom;

            // Get connections from base grid, offset to correct extended layer.
            baseGrid_->getConnections(result, baseGridIndex, fromDim, toDim, cutAtBorder);
            for (size_t r = resultSizeIn; r < result.size(); ++r) {
                result[r] += extendedLayer * numBaseGridPrimitivesTo;
            }
            resultSizeIn = result.size();

            // Going down in primitive dimension, so we're not leaving the base grid layer.
            if (from > to) return;

            ind baseGridElements = numBaseGridPrimitivesTo * numExtrudedVertices_;

            // Extruded from input directly?
            if (to == from + 1) {
                // if (extendedLayer > 0) {
                //     result.push_back(baseGridElements +
                //                      numBaseGridPrimitivesFrom * (extendedLayer - 1) +
                //                      baseGridIndex);
                // }
                // if (extendedLayer < numExtrudedVertices_ - 1) {
                //     result.push_back(baseGridElements + numBaseGridPrimitivesFrom * extendedLayer
                //     +
                //                      baseGridIndex);
                // }

                addExtendedPrimitives(baseGridIndex, extendedLayer, from);
                return;
            }

            // To get neighbors between layers, find the primitives we extruded from
            // (i.e., a face is made by extruding from an edge).
            std::vector<ind> baseGridConnections;
            baseGrid_->getConnections(baseGridConnections, baseGridIndex, fromDim,
                                      GridPrimitive{to - 1}, cutAtBorder);
            for (ind& primitive : baseGridConnections) {
                addExtendedPrimitives(primitive, extendedLayer, to - 1);
            }

        } else {
            std::cout << "That's complicated shit!" << std::endl;
            return;
        }
    }

    /** Get a unique identifier of this grid type. **/
    virtual const std::string& getIdentifier() const override {
        return "Extended_" + baseGrid_->getIdentifier();
    }

public:
    const std::shared_ptr<const Connectivity> baseGrid_;

protected:
    ind numExtrudedVertices_;
};

}  // namespace discretedata
}  // namespace inviwo
