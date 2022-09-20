/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/mesh.h>  // for Mesh
#include <inviwo/core/ports/dataoutport.h>             // for DataOutport
#include <inviwo/core/ports/meshport.h>                // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>     // for OptionProperty
#include <inviwo/core/util/staticstring.h>             // for operator+

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for operator==, operator+, string
#include <string_view>  // for operator==
#include <vector>       // for vector, operator!=, operator==

#include <fmt/core.h>    // for format
#include <glm/vec3.hpp>  // for operator+

namespace inviwo {

/** \docpage{org.inviwo.MeshConverterProcessor, Mesh Converter Processor}
 * ![](org.inviwo.MeshConverterProcessor.png?classIdentifier=org.inviwo.MeshConverterProcessor)
 * Convert a mesh into either a point mesh or a line mesh.
 *
 * ### Inports
 *   * __inport__ Input meshes
 *
 * ### Outports
 *   * __outport__ Transformed meshes
 *
 * ### Properties
 *   * __type__ Conversion type, lines or points.
 */
class IVW_MODULE_BASE_API MeshConverterProcessor : public Processor {
public:
    MeshConverterProcessor();
    virtual ~MeshConverterProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    enum class Type { ToPoints, ToLines };

    MeshFlatMultiInport inport_;
    DataOutport<std::vector<std::shared_ptr<Mesh>>> outport_;
    OptionProperty<Type> type_;
};

}  // namespace inviwo
