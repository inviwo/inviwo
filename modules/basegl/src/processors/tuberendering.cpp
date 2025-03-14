/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

/*
Code for rendering tubes is heavily inspired by a blog post written by Philip Rideout
(Tron, Volumetric Lines, and Meshless Tubes)
at "The little grasshopper, Graphics Programming Tips"
https://prideout.net/blog/old/blog/index.html@p=61.html

*/

#include <modules/basegl/processors/tuberendering.h>

#include <inviwo/core/algorithm/boundingbox.h>                 // for boundingBox
#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for BufferType, ConnectivityType
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh, Mesh::MeshInfo
#include <inviwo/core/ports/imageport.h>                       // for BaseImageInport, ImageInport
#include <inviwo/core/ports/inportiterable.h>                  // for InportIterable<>::const_it...
#include <inviwo/core/ports/meshport.h>                        // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>              // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>             // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>          // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>          // for InvalidationLevel, Invalid...
#include <inviwo/core/properties/propertysemantics.h>          // for PropertySemantics, Propert...
#include <inviwo/core/util/glmvec.h>                           // for vec4
#include <inviwo/core/util/iterrange.h>                        // for iter_range
#include <modules/basegl/datastructures/meshshadercache.h>     // for MeshShaderCache::Requirement
#include <modules/opengl/geometry/meshgl.h>                    // for MeshGL
#include <modules/opengl/inviwoopengl.h>                       // for GL_BACK, GL_DEPTH_TEST
#include <modules/opengl/openglutils.h>                        // for CullFaceState, GlBoolState
#include <modules/opengl/rendering/meshdrawergl.h>             // for MeshDrawerGL::DrawObject
#include <modules/opengl/shader/shader.h>                      // for Shader
#include <modules/opengl/shader/shaderobject.h>                // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                  // for ShaderType, ShaderType::Ve...
#include <modules/opengl/shader/shaderutils.h>                 // for addDefines, setShaderUniforms
#include <modules/opengl/texture/textureunit.h>                // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>               // for activateTargetAndClearOrCo...

#include <cstddef>      // for size_t
#include <functional>   // for __base, function
#include <map>          // for __map_iterator, map, opera...
#include <memory>       // for shared_ptr
#include <string_view>  // for string_view

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TubeRendering::processorInfo_{
    "org.inviwo.TubeRendering",  // Class identifier
    "Tube Rendering",            // Display name
    "Mesh Rendering",            // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo& TubeRendering::getProcessorInfo() const { return processorInfo_; }

TubeRendering::TubeRendering()
    : Processor()
    , inport_("mesh")
    , imageInport_("imageInport")
    , outport_("outport")
    , tubeProperties_("tubeProperties", "Tube Properties")
    , forceRadius_("forceRadius", "Force Radius", false, InvalidationLevel::InvalidResources)
    , defaultRadius_("defaultRadius", "Tube Radius", 0.01f, 0.0001f, 2.f, 0.0001f)
    , forceColor_("forceColor", "Force Color", false, InvalidationLevel::InvalidResources)
    , defaultColor_("defaultColor", "Default Color", vec4(0.7f, 0.7f, 0.7f, 1.0f), vec4(0.0f),
                    vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                    PropertySemantics::Color)
    , useMetaColor_("useMetaColor", "Use meta color mapping", false,
                    InvalidationLevel::InvalidResources)
    , metaColor_("metaColor", "Meta Color Mapping")
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , lighting_("lighting", "Lighting", &camera_)
    , shaderItems_{{{ShaderType::Vertex, "tuberendering.vert"},
                    {ShaderType::Geometry, "tuberendering.geom"},
                    {ShaderType::Fragment, "tuberendering.frag"}}}
    , shaderRequirements_{{{BufferType::PositionAttrib, MeshShaderCache::Mandatory, "vec3"},
                           {BufferType::ColorAttrib, MeshShaderCache::Optional, "vec4"},
                           {BufferType::RadiiAttrib, MeshShaderCache::Optional, "float"},
                           {BufferType::PickingAttrib, MeshShaderCache::Optional, "uint"},
                           {BufferType::ScalarMetaAttrib, MeshShaderCache::Optional, "float"}}}
    , adjacencyShaders_{shaderItems_, shaderRequirements_,
                        [&](Shader& shader) -> void {
                            shader.onReload(
                                [this]() { invalidate(InvalidationLevel::InvalidResources); });
                            for (auto& obj : shader.getShaderObjects()) {
                                obj.addShaderDefine("HAS_ADJACENCY");
                            }
                            configureShader(shader);
                        }}
    , shaders_{shaderItems_, shaderRequirements_, [&](Shader& shader) -> void {
                   shader.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
                   configureShader(shader);
               }} {

    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(outport_);

    tubeProperties_.addProperties(forceRadius_, defaultRadius_, forceColor_, defaultColor_,
                                  useMetaColor_, metaColor_);
    addProperties(tubeProperties_, camera_, lighting_, trackball_);
}

void TubeRendering::initializeResources() {
    for (auto& item : adjacencyShaders_.getShaders()) {
        configureShader(item.second);
    }
    for (auto& item : shaders_.getShaders()) {
        configureShader(item.second);
    }
}

void TubeRendering::configureShader(Shader& shader) {
    utilgl::addDefines(shader, lighting_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_RADIUS", forceRadius_);
    shader[ShaderType::Vertex]->setShaderDefine("FORCE_COLOR", forceColor_);
    shader[ShaderType::Vertex]->setShaderDefine("USE_SCALARMETACOLOR", useMetaColor_);
    shader.build();
}

void TubeRendering::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    const auto hasLineAdjacency = [](Mesh::MeshInfo mi) {
        return mi.dt == DrawType::Lines &&
               (mi.ct == ConnectivityType::StripAdjacency || mi.ct == ConnectivityType::Adjacency);
    };
    const auto hasLine = [](Mesh::MeshInfo mi) {
        return mi.dt == DrawType::Lines &&
               (mi.ct == ConnectivityType::None || mi.ct == ConnectivityType::Strip);
    };

    const auto hasAnyLine = [](const Mesh& mesh, auto test) {
        if (mesh.getNumberOfIndicies() > 0) {
            for (size_t i = 0; i < mesh.getNumberOfIndicies(); ++i) {
                if (test(mesh.getIndexMeshInfo(i))) return true;
            }
        } else {
            if (test(mesh.getDefaultMeshInfo())) return true;
        }
        return false;
    };

    // The geometry shader generates a six-sided bounding box for each line segment. The fragment
    // shader does not consider if the current fragment is on a front- or backface. The ray-cylinder
    // intersection test will thus give the same result for both, hence resulting in z-fighting. To
    // avoid this we turn on face culling.
    utilgl::CullFaceState cullstate(GL_BACK);

    const auto draw = [this](const Mesh& mesh, Shader& shader, auto test) {
        shader.activate();
        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader, units, metaColor_);
        utilgl::setUniforms(shader, camera_, lighting_, defaultColor_, defaultRadius_);

        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        MeshDrawerGL::DrawObject drawer(mesh.getRepresentation<MeshGL>(),
                                        mesh.getDefaultMeshInfo());
        utilgl::setShaderUniforms(shader, mesh, "geometry");
        if (mesh.getNumberOfIndicies() > 0) {
            for (size_t i = 0; i < mesh.getNumberOfIndicies(); ++i) {
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
        shader.deactivate();
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
