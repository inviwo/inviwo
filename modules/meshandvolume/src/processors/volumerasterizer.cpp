/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/meshandvolume/processors/volumerasterizer.h>

#include <modules/opengl/volume/volumegl.h>
#include <variant>
#include <sstream>
#include <inviwo/core/algorithm/boundingbox.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/texture3d.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shaderutils.h>
#pragma optimize("",off)
namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRasterizer::processorInfo_{
    "org.inviwo.VolumeRasterizer",  // Class identifier
    "Volume Rasterizer",             // Display name
    "Volume Rendering",          // Category
    CodeState::Experimental,        // Code state
    Tags::GL,                     // Tags
};
const ProcessorInfo VolumeRasterizer::getProcessorInfo() const { return processorInfo_; }

void VolumeRasterizer::setUniforms(Shader& shader, std::string_view prefix) const {
    shader.setUniform(
        "volumeid", volumeInport_.getData()->getRepresentation<VolumeGL>()->getTexture()->getID());
    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader, units, exitPort_, ImageType::ColorDepth);
    /*
    uniform ImageParameters entryParameters;
    uniform sampler2D entryColor;
    uniform sampler2D entryDepth;
    uniform sampler2D entryPicking;
    uniform sampler2D entryNormal;

    uniform ImageParameters exitParameters;
    uniform sampler2D exitColor;
    uniform sampler2D exitDepth;
    */

    std::array<std::pair<std::string_view, std::variant<bool, int, float, vec4>>, 15> uniforms{
        {{"externalColor", vec4{1.0f, 0.0f, 1.0f, 1.0f}}}};

    for (const auto& [key, val] : uniforms) {
        std::visit([&, akey = key](
                       auto aval) { shader.setUniform(fmt::format("{}{}", prefix, akey), aval); },
                   val);
    }
}

VolumeRasterizer::VolumeRasterizer()
    : Processor()
    , volumeInport_{"volume"}
    , entryPort_{"entry"}
    , exitPort_{"exit"}
    , outport_("outport")
    , shader_(std::make_shared<Shader>("myvolumerasterizer.vert", "myvolumerasterizer.frag",
                                       Shader::Build::No))
    , tf_("tf", "TF & Isovalues", &volumeInport_, InvalidationLevel::InvalidResources)
    , channel_("channel", "Render Channel",
               {{"channel1", "Channel 1", 0},
                {"channel2", "Channel 2", 1},
                {"channel3", "Channel 3", 2},
                {"channel4", "Channel 4", 3}},
               0)
    , camera_("camera", "Camera", util::boundingBox(volumeInport_))
    , lighting_("lighting", "Lighting", &camera_) {

    addPort(volumeInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);
    addProperties(channel_, tf_, camera_, lighting_);
}

void VolumeRasterizer::process() {
    LogWarn("Entry has data " << entryPort_.hasData());
    std::shared_ptr<const Rasterization> rasterization =
        std::make_shared<const VolumeRasterization>(*this);

    outport_.setData(rasterization);
}

void VolumeRasterizer::initializeResources() {
    auto fso = shader_->getFragmentShaderObject();

    fso->addShaderExtension("GL_NV_gpu_shader5", true);
    fso->addShaderExtension("GL_EXT_shader_image_load_store", true);
    fso->addShaderExtension("GL_NV_shader_buffer_load", true);
    fso->addShaderExtension("GL_EXT_bindable_uniform", true);

    // Defines needed?
    const std::array<std::pair<std::string, bool>, 15> defines = {{{"USE_FRAGMENT_LIST", true}}};

    for (auto&& [key, val] : defines) {
        for (auto&& so : shader_->getShaderObjects()) {
            so.setShaderDefine(key, val);
        }
    }
    shader_->build();
}

VolumeRasterization::VolumeRasterization(const VolumeRasterizer& processor)
    : raycastState_{processor.tf_.tf_.get(), processor.lighting_.getState(),
                    processor.channel_.get()}
    , shader_(processor.shader_)
    , entry_(processor.entryPort_.getData())
    , exit_(processor.exitPort_.getData())
    , volume_(processor.volumeInport_.getData()) {
    if (!entry_) {
        throw Exception("Entry no data", IVW_CONTEXT);
    }
}

void VolumeRasterization::rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                                    std::function<void(Shader&)> setUniformsRenderer) const {
    shader_->activate();

    shader_->setUniform("halfScreenSize", imageSize / ivec2(2));

    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(*shader_, cont, *volume_, "volume");
    utilgl::bindAndSetUniforms(*shader_, cont, *entry_, "entry", ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(*shader_, cont, *exit_, "exit", ImageType::ColorDepth);

    setUniformsRenderer(*shader_);

    // Set opengl states?
    {

        utilgl::singleDrawImagePlaneRect();  // shader use entry/exit to set fragment flag to volume
                                             // and flag for entry exit
                                             // empty entry/exit = black and entry=exit in shader
    }
    shader_->deactivate();
}

Rasterization* VolumeRasterization::clone() const { return new VolumeRasterization(*this); }

const VolumeRasterization::RaycastingState* VolumeRasterization::getRaycastingState() const {
    return &raycastState_;
}

}  // namespace inviwo
