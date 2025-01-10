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

#include <modules/basegl/processors/multichannelraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>                // for boundingBox
#include <inviwo/core/datastructures/histogram.h>             // for HistogramSelection
#include <inviwo/core/datastructures/image/imagetypes.h>      // for ImageType, ImageType::Color...
#include <inviwo/core/io/serialization/deserializer.h>        // for Deserializer
#include <inviwo/core/io/serialization/ticpp.h>               // for TxElement
#include <inviwo/core/io/serialization/versionconverter.h>    // for getElement, renamePort, Nod...
#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>            // for CodeState, CodeState::Exper...
#include <inviwo/core/processors/processortags.h>             // for Tags, Tags::GL
#include <inviwo/core/properties/compositeproperty.h>         // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <inviwo/core/util/formats.h>                         // for DataFormatBase
#include <inviwo/core/util/stringconversion.h>                // for StrBuffer
#include <inviwo/core/util/zip.h>                             // for enumerate, zipIterator, zipper
#include <modules/opengl/shader/shader.h>                     // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>               // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                // for addShaderDefines, addShader...
#include <modules/opengl/texture/textureunit.h>               // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>              // for bindAndSetUniforms, activat...
#include <modules/opengl/volume/volumeutils.h>                // for bindAndSetUniforms

#include <bitset>       // for bitset<>::reference
#include <cstddef>      // for size_t
#include <functional>   // for __base, function
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <utility>      // for pair

#include <fmt/core.h>  // for format, basic_string_view

namespace inviwo {

const ProcessorInfo MultichannelRaycaster::processorInfo_{
    "org.inviwo.MultichannelRaycaster",  // Class identifier
    "Multichannel Raycaster",            // Display name
    "Volume Rendering",                  // Category
    CodeState::Experimental,             // Code state
    Tags::GL,                            // Tags
};
const ProcessorInfo& MultichannelRaycaster::getProcessorInfo() const { return processorInfo_; }

MultichannelRaycaster::MultichannelRaycaster()
    : Processor()
    , shader_("multichannelraycaster.frag", Shader::Build::No)
    , volumePort_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , backgroundPort_("bg")
    , outport_("outport")
    , transferFunctions_("transferfunctions", "Transfer functions")
    , tfs_{{{"transferFunction1", "Channel 1", &volumePort_},
            {"transferFunction2", "Channel 2", &volumePort_},
            {"transferFunction3", "Channel 3", &volumePort_},
            {"transferFunction4", "Channel 4", &volumePort_}}}
    , raycasting_("raycaster", "Raycasting")
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , lighting_("lighting", "Lighting", &camera_)
    , positionIndicator_("positionindicator", "Position Indicator") {

    transferFunctions_.addProperties(tfs_[0], tfs_[1], tfs_[2], tfs_[3]);
    for (auto&& [i, tf] : util::enumerate(tfs_)) {
        HistogramSelection selection{};
        selection[i] = true;
        tf.setHistogramSelection(selection);
        tf.setCurrentStateAsDefault();
    }

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addPort(backgroundPort_, "ImagePortGroup1");

    backgroundPort_.setOptional(true);

    addProperties(raycasting_, camera_, lighting_, positionIndicator_, transferFunctions_);

    volumePort_.onChange([this]() { initializeResources(); });

    backgroundPort_.onConnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
    backgroundPort_.onDisconnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
}

void MultichannelRaycaster::initializeResources() {
    utilgl::addShaderDefines(shader_, raycasting_);
    utilgl::addShaderDefines(shader_, camera_);
    utilgl::addShaderDefines(shader_, lighting_);
    utilgl::addShaderDefines(shader_, positionIndicator_);
    utilgl::addShaderDefinesBGPort(shader_, backgroundPort_);

    if (volumePort_.hasData()) {
        size_t channels = volumePort_.getData()->getDataFormat()->getComponents();
        for (auto&& [i, tf] : util::enumerate(tfs_)) {
            tf.setVisible(i < channels);
        }

        shader_.getFragmentShaderObject()->addShaderDefine("NUMBER_OF_CHANNELS",
                                                           fmt::format("{}", channels));
        StrBuffer str;
        for (size_t i = 0; i < channels; ++i) {
            str.append(
                "color[{0}] = APPLY_CHANNEL_CLASSIFICATION(transferFunction{1}, voxel, {0});", i,
                i + 1);
        }
        shader_.getFragmentShaderObject()->addShaderDefine("SAMPLE_CHANNELS", str.view());

        shader_.build();
    }
}

void MultichannelRaycaster::process() {
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, volumePort_);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);
    if (backgroundPort_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, units, backgroundPort_, ImageType::ColorDepthPicking);
    }

    size_t channels = volumePort_.getData()->getDataFormat()->getComponents();
    for (size_t channel = 0; channel < channels; channel++) {
        utilgl::bindAndSetUniforms(shader_, units, tfs_[channel]);
    }
    utilgl::setUniforms(shader_, outport_, camera_, lighting_, raycasting_, positionIndicator_);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void MultichannelRaycaster::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});

    NodeVersionConverter vc([](TxElement* node) {
        if (TxElement* p1 = xml::getElement(
                node, "InPorts/InPort&type=org.inviwo.ImageMultiInport&identifier=exit-points")) {
            p1->SetAttribute("identifier", "exit");
        }
        if (TxElement* p2 = xml::getElement(
                node, "InPorts/InPort&type=org.inviwo.ImageMultiInport&identifier=entry-points")) {
            p2->SetAttribute("identifier", "entry");
        }
        return true;
    });

    d.convertVersion(&vc);
    Processor::deserialize(d);
}

}  // namespace inviwo
