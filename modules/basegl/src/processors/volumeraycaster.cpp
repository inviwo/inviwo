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
#include <modules/opengl/openglutils.h>

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

#include <modules/opengl/uniform/camerauniforms.h>
#include <modules/opengl/uniform/volumeuniforms.h>
#include <modules/opengl/uniform/layeruniforms.h>
#include <modules/opengl/uniform/imageuniforms.h>
#include <modules/opengl/uniform/simplelightinguniforms.h>
#include <modules/opengl/uniform/volumeindicatoruniforms.h>
#include <modules/opengl/uniform/raycastinguniform.h>
#include <modules/opengl/uniform/isotfuniforms.h>
#include <modules/opengl/uniform/propertyuniforms.h>


#include <inviwo/tracy/tracy.h>
#include <inviwo/tracy/tracyopengl.h>

#include <type_traits>

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
    , channel_("channel", "Render Channel",
               std::vector<OptionPropertyIntOption>{{"Channel 1", "Channel 1", 0}}, size_t{0})
    , raycasting_("raycaster", "Raycasting")
    , isotf_("isotfComposite", "TF & Isovalues", &volumePort_)
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , lighting_("lighting", "Lighting", &camera_.property)
    , indicator_("positionindicator", "Position Indicator")
    , toggleShading_(
          "toggleShading", "Toggle Shading", [this](Event* e) { toggleShading(e); }, IvwKey::L) {
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addPort(backgroundPort_, "ImagePortGroup1");

    backgroundPort_.setOptional(true);

    channel_.property.setSerializationMode(PropertySerializationMode::All);

    auto updateTFHistSel = [this]() {
        HistogramSelection selection{};
        selection[channel_.property.getSelectedIndex()] = true;
        isotf_.property.setHistogramSelection(selection);
    };
    updateTFHistSel();
    channel_.property.onChange(updateTFHistSel);

    volumePort_.onChange([this]() {
        if (volumePort_.hasData()) {
            size_t channels = volumePort_.getData()->getDataFormat()->getComponents();

            if (channels == channel_.property.size()) return;

            std::vector<OptionPropertyIntOption> channelOptions;
            for (size_t i = 0; i < channels; i++) {
                channelOptions.emplace_back("Channel " + toString(i + 1),
                                            "Channel " + toString(i + 1), static_cast<int>(i));
            }
            channel_.property.replaceOptions(channelOptions);
            channel_.property.setCurrentStateAsDefault();
        }
    });
    backgroundPort_.onConnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
    backgroundPort_.onDisconnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });

    // change the currently selected channel when a pre-computed gradient is selected
    raycasting_.property.gradientComputation_.onChange([this]() {
        if (channel_.property.size() == 4) {
            if (raycasting_.property.gradientComputation_.get() ==
                RaycastingProperty::GradientComputation::PrecomputedXYZ) {
                channel_.property.set(3);
            } else if (raycasting_.property.gradientComputation_.get() ==
                       RaycastingProperty::GradientComputation::PrecomputedYZW) {
                channel_.property.set(0);
            }
        }
    });

    addProperties(channel_.property, raycasting_.property, isotf_.property, camera_.property,
                  lighting_.property, indicator_.property, toggleShading_);
}

const ProcessorInfo VolumeRaycaster::getProcessorInfo() const { return processorInfo_; }

void VolumeRaycaster::initializeResources() {
    utilgl::addDefines(shader_, raycasting_.property, isotf_.property, camera_.property,
                       lighting_.property, indicator_.property);
    utilgl::addShaderDefinesBGPort(shader_, backgroundPort_);
    shader_.build();

    camera_.bind(shader_);
    lighting_.bind(shader_);
    raycasting_.bind(shader_);
    indicator_.bind(shader_);
    isotf_.bind(shader_, isotf_.property.isovalues_.getIdentifier());
    channel_.bind(shader_);

    volumeUniforms_.bind(shader_, volumePort_.getIdentifier());
    entryUniforms_.bind(shader_, entryPort_.getIdentifier());
    exitUniforms_.bind(shader_, exitPort_.getIdentifier());
    backgroundUniforms_.bind(shader_, backgroundPort_.getIdentifier());
    outUniforms_.bind(shader_, outport_.getIdentifier());
    useNormals_.bind(shader_, "useNormals");

    utilgl::Activate as{&shader_};

    shader_.setUniform("volume", volumeNum.number);
    shader_.setUniform(isotf_.property.tf_.getIdentifier(), tfNum.number);
    shader_.setUniform("entryColor", entryNums.color().number);
    shader_.setUniform("entryDepth", entryNums.depth().number);
    shader_.setUniform("entryPicking", entryNums.picking().number);
    shader_.setUniform("entryNormal", entryNums[3].number);
    shader_.setUniform("exitColor", exitNums.color().number);
    shader_.setUniform("exitDepth", exitNums.depth().number);

    if (backgroundPort_.isConnected()) {
        shader_.setUniform("bgColor", backgroundNums.color().number);
        shader_.setUniform("bgDepth", backgroundNums.depth().number);
        shader_.setUniform("bgPicking", backgroundNums.picking().number);
    }
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

    utilgl::activateAndClearTarget(outport_);

    const auto entry = entryPort_.getData();
    const auto exit = exitPort_.getData();
    const auto back = backgroundPort_.getData();

    utilgl::Activate as{&shader_};

    {
        TRACY_ZONE_SCOPED_NC("BindAndSetUniforms", 0x008800);
        TRACY_GPU_ZONE_C("BindAndSetUniforms", 0x008800);

        volume.getRepresentation<VolumeGL>()->bindTexture(volumeNum.unit());

        if (const auto tfLayer = isotf_.property.tf_.get().getData()) {
            const auto transferFunctionGL = tfLayer->getRepresentation<LayerGL>();
            transferFunctionGL->bindTexture(tfNum.unit());
        }

        utilgl::bindTextures(*entry, entryNums.color().unit(), entryNums.depth().unit(),
                             entryNums.picking().unit());
        if (auto normals = entry->getColorLayer(1)) {
            normals->getRepresentation<LayerGL>()->bindTexture(entryNums[3].unit());
            useNormals_.setUniforms(true);
        } else {
            useNormals_.setUniforms(false);
        }

        utilgl::bindTextures(*exit, exitNums.color().unit(), exitNums.depth().unit());
        if (backgroundPort_.hasData()) {
            utilgl::bindTextures(*back, backgroundNums.color().unit(),
                                 backgroundNums.depth().unit(), backgroundNums.picking().unit());
            backgroundUniforms_.setUniforms(*back);
        }

        camera_.setUniforms();
        lighting_.setUniforms();
        raycasting_.setUniforms();
        indicator_.setUniforms();
        isotf_.setUniforms();
        channel_.setUniforms();

        volumeUniforms_.setUniforms(volume);
        entryUniforms_.setUniforms(*entry);
        exitUniforms_.setUniforms(*exit);
        outUniforms_.setUniforms(*outport_.getData());
    }
    {
        TRACY_ZONE_SCOPED_NC("Raycast", 0x008800);
        TRACY_GPU_ZONE_C("Raycast", 0x008800);
        utilgl::singleDrawImagePlaneRect();
    }

    utilgl::deactivateCurrentTarget();
}

void VolumeRaycaster::toggleShading(Event*) {
    if (lighting_.property.shadingMode_.get() == ShadingMode::None) {
        lighting_.property.shadingMode_.set(ShadingMode::Phong);
    } else {
        lighting_.property.shadingMode_.set(ShadingMode::None);
    }
}

// override to do member renaming.
void VolumeRaycaster::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace inviwo
