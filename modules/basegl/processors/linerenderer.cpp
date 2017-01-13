/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/basegl/processors/linerenderer.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LineRenderer::processorInfo_{
    "org.inviwo.LineRenderer",      // Class identifier
    "Line Renderer",                // Display name
    "Rendering",              // Category
    CodeState::Experimental,  // Code state
    Tags::GL,               // Tags
};
const ProcessorInfo LineRenderer::getProcessorInfo() const {
    return processorInfo_;
}


LineRenderer::LineRenderer()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , outport_("image")
    , camera_("camera", "Camera")
    , trackball_(&camera_)
    , lineWidth_("lineWidth", "Line Width (pixel)", 1.0f, 0.0f, 50.0f, 0.1f)
    , antialising_("antialising", "Antialising (pixel)", 1.0f, 0.0f, 10.0f, 0.1f)
    , miterLimit_("miterLimit", "Miter Limit", 0.8f, 0.0f, 1.0f, 0.1f)
    , useAdjacency_("useAdjacency", "Use Adjacency Information", true)
    , shader_("linerenderer.vert", "linerenderer.geom", "linerenderer.frag", false) 
{
    outport_.addResizeEventListener(&camera_);

    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);
    imageInport_.setOptional(true);

    addProperty(lineWidth_);
    addProperty(antialising_);
    addProperty(miterLimit_);
    addProperty(useAdjacency_);    

    addProperty(camera_);
    addProperty(trackball_);

    useAdjacency_.onChange([this]() { invalidate(InvalidationLevel::InvalidResources); });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void LineRenderer::initializeResources() { 
    shader_.getGeometryShaderObject()->addShaderDefine("ENABLE_ADJACENCY", useAdjacency_.get() ? "1" : "0");

    shader_.build();
}
    
void LineRenderer::process() {
    if (imageInport_.isConnected()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepth);
    }
    else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_.activate();

    shader_.setUniform("screenDim_", vec2(outport_.getDimensions()));
    utilgl::setShaderUniforms(shader_, camera_, "camera_");

    shader_.setUniform("lineWidth_", lineWidth_.get());
    shader_.setUniform("antialias_", antialising_.get());
    shader_.setUniform("miterLimit_", miterLimit_.get());
    
    drawMeshes();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void LineRenderer::drawMeshes() {
    auto drawMode = (useAdjacency_.get() ? MeshDrawerGL::DrawMode::LineStripAdjacency
                                         : MeshDrawerGL::DrawMode::LineStrip);

    for (auto& elem : inport_.getVectorData()) {
        MeshDrawerGL::DrawObject drawer(elem->getRepresentation<MeshGL>(),
                                        elem->getDefaultMeshInfo());
        utilgl::setShaderUniforms(shader_, *elem, "geometry_");
        drawer.draw(drawMode);
    }
}

} // namespace

