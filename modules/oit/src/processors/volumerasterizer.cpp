/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <modules/oit/processors/volumerasterizer.h>

#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/util/document.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/texture3d.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>             // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>  // for MeshDrawerG...
#include <modules/opengl/geometry/meshgl.h>         // for MeshGL
#include <modules/oit/datastructures/transformedrasterization.h>
#include <modules/oit/raycastingstate.h>

#include <fmt/core.h>  // for format

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRasterizer::processorInfo_{
    "org.inviwo.VolumeRasterizer",  // Class identifier
    "Volume Rasterizer",            // Display name
    "Volume Rendering",             // Category
    CodeState::Experimental,        // Code state
    Tags::GL,                       // Tags
    R"(
        Volume renderer for rendering one or more volumes together with arbitrary, transparent meshes.
        Does not yet have support for illumination or gradients.
        In contrast to regular volume raycasting, the sampling takes place in world coordinates with the
        sampling distance also being defined in this global coordinate space since multiple volume are
        sampled at the same time. This may necessitate scaling opacity values of individual transfer
        functions by means of the `Opacity Scaling` property.
    )"_unindentHelp};
const ProcessorInfo& VolumeRasterizer::getProcessorInfo() const { return processorInfo_; }

VolumeRasterizer::VolumeRasterizer()
    : Rasterizer{}
    , volumeInport_{"volume", "Input volume"_help}
    , meshInport_{"geometry",
                  "Input mesh used for entry and exit points during ray casting. The mesh needs to be closed."_help}
    , shader_{"volumerasterizer.vert", "volumerasterizer.frag", Shader::Build::No}
    , tf_{"tf", "TF & Isovalues", &volumeInport_, InvalidationLevel::InvalidResources}
    , channel_{"channel",
               "Render Channel",
               {{"channel1", "Channel 1", 0},
                {"channel2", "Channel 2", 1},
                {"channel3", "Channel 3", 2},
                {"channel4", "Channel 4", 3}},
               0}
    , opacityScaling_{"opacityScaling", "Opacity Scaling",
                      util::ordinalScale(1.0f, 10.0f)
                          .setInc(0.01f)
                          .set("Scaling factor for the opacity in the transfer function since the "
                               "sampling "
                               "distance is given in world coordinate space."_help)}
    , tfLookup_{std::make_shared<TFLookupTable>(tf_.tf_.get())} {

    addPorts(volumeInport_, meshInport_);
    addProperties(channel_, opacityScaling_, tf_);
}

void VolumeRasterizer::initializeResources() {
    Rasterizer::configureShader(shader_);

    const std::array<std::pair<std::string, bool>, 1> defines = {{{"USE_FRAGMENT_LIST", true}}};

    for (auto&& [key, val] : defines) {
        for (auto&& so : shader_.getShaderObjects()) {
            so.setShaderDefine(key, val);
        }
    }
    shader_.build();
}

void VolumeRasterizer::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) {
    const auto boundingMesh = meshInport_.getData();

    const mat4 meshDataToVolumeData =
        volumeInport_.getData()->getCoordinateTransformer().getWorldToDataMatrix() *
        boundingMesh->getCoordinateTransformer().getDataToWorldMatrix();

    const utilgl::Activate activate{&shader_};

    setUniforms(shader_);
    shader_.setUniform("halfScreenSize", imageSize / ivec2(2));
    shader_.setUniform("meshDataToVolData", meshDataToVolumeData);

    {
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_FALSE);
        utilgl::DepthMaskState depthMask(GL_FALSE);

        utilgl::CullFaceState culling(GL_NONE);
        utilgl::GlBoolState blend(GL_BLEND, GL_FALSE);

        auto transform = CompositeTransform(boundingMesh->getModelMatrix(),
                                            boundingMesh->getWorldMatrix() * worldMatrixTransform);
        utilgl::setShaderUniforms(shader_, transform, "geometry");

        MeshDrawerGL::DrawObject drawer{boundingMesh->getRepresentation<MeshGL>(),
                                        boundingMesh->getDefaultMeshInfo()};
        drawer.draw();
    }
}

std::optional<mat4> VolumeRasterizer::boundingBox() const {
    return util::boundingBox(volumeInport_)();
}

Document VolumeRasterizer::getInfo() const {
    Document doc;
    doc.append("p", "Volume rasterization functor");
    return doc;
}

void VolumeRasterizer::setUniforms(Shader& shader) {
    Rasterizer::setUniforms(shader);

    shader_.setUniform("externalColor", vec4{1.0f, 0.0f, 1.0f, 1.0f});
}

std::optional<RaycastingState> VolumeRasterizer::getRaycastingState() const {
    return RaycastingState{
        .tfLookup = tfLookup_,
        .channel = channel_,
        .opacityScaling = opacityScaling_,
        .volume = volumeInport_.getData(),
    };
}

}  // namespace inviwo
