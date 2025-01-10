/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/basegl/processors/lightingraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>                // for boundingBox
#include <inviwo/core/datastructures/image/imagetypes.h>      // for ImageType, ImageType::Color...
#include <inviwo/core/io/serialization/deserializer.h>        // for Deserializer
#include <inviwo/core/io/serialization/ticpp.h>               // for TxElement
#include <inviwo/core/io/serialization/versionconverter.h>    // for getElement, NodeVersionConv...
#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>            // for CodeState, CodeState::Exper...
#include <inviwo/core/processors/processortags.h>             // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>              // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>            // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyOption, Optio...
#include <inviwo/core/properties/simplelightingproperty.h>    // for SimpleLightingProperty
#include <inviwo/core/properties/simpleraycastingproperty.h>  // for SimpleRaycastingProperty
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <inviwo/core/util/formats.h>                         // for DataFormatBase
#include <inviwo/core/util/stringconversion.h>                // for toString
#include <modules/opengl/shader/shader.h>                     // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>               // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                // for addDefines, addShaderDefine...
#include <modules/opengl/texture/textureunit.h>               // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>              // for bindAndSetUniforms, activat...
#include <modules/opengl/volume/volumeutils.h>                // for bindAndSetUniforms

#include <cstddef>      // for size_t
#include <functional>   // for __base, function
#include <memory>       // for shared_ptr
#include <string>       // for operator+, string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector

namespace inviwo {

const ProcessorInfo LightingRaycaster::processorInfo_{
    "org.inviwo.LightingRaycaster",  // Class identifier
    "Lighting Raycaster",            // Display name
    "Volume Rendering",              // Category
    CodeState::Experimental,         // Code state
    Tags::GL,                        // Tags
};
const ProcessorInfo& LightingRaycaster::getProcessorInfo() const { return processorInfo_; }

LightingRaycaster::LightingRaycaster()
    : Processor()
    , shader_("lighting/lightingraycasting.frag", Shader::Build::No)
    , volumePort_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , lightVolumePort_("lightVolume")
    , backgroundPort_("bg")
    , outport_("outport")
    , enableLightColor_("supportColoredLight", "Enable Light Color", false,
                        InvalidationLevel::InvalidResources)
    , lightVolumeScaling_("lightVolumeScaling", "Light Volume Scaling",
                          util::ordinalScale(1.0f, 2.0f))
    , transferFunction_("transferFunction", "Transfer function", &volumePort_)
    , channel_("channel", "Render Channel")
    , raycasting_("raycaster", "Raycasting")
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , lighting_("lighting", "Lighting", &camera_) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(lightVolumePort_);
    addPort(backgroundPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");

    backgroundPort_.setOptional(true);

    channel_.addOption("Channel 1", "Channel 1", 0);
    channel_.setCurrentStateAsDefault();

    volumePort_.onChange([this]() {
        if (volumePort_.hasData()) {
            size_t channels = volumePort_.getData()->getDataFormat()->getComponents();

            if (channels == channel_.size()) return;

            std::vector<OptionPropertyIntOption> channelOptions;
            for (size_t i = 0; i < channels; i++) {
                channelOptions.emplace_back("Channel " + toString(i + 1),
                                            "Channel " + toString(i + 1), static_cast<int>(i));
            }
            channel_.replaceOptions(channelOptions);
            channel_.setCurrentStateAsDefault();
        }
    });

    backgroundPort_.onConnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
    backgroundPort_.onDisconnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });

    addProperties(raycasting_, camera_, lighting_, channel_, transferFunction_);
    lighting_.addProperties(enableLightColor_, lightVolumeScaling_);
}

void LightingRaycaster::initializeResources() {
    utilgl::addDefines(shader_, raycasting_, camera_, lighting_);
    utilgl::addShaderDefinesBGPort(shader_, backgroundPort_);
    shader_.getFragmentShaderObject()->setShaderDefine("LIGHT_COLOR_ENABLED", enableLightColor_);

    shader_.build();
}

void LightingRaycaster::process() {
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, volumePort_);
    utilgl::bindAndSetUniforms(shader_, units, lightVolumePort_);
    utilgl::bindAndSetUniforms(shader_, units, transferFunction_);
    utilgl::bindAndSetUniforms(shader_, units, *entryPort_.getData(), "entry",
                               ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, *exitPort_.getData(), "exit", ImageType::ColorDepth);
    if (backgroundPort_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, units, backgroundPort_, ImageType::ColorDepthPicking);
    }
    utilgl::setUniforms(shader_, outport_, camera_, lighting_, raycasting_, channel_,
                        lightVolumeScaling_);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void LightingRaycaster::deserialize(Deserializer& d) {
    NodeVersionConverter tvc([](TxElement* node) {
        TxElement* p = xml::getElement(
            node, "Properties/Property&type=OptionPropertyString&identifier=shadingMode");
        if (p) p->SetAttribute("type", "OptionPropertyInt");
        return true;
    });
    d.convertVersion(&tvc);
    Processor::deserialize(d);
}

}  // namespace inviwo
