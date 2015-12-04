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

#include "spotlightsourceprocessor.h"
#include <inviwo/core/datastructures/light/spotlight.h>


namespace inviwo {

const ProcessorInfo SpotLightSourceProcessor::processorInfo_{
    "org.inviwo.Spotlightsource",  // Class identifier
    "Spot light source",           // Display name
    "Light source",                // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo SpotLightSourceProcessor::getProcessorInfo() const {
    return processorInfo_;
}

SpotLightSourceProcessor::SpotLightSourceProcessor()
    : Processor()
    , outport_("SpotLightSource")
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid)
    , lightPosition_("lightPosition", "Light Source Position",
    FloatVec3Property("position", "Position", vec3(100.f), vec3(-100.f), vec3(100.f)), &camera_)
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)", 50.f, 0.f, 100.f)
    , lightSize_("lightSize", "Light size", vec2(1.5f, 1.5f), vec2(0.0f, 0.0f), vec2(3.0f, 3.0f))
    , lightDiffuse_("lightDiffuse", "Color", vec3(1.0f))
    , lightConeRadiusAngle_("lightConeRadiusAngle", "Light Cone Radius Angle", 30.f, 1.f, 90.f)
    , lightFallOffAngle_("lightFallOffAngle", "Light Fall Off Angle", 5.f, 0.f, 30.f)
    , lightSource_{std::make_shared<SpotLight>()} {
    
    addPort(outport_);

    addProperty(lightPosition_);
    lighting_.addProperty(lightConeRadiusAngle_);
    lighting_.addProperty(lightFallOffAngle_);
    lighting_.addProperty(lightDiffuse_);
    lighting_.addProperty(lightPowerProp_);
    lighting_.addProperty(lightSize_);
    addProperty(lighting_);
    addProperty(camera_);

    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightDiffuse_.setCurrentStateAsDefault();
}

void SpotLightSourceProcessor::process() {
    updateSpotLightSource(lightSource_.get());
    outport_.setData(lightSource_);
}

void SpotLightSourceProcessor::updateSpotLightSource(SpotLight* lightSource) {
    vec3 lightPos = lightPosition_.get();
    vec3 dir;
    switch (static_cast<PositionProperty::Space>(lightPosition_.referenceFrame_.getSelectedValue())) {
    case PositionProperty::Space::VIEW:
        dir = glm::normalize(camera_.getLookTo() - lightPos);
    case PositionProperty::Space::WORLD:
    default:
        dir = glm::normalize(vec3(0.f) - lightPos);
    }
    mat4 transformationMatrix = getLightTransformationMatrix(lightPos, dir);
    // Offset by 0.5 to get to texture coordinates
    lightSource->setModelMatrix(glm::translate(vec3(0.5f)));
    lightSource->setWorldMatrix(transformationMatrix);

    lightSource->setSize(lightSize_.get());
    lightSource->setIntensity(lightPowerProp_.get()*lightDiffuse_.get());
    lightSource->setDirection(dir);
    lightSource->setConeRadiusAngle(lightConeRadiusAngle_.get());
    lightSource->setConeFallOffAngle(lightFallOffAngle_.get());
}

} // namespace

