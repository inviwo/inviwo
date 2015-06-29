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

#include "isoraycaster.h"
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/shaderutils.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/volumeutils.h>

namespace inviwo {

ProcessorClassIdentifier(ISORaycaster, "org.inviwo.ISORaycaster");
ProcessorDisplayName(ISORaycaster, "ISO Raycaster");
ProcessorTags(ISORaycaster, Tags::GL);
ProcessorCategory(ISORaycaster, "Volume Rendering");
ProcessorCodeState(ISORaycaster, CODE_STATE_STABLE);

ISORaycaster::ISORaycaster()
    : Processor()
    , shader_(nullptr)
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , outport_("outport")
    , channel_("channel", "Render Channel")
    , raycasting_("raycasting", "Raycasting")
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, 3.5f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f)) 
    , lighting_("lighting", "Lighting", &camera_)
{
    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");

    volumePort_.onChange(this, &ISORaycaster::onVolumeChange);

    addProperty(channel_);
    addProperty(raycasting_);
    addProperty(camera_);
    addProperty(lighting_);


    std::stringstream ss;
    ss << "Channel " << 0;
    channel_.addOption(ss.str() , ss.str(), 0);

    raycasting_.compositingMode_.setVisible(false);
    setAllPropertiesCurrentStateAsDefault();
}


void ISORaycaster::initialize() {
    Processor::initialize();
    shader_ = new Shader("isoraycasting.frag", false);
    initializeResources();
}

void ISORaycaster::deinitialize() {
    if (shader_) delete shader_;
    shader_ = nullptr;
    Processor::deinitialize();
}


void ISORaycaster::initializeResources(){
    
    utilgl::addShaderDefines(shader_, raycasting_);
    utilgl::addShaderDefines(shader_, camera_);
    utilgl::addShaderDefines(shader_, lighting_);
  
    shader_->build();
}
    
void ISORaycaster::onVolumeChange(){
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

void ISORaycaster::process() {
    TextureUnit entryColorUnit, entryDepthUnit, exitColorUnit, exitDepthUnit, volUnit;
    utilgl::bindTextures(entryPort_, entryColorUnit.getEnum(), entryDepthUnit.getEnum());
    utilgl::bindTextures(exitPort_, exitColorUnit.getEnum(), exitDepthUnit.getEnum());
    utilgl::bindTexture(volumePort_, volUnit);

    utilgl::activateTargetAndCopySource(outport_, entryPort_, COLOR_DEPTH);
    utilgl::clearCurrentTarget();
    shader_->activate();
    
    utilgl::setShaderUniforms(shader_, outport_, "outportParameters_");
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

    utilgl::singleDrawImagePlaneRect();
    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
}

} // namespace
