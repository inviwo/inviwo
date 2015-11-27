/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "lightingraycaster.h"
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

const ProcessorInfo LightingRaycaster::processorInfo_{
    "org.inviwo.LightingRaycaster",  // Class identifier
    "Lighting Raycaster",            // Display name
    "Volume Rendering",              // Category
    CodeState::Experimental,         // Code state
    Tags::GL,                        // Tags
};
const ProcessorInfo LightingRaycaster::getProcessorInfo() const {
    return processorInfo_;
}

LightingRaycaster::LightingRaycaster()
    : Processor()
    , shader_("lighting/lightingraycasting.frag", false)
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , lightVolumePort_("lightVolume")
    , outport_("outport")
    , enableLightColor_("supportColoredLight", "Enable Light Color", false,
                        InvalidationLevel::InvalidResources)
    , transferFunction_("transferFunction", "Transfer function", TransferFunction(), &volumePort_)
    , channel_("channel", "Render Channel")
    , raycasting_("raycaster", "Raycasting")
    , camera_("camera", "Camera")
    , lighting_("lighting", "Lighting", &camera_) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(lightVolumePort_);
    addPort(outport_, "ImagePortGroup1");

    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();

    volumePort_.onChange(this, &LightingRaycaster::onVolumeChange);

    addProperty(raycasting_);
    addProperty(camera_);
    lighting_.addProperty(enableLightColor_);
    addProperty(lighting_);
    addProperty(channel_);
    addProperty(transferFunction_);
}

void LightingRaycaster::initializeResources() {
    utilgl::addShaderDefines(shader_, raycasting_);
    utilgl::addShaderDefines(shader_, camera_);
    utilgl::addShaderDefines(shader_, lighting_);

    if (enableLightColor_.get())
        shader_.getFragmentShaderObject()->addShaderDefine("LIGHT_COLOR_ENABLED");
    else
        shader_.getFragmentShaderObject()->removeShaderDefine("LIGHT_COLOR_ENABLED");

    shader_.build();
}

void LightingRaycaster::onVolumeChange() {
    if (volumePort_.hasData()) {
        std::size_t channels = volumePort_.getData()->getDataFormat()->getComponents();

        if (channels == channel_.size())
            return;

        channel_.clearOptions();
        for (int i = 0; i < static_cast<int>(channels); i++) {
            std::stringstream ss;
            ss << "Channel " << i;
            channel_.addOption(ss.str(), ss.str(), i);
        }
        channel_.setCurrentStateAsDefault();
    }
}


void LightingRaycaster::process() {
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, volumePort_);
    utilgl::bindAndSetUniforms(shader_, units, lightVolumePort_);
    utilgl::bindAndSetUniforms(shader_, units, transferFunction_);
    utilgl::bindAndSetUniforms(shader_, units, *entryPort_.getData(), "entry", ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, *exitPort_.getData(), "exit", ImageType::ColorDepth);
    utilgl::setUniforms(shader_, outport_, camera_, lighting_, raycasting_, channel_);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void LightingRaycaster::deserialize(Deserializer& d) {
    NodeVersionConverter tvc([](TxElement* node) {
        TxElement* p = util::xmlGetElement(
            node, "Properties/Property&type=OptionPropertyString&identifier=shadingMode");
        if (p) p->SetAttribute("type", "OptionPropertyInt");
        return true;
    });
    d.convertVersion(&tvc);
    Processor::deserialize(d);
}

} // namespace

