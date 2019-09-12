/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/basegl/processors/pointrenderer.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PointRenderer::processorInfo_{
    "org.inviwo.PointRenderer",  // Class identifier
    "Point Renderer",            // Display name
    "Mesh Rendering",            // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo PointRenderer::getProcessorInfo() const { return processorInfo_; }

PointRenderer::PointRenderer()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , outport_("image")
    , pointSize_("pointSize", "Point Size (pixel)", 1.0f, 0.00001f, 50.0f, 0.1f)
    , borderWidth_("borderWidth", "Border Width (pixel)", 2.0f, 0.0f, 50.0f, 0.1f)
    , borderColor_("borderColor", "Border Color", vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f),
                   vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                   PropertySemantics::Color)
    , antialising_("antialising", "Antialising (pixel)", 1.5f, 0.0f, 10.0f, 0.1f)
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , shader_("pointrenderer.vert", "pointrenderer.frag") {

    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(outport_);

    addProperties(pointSize_, borderWidth_, borderColor_, antialising_, camera_, trackball_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void PointRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);

    utilgl::PolygonModeState polygon(GL_POINT, 1.0f, pointSize_.get());
    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_.activate();

    utilgl::setUniforms(shader_, camera_, pointSize_, borderWidth_, borderColor_, antialising_);

    drawMeshes();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void PointRenderer::drawMeshes() {
    for (auto& elem : inport_.getVectorData()) {
        MeshDrawerGL::DrawObject drawer(elem->getRepresentation<MeshGL>(),
                                        elem->getDefaultMeshInfo());
        utilgl::setShaderUniforms(shader_, *elem, "geometry");
        drawer.draw(MeshDrawerGL::DrawMode::Points);
    }
}

}  // namespace inviwo
