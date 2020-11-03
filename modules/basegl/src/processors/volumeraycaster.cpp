/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <modules/basegl/processors/volumeraycaster.h>

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/datastructures/histogram.h>                       // for HistogramSelection
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/light/lightingstate.h>             // for ShadingMode, Shad...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/interaction/events/keyboardkeys.h>                // for IvwKey, IvwKey::L
#include <inviwo/core/io/serialization/versionconverter.h>              // for renamePort
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Imag...
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/poolprocessor.h>                       // for PoolProcessor
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/cameraproperty.h>                      // for CameraProperty
#include <inviwo/core/properties/eventproperty.h>                       // for EventProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/isotfproperty.h>                       // for IsoTFProperty
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/properties/raycastingproperty.h>                  // for RaycastingProperty
#include <inviwo/core/properties/simplelightingproperty.h>              // for SimpleLightingPro...
#include <inviwo/core/properties/valuewrapper.h>                        // for PropertySerializa...
#include <inviwo/core/properties/volumeindicatorproperty.h>             // for VolumeIndicatorPr...
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <modules/opengl/image/layergl.h>                               // for LayerGL
#include <modules/opengl/inviwoopengl.h>                                // for glFinish
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shaderutils.h>                          // for addDefines, addSh...
#include <modules/opengl/texture/texture2d.h>                           // for Texture2D
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for bindAndSetUniforms
#include <modules/opengl/volume/volumegl.h>                             // IWYU pragma: keep
#include <modules/opengl/volume/volumeutils.h>                          // for bindAndSetUniforms

#include <bitset>         // for bitset<>::reference
#include <cstddef>        // for size_t
#include <functional>     // for __base
#include <memory>         // for shared_ptr, uniqu...
#include <string>         // for string, operator+
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair
#include <vector>         // for vector

#include <inviwo/tracy/tracy.h>
#include <inviwo/tracy/tracyopengl.h>

namespace inviwo {
class Deserializer;
class Event;

const ProcessorInfo VolumeRaycaster::processorInfo_{
    "org.inviwo.VolumeRaycaster",  // Class identifier
    "Volume Raycaster",            // Display name
    "Volume Rendering",            // Category
    CodeState::Stable,             // Code state
    "GL, DVR, Raycasting"          // Tags
};

VolumeRaycaster::VolumeRaycaster()
    : PoolProcessor()
    , shader_("raycasting.frag", Shader::Build::No)
    , volumePort_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , backgroundPort_("bg")
    , outport_("outport")
    , channel_("channel", "Render Channel", {{"Channel 1", "Channel 1", 0}}, 0)
    , raycasting_("raycaster", "Raycasting")
    , isotfComposite_("isotfComposite", "TF & Isovalues", &volumePort_)
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , lighting_("lighting", "Lighting", &camera_)
    , positionIndicator_("positionindicator", "Position Indicator")
    , toggleShading_(
          "toggleShading", "Toggle Shading", [this](Event* e) { toggleShading(e); }, IvwKey::L) {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addPort(backgroundPort_, "ImagePortGroup1");

    backgroundPort_.setOptional(true);

    channel_.setSerializationMode(PropertySerializationMode::All);

    auto updateTFHistSel = [this]() {
        HistogramSelection selection{};
        selection[channel_] = true;
        isotfComposite_.setHistogramSelection(selection);
    };
    updateTFHistSel();
    channel_.onChange(updateTFHistSel);

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

    // change the currently selected channel when a pre-computed gradient is selected
    raycasting_.gradientComputation_.onChange([this]() {
        if (channel_.size() == 4) {
            if (raycasting_.gradientComputation_.get() ==
                RaycastingProperty::GradientComputation::PrecomputedXYZ) {
                channel_.set(3);
            } else if (raycasting_.gradientComputation_.get() ==
                       RaycastingProperty::GradientComputation::PrecomputedYZW) {
                channel_.set(0);
            }
        }
    });

    addProperty(channel_);
    addProperty(raycasting_);
    addProperty(isotfComposite_);

    addProperty(camera_);
    addProperty(lighting_);
    addProperty(positionIndicator_);
    addProperty(toggleShading_);
}

const ProcessorInfo VolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

void VolumeRaycaster::initializeResources() {
    utilgl::addDefines(shader_, raycasting_, isotfComposite_, camera_, lighting_,
                       positionIndicator_);
    utilgl::addShaderDefinesBGPort(shader_, backgroundPort_);
    shader_.build();
}

void VolumeRaycaster::process() {
    if (volumePort_.isChanged()) {
        dispatchOne(
            [volume = volumePort_.getData()]() {
                volume->getRep<kind::GL>();
                glFinish();
                return volume;
            },
            [this](std::shared_ptr<const Volume> volume) {
                raycast(*volume);
                newResults();
            });
    } else {
        raycast(*volumePort_.getData());
    }
}

void VolumeRaycaster::raycast(const Volume& volume) {
    TRACY_ZONE_SCOPED_NC("VolumeRaycaster", 0x008800);

    if (!volume.getRep<kind::GL>()) {
        throw Exception("Could not find VolumeGL representation", IVW_CONTEXT);
    }

    {
        TRACY_ZONE_SCOPED_NC("Clear", 0x008800);
        TRACY_GPU_ZONE_C("Clear", 0x008800);
    utilgl::activateAndClearTarget(outport_);
    }
    shader_.activate();
    TextureUnitContainer units;

    {
        TRACY_ZONE_SCOPED_NC("BindAndSetUniforms", 0x008800);
        TRACY_GPU_ZONE_C("BindAndSetUniforms", 0x008800);
    utilgl::bindAndSetUniforms(shader_, units, volume, "volume");
    utilgl::bindAndSetUniforms(shader_, units, isotfComposite_);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);
    if (backgroundPort_.hasData()) {
            utilgl::bindAndSetUniforms(shader_, units, backgroundPort_,
                                       ImageType::ColorDepthPicking);
    }
    if (auto normals = entryPort_.getData()->getColorLayer(1)) {
        utilgl::bindAndSetUniforms(shader_, units,
                                   *normals->getRepresentation<LayerGL>()->getTexture(),
                                   std::string_view{"entryNormal"});
        shader_.setUniform("useNormals", true);
    } else {
        shader_.setUniform("useNormals", false);
    }

    utilgl::setUniforms(shader_, outport_, camera_, lighting_, raycasting_, positionIndicator_,
                        channel_, isotfComposite_);
    }
    {
        TRACY_ZONE_SCOPED_NC("Raycast", 0x008800);
        TRACY_GPU_ZONE_C("Raycast", 0x008800);
    utilgl::singleDrawImagePlaneRect();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void VolumeRaycaster::toggleShading(Event*) {
    if (lighting_.shadingMode_.get() == ShadingMode::None) {
        lighting_.shadingMode_.set(ShadingMode::Phong);
    } else {
        lighting_.shadingMode_.set(ShadingMode::None);
    }
}

// override to do member renaming.
void VolumeRaycaster::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace inviwo
