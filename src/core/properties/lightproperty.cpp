/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <inviwo/core/properties/lightproperty.h>

namespace inviwo {

	PropertyClassIdentifier(LightProperty, "org.inviwo.LightProperty");

LightProperty::LightProperty(std::string identifier, std::string displayName,
							 InvalidationLevel invalidationLevel,
							 PropertySemantics semantics)
	: CompositeProperty(identifier, displayName, invalidationLevel, semantics)
	, lightPosition_("lightPosition", "Position", vec3(0.0f, 5.0f, 5.0f), vec3(-10, -10, -10), vec3(10, 10, 10))
	, lightAttenuation_("lightAttenuation", "Attenuation", vec3(1.0f, 0.0f, 0.0f))
	, ambientColor_("lightColorAmbient", "Ambient color", vec3(0.15f))
	, diffuseColor_("lightColorDiffuse", "Diffuse color", vec3(0.6f))
	, specularColor_("lightColorSpecular", "Specular color", vec3(0.4f))
{
	lightPosition_.setSemantics(PropertySemantics("Spherical"));
	ambientColor_.setSemantics(PropertySemantics::Color);
	diffuseColor_.setSemantics(PropertySemantics::Color);
	specularColor_.setSemantics(PropertySemantics::Color);

	addProperty(lightPosition_);
	addProperty(ambientColor_);
	addProperty(diffuseColor_);
	addProperty(specularColor_);

	addProperty(lightAttenuation_);
}

LightProperty::LightProperty(const LightProperty& rhs) 
	: CompositeProperty(rhs)
	, lightPosition_(rhs.lightPosition_)
	, lightAttenuation_(rhs.lightAttenuation_)
	, ambientColor_(rhs.ambientColor_)
	, diffuseColor_(rhs.diffuseColor_)
	, specularColor_(rhs.specularColor_)
{
	addProperty(lightPosition_);
	addProperty(ambientColor_);
	addProperty(diffuseColor_);
	addProperty(specularColor_);
	addProperty(lightAttenuation_);
}

LightProperty& LightProperty::operator=(const LightProperty& that) {
	if (this != &that) {
		CompositeProperty::operator=(that);
		lightPosition_ = that.lightPosition_;
		lightAttenuation_ = that.lightAttenuation_;
		ambientColor_ = that.ambientColor_;
		diffuseColor_ = that.diffuseColor_;
		specularColor_ = that.specularColor_;
	}
	return *this;
}

LightProperty* LightProperty::clone() const {
	return new LightProperty(*this);
}

LightProperty::~LightProperty() {}

vec3 LightProperty::getTransformedPosition(const CameraProperty* camera, CoordinateSpace space) const {
    switch (space) {
        case CoordinateSpace::View:
		return camera ? vec3(camera->inverseViewMatrix() * vec4(lightPosition_.get(), 1.0f))
			: lightPosition_.get();
	case CoordinateSpace::World:
	default:
		return lightPosition_.get();
	}
}

} // namespace

