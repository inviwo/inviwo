/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "compositeprocessorgl.h"
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/shaderutils.h>

namespace inviwo {

CompositeProcessorGL::CompositeProcessorGL()
    : Processor(), shaderFileName_("composite.frag"), shader_(shaderFileName_) {}

CompositeProcessorGL::CompositeProcessorGL(std::string programFileName)
    : Processor(), shaderFileName_(programFileName), shader_(shaderFileName_) {}

void CompositeProcessorGL::compositePortsToOutport(ImageOutport& outport, ImageType type, ImageInport& inport) {
    if (inport.isReady() && outport.hasData()) {        
        utilgl::activateTarget(outport, type);
        shader_.activate();

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(&shader_, units, inport.getData(),  "tex0", COLOR_DEPTH_PICKING);
        utilgl::bindAndSetUniforms(&shader_, units, outport.getData(), "tex1", COLOR_DEPTH_PICKING);
        utilgl::setShaderUniforms(&shader_, outport, "outportParameters");
        utilgl::singleDrawImagePlaneRect();

        shader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }
}

void CompositeProcessorGL::initializeResources() { shader_.rebuild(); }

}  // namespace
