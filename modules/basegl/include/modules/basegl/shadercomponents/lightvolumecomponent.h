/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/light/baselightsource.h>
#include <inviwo/core/datastructures/light/lightingstate.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <modules/basegl/shadercomponents/shadercomponent.h>

#include <vector>
#include <tuple>
#include <string_view>

namespace inviwo {

class Inport;
class Processor;
class Property;
class Shader;
class TextureUnitContainer;

/**
 * \brief Raycaster component for considering a light volume
 * Adds a volume inport for a light volume. The light volume is bound as texture and used for
 * volumetric illumination.
 */
class IVW_MODULE_BASEGL_API LightVolumeComponent : public ShaderComponent {
public:
    LightVolumeComponent(Processor& processor, std::string_view volumeName,
                         std::string_view gradientName);

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer& cont) override;

    virtual void initializeResources(Shader& shader) override;

    virtual std::vector<std::tuple<Inport*, std::string>> getInports() override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

    bool setUsedChannels(size_t channels);

private:
    VolumeInport lightVolume_;
    DataInport<LightSource> lightSource_;

    FloatProperty scaling_;
    OptionProperty<ShadingMode> shadingMode_;
    CompositeProperty material_;
    FloatVec3Property ambientColor_;
    FloatVec3Property diffuseColor_;
    FloatVec3Property specularColor_;
    FloatProperty specularExponent_;

    std::string volumeVarName_;
    std::string gradientVarName_;
    size_t usedChannels_;
};

}  // namespace inviwo
