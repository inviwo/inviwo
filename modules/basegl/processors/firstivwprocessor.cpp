/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "firstivwprocessor.h"
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(FirstIvwProcessor, "org.inviwo.FirstIVWProcessor");
ProcessorDisplayName(FirstIvwProcessor,  "First Processor");
ProcessorTags(FirstIvwProcessor, Tags::GL);
ProcessorCategory(FirstIvwProcessor, "Various");
ProcessorCodeState(FirstIvwProcessor, CODE_STATE_STABLE);


FirstIvwProcessor::FirstIvwProcessor()
    : Processor()
    , color_("color", "Color", vec3(1.0f), vec3(0.0f), vec3(1.0f), vec3(0.1f))
    , outport_("outport") {
    addProperty(color_);
    addPort(outport_);
}

void FirstIvwProcessor::initialize() {
    Processor::initialize();
    shader_ = new Shader("minimal.vert", "img_color.frag");

    quad_ = new Position2dBuffer();
    Position2dBufferRAM* quadRAM = quad_->getEditableRepresentation<Position2dBufferRAM>();
    quadRAM->add(vec2(-1.0f, -1.0f));
    quadRAM->add(vec2(1.0f, -1.0f));
    quadRAM->add(vec2(-1.0f, 1.0f));
    quadRAM->add(vec2(1.0f, 1.0f));
    quadGL_ = quad_->getRepresentation<BufferGL>();

    triangle_ = new Position2dBuffer();
    Position2dBufferRAM* triangleRAM = triangle_->getEditableRepresentation<Position2dBufferRAM>();
    triangleRAM->add(vec2(0.0f, 1.0f));
    triangleRAM->add(vec2(-1.0f, -1.0f));
    triangleRAM->add(vec2(1.0f, -1.0f));
    triangleGL_ = triangle_->getRepresentation<BufferGL>();
}

void FirstIvwProcessor::deinitialize() {
    delete shader_;
    delete quad_;
    delete triangle_;
    Processor::deinitialize();
}

void FirstIvwProcessor::process() {
    glDisable(GL_DEPTH_TEST);
    utilgl::activateAndClearTarget(outport_, COLOR_ONLY);
    shader_->activate();

    //Render Quad
    shader_->setUniform("color_", vec4(color_.get().x, color_.get().y, color_.get().z, 1.f));
    quadGL_->enable();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    quadGL_->disable();

    //Render Triangle
    shader_->setUniform("color_", vec4(0.f, 1.f, 0.f, 1.f));
    triangleGL_->enable();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    triangleGL_->disable();

    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
    glEnable(GL_DEPTH_TEST);
}

} // namespace
