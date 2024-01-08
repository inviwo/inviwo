/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/properties/simplelightingproperty.h>    // for SimpleLightingProperty
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent

#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {
class CameraProperty;
class Property;
class Shader;
class TextureUnitContainer;

/**
 * Adds a SimpleLightingProperty and binds it to the `lighting` LightParameters uniform.
 */
class IVW_MODULE_BASEGL_API LightComponent : public ShaderComponent {
public:
    LightComponent(CameraProperty* camera);

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer&) override;

    virtual void initializeResources(Shader& shader) override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

private:
    SimpleLightingProperty lighting_;
};

}  // namespace inviwo
