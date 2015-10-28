/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "vector2ddivergence.h"
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Vector2DDivergence::processorInfo_{
    "org.inviwo.Vector2DDivergence",  // Class identifier
    "Vector 2D Divergence",           // Display name
    "Vector Field Topology",          // Category
    CodeState::Experimental,          // Code state
    Tags::GL,                         // Tags
};
const ProcessorInfo Vector2DDivergence::getProcessorInfo() const {
    return processorInfo_;
}

Vector2DDivergence::Vector2DDivergence()
    : Processor()
    , inport_("inport", true)
    , outport_("outport", DataVec4Float32::get())
    , shader_("vector2ddivergence.frag")
{

    addPort(inport_);
    addPort(outport_);
}
    
void Vector2DDivergence::process() {
    utilgl::activateAndClearTarget(outport_);

    shader_.activate();
    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, inport_, ImageType::ColorOnly);


    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

} // namespace


