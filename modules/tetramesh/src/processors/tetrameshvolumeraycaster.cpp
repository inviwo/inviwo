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

#include <inviwo/tetramesh/processors/tetrameshvolumeraycaster.h>
#include <inviwo/tetramesh/datastructures/tetrameshprovider.h>
#include <inviwo/tetramesh/util/tetrameshutils.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/exception.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

namespace detail {

std::function<std::optional<mat4>()> boundingBox(const TetraMeshInport& tetra) {
    return [port = &tetra]() -> std::optional<mat4> {
        if (port->hasData()) {
            auto data = port->getData();
            return utiltetra::boundingBox(**port->getData());
        } else {
            return std::nullopt;
        }
    };
}

}  // namespace detail

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TetraMeshVolumeRaycaster::processorInfo_{
    "org.inviwo.TetraMeshVolumeRaycaster",  // Class identifier
    "TetraMesh Volume Raycaster",           // Display name
    "Unstructured Grids",                   // Category
    CodeState::Stable,                      // Code state
    Tag::GL | Tag{"Unstructured"},          // Tags
    R"(Creates an OpenGL representation of a tetrahedral grid and renders it using volume rendering.)"_unindentHelp};

const ProcessorInfo TetraMeshVolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

TetraMeshVolumeRaycaster::TetraMeshVolumeRaycaster()
    : Processor{}
    , inport_{"inport", "Tetra mesh data used for volume rendering"_help}
    , imageInport_{"background", "Optional background image"_help}
    , outport_{"outport", "Rendered output image"_help}
    , camera_{"camera", "Camera", detail::boundingBox(inport_)}
    , trackball_{&camera_}
    , lighting_{"lighting", "Lighting", &camera_}
    , tf_{"transferFunction", "Transfer Function"}
    , opacityScaling_{"opacityScaling", "Opacity Scaling",
                      util::ordinalScale(1.0f, 10.0f)
                          .setInc(0.01f)
                          .set("Scaling factor for the opacity in the transfer function."_help)}
    , maxSteps_{"maxSteps", "Max Steps",
                util::ordinalCount(10000).set(
                    "Upper limit of tetrahedras a ray can traverse."_help)}
    , shader_{"tetramesh_traversal.vert", "tetramesh_traversal.frag", Shader::Build::No} {

    addPorts(inport_, imageInport_, outport_);

    imageInport_.setOptional(true);

    addProperties(tf_, opacityScaling_, maxSteps_, camera_, lighting_, trackball_);

    lighting_.shadingMode_.setSelectedValue(ShadingMode::None);
    lighting_.shadingMode_.setCurrentStateAsDefault();

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void TetraMeshVolumeRaycaster::initializeResources() {
    utilgl::addDefines(shader_, camera_, lighting_);
    shader_.build();
}

void TetraMeshVolumeRaycaster::process() {
    TetraMeshProvider* meshProvider{*inport_.getData()};
    if (!meshProvider) {
        throw Exception(IVW_CONTEXT, "Tetra mesh does not have a valid data provider");
    }

    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    TextureUnitContainer texContainer;
    shader_.activate();

    meshProvider->bindBuffers();
    const auto& mesh = meshProvider->getBoundaryMesh();

    utilgl::setUniforms(shader_, camera_, lighting_, opacityScaling_, maxSteps_);
    utilgl::setShaderUniforms(shader_, *mesh, "geometry");
    utilgl::bindAndSetUniforms(shader_, texContainer, tf_);
    if (imageInport_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, texContainer, imageInport_, ImageType::ColorOnly);
    }
    shader_.setUniform("useBackground", imageInport_.hasData());

    const dvec2 dataRange{meshProvider->getDataRange()};
    const double scalingFactor = 1.0 / (dataRange.y - dataRange.x);
    const double offset = -dataRange.x;
    shader_.setUniform("tfScalarScaling", static_cast<float>(scalingFactor));
    shader_.setUniform("tfScalarOffset", static_cast<float>(offset));

    utilgl::CullFaceState cf(GL_BACK);
    utilgl::GlBoolState cull(GL_CULL_FACE, true);

    auto drawer = MeshDrawerGL::getDrawObject(mesh.get());
    drawer.draw();

    meshProvider->unbindBuffers();

    shader_.deactivate();

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
