/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/pathtracing/processors/dummyprocessor.h>
#include <modules/opengl/openglmodule.h>
#include <modules/opengl/shader/shadermanager.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/image/layergl.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

#include <inviwo/core/util/consolelogger.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DummyProcessor::processorInfo_{
    "org.inviwo.DummyProcessor",  // Class identifier
    "Dummy Processor",        // Display name
    "Testing",                   // Category
    CodeState::Experimental,       // Code state
    Tags::GL,                    // Tags
};

const ProcessorInfo DummyProcessor::getProcessorInfo() const { return processorInfo_; }

DummyProcessor::DummyProcessor()
    : Processor{}
    
    , inport_{"inport", "<description of the inport data and any requirements on the data>"_help}
    , outport_{"outport", "<description of the generated outport data>"_help}
    , shader_({{ShaderType::Compute, "simpleshift.comp"}})
    , value_{
        "value", "Value", 1, 0, 100 } {

    addPorts(inport_, outport_);
    addProperties(value_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); } );
}

void DummyProcessor::process() {
    /* On how Textures/Images work in OGL
        https://www.khronos.org/opengl/wiki/Texture
        https://community.khronos.org/t/what-is-a-texture-unit/63250/2
    */

    glActiveTexture(GL_TEXTURE0);

    // How do we get a LayerGL out of inport data? 
    //It seems that all is stopping us is the fact that getData returns a const Image shrd pointer instead just Image shrd pointer.
    //

    auto imgInternal = std::make_shared<Image>(size2_t{512, 512}, DataFormat<float>::get());
    auto img = inport_.getData()->clone();

    //auto layerGL = imgInternal->getColorLayer()->getEditableRepresentation<LayerGL>();
    auto layerGL = img->getColorLayer()->getEditableRepresentation<LayerGL>();

    auto texHandle = layerGL->getTexture()->getID();

    glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    layerGL->setSwizzleMask(swizzlemasks::luminance); // ??? does something i geuss...

    shader_.activate();
    utilgl::setUniforms(shader_, value_);

    layerGL->getTexture()->bind();

    shader_.setUniform("img", 0);

    glDispatchCompute(512/16, 512/16, 1);

    shader_.deactivate();

    outport_.setData(img);
}

}  // namespace inviwo
