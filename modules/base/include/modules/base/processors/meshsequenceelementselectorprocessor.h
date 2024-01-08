/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/mesh.h>                // for Mesh
#include <inviwo/core/ports/meshport.h>                              // for MeshOutport
#include <inviwo/core/ports/outportiterable.h>                       // for OutportIterable
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/util/glmvec.h>                                 // for uvec3
#include <modules/base/processors/vectorelementselectorprocessor.h>  // for VectorElementSelecto...

#include <string>  // for string
#include <vector>  // for vector

#include <fmt/core.h>    // for format, format_to
#include <glm/vec3.hpp>  // for operator+

namespace inviwo {

/** \docpage{org.inviwo.MeshTimeStepSelector, Mesh Sequence Element Selector}
 * ![](org.inviwo.MeshTimeStepSelector.png?classIdentifier=org.inviwo.MeshTimeStepSelector)
 *
 * Select a specific volume out of a sequence of meshs
 *
 * ### Inport
 *   * __inport__ Sequence of meshs
 * ### Outport
 *   * __outport__ Selected mesh
 *
 * ### Properties
 *   * __Step__ The mesh sequence index to extract
 */

class IVW_MODULE_BASE_API MeshSequenceElementSelectorProcessor
    : public VectorElementSelectorProcessor<Mesh, MeshOutport> {
public:
    MeshSequenceElementSelectorProcessor();
    virtual ~MeshSequenceElementSelectorProcessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
};

}  // namespace inviwo
