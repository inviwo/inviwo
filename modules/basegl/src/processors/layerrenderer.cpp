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

#include <modules/basegl/processors/layerrenderer.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/texture/textureunit.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerRenderer::processorInfo_{
    "org.inviwo.LayerRenderer",  // Class identifier
    "Layer Renderer",            // Display name
    "Mesh Rendering",            // Category
    CodeState::Experimental,     // Code state
    Tags::GL | Tag{"Layer"},     // Tags
    R"(Renders a set of Layers using OpenGL. The basis transformation of each layer is used to "
      "position it in 3D world coordinates.)"_unindentHelp};

const ProcessorInfo LayerRenderer::getProcessorInfo() const { return processorInfo_; }

LayerRenderer::LayerRenderer()
    : Processor{}
    , inport_{"layers", "Input layers"_help}
    , background_{"background", "Background Image (optional)"_help}
    , outport_{"outport",
               "Output image containing the rendered layers and optional background"_help}
    , camera_{"camera", "Camera", util::boundingBox(inport_)}
    , trackball_(&camera_)
    , shader_{"layerrendering.vert", "layerrendering.frag", Shader::Build::No}
    , mesh_{DrawType::Triangles, ConnectivityType::Strip} {

    background_.setOptional(true);
    addPorts(inport_, background_, outport_);
    addProperties(camera_, trackball_);

    auto verticesBuffer =
        util::makeBuffer<vec2>({{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});
    auto indices_ = util::makeIndexBuffer({0, 1, 2, 3});

    mesh_.addBuffer(BufferType::PositionAttrib, verticesBuffer);
    mesh_.addBuffer(BufferType::TexCoordAttrib, verticesBuffer);
    mesh_.addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip), indices_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void LayerRenderer::initializeResources() {
    utilgl::addShaderDefines(shader_, ShadingMode::None);
    shader_.build();
}

void LayerRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, background_);
    shader_.activate();

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
    utilgl::CullFaceState culling(GL_NONE);
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    utilgl::setUniforms(shader_, camera_);
    MeshDrawerGL::DrawObject drawer{mesh_.getRepresentation<MeshGL>(), mesh_.getDefaultMeshInfo()};

    TextureUnit unit;
    shader_.setUniform("colorTex", unit);

    for (const auto& layer : inport_) {
        shader_.setUniform("geometry.dataToWorld", mat4(1.0f));
                           //layer->getCoordinateTransformer().getDataToWorldMatrix());
        layer->getRepresentation<LayerGL>()->bindTexture(unit);
        drawer.draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
