/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#pragma once

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/raycastingproperty.h>

#include <modules/basegl/processors/raycasting/volumeraycasterbase.h>
#include <modules/basegl/shadercomponents/backgroundcomponent.h>         // for BackgroundComponent
#include <modules/basegl/shadercomponents/cameracomponent.h>             // for CameraComponent
#include <modules/basegl/shadercomponents/entryexitcomponent.h>          // for EntryExitComponent
#include <modules/basegl/shadercomponents/lightcomponent.h>              // for LightComponent
#include <modules/basegl/shadercomponents/positionindicatorcomponent.h>  // for PositionIndicato...
#include <modules/basegl/shadercomponents/raycastingcomponent.h>         // for RaycastingComponent
#include <modules/basegl/shadercomponents/sampletransformcomponent.h>    // for SampleTransformC...
#include <modules/basegl/shadercomponents/volumecomponent.h>             // for VolumeComponent

namespace inviwo {

class IVW_MODULE_BASEGL_API TexturedVolumeComponent : public ShaderComponent {
public:
    enum class Operation : std::uint8_t {
        None,
        ReplacePrimaryColor,
        MultiplyPrimaryColor,
        MultiplyPrimaryAll
    };

    TexturedVolumeComponent(std::string_view name, VolumeInport& volume);

    virtual std::string_view getName() const override;

    virtual void initializeResources(Shader& shader) override;

    virtual void process(Shader& shader, TextureUnitContainer& cont) override;

    virtual std::vector<std::tuple<Inport*, std::string>> getInports() override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

    size_t selectedChannel() const { return primaryChannel_.getSelectedIndex(); }

private:
    using enum RaycastingProperty::RenderingType;
    VolumeInport secondaryVolumePort_;

    CompositeProperty dependentLookup_;
    OptionPropertyInt primaryChannel_;
    OptionPropertyInt secondaryChannel_;
    TransferFunctionProperty primaryTF_;
    TransferFunctionProperty secondaryTF_;
    OptionProperty<Operation> operation_;
    RaycastingProperty raycasting_;

    std::string volume_;
};

class IVW_MODULE_BASEGL_API TexturedVolumeRaycaster : public VolumeRaycasterBase {
public:
    explicit TexturedVolumeRaycaster(std::string_view identifier = "",
                                     std::string_view displayName = "");

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    VolumeComponent volume_;
    TexturedVolumeComponent textureComponent_;
    EntryExitComponent entryExit_;
    BackgroundComponent background_;
    CameraComponent camera_;
    LightComponent light_;
    PositionIndicatorComponent positionIndicator_;
    SampleTransformComponent sampleTransform_;
};

}  // namespace inviwo
