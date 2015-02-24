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

#include "multichannelraycaster.h"
#include <modules/opengl/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/volumeutils.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/opengl/glwrap/shader.h>

namespace inviwo {

ProcessorClassIdentifier(MultichannelRaycaster, "org.inviwo.MultichannelRaycaster");
ProcessorDisplayName(MultichannelRaycaster,  "Multichannel Raycaster");
ProcessorTags(MultichannelRaycaster, Tags::GL);
ProcessorCategory(MultichannelRaycaster, "Volume Rendering");
ProcessorCodeState(MultichannelRaycaster, CODE_STATE_EXPERIMENTAL);

MultichannelRaycaster::MultichannelRaycaster()
    : Processor()
    , shader_(nullptr)
    , shaderFileName_("multichannelraycaster.frag")
    , volumePort_("volume")
    , entryPort_("entry-points")
    , exitPort_("exit-points")
    , outport_("outport")
    , transferFunctions_("transfer-functions", "Transfer functions")
    , raycasting_("raycaster", "Raycasting")
    , camera_("camera", "Camera")
    , lighting_("lighting", "Lighting", &camera_) {
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction1", "Channel 1", TransferFunction(), &volumePort_));
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction2", "Channel 2", TransferFunction(), &volumePort_));
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction3", "Channel 3", TransferFunction(), &volumePort_));
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction4", "Channel 4", TransferFunction(), &volumePort_));

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");

    addProperty(raycasting_);
    addProperty(camera_);
    addProperty(lighting_);

    addProperty(transferFunctions_);
    
    volumePort_.onChange(this, &MultichannelRaycaster::initializeResources);
}


MultichannelRaycaster::~MultichannelRaycaster() {
    std::vector<Property*> tfs = transferFunctions_.getProperties();
    while (!tfs.empty()) {
        delete tfs.back();
        tfs.pop_back();
    }
}

void MultichannelRaycaster::initialize() {
    Processor::initialize();
    shader_ = new Shader(shaderFileName_, false);
    initializeResources();
}

void MultichannelRaycaster::deinitialize() {
    delete shader_;
    shader_ = nullptr;
    Processor::deinitialize();
}

void MultichannelRaycaster::initializeResources() {
    utilgl::addShaderDefines(shader_, raycasting_);
    utilgl::addShaderDefines(shader_, camera_);
    utilgl::addShaderDefines(shader_, lighting_);

    if (volumePort_.hasData()) {
        const int channels = volumePort_.getData()->getDataFormat()->getComponents();
        
        std::vector<Property*> tfs = transferFunctions_.getProperties();
        for (int i = 0; i < static_cast<int>(tfs.size()); i++) {
            tfs[i]->setVisible(i < channels ? true : false);
        }
        
        std::stringstream ss;
        ss << channels;
        shader_->getFragmentShaderObject()->addShaderDefine("NUMBER_OF_CHANNELS", ss.str());

        std::stringstream ss2;
        for (int i = 0; i < channels; ++i) {
            ss2 << "color = APPLY_CLASSIFICATION(transferFuncC" << i+1 << "_, voxel[" << i << "])"         
                << "color.rgb = APPLY_LIGHTING(light_, color.rgb, color.rgb, vec3(1.0), "
                << "worldSpacePosition, normalize(-gradients[" << i <<"]), toCameraDir);"
                << "result = APPLY_COMPOSITING(result, color, samplePos, voxel, "
                << "gradients[" << i <<"], camera_, isoValue_, t, tDepth, tIncr);";
        }
        shader_->getFragmentShaderObject()->addShaderDefine("SAMPLE_CHANNELS", ss2.str());
        shader_->build();
    }
}

void MultichannelRaycaster::process() {   
    LGL_ERROR;
    TextureUnit entryColorUnit, entryDepthUnit, exitColorUnit, exitDepthUnit, volUnit;
    utilgl::bindTextures(entryPort_, entryColorUnit.getEnum(), entryDepthUnit.getEnum());
    utilgl::bindTextures(exitPort_, exitColorUnit.getEnum(), exitDepthUnit.getEnum());
    utilgl::bindTexture(volumePort_, volUnit);

    std::vector<Property*> tfs = transferFunctions_.getProperties();
    const int channels =  volumePort_.getData()->getDataFormat()->getComponents();
    TextureUnit* transFuncUnits = new TextureUnit[channels];
    GLint* tfUnitNumbers = new GLint[channels];
    for (int channel = 0; channel < channels; channel++) {
        utilgl::bindTexture(*static_cast<TransferFunctionProperty*>(tfs[channel]), 
                            transFuncUnits[channel]);
        tfUnitNumbers[channel] = transFuncUnits[channel].getUnitNumber();
    }
    
    utilgl::activateTargetAndCopySource(outport_, entryPort_, COLOR_DEPTH);
    utilgl::clearCurrentTarget();
    shader_->activate();
    
    utilgl::setShaderUniforms(shader_, outport_, "outportParameters_");

    for (int channel = 0; channel < channels; channel++)
        shader_->setUniform("transferFuncC" + toString<int>(channel+1) + "_", tfUnitNumbers[channel]);

    shader_->setUniform("entryColorTex_", entryColorUnit.getUnitNumber());
    shader_->setUniform("entryDepthTex_", entryDepthUnit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, entryPort_, "entryParameters_");
    shader_->setUniform("exitColorTex_", exitColorUnit.getUnitNumber());
    shader_->setUniform("exitDepthTex_", exitDepthUnit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, exitPort_, "exitParameters_");    
    shader_->setUniform("volume_", volUnit.getUnitNumber());
    utilgl::setShaderUniforms(shader_, volumePort_, "volumeParameters_");
    utilgl::setShaderUniforms(shader_, raycasting_);
    utilgl::setShaderUniforms(shader_, camera_, "camera_");
    utilgl::setShaderUniforms(shader_, lighting_, "light_");
    
    utilgl::singleDrawImagePlaneRect();

    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
    LGL_ERROR;

    delete[] transFuncUnits;
    delete[] tfUnitNumbers;
}


}  // namespace
