/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "volumeraycaster.h"
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/io/serialization/ivwserialization.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/shaderutils.h>
#include <modules/opengl/volumeutils.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeRaycaster,  "org.inviwo.VolumeRaycaster");
ProcessorDisplayName(VolumeRaycaster,  "Volume Raycaster");
ProcessorTags(VolumeRaycaster, Tags::GL);
ProcessorCategory(VolumeRaycaster, "Volume Rendering");
ProcessorCodeState(VolumeRaycaster, CODE_STATE_STABLE);

VolumeRaycaster::VolumeRaycaster()
    : Processor()
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , outport_("outport")
    , transferFunction_("transferFunction", "Transfer function", TransferFunction(), &volumePort_)
    , channel_("channel", "Render Channel")
    , raycasting_("raycaster", "Raycasting")
    , camera_("camera", "Camera")
    , lighting_("lighting", "Lighting", &camera_)
    , positionIndicator_("positionindicator", "Position Indicator")
    , toggleShading_("toggleShading", "Toggle Shading",
        new KeyboardEvent('L'), 
        new Action(this, &VolumeRaycaster::toggleShading)) {
    
    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    
    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();
    
    volumePort_.onChange(this, &VolumeRaycaster::onVolumeChange);

    addProperty(channel_);
    addProperty(transferFunction_);
    addProperty(raycasting_);
    addProperty(camera_);
    addProperty(lighting_);
    addProperty(positionIndicator_);
    addProperty(toggleShading_);
}

VolumeRaycaster::~VolumeRaycaster() {
}

void VolumeRaycaster::initialize() {
    Processor::initialize();
    shader_ = new Shader("raycasting.frag", false);
    initializeResources();
}

void VolumeRaycaster::deinitialize() {
    if (shader_) delete shader_;
    shader_ = NULL;
    Processor::deinitialize();
}

void VolumeRaycaster::initializeResources() {
    utilgl::addShaderDefines(shader_, raycasting_);
    utilgl::addShaderDefines(shader_, camera_);
    utilgl::addShaderDefines(shader_, lighting_);
    utilgl::addShaderDefines(shader_, positionIndicator_);
    shader_->build();
}

void VolumeRaycaster::onVolumeChange() {
    if (volumePort_.hasData()){
        int channels = volumePort_.getData()->getDataFormat()->getComponents();

        if(channels == static_cast<int>(channel_.size()))
            return;
        
        channel_.clearOptions();
        for (int i = 0; i < channels; i++) {
            std::stringstream ss;
            ss << "Channel " << i;
            channel_.addOption(ss.str() , ss.str(), i);
        }
        channel_.setCurrentStateAsDefault();
    }
}

void VolumeRaycaster::process() {
    entryPort_.passOnDataToOutport(&outport_);

    TextureUnit tfUnit, entryColorUnit, entryDepthUnit, exitColorUnit, exitDepthUnit, volUnit;
    utilgl::bindTexture(transferFunction_, tfUnit);
    utilgl::bindTextures(entryPort_, entryColorUnit, entryDepthUnit);
    utilgl::bindTextures(exitPort_, exitColorUnit, exitDepthUnit);
    utilgl::bindTexture(volumePort_, volUnit);

    utilgl::activateAndClearTarget(outport_, COLOR_DEPTH);
    shader_->activate();

    utilgl::setShaderUniforms(shader_, outport_, "outportParameters_");
    shader_->setUniform("transferFunc_", tfUnit.getUnitNumber());
    shader_->setUniform("entryColorTex_", entryColorUnit.getUnitNumber());
    shader_->setUniform("entryDepthTex_", entryDepthUnit.getUnitNumber());   
    utilgl::setShaderUniforms(shader_, entryPort_, "entryParameters_");
    shader_->setUniform("exitColorTex_", exitColorUnit.getUnitNumber());
    shader_->setUniform("exitDepthTex_", exitDepthUnit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, exitPort_, "exitParameters_");     
    shader_->setUniform("channel_", channel_.getSelectedValue());
    shader_->setUniform("volume_", volUnit.getUnitNumber());    
    utilgl::setShaderUniforms(shader_, volumePort_, "volumeParameters_");
    utilgl::setShaderUniforms(shader_, raycasting_);
    utilgl::setShaderUniforms(shader_, camera_, "camera_");
    utilgl::setShaderUniforms(shader_, lighting_, "light_");
    utilgl::setShaderUniforms(shader_, positionIndicator_, "positionIndicator_");

    utilgl::singleDrawImagePlaneRect();

    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
}

void VolumeRaycaster::toggleShading(Event*) {
    if (lighting_.shadingMode_.get() ==  ShadingMode::None) {
        lighting_.shadingMode_.set(ShadingMode::Phong);
    } else {
        lighting_.shadingMode_.set(ShadingMode::None);
    }
}

} // namespace
