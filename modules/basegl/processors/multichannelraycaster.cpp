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
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

const ProcessorInfo MultichannelRaycaster::processorInfo_{
    "org.inviwo.MultichannelRaycaster",  // Class identifier
    "Multichannel Raycaster",            // Display name
    "Volume Rendering",                  // Category
    CodeState::Experimental,             // Code state
    Tags::GL,                            // Tags
};
const ProcessorInfo MultichannelRaycaster::getProcessorInfo() const {
    return processorInfo_;
}

MultichannelRaycaster::MultichannelRaycaster()
    : Processor()
    , shader_("multichannelraycaster.frag", false)
    , volumePort_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , outport_("outport")
    , transferFunctions_("transfer-functions", "Transfer functions")
    , raycasting_("raycaster", "Raycasting")
    , camera_("camera", "Camera")
    , lighting_("lighting", "Lighting", &camera_)
    , positionIndicator_("positionindicator", "Position Indicator") {
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction1", "Channel 1", TransferFunction(), &volumePort_), false);
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction2", "Channel 2", TransferFunction(), &volumePort_), false);
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction3", "Channel 3", TransferFunction(), &volumePort_), false);
    transferFunctions_.addProperty(new TransferFunctionProperty(
        "transferFunction4", "Channel 4", TransferFunction(), &volumePort_), false);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");

    addProperty(raycasting_);
    addProperty(camera_);
    addProperty(lighting_);
    addProperty(positionIndicator_);
    addProperty(transferFunctions_);
    
    volumePort_.onChange(this, &MultichannelRaycaster::initializeResources);
}

MultichannelRaycaster::~MultichannelRaycaster() {
    for (auto tf : transferFunctions_.getProperties()) {
        delete tf;
    }
}

void MultichannelRaycaster::initializeResources() {
    utilgl::addShaderDefines(shader_, raycasting_);
    utilgl::addShaderDefines(shader_, camera_);
    utilgl::addShaderDefines(shader_, lighting_);
    utilgl::addShaderDefines(shader_, positionIndicator_);

    if (volumePort_.hasData()) {
        size_t channels = volumePort_.getData()->getDataFormat()->getComponents();
        
        auto tfs = transferFunctions_.getPropertiesByType<TransferFunctionProperty>();
        for (size_t i = 0; i < tfs.size(); i++) {
            tfs[i]->setVisible(i < channels ? true : false);
        }
        
        std::stringstream ss;
        ss << channels;
        shader_.getFragmentShaderObject()->addShaderDefine("NUMBER_OF_CHANNELS", ss.str());

        std::stringstream ss2;
        for (size_t i = 0; i < channels; ++i) {
            ss2 << "color[" << i << "] = APPLY_CHANNEL_CLASSIFICATION(transferFunction" << i + 1 << ", voxel, " << i << ");";
        }
        shader_.getFragmentShaderObject()->addShaderDefine("SAMPLE_CHANNELS", ss2.str());
        shader_.build();
    }
}

void MultichannelRaycaster::process() {
    LGL_ERROR;
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, volumePort_);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);

    auto tfs = transferFunctions_.getPropertiesByType<TransferFunctionProperty>();
    size_t channels = volumePort_.getData()->getDataFormat()->getComponents();
    for (size_t channel = 0; channel < channels; channel++) {
        utilgl::bindAndSetUniforms(shader_, units, *tfs[channel]);
    }
    utilgl::setUniforms(shader_, outport_, camera_, lighting_, raycasting_, positionIndicator_);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
    LGL_ERROR;
}

void MultichannelRaycaster::deserialize(IvwDeserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});

    NodeVersionConverter vc([](TxElement* node) {      
        if (TxElement* p1 = util::xmlGetElement(
            node, "InPorts/InPort&type=org.inviwo.ImageMultiInport&identifier=exit-points")) {
            p1->SetAttribute("identifier", "exit");
        } 
        if (TxElement* p2 = util::xmlGetElement(
            node, "InPorts/InPort&type=org.inviwo.ImageMultiInport&identifier=entry-points")) {
            p2->SetAttribute("identifier", "entry");
        }
        return true;
    });

    d.convertVersion(&vc);
    Processor::deserialize(d);
}

}  // namespace

