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

#include <modules/discretedata/connectivity/explicitconnectivity.h>
#include <modules/discretedata/util/util.h>
#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/bufferchannel.h>
#include <inviwo/core/datastructures/spatialdata.h>

#include <initializer_list>

namespace inviwo {
namespace discretedata {

class ElementIterator;

/**
 * \brief A grid of triangles
 */
class TriangleMesh : public ExplicitConnectivity {
public:
    /**
     * \brief Create the mesh
     * @param numVertices Number of vertices
     * @param indices Triangle list
     */
    TriangleMesh(ind numVertices, std::shared_ptr<const DataChannel<ind, 3>> triangleList);

    /**
     * \brief Create the mesh
     * @param numVertices Number of vertices
     * @param indices Triangle list, in packs of 3
     */
    TriangleMesh(ind numVertices, const std::vector<ind>& triangleList);

    virtual ~TriangleMesh() = default;

    virtual const CellStructure* getCellType(GridPrimitive, ind) const override {
        return CellStructureByCellType[(int)CellType::Triangle];
    }

protected:
    void setBaseMap();

    ind numVertices_;
    std::shared_ptr<const DataChannel<ind, 3>> triangleList_;
};

}  // namespace discretedata
}  // namespace inviwo
