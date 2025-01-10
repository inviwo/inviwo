/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/interaction/cameratrackball.h>          // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/ports/meshport.h>                       // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>              // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>            // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>         // for CompositeProperty
#include <inviwo/core/properties/ordinalproperty.h>           // for FloatProperty, FloatVec4Pro...
#include <inviwo/core/properties/simplelightingproperty.h>    // for SimpleLightingProperty
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <modules/basegl/datastructures/meshshadercache.h>    // for MeshShaderCache::Requirement
#include <modules/opengl/shader/shadertype.h>                 // for ShaderType

#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

namespace inviwo {
class Shader;

class IVW_MODULE_BASEGL_API TubeRendering : public Processor {
public:
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    TubeRendering();
    virtual ~TubeRendering() = default;

    virtual void process() override;

    virtual void initializeResources() override;

protected:
    void configureShader(Shader& shader);

    MeshFlatMultiInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    CompositeProperty tubeProperties_;
    BoolProperty forceRadius_;
    FloatProperty defaultRadius_;
    BoolProperty forceColor_;
    FloatVec4Property defaultColor_;
    BoolProperty useMetaColor_;
    TransferFunctionProperty metaColor_;

    CameraProperty camera_;
    CameraTrackball trackball_;
    SimpleLightingProperty lighting_;
    std::vector<std::pair<ShaderType, std::string>> shaderItems_;
    std::vector<MeshShaderCache::Requirement> shaderRequirements_;
    MeshShaderCache adjacencyShaders_;
    MeshShaderCache shaders_;
};

}  // namespace inviwo
