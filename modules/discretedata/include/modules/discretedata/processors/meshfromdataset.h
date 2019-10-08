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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <type_traits>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/meshram.h>

#include <modules/discretedata/ports/datasetport.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.MeshFromDataSet, Mesh From Data Set}
    ![](org.inviwo.MeshFromDataSet.png?classIdentifier=org.inviwo.MeshFromDataSet)

    Converts a DataSet into a Mesh.

    ### Inports
      * __InDataSet__ Input DataSet to be converted.

    ### Outports
      * __OutMesh__ Converted Mesh.
*/

/** \class MeshFromDataSet
    \brief Converts a DataSet to a Mesh
*/
class IVW_MODULE_DISCRETEDATA_API MeshFromDataSet : public Processor {
    // Construction / Deconstruction
public:
    MeshFromDataSet();
    virtual ~MeshFromDataSet() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

    // Ports
public:
    /// Input dataset
    DataSetInport portInDataSet_;

    /// Output Mesh
    MeshOutport portOutMesh_;

    DataChannelProperty positionChannel_, colorChannel_;

    GridPrimitiveProperty primitive_;
};

}  // namespace discretedata
}  // namespace inviwo
