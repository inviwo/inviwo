/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/basegl/properties/stipplingproperty.h>

#include <inviwo/core/properties/compositeproperty.h>                  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                     // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                    // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>                  // for PropertySemantics
#include <inviwo/core/util/staticstring.h>                             // for operator+
#include <modules/basegl/datastructures/stipplingsettingsinterface.h>  // for StipplingSettingsI...
#include <modules/opengl/shader/shader.h>                              // for Shader
#include <modules/opengl/shader/shaderobject.h>                        // for ShaderObject

namespace inviwo {

const std::string StipplingProperty::classIdentifier = "org.inviwo.StipplingProperty";
std::string StipplingProperty::getClassIdentifier() const { return classIdentifier; }

StipplingProperty::StipplingProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , mode_("stippleMode", "Stipple Mode",
            {{"none", "None", Mode::None},
             {"screenspace", "Screen Space", Mode::ScreenSpace},
             {"worldspace", "World Space", Mode::WorldSpace}},
            0, InvalidationLevel::InvalidResources)
    , length_("stippleLen", "Length", 30.0f, 0.0f, 100.0f)
    , spacing_("stippleSpacing", "Spacing", 10.0f, 0.0f, 100.0f)
    , offset_("stippleOffset", "Offset", 0.0f, 0.0f, 100.0f)
    , worldScale_("stippleWorldScale", "World Scale", 4.0f, 1.0f, 20.0f) {
    addProperty(mode_);
    addProperty(length_);
    addProperty(spacing_);
    addProperty(offset_);
    addProperty(worldScale_);

    mode_.onChange([this]() { worldScale_.setVisible(mode_.get() == Mode::WorldSpace); });
    worldScale_.setVisible(mode_.get() == Mode::WorldSpace);
}

StipplingProperty::StipplingProperty(const StipplingProperty& rhs)
    : CompositeProperty(rhs)
    , mode_(rhs.mode_)
    , length_(rhs.length_)
    , spacing_(rhs.spacing_)
    , offset_(rhs.offset_)
    , worldScale_(rhs.worldScale_) {

    addProperty(mode_);
    addProperty(length_);
    addProperty(spacing_);
    addProperty(offset_);
    addProperty(worldScale_);

    mode_.onChange([this]() { worldScale_.setVisible(mode_.get() == Mode::WorldSpace); });
    worldScale_.setVisible(mode_.get() == Mode::WorldSpace);
}

StipplingProperty* StipplingProperty::clone() const { return new StipplingProperty(*this); }

namespace utilgl {

void addShaderDefines(Shader& shader, const StipplingProperty& property) {
    addShaderDefines(shader, property.mode_.get());
}

void addShaderDefines(Shader& shader, const StipplingProperty::Mode& mode) {
    std::string value;
    switch (mode) {
        case StipplingProperty::Mode::ScreenSpace:
            value = "1";
            break;
        case StipplingProperty::Mode::WorldSpace:
            value = "2";
            break;
        case StipplingProperty::Mode::None:
        default:
            break;
    }

    auto fragShader = shader.getFragmentShaderObject();
    fragShader->setShaderDefine("ENABLE_STIPPLING", mode != StipplingProperty::Mode::None);
    fragShader->addShaderDefine("STIPPLE_MODE", value);
}

void setShaderUniforms(Shader& shader, const StipplingProperty& property, const std::string& name) {
    shader.setUniform(name + ".length", property.length_.get());
    shader.setUniform(name + ".spacing", property.spacing_.get());
    shader.setUniform(name + ".offset", property.offset_.get());
    shader.setUniform(name + ".worldScale", property.worldScale_.get());
}

}  // namespace utilgl

}  // namespace inviwo
