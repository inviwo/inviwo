/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

namespace util {
MeshDrawerGL::DrawMode getDrawMode(LineRenderer::LineDrawMode drawMode, bool useAdjacency);
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LineRenderer::processorInfo_{
    "org.inviwo.LineRenderer",  // Class identifier
    "Line Renderer",            // Display name
    "Mesh Rendering",           // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
};
const ProcessorInfo LineRenderer::getProcessorInfo() const { return processorInfo_; }

LineRenderer::LineRenderer()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , outport_("image")
    , lineWidth_("lineWidth", "Line Width (pixel)", 1.0f, 0.0f, 50.0f, 0.1f)
    , antialiasing_("antialiasing", "Antialiasing (pixel)", 0.5f, 0.0f, 10.0f, 0.1f)
    , miterLimit_("miterLimit", "Miter Limit", 0.8f, 0.0f, 1.0f, 0.1f)
    , roundCaps_("roundCaps", "Round Caps", true)
    , pseudoLighting_("pseudoLighting", "Pseudo Lighting", true,
                      InvalidationLevel::InvalidResources)
    , roundDepthProfile_("roundDepthProfile", "Round Depth Profile", true,
                         InvalidationLevel::InvalidResources)
    , writeDepth_("writeDepth", "Write Depth Layer", true)
    , drawMode_("drawMode", "Draw Mode",
                {{"auto", "Automatic", LineDrawMode::Auto},
                 {"lineSegments", "Line Segments", LineDrawMode::LineSegments},
                 {"lineStrip", "Line Strip", LineDrawMode::LineStrip},
                 {"lineLoop", "Line Loop", LineDrawMode::LineLoop}},
                0, InvalidationLevel::InvalidResources)
    , useAdjacency_("useAdjacency", "Use Adjacency Information", true,
                    InvalidationLevel::InvalidResources)
    , stippling_("stippling", "Stippling")
    , camera_("camera", "Camera")
    , trackball_(&camera_)
    , lineShaders_{
          {{ShaderType::Vertex, "linerenderer.vert"},
           {ShaderType::Geometry, "linerenderer.geom"},
           {ShaderType::Fragment, "linerenderer.frag"}},

          {{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
           {BufferType::ColorAttrib, MeshShaderCache::Mandatory, "vec4"},
           {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"}},

          [&](Shader& shader) -> void {
              shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
              configureShader(shader);
          }} {
    outport_.addResizeEventListener(&camera_);

    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);
    imageInport_.setOptional(true);

    addProperty(lineWidth_);
    addProperty(antialiasing_);
    addProperty(miterLimit_);
    addProperty(roundCaps_);
    addProperty(pseudoLighting_);
    addProperty(roundDepthProfile_);
    addProperty(writeDepth_);
    addProperty(drawMode_);
    addProperty(useAdjacency_);

    addProperty(stippling_);

    addProperty(camera_);
    addProperty(trackball_);

    drawMode_.onChange([this]() {
        bool noAdjacencySupport = (drawMode_.get() == LineDrawMode::LineLoop);
        useAdjacency_.setReadOnly(noAdjacencySupport);
    });
}

void LineRenderer::initializeResources() {
    for (auto& item : lineShaders_.getShaders()) {
        configureShader(item.second);
    }
}

void LineRenderer::configureShader(Shader& shader) {
    bool adjacencySupport = (drawMode_.get() != LineDrawMode::LineLoop);

    shader[ShaderType::Geometry]->addShaderDefine(
        "ENABLE_ADJACENCY", useAdjacency_.get() && adjacencySupport ? "1" : "0");

    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_PSEUDO_LIGHTING", pseudoLighting_);
    shader[ShaderType::Fragment]->setShaderDefine("ENABLE_ROUND_DEPTH_PROFILE", roundDepthProfile_);

    utilgl::addShaderDefines(shader, stippling_);
    shader.build();
}

void LineRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    utilgl::DepthMaskState depthMask(writeDepth_.get());

    utilgl::DepthFuncState depthFunc(GL_LEQUAL);

    drawMeshes();

    utilgl::deactivateCurrentTarget();
}

void LineRenderer::drawMeshes() {
    bool autoDrawMode = (drawMode_.get() == LineDrawMode::Auto);
    auto drawmode = util::getDrawMode(drawMode_.get(), useAdjacency_.get());

    for (const auto& elem : inport_) {
        auto& shader = lineShaders_.getShader(*elem);
        shader.activate();
        shader.setUniform("screenDim", vec2(outport_.getDimensions()));
        utilgl::setUniforms(shader, camera_, lineWidth_, antialiasing_, miterLimit_, roundCaps_,
                            stippling_);
        MeshDrawerGL::DrawObject drawer(elem->getRepresentation<MeshGL>(),
                                        elem->getDefaultMeshInfo());
        utilgl::setShaderUniforms(shader, *elem, "geometry");
        if (autoDrawMode) {
            drawer.draw();
        } else {
            drawer.draw(drawmode);
        }
        shader.deactivate();
    }
}

namespace util {

MeshDrawerGL::DrawMode getDrawMode(LineRenderer::LineDrawMode drawMode, bool useAdjacency) {
    switch (drawMode) {
        case LineRenderer::LineDrawMode::LineSegments:
            if (useAdjacency) {
                return MeshDrawerGL::DrawMode::LinesAdjacency;
            } else {
                return MeshDrawerGL::DrawMode::Lines;
            }
        case LineRenderer::LineDrawMode::LineStrip:
            if (useAdjacency) {
                return MeshDrawerGL::DrawMode::LineStripAdjacency;
            } else {
                return MeshDrawerGL::DrawMode::LineStrip;
            }
        case LineRenderer::LineDrawMode::LineLoop:
            return MeshDrawerGL::DrawMode::LineLoop;
        case LineRenderer::LineDrawMode::Auto:
        default:
            return MeshDrawerGL::DrawMode::NotSpecified;
    }
}

}  // namespace util

}  // namespace inviwo
