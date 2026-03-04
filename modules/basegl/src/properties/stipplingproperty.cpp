/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/staticstring.h>
#include <modules/basegl/datastructures/stipplingdata.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>

namespace inviwo {

std::string_view StipplingProperty::getClassIdentifier() const { return classIdentifier; }

StipplingProperty::StipplingProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, invalidationLevel, std::move(semantics)}
    , mode{"stippleMode",
           "Stipple Mode",
           {{"none", "None", StipplingData::Mode::None},
            {"screenspace", "Screen Space", StipplingData::Mode::ScreenSpace},
            {"worldspace", "World Space", StipplingData::Mode::WorldSpace}},
           0,
           InvalidationLevel::InvalidResources}
    , length{"stippleLen", "Length",
             util::ordinalLength(20.0f, 100.0f)
                 .set("Length of a dash, in pixels if Stippling Mode is ScreenSpace"_help)}
    , spacing{"stippleSpacing", "Spacing",
              util::ordinalLength(10.0f, 100.0f)
                  .set(
                      "Distance between two dashes, in pixels if Stippling Mode is ScreenSpace"_help)}
    , offset{"stippleOffset", "Offset",
             util::ordinalLength(0.0f, 100.0f)
                 .set("Offset of first dash, in pixels if Stippling Mode is ScreenSpace"_help)}
    , worldScale{
          "stippleWorldScale", "World Scale",
          util::ordinalScale(4.0f, 20.0f)
              .set(
                  "Scaling of parameters. Only applicable if Stippling Mode is WorldSpace."_help)} {

    addProperties(mode, length, spacing, offset, worldScale);

    mode.onChange(
        [this]() { worldScale.setVisible(mode.get() == StipplingData::Mode::WorldSpace); });
    worldScale.setVisible(mode.get() == StipplingData::Mode::WorldSpace);
}

StipplingProperty::StipplingProperty(const StipplingProperty& rhs)
    : CompositeProperty(rhs)
    , mode(rhs.mode)
    , length(rhs.length)
    , spacing(rhs.spacing)
    , offset(rhs.offset)
    , worldScale(rhs.worldScale) {

    addProperties(mode, length, spacing, offset, worldScale);

    mode.onChange(
        [this]() { worldScale.setVisible(mode.get() == StipplingData::Mode::WorldSpace); });
    worldScale.setVisible(mode.get() == StipplingData::Mode::WorldSpace);
}

StipplingProperty* StipplingProperty::clone() const { return new StipplingProperty(*this); }

void StipplingProperty::update(StipplingData& data) const {
    data.length = length.get();
    data.spacing = spacing.get();
    data.offset = offset.get();
    data.worldScale = worldScale.get();
    data.mode = mode.get();
}

namespace utilgl {

void addShaderDefines(Shader& shader, const StipplingProperty& property) {
    addShaderDefines(shader, property.mode.get());
}

void addShaderDefines(Shader& shader, const StipplingData::Mode& mode) {
    std::string value;
    switch (mode) {
        case StipplingData::Mode::ScreenSpace:
            value = "1";
            break;
        case StipplingData::Mode::WorldSpace:
            value = "2";
            break;
        case StipplingData::Mode::None:
        default:
            break;
    }

    auto fragShader = shader.getFragmentShaderObject();
    fragShader->setShaderDefine("ENABLE_STIPPLING", mode != StipplingData::Mode::None);
    fragShader->addShaderDefine("STIPPLE_MODE", value);
}

void setShaderUniforms(Shader& shader, const StipplingProperty& property, const std::string& name) {
    shader.setUniform(name + ".length", property.length.get());
    shader.setUniform(name + ".spacing", property.spacing.get());
    shader.setUniform(name + ".offset", property.offset.get());
    shader.setUniform(name + ".worldScale", property.worldScale.get());
}

}  // namespace utilgl

}  // namespace inviwo
