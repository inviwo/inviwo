/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/processors/2d/vector2dmagnitude.h>

#include <inviwo/core/datastructures/image/imagetypes.h>  // for ImageType, ImageType::ColorOnly
#include <inviwo/core/datastructures/image/layer.h>       // for Layer
#include <inviwo/core/ports/imageport.h>                  // for ImageOutport, ImageInport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>        // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>         // for Tags, Tags::GL
#include <inviwo/core/util/formats.h>                     // for DataFloat32, DataFormat
#include <modules/opengl/shader/shader.h>                 // for Shader
#include <modules/opengl/texture/textureunit.h>           // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>          // for activateAndClearTarget, bindAnd...

#include <memory>                                         // for shared_ptr
#include <string>                                         // for string
#include <string_view>                                    // for string_view
#include <type_traits>                                    // for remove_extent_t

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Vector2DMagnitude::processorInfo_{
    "org.inviwo.Vector2DMagnitude",  // Class identifier
    "Vector 2D Magnitude",           // Display name
    "Vector Field Visualization",    // Category
    CodeState::Stable,               // Code state
    Tags::GL,                        // Tags
};
const ProcessorInfo Vector2DMagnitude::getProcessorInfo() const { return processorInfo_; }

Vector2DMagnitude::Vector2DMagnitude()
    : Processor()
    , inport_("inport", true)
    , outport_("outport", DataFloat32::get())
    , shader_("vector2dmagnitude.frag") {

    addPort(inport_);
    addPort(outport_);
}

void Vector2DMagnitude::process() {
    utilgl::activateAndClearTarget(outport_);
    outport_.getEditableData()->getColorLayer()->setSwizzleMask(swizzlemasks::luminance);

    shader_.activate();
    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, inport_, ImageType::ColorOnly);

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
