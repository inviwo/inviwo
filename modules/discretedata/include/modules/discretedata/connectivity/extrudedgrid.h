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
    ExtrudedGrid(const std::shared_ptr<const Connectivity>& baseGrid, ind numExtrudedVertices);

    virtual ~ExtrudedGrid() = default;

    virtual const CellStructure* getCellType(GridPrimitive dim, ind index) const override {
        return nullptr;
    }

    /** Append the indices of all primitves connected to the given index. **/
    virtual void getConnections(std::vector<ind>& result, ind indexLinear, GridPrimitive fromDim,
                                GridPrimitive toDim, bool cutAtBorder = false) const override;

    /** Get a unique identifier of this grid type. **/
    virtual const std::string& getIdentifier() const override { return identifier_; }

public:
    const std::shared_ptr<const Connectivity> baseGrid_;
    const ind numExtrudedVertices_;

protected:
    const std::string identifier_;
};

}  // namespace discretedata
}  // namespace inviwo
