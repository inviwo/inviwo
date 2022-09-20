/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for ConnectivityType, DrawType
#include <inviwo/core/ports/bufferport.h>                      // for BufferInport
#include <inviwo/core/ports/meshport.h>                        // for MeshOutport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty
#include <inviwo/core/util/staticstring.h>                     // for operator+

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operat...

namespace inviwo {

/**
 * @brief Put buffers into mesh without copying data.
 * Supported buffer types are specified in BufferType
 */
class IVW_MODULE_BASE_API BufferToMeshProcessor : public Processor {
public:
    BufferToMeshProcessor();
    virtual ~BufferToMeshProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // See BufferType for supported types
    BufferInport vertices_;
    BufferInport indices_;             // Optional
    BufferInport vertexColors_;        // Optional
    BufferInport textureCoordinates_;  // Optional
    BufferInport normals_;             // Optional
    BufferInport curvature_;           // Optional
    MeshOutport outport_;

    OptionProperty<DrawType> drawType_;
    OptionProperty<ConnectivityType> connectivity_;
};

}  // namespace inviwo
