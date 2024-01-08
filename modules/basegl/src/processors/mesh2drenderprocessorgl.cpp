/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/basegl/processors/mesh2drenderprocessorgl.h>

#include <inviwo/core/algorithm/markdown.h>            // for operator""_help, operator""_uninde...
#include <inviwo/core/datastructures/camera/camera.h>  // for mat4
#include <inviwo/core/datastructures/geometry/mesh.h>  // for hasPickIDBuffer, Mesh
#include <inviwo/core/ports/imageport.h>               // for ImageInport
#include <inviwo/core/ports/inportiterable.h>          // for InportIterable<>::const_iterator
#include <inviwo/core/ports/meshport.h>                // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/ordinalproperty.h>    // for FloatProperty
#include <inviwo/core/interaction/events/resizeevent.h>
#include <modules/opengl/geometry/meshgl.h>         // for MeshGL
#include <modules/opengl/inviwoopengl.h>            // for GL_DEPTH_TEST, GL_ONE_MINUS_SRC_ALPHA
#include <modules/opengl/openglutils.h>             // for BlendModeState, GlBoolState
#include <modules/opengl/rendering/meshdrawergl.h>  // for MeshDrawerGL::DrawObject, MeshDraw...
#include <modules/opengl/shader/shader.h>           // for Shader
#include <modules/opengl/shader/shaderutils.h>      // for setShaderUniforms
#include <modules/opengl/texture/textureutils.h>    // for activateTargetAndClearOrCopySource

#include <functional>   // for __base
#include <memory>       // for shared_ptr, shared_ptr<>::element_...
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/ext/matrix_clip_space.hpp>  // for ortho

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Mesh2DRenderProcessorGL::processorInfo_{
    "org.inviwo.Mesh2DRenderProcessorGL",  // Class identifier
    "2D Mesh Renderer",                    // Display name
    "Mesh Rendering",                      // Category
    CodeState::Stable,                     // Code state
    Tags::GL,                              // Tags
    "Render a mesh using only an orthographic projection without any illumination calculation."_unindentHelp};
const ProcessorInfo Mesh2DRenderProcessorGL::getProcessorInfo() const { return processorInfo_; }

Mesh2DRenderProcessorGL::Mesh2DRenderProcessorGL()
    : Processor()
    , inport_("inputMesh", "Input meshes"_help)
    , imageInport_("imageInport", "background image (optional)"_help)
    , outport_("outputImage",
               "output image containing the rendered meshes and the optional input image"_help)
    , shader_("mesh2drendering.vert", "mesh2drendering.frag")
    , enableDepthTest_("enableDepthTest", "Enable Depth Test",
                       "Toggles the depth test during rendering"_help, true)
    , frustum_("frustum", "Orthographic Frustum",
               "Parameters to define the orthographic projection"_help)
    , useCanvasDims_("useCanvasDims", "Use Canvas Dimensions", false)
    , top_("top", "Top", util::ordinalSymmetricVector(1.0f))
    , bottom_("bottom", "Bottom", util::ordinalSymmetricVector(0.0f))
    , left_("left", "Left", util::ordinalSymmetricVector(0.0f))
    , right_("right", "Right", util::ordinalSymmetricVector(1.0f))
    , near_("near", "Near", util::ordinalSymmetricVector(-1.0f).set(PropertySemantics::Default))
    , far_("far", "Far", util::ordinalSymmetricVector(1.0f).set(PropertySemantics::Default)) {

    addPorts(inport_, imageInport_, outport_);
    imageInport_.setOptional(true);

    addProperties(enableDepthTest_, frustum_);
    frustum_.addProperties(useCanvasDims_, left_, right_, bottom_, top_, near_, far_);

    top_.readonlyDependsOn(useCanvasDims_, [](BoolProperty& p) { return p; });
    bottom_.readonlyDependsOn(useCanvasDims_, [](BoolProperty& p) { return p; });
    left_.readonlyDependsOn(useCanvasDims_, [](BoolProperty& p) { return p; });
    right_.readonlyDependsOn(useCanvasDims_, [](BoolProperty& p) { return p; });

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

Mesh2DRenderProcessorGL::~Mesh2DRenderProcessorGL() = default;

void Mesh2DRenderProcessorGL::process() {
    if (useCanvasDims_.isModified() && useCanvasDims_) {
        updateFrustum(outport_.getDimensions());
    }

    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);
    shader_.activate();

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, enableDepthTest_);
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const mat4 proj =
        glm::ortho(left_.get(), right_.get(), bottom_.get(), top_.get(), near_.get(), far_.get());
    shader_.setUniform("projectionMatrix", proj);
    for (auto mesh : inport_) {
        utilgl::setShaderUniforms(shader_, *mesh, "geometry");
        shader_.setUniform("pickingEnabled", meshutil::hasPickIDBuffer(mesh.get()));
        MeshDrawerGL::DrawObject drawer{mesh->getRepresentation<MeshGL>(),
                                        mesh->getDefaultMeshInfo()};
        drawer.draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void Mesh2DRenderProcessorGL::propagateEvent(Event* event, Outport* source) {
    if (event->hash() == ResizeEvent::chash() && useCanvasDims_) {
        auto resizeEvent = static_cast<ResizeEvent*>(event);

        updateFrustum(resizeEvent->size());
    }

    Processor::propagateEvent(event, source);
}

void Mesh2DRenderProcessorGL::updateFrustum(const size2_t& dims) {
    top_.set(static_cast<float>(dims.y));
    right_.set(static_cast<float>(dims.x));
    bottom_.set(0.0f);
    left_.set(0.0f);
}

}  // namespace inviwo
