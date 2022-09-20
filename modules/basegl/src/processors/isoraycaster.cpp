/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <modules/basegl/processors/isoraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>                // for boundingBox
#include <inviwo/core/datastructures/image/imagetypes.h>      // for ImageType, ImageType::Color...
#include <inviwo/core/io/serialization/versionconverter.h>    // for renamePort
#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>            // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>             // for Tags, Tags::GL
#include <inviwo/core/properties/cameraproperty.h>            // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyInt, OptionPr...
#include <inviwo/core/properties/ordinalproperty.h>           // for FloatVec4Property
#include <inviwo/core/properties/propertysemantics.h>         // for PropertySemantics, Property...
#include <inviwo/core/properties/simplelightingproperty.h>    // for SimpleLightingProperty
#include <inviwo/core/properties/simpleraycastingproperty.h>  // for SimpleRaycastingProperty
#include <inviwo/core/util/formats.h>                         // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                          // for vec4
#include <modules/opengl/shader/shader.h>                     // for Shader, Shader::Build
#include <modules/opengl/shader/shaderutils.h>                // for addShaderDefines, addShader...
#include <modules/opengl/texture/textureunit.h>               // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>              // for bindAndSetUniforms, activat...
#include <modules/opengl/volume/volumeutils.h>                // for bindAndSetUniforms

#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <sstream>      // for stringstream, operator<<
#include <string>       // for char_traits, string, basic_...
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <utility>      // for pair

namespace inviwo {
class Deserializer;

const ProcessorInfo ISORaycaster::processorInfo_{
    "org.inviwo.ISORaycaster",  // Class identifier
    "ISO Raycaster",            // Display name
    "Volume Rendering",         // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
};
const ProcessorInfo ISORaycaster::getProcessorInfo() const { return processorInfo_; }

ISORaycaster::ISORaycaster()
    : Processor()
    , shader_("isoraycasting.frag", Shader::Build::No)
    , volumePort_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , backgroundPort_("bg")
    , outport_("outport")
    , surfaceColor_("surfaceColor", "Surface Color", vec4(1, 1, 1, 1))
    , channel_("channel", "Render Channel")
    , raycasting_("raycasting", "Raycasting")
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , lighting_("lighting", "Lighting", &camera_) {
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addPort(backgroundPort_, "ImagePortGroup1");

    surfaceColor_.setSemantics(PropertySemantics::Color);

    backgroundPort_.setOptional(true);

    volumePort_.onChange([this]() {
        if (volumePort_.hasData()) {
            std::size_t channels = volumePort_.getData()->getDataFormat()->getComponents();

            if (channels == channel_.size()) return;

            channel_.clearOptions();
            for (int i = 0; i < static_cast<int>(channels); i++) {
                std::stringstream ss;
                ss << "Channel " << i;
                channel_.addOption(ss.str(), ss.str(), i);
            }
            channel_.setCurrentStateAsDefault();
        }
    });

    backgroundPort_.onConnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
    backgroundPort_.onDisconnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });

    addProperty(surfaceColor_);
    addProperty(channel_);
    addProperty(raycasting_);
    addProperty(camera_);
    addProperty(lighting_);

    std::stringstream ss;
    ss << "Channel " << 0;
    channel_.addOption(ss.str(), ss.str(), 0);

    raycasting_.compositingMode_.setVisible(false);
    setAllPropertiesCurrentStateAsDefault();
}

void ISORaycaster::initializeResources() {
    utilgl::addShaderDefines(shader_, raycasting_);
    utilgl::addShaderDefines(shader_, camera_);
    utilgl::addShaderDefines(shader_, lighting_);
    utilgl::addShaderDefinesBGPort(shader_, backgroundPort_);
    shader_.build();
}

void ISORaycaster::process() {
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, volumePort_);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);
    if (backgroundPort_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, units, backgroundPort_, ImageType::ColorDepthPicking);
    }

    utilgl::setUniforms(shader_, outport_, camera_, lighting_, raycasting_, channel_,
                        surfaceColor_);

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void ISORaycaster::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace inviwo
