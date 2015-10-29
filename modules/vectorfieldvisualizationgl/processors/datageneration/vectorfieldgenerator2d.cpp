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

#include "vectorfieldgenerator2d.h"
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/datastructures/image/imageram.h>

namespace inviwo {

const ProcessorInfo VectorFieldGenerator2D::processorInfo_{
    "org.inviwo.VectorFieldGenerator2D",  // Class identifier
    "Vector Field Generator 2D",          // Display name
    "Data Creation",                      // Category
    CodeState::Experimental,              // Code state
    "GL",                                 // Tags
};
const ProcessorInfo VectorFieldGenerator2D::getProcessorInfo() const {
    return processorInfo_;
}

    VectorFieldGenerator2D::VectorFieldGenerator2D()
        : Processor()
        , outport_("outport", DataVec4Float32::get(),false)
        , shader_("vectorfieldgenerator2d.frag", false)
        , fbo_()
        , size_("size", "Volume size", ivec2(16), ivec2(1), ivec2(1024))
        , xValue_("x", "X", "-x", InvalidationLevel::InvalidResources)
        , yValue_("y", "Y", "y", InvalidationLevel::InvalidResources)
        , xRange_("xRange", "X Range", -1, 1, -10, 10)
        , yRange_("yRange", "Y Range", -1, 1, -10, 10)
    {
        addPort(outport_);

        addProperty(size_);
        addProperty(xValue_);
        addProperty(yValue_);

        addProperty(xRange_);
        addProperty(yRange_);
    }

VectorFieldGenerator2D::~VectorFieldGenerator2D()  {
    
}


void VectorFieldGenerator2D::initializeResources() {
    shader_.getFragmentShaderObject()->addShaderDefine("X_VALUE(x,y)", xValue_.get());
    shader_.getFragmentShaderObject()->addShaderDefine("Y_VALUE(x,y)", yValue_.get());

    shader_.build();
}

void VectorFieldGenerator2D::process() {

    image_ = std::make_shared<Image>(size_.get(), DataVec4Float32::get());

    utilgl::activateAndClearTarget(*(image_.get()), ImageType::ColorOnly);

    shader_.activate();
    utilgl::setUniforms(shader_, xRange_, yRange_);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();

    outport_.setData(image_);

   /* auto ram = image_->getEditableRepresentation<ImageRAM>();
    auto i = 0;*/
}

} // namespace


