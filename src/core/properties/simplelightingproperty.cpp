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

#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>

namespace inviwo {

std::string_view SimpleLightingProperty::getClassIdentifier() const { return classIdentifier; }

SimpleLightingProperty::SimpleLightingProperty(std::string_view identifier,
                                               std::string_view displayName, Document help,
                                               const vec3& position, CameraProperty* camera,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : SimpleLightingProperty{
          identifier, displayName,       help,     LightingConfig{.position = position},
          camera,     invalidationLevel, semantics} {}

SimpleLightingProperty::SimpleLightingProperty(std::string_view identifier,
                                               std::string_view displayName, Document help,
                                               const LightingConfig& config, CameraProperty* camera,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, help, invalidationLevel, semantics)
    , shadingMode_("shadingMode", "Shading",
                   OptionPropertyState<ShadingMode>{
                       .options = {{"none", "No Shading", ShadingMode::None},
                                   {"ambient", "Ambient", ShadingMode::Ambient},
                                   {"diffuse", "Diffuse", ShadingMode::Diffuse},
                                   {"specular", "Specular", ShadingMode::Specular},
                                   {"blinnphong", "Blinn-Phong", ShadingMode::BlinnPhong},
                                   {"phong", "Phong", ShadingMode::Phong},
                                   {"blinnphongfront", "Blinn-Phong frontside only",
                                    ShadingMode::BlinnPhongFront},
                                   {"blinnphongback", "Blinn-Phong backside only",
                                    ShadingMode::BlinnPhongBack},
                                   {"phongfront", "Phong frontside only", ShadingMode::PhongFront},
                                   {"phongback", "Phong backside only", ShadingMode::PhongBack}}}
                       .setSelectedValue(LightingConfig::defaultShadingMode)
                       .set(InvalidationLevel::InvalidResources))
    , lightPosition_("lightPosition", "Position", "Position of the light source"_help,
                     config.position.value_or(LightingConfig::defaultPosition),
                     config.referenceSpace.value_or(LightingConfig::defaultReferenceSpace), camera,
                     PropertySemantics::LightPosition)
    , ambientColor_{"lightColorAmbient", "Ambient color",
                    util::ordinalColor(config.ambient.value_or(LightingConfig::defaultAmbient))}
    , diffuseColor_{"lightColorDiffuse", "Diffuse color",
                    util::ordinalColor(config.diffuse.value_or(LightingConfig::defaultDiffuse))}
    , specularColor_{"lightColorSpecular", "Specular color",
                     util::ordinalColor(config.specular.value_or(LightingConfig::defaultSpecular))}
    , specularExponent_{"materialShininess", "Shininess",
                        util::ordinalScale(config.specularExponent.value_or(
                            LightingConfig::defaultSpecularExponent))}
    , camera_(camera) {
    addProperties(shadingMode_, lightPosition_, ambientColor_, diffuseColor_, specularColor_,
                  specularExponent_);
}

SimpleLightingProperty::SimpleLightingProperty(std::string_view identifier,
                                               std::string_view displayName, CameraProperty* camera,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : SimpleLightingProperty{identifier, displayName,       {},       LightingConfig{},
                             camera,     invalidationLevel, semantics} {}

SimpleLightingProperty::SimpleLightingProperty(const SimpleLightingProperty& rhs)
    : CompositeProperty(rhs)
    , shadingMode_(rhs.shadingMode_)
    , lightPosition_(rhs.lightPosition_)
    , ambientColor_(rhs.ambientColor_)
    , diffuseColor_(rhs.diffuseColor_)
    , specularColor_(rhs.specularColor_)
    , specularExponent_(rhs.specularExponent_)
    , camera_(rhs.camera_) {
    addProperties(shadingMode_, lightPosition_, ambientColor_, diffuseColor_, specularColor_,
                  specularExponent_);
}

SimpleLightingProperty* SimpleLightingProperty::clone() const {
    return new SimpleLightingProperty(*this);
}

SimpleLightingProperty::~SimpleLightingProperty() = default;

LightingState SimpleLightingProperty::getState() const {
    return {shadingMode_,   lightPosition_.get(CoordinateSpace::World),
            ambientColor_,  diffuseColor_,
            specularColor_, specularExponent_};
}

LightingConfig SimpleLightingProperty::config() const {
    return LightingConfig{
        .shadingMode = shadingMode_,
        .position = lightPosition_.get(CoordinateSpace::World),
        .referenceSpace = CoordinateSpace::World,
        .ambient = ambientColor_,
        .diffuse = diffuseColor_,
        .specular = specularColor_,
        .specularExponent = specularExponent_,
    };
}

}  // namespace inviwo
