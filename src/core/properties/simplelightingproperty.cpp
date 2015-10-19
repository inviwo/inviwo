/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
    , shadingMode_("shadingMode", "Shading", INVALID_RESOURCES)
    , referenceFrame_("referenceFrame", "Space")
    , lightPosition_("lightPosition", "Position", vec3(0.0f, 5.0f, 5.0f), vec3(-10, -10, -10),
    vec3(10, 10, 10))
    , lightAttenuation_("lightAttenuation", "Attenuation", vec3(1.0f, 0.0f, 0.0f))
    , applyLightAttenuation_("applyLightAttenuation", "Enable Light Attenuation", false)

    , ambientColor_("lightColorAmbient", "Ambient color", vec3(0.15f))
    , diffuseColor_("lightColorDiffuse", "Diffuse color", vec3(0.6f))
    , specularColor_("lightColorSpecular", "Specular color", vec3(0.4f))
    , specularExponent_("materialShininess", "Shininess", 60.0f, 1.0f, 180.0f)
    , camera_(camera) {

    shadingMode_.addOption("none", "No Shading", ShadingMode::None);
    shadingMode_.addOption("ambient", "Ambient", ShadingMode::Ambient);
    shadingMode_.addOption("diffuse", "Diffuse", ShadingMode::Diffuse);
    shadingMode_.addOption("specular", "Specular", ShadingMode::Specular);
    shadingMode_.addOption("blinnphong", "Blinn Phong", ShadingMode::BlinnPhong);
    shadingMode_.addOption("phong", "Phong", ShadingMode::Phong);
    shadingMode_.setSelectedValue(ShadingMode::Phong);
    shadingMode_.setCurrentStateAsDefault();

    referenceFrame_.addOption("world", "World", static_cast<int>(Space::WORLD));
    referenceFrame_.setSelectedValue(static_cast<int>(Space::WORLD));
    if (camera_) {
        referenceFrame_.addOption("view", "View", static_cast<int>(Space::VIEW));
        referenceFrame_.setSelectedValue(static_cast<int>(Space::VIEW));
    }
    
    referenceFrame_.setCurrentStateAsDefault();

    lightPosition_.setSemantics(PropertySemantics("Spherical"));
    ambientColor_.setSemantics(PropertySemantics::Color);
    diffuseColor_.setSemantics(PropertySemantics::Color);
    specularColor_.setSemantics(PropertySemantics::Color);

    // add properties
    addProperty(shadingMode_);
    addProperty(referenceFrame_);
    addProperty(lightPosition_);
    addProperty(ambientColor_);
    addProperty(diffuseColor_);
    addProperty(specularColor_);
    addProperty(specularExponent_);
    addProperty(applyLightAttenuation_);
    addProperty(lightAttenuation_);
}

SimpleLightingProperty::SimpleLightingProperty(const SimpleLightingProperty& rhs)
    : CompositeProperty(rhs)
    , shadingMode_(rhs.shadingMode_)
    , referenceFrame_(rhs.referenceFrame_)
    , lightPosition_(rhs.lightPosition_)
    , lightAttenuation_(rhs.lightAttenuation_)
    , applyLightAttenuation_(rhs.applyLightAttenuation_)
    , ambientColor_(rhs.ambientColor_) 
    , diffuseColor_(rhs.diffuseColor_)
    , specularColor_(rhs.specularColor_)
    , specularExponent_(rhs.specularExponent_) {

    // add properties
    addProperty(shadingMode_);
    addProperty(referenceFrame_);
    addProperty(lightPosition_);
    addProperty(ambientColor_);
    addProperty(diffuseColor_);
    addProperty(specularColor_);
    addProperty(specularExponent_);
    addProperty(applyLightAttenuation_);
    addProperty(lightAttenuation_);
}

SimpleLightingProperty& SimpleLightingProperty::operator=(const SimpleLightingProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        shadingMode_ = that.shadingMode_;
        referenceFrame_ = that.referenceFrame_;
        lightPosition_ = that.lightPosition_;
        lightAttenuation_ = that.lightAttenuation_;
        applyLightAttenuation_ = that.applyLightAttenuation_;
        ambientColor_ = that.ambientColor_;
        diffuseColor_ = that.diffuseColor_;
        specularColor_ = that.specularColor_;
        specularExponent_ = that.specularExponent_;
    }
    return *this;
}

SimpleLightingProperty* SimpleLightingProperty::clone() const {
    return new SimpleLightingProperty(*this);
}

SimpleLightingProperty::~SimpleLightingProperty() {}

inviwo::vec3 SimpleLightingProperty::getTransformedPosition() const {
    switch (static_cast<Space>(referenceFrame_.getSelectedValue())) {
        case Space::VIEW:
            return camera_ ? vec3(camera_->inverseViewMatrix() * vec4(lightPosition_.get(), 1.0f))
                           : lightPosition_.get();
        case Space::WORLD:
        default:
            return lightPosition_.get();
    }
}

}  // namespace
