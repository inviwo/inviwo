/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/document.h>
#include <modules/basegl/shadercomponents/shadercomponent.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace inviwo {

class Inport;
class Property;
class Shader;
class TextureUnitContainer;

/**
 * @brief A ShaderComponent that samples a TemporalVolume with time-dependent interpolation.
 *
 * The component adds a TemporalVolumeInport and samples the volume for the current time. When
 * linear interpolation is enabled it uploads the two bracketing frames as 3D textures (`<name>`
 * and `<name>B`) and blends them in the shader using `<name>Blend`, producing the same
 * `<name>Voxel`, `<name>VoxelPrev`, and `<name>Gradient` symbols as the regular VolumeComponent so
 * it can be combined with the standard RaycastingComponent.
 *
 * It also owns the IsoTFProperty used for classification (operating in normalized TF space) and
 * schedules background prefetching of upcoming frames.
 *
 * @see TemporalVolume, VolumeComponent, RaycastingComponent
 */
class IVW_MODULE_BASEGL_API TemporalVolumeComponent : public ShaderComponent {
public:
    enum class Gradients : std::uint8_t { None, Single };
    enum class Interpolation : std::uint8_t { Nearest, Linear };

    explicit TemporalVolumeComponent(std::string_view name,
                                     Gradients gradients = Gradients::Single, Document help = {});

    virtual std::string_view getName() const override;
    virtual void initializeResources(Shader& shader) override;
    virtual void process(Shader& shader, TextureUnitContainer& cont) override;
    virtual std::vector<std::tuple<Inport*, std::string>> getInports() override;
    virtual std::vector<Property*> getProperties() override;
    virtual std::vector<Segment> getSegments() override;

    std::string getGradientString() const;

    std::optional<size_t> channelsForVolume() const;

    /// The IsoTFProperty used for classification, to be passed to a RaycastingComponent.
    IsoTFProperty& isoTF();

    TemporalVolumeInport volumePort;
    Gradients gradients;

    DoubleProperty time;
    OptionProperty<Interpolation> interpolation;
    BoolProperty prefetch;
    IntSizeTProperty prefetchAhead;
    IsoTFProperty isoTF_;
};

}  // namespace inviwo
