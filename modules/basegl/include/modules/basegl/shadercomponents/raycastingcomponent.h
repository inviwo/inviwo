/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/shadercomponent.h>
#include <inviwo/core/properties/raycastingproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/volumeport.h>

#include <functional>

namespace inviwo {

class IsoTFProperty;

/**
 * The voxel data from `<volume>Voxel[channel]` will be classified using the transferfunction and
 * isovalues from the IsoTFProperty, and composited into `result`.
 */
class IVW_MODULE_BASEGL_API RaycastingComponent : public ShaderComponent {
public:
    RaycastingComponent(std::string_view volume, IsoTFProperty& isotf);

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer&) override;

    virtual void initializeResources(Shader& shader) override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

    bool setUsedChannels(size_t channels);

private:
    std::string volume_;
    IsoTFProperty& isotf_;

    OptionPropertyInt channel_;
    RaycastingProperty raycasting_;
};

/**
 * The voxel data from each channel in `<volume>Voxel` will be classified using the transferfunction
 * and isovalues from the corresponding IsoTFProperty, and composited into `result`.
 */
class IVW_MODULE_BASEGL_API MultiRaycastingComponent : public ShaderComponent {
public:
    MultiRaycastingComponent(std::string_view volume,
                             std::array<std::reference_wrapper<IsoTFProperty>, 4> isotfs);

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer&) override;

    virtual void initializeResources(Shader& shader) override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

    bool setUsedChannels(size_t channels);

private:
    size_t usedChannels_;
    std::string volume_;
    std::array<std::reference_wrapper<IsoTFProperty>, 4> isotfs_;

    RaycastingProperty raycasting_;
};

}  // namespace inviwo
