/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

PropertyClassIdentifier(SimpleLightingProperty, "org.inviwo.SimpleLightingProperty");

SimpleLightingProperty::SimpleLightingProperty(std::string identifier, std::string displayName,
                                               CameraProperty* camera,
                                               InvalidationLevel invalidationLevel,
                                               PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , shadingMode_(
          "shadingMode", "Shading",
          {{"none", "No Shading", ShadingMode::None},
           {"ambient", "Ambient", ShadingMode::Ambient},
           {"diffuse", "Diffuse", ShadingMode::Diffuse},
           {"specular", "Specular", ShadingMode::Specular},
           {"blinnphong", "Blinn Phong", ShadingMode::BlinnPhong},
           {"phong", "Phong", ShadingMode::Phong},
           {"orennayar", "Oren Nayar", ShadingMode::OrenNayar},
           {"orennayardiffuse", "Oren Nayar (Diffuse only)", ShadingMode::OrenNayarDiffuse}},
          5, InvalidationLevel::InvalidResources)
    , referenceFrame_("referenceFrame", "Space",
                      {{"world", "World", static_cast<int>(CoordinateSpace::World)}}, 0)
    , specularExponent_("materialShininess", "Shininess", 60.0f, 1.0f, 180.0f)
    , roughness_("materialRoughness", "Roughness", 0.4f, 0.0f, 1.0f)
    , applyLightAttenuation_("applyLightAttenuation", "Enable Light Attenuation", false,
                             InvalidationLevel::InvalidResources)
    , lights_("lightList", "Lights", "Light", LightProperty("light", "Light"), maxNumberOfLights)
    , camera_(camera) {

    if (camera_) {
        referenceFrame_.addOption("view", "View", static_cast<int>(CoordinateSpace::View));
        referenceFrame_.setSelectedValue(static_cast<int>(CoordinateSpace::View));
        referenceFrame_.setCurrentStateAsDefault();
    }

    // add properties
    addProperty(shadingMode_);
    addProperty(referenceFrame_);
    addProperty(specularExponent_);
    addProperty(roughness_);
    addProperty(applyLightAttenuation_);

    addProperty(lights_);
    if (lights_.size() == 0) {
        lights_.addElement();
    }
}

SimpleLightingProperty::SimpleLightingProperty(const SimpleLightingProperty& rhs)
    : CompositeProperty(rhs)
    , shadingMode_(rhs.shadingMode_)
    , referenceFrame_(rhs.referenceFrame_)
    , specularExponent_(rhs.specularExponent_)
    , roughness_(rhs.roughness_)
    , applyLightAttenuation_(rhs.applyLightAttenuation_)
    , lights_(rhs.lights_)
    , camera_(rhs.camera_){

    // add properties
    addProperty(shadingMode_);
    addProperty(referenceFrame_);
    addProperty(specularExponent_);
    addProperty(roughness_);
    addProperty(applyLightAttenuation_);

    addProperty(lights_);
}

SimpleLightingProperty& SimpleLightingProperty::operator=(const SimpleLightingProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        shadingMode_ = that.shadingMode_;
        referenceFrame_ = that.referenceFrame_;
        specularExponent_ = that.specularExponent_;
        roughness_ = that.roughness_;
        applyLightAttenuation_ = that.applyLightAttenuation_;
        lights_ = that.lights_;
        camera_ = that.camera_;
    }
    return *this;
}

SimpleLightingProperty* SimpleLightingProperty::clone() const {
    return new SimpleLightingProperty(*this);
}

SimpleLightingProperty::~SimpleLightingProperty() = default;

}  // namespace inviwo
