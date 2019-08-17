/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadataowner.h>

#include <modules/discretedata/discretedatatypes.h>

namespace inviwo {
namespace discretedata {

/**
 * \brief An untyped scalar or vector component of a data set.
 *
 * General version of a DataChannel for use in general containers
 * (see DataSet).
 *
 * @author Anke Friederici and Tino Weinkauf
 */
class IVW_MODULE_DISCRETEDATA_API Channel : public MetaDataOwner {
public:
    /**
     * \brief Direct construction
     * @param numComponents Size of vector at each position
     * @param name Name associated with the channel
     * @param dataFormat Data format
     * @param definedOn GridPrimitive the data is defined on, default: 0D vertices
     */
    Channel(ind numComponents, const std::string& name, DataFormatId dataFormat,
            GridPrimitive definedOn = GridPrimitive::Vertex);

    virtual ~Channel() = default;

    const std::string getName() const;

    void setName(const std::string&);

    GridPrimitive getGridPrimitiveType() const;

    DataFormatId getDataFormatId() const;

    ind getNumComponents() const;

    virtual ind size() const = 0;

protected:
    /**
     * Sets the "GridPrimitiveType" meta data
     * Should be constant, only DataSet is allowed to write.
     */
    void setGridPrimitiveType(GridPrimitive);

    void setDataFormatId(DataFormatId);

    /**
     * Sets the "NumComponents" meta data
     * Should be constant, only DataSet is allowed to write.
     */
    void setNumComponents(ind);

private:
    std::string name_;
    const DataFormatBase* format_;
    GridPrimitive grid_;
    ind numComponents_;
};

}  // namespace discretedata
}  // namespace inviwo
