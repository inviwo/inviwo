/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/basegl/processors/ribbonrenderer.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>

#include <fmt/base.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo RibbonRenderer::processorInfo_{
    "org.inviwo.RibbonRenderer",  // Class identifier
    "Ribbon Renderer",            // Display name
    "Mesh Rendering",             // Category
    CodeState::Experimental,      // Code state
    Tags::GL,                     // Tags
    R"(Renders line meshes as ribbons by extruding the line in direction of the normal.)"_unindentHelp,
};

const ProcessorInfo& RibbonRenderer::getProcessorInfo() const { return processorInfo_; }

RibbonRenderer::RibbonRenderer()
    : Processor{}
    , inport_{"mesh", "Line Meshes with a normal buffer"_help}
    , backgroundInport_{"background", "Optional background image"_help}
    , outport_{"outport", "Resulting image"_help}
    , ribbonProperties_{"ribbonProperties", "Ribbon Properties"}
    , subdivisions_{"subdivisions", "Subdivisions",
                    util::ordinalCount(0, 10).set(InvalidationLevel::InvalidResources)}
    , widthScaling_{"widthScaling", "Width Scaling", util::ordinalScale(1.0f, 2.0f).setInc(0.001f)}
    , forceWidth_{"forceWidth", "Force Width", false, InvalidationLevel::InvalidResources}
    , defaultWidth_{"defaultWidth", "Ribbon Width", util::ordinalScale(0.1f, 2.0f).setInc(0.0001f)}
    , forceColor_{"forceColor", "Force Color", false, InvalidationLevel::InvalidResources}
    , defaultColor_{"defaultColor", "Default Color",
                    util::ordinalColor(vec4{0.7f, 0.7f, 0.7f, 1.0f})}
    , useMetaColor_("useMetaColor", "Use meta color mapping", false,
                    InvalidationLevel::InvalidResources)
    , metaColor_("metaColor", "Meta Color Mapping")
    , camera_{"camera", "Camera", util::boundingBox(inport_)}
    , trackball_{&camera_}
    , lighting_{"lighting", "Lighting", &camera_}
    , shaderItems_{{{ShaderType::Vertex, "ribbonrenderer.vert"},
                    {ShaderType::Geometry, "ribbonrenderer.geom"},
                    {ShaderType::Fragment, "ribbonrenderer.frag"}}}
    , shaderRequirements_{{{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                           {BufferType::NormalAttrib, MeshShaderCache::Optional, "vec3"},
                           {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                           {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
                           {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"}}}
    , adjacencyShaders_{shaderItems_, shaderRequirements_,
                        [&](Shader& shader) {
                            shader.onReload(
                                [this]() { invalidate(InvalidationLevel::InvalidResources); });
                            for (auto& obj : shader.getShaderObjects()) {
                                obj.addShaderDefine("HAS_ADJACENCY");
                            }
                            configureShader(shader);
                        }}
    , shaders_{shaderItems_, shaderRequirements_, [&](Shader& shader) {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   configureShader(shader);
               }} {

    addPorts(inport_, outport_);
    addPort(backgroundInport_).setOptional(true);

    ribbonProperties_.addProperties(subdivisions_, widthScaling_, forceWidth_, defaultWidth_,
                                    forceColor_, defaultColor_, useMetaColor_, metaColor_);
    addProperties(ribbonProperties_, camera_, lighting_, trackball_);
}

void RibbonRenderer::initializeResources() {
    for (auto& [_, shader] : adjacencyShaders_.getShaders()) {
        configureShader(shader);
    }
    for (auto& [_, shader] : shaders_.getShaders()) {
        configureShader(shader);
    }
}

void RibbonRenderer::configureShader(Shader& shader) const {
    utilgl::addDefines(shader, lighting_);
    shader[ShaderType::Geometry]->setShaderDefine("SUBDIVISIONS", true,
                                                  fmt::format("{}", subdivisions_));
    shader[ShaderType::Geometry]->setShaderDefine("MAX_VERTICES_OUT", true,
                                                  fmt::format("{}", 6 + subdivisions_ * 6));
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_WIDTH", forceWidth_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_COLOR", forceColor_);
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor_);
    shader.build();
}

namespace {

void drawMesh(const Mesh& mesh, Shader& shader, auto test) {
    MeshDrawerGL::DrawObject drawer(mesh.getRepresentation<MeshGL>(), mesh.getDefaultMeshInfo());
    utilgl::setShaderUniforms(shader, mesh, "geometry");
    if (mesh.getNumberOfIndices() > 0) {
        for (size_t i = 0; i < mesh.getNumberOfIndices(); ++i) {
            const auto mi = mesh.getIndexMeshInfo(i);
            if (test(mi)) {
                drawer.draw(i);
            }
        }
    } else {
        // no index buffers, check mesh default draw type
        const auto mi = mesh.getDefaultMeshInfo();
        if (test(mi)) {
            drawer.draw();
        }
    }
}

auto hasLineAdjacency(Mesh::MeshInfo mi) {
    return mi.dt == DrawType::Lines &&
           (mi.ct == ConnectivityType::StripAdjacency || mi.ct == ConnectivityType::Adjacency);
}

auto hasLine(Mesh::MeshInfo mi) {
    return mi.dt == DrawType::Lines &&
           (mi.ct == ConnectivityType::None || mi.ct == ConnectivityType::Strip);
}

auto hasAnyLine(const Mesh& mesh, auto test) {
    if (mesh.getNumberOfIndices() > 0) {
        for (size_t i = 0; i < mesh.getNumberOfIndices(); ++i) {
            if (test(mesh.getIndexMeshInfo(i))) return true;
        }
    } else {
        if (test(mesh.getDefaultMeshInfo())) return true;
    }
    return false;
}

}  // namespace

void RibbonRenderer::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, backgroundInport_);

    const utilgl::CullFaceState cullstate(GL_NONE);
    const utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
    const utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto draw = [this](const Mesh& mesh, Shader& shader, auto test) {
        const utilgl::Activate activateShader{&shader};
        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader, units, metaColor_);
        utilgl::setUniforms(shader, camera_, lighting_, widthScaling_, defaultWidth_,
                            defaultColor_);
        drawMesh(mesh, shader, test);
    };

    for (const auto& mesh : inport_) {
        if (hasAnyLine(*mesh, hasLineAdjacency)) {
            draw(*mesh, adjacencyShaders_.getShader(*mesh), hasLineAdjacency);
        }
        if (hasAnyLine(*mesh, hasLine)) {
            draw(*mesh, shaders_.getShader(*mesh), hasLine);
        }
    }
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
