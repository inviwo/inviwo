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
    , shadingMode_("shadingMode", "Shading", InvalidationLevel::InvalidResources)
    , referenceFrame_("referenceFrame", "Space")
    , specularExponent_("materialShininess", "Shininess", 60.0f, 1.0f, 180.0f)
	, roughness_("materialRoughness", "Roughness", 0.4f, 0.0f, 1.0f)
    , camera_(camera) 
	, addLight_("addLight", "Add Light")
	, deleteLight_("deleteLight", "Delete Light")
	, lightSelection_("lightSelection", "Light Selection")
	, numLights_(0)
{
    shadingMode_.addOption("none", "No Shading", ShadingMode::None);
    shadingMode_.addOption("ambient", "Ambient", ShadingMode::Ambient);
    shadingMode_.addOption("diffuse", "Diffuse", ShadingMode::Diffuse);
    shadingMode_.addOption("specular", "Specular", ShadingMode::Specular);
    shadingMode_.addOption("blinnphong", "Blinn Phong", ShadingMode::BlinnPhong);
    shadingMode_.addOption("phong", "Phong", ShadingMode::Phong);
	shadingMode_.addOption("orennayar", "Oren Nayar", ShadingMode::OrenNayar);
	shadingMode_.addOption("orennayardiffuse", "Oren Nayar (Diffuse only)", ShadingMode::OrenNayarDiffuse);
    shadingMode_.setSelectedValue(ShadingMode::Phong);
    shadingMode_.setCurrentStateAsDefault();

    referenceFrame_.addOption("world", "World", static_cast<int>(Space::WORLD));
    referenceFrame_.setSelectedValue(static_cast<int>(Space::WORLD));
    if (camera_) {
        referenceFrame_.addOption("view", "View", static_cast<int>(Space::VIEW));
        referenceFrame_.setSelectedValue(static_cast<int>(Space::VIEW));
    }
    
    referenceFrame_.setCurrentStateAsDefault();

    

    // add properties
    addProperty(shadingMode_);
    addProperty(referenceFrame_);
    addProperty(specularExponent_);
	addProperty(roughness_);
	
	addLight_.onChange(this, &SimpleLightingProperty::addLight);
	deleteLight_.onChange(this, &SimpleLightingProperty::deleteLight);

	addProperty(lightSelection_);
	addProperty(deleteLight_);
	addProperty(addLight_);

	addLight();
    
}

SimpleLightingProperty::SimpleLightingProperty(const SimpleLightingProperty& rhs)
	: CompositeProperty(rhs)
	, shadingMode_(rhs.shadingMode_)
	, referenceFrame_(rhs.referenceFrame_)
	, specularExponent_(rhs.specularExponent_)
	, roughness_(rhs.roughness_)
	, addLight_(rhs.addLight_)
	, deleteLight_(rhs.deleteLight_)
	, lightSelection_(rhs.lightSelection_)
	, numLights_(rhs.numLights_)
{

    // add properties
    addProperty(shadingMode_);
    addProperty(referenceFrame_);
    addProperty(specularExponent_);
	addProperty(roughness_);

	addProperty(lightSelection_);
	addProperty(deleteLight_);
	addProperty(addLight_);

	for(auto property : rhs.getPropertiesByType<LightProperty>())
		this->addProperty(property->clone());
}

SimpleLightingProperty& SimpleLightingProperty::operator=(const SimpleLightingProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        shadingMode_ = that.shadingMode_;
        referenceFrame_ = that.referenceFrame_;
        specularExponent_ = that.specularExponent_;
		roughness_ = that.roughness_;
		addLight_ = that.addLight_;
		deleteLight_ = that.deleteLight_;
		lightSelection_ = that.lightSelection_;
		numLights_ = that.numLights_;
		for (auto property : this->getPropertiesByType<LightProperty>())
			this->removeProperty(property);
		for (auto property : that.getPropertiesByType<LightProperty>())
			this->addProperty(property);
			
    }
    return *this;
}

SimpleLightingProperty* SimpleLightingProperty::clone() const {
    return new SimpleLightingProperty(*this);
}

SimpleLightingProperty::~SimpleLightingProperty() {}

void SimpleLightingProperty::addLight() {
	if (numLights_ >= MAX_NUMBER_OF_LIGHTS) return;
	numLights_++;
	std::string num = std::to_string(numLights_);

	lightSelection_.addOption("lightOption_" + num, "Light " + num);

	auto property = new LightProperty("light_" + num, "Light " + num);
	property->setSerializationMode(PropertySerializationMode::All);
	this->addProperty(property, true);
}

void SimpleLightingProperty::deleteLight() {
	if (numLights_ <= 0) return;
	auto beforeDeletion = this->getPropertiesByType<LightProperty>(false);
	int selectedElement = lightSelection_.getSelectedIndex();

	std::string identifier = beforeDeletion.at(selectedElement)->getIdentifier();
	removeProperty(identifier);
	lightSelection_.removeOption(selectedElement);
	numLights_--;

	auto afterDeletion = this->getPropertiesByType<LightProperty>(false);

	size_t loopCount = 1;
	for (auto prop : afterDeletion) {
		prop->setDisplayName("Light " + std::to_string(loopCount));
		prop->setIdentifier("light_" + std::to_string(loopCount));
		loopCount++;
	}

	lightSelection_.clearOptions();

	for (size_t i = 1; i < afterDeletion.size() + 1; i++) {
		std::string str_i = std::to_string(i);
		lightSelection_.addOption("lightOption_" + str_i, "Light " + str_i);
	}
}

}  // namespace
