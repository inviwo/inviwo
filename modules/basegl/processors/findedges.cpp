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

#include "findedges.h"
#include <inviwo/core/datastructures/image/imageram.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(FindEdges, "org.inviwo.FindEdges");
ProcessorDisplayName(FindEdges, "Find Edges");
ProcessorTags(FindEdges, Tags::GL);
ProcessorCategory(FindEdges, "Image Operation");
ProcessorCodeState(FindEdges, CODE_STATE_EXPERIMENTAL);

FindEdges::FindEdges()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , alpha_("alpha", "Alpha", 0.5f, 0.0f, 1.0f) {

    addPort(inport_);
    addPort(outport_);
    addProperty(alpha_);
}

FindEdges::~FindEdges() {}

void FindEdges::initialize() {
    Processor::initialize();
    shader_ = new Shader("img_findedges.frag");
}

void FindEdges::deinitialize() {
    delete shader_;
    Processor::deinitialize();
}

void FindEdges::process() {
    TextureUnit image;
    utilgl::activateTargetAndCopySource(outport_, inport_, COLOR_ONLY);
    utilgl::bindColorTexture(inport_, image);
    shader_->activate();
    shader_->setUniform("inport_", image.getUnitNumber());
    shader_->setUniform("alpha_", alpha_.get());
    shader_->setUniform("dimensions_",
                        vec2(1.f / outport_.getDimensions()[0], 1.f / outport_.getDimensions()[1]));
    utilgl::singleDrawImagePlaneRect();
    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace
