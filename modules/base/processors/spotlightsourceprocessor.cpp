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

ProcessorClassIdentifier(SpotLightSourceProcessor, "org.inviwo.Spot light source");
ProcessorDisplayName(SpotLightSourceProcessor,  "Spot light source");
ProcessorTags(SpotLightSourceProcessor, Tags::CPU);
ProcessorCategory(SpotLightSourceProcessor, "Light source");
ProcessorCodeState(SpotLightSourceProcessor, CODE_STATE_EXPERIMENTAL);

SpotLightSourceProcessor::SpotLightSourceProcessor()
    : Processor()
    , outport_("SpotLightSource")
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)", 50.f, 0.f, 100.f)
    , lightSize_("lightSize", "Light size", vec2(1.5f, 1.5f), vec2(0.0f, 0.0f), vec2(3.0f, 3.0f))
    , lightDiffuse_("lightDiffuse", "Color", vec4(1.0f))
    , lightPosition_("lightPosition", "Light Source Position", vec3(1.f, 0.65f, 0.65f), vec3(-1.f), vec3(1.f)) 
    , lightConeRadiusAngle_("lightConeRadiusAngle", "Light Cone Radius Angle", 30.f, 1.f, 90.f)
    , lightFallOffAngle_("lightFallOffAngle", "Light Fall Off Angle", 5.f, 0.f, 30.f){
    addPort(outport_);

    lighting_.addProperty(lightPosition_);
    lighting_.addProperty(lightConeRadiusAngle_);
    lighting_.addProperty(lightFallOffAngle_);
    lighting_.addProperty(lightDiffuse_);
    lighting_.addProperty(lightPowerProp_);
    lighting_.addProperty(lightSize_);
    addProperty(lighting_);

    lightPosition_.setSemantics(PropertySemantics::LightPosition);
    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightSource_ = new SpotLight();
}

SpotLightSourceProcessor::~SpotLightSourceProcessor() {
    delete lightSource_;
}

void SpotLightSourceProcessor::process() {
    updateSpotLightSource(lightSource_);
    outport_.setData(lightSource_, false);
}

void SpotLightSourceProcessor::updateSpotLightSource(SpotLight* lightSource) {
    vec3 lightPos = lightPosition_.get();
    vec3 dir = glm::normalize(vec3(0.f) - lightPos);
    mat4 transformationMatrix = getLightTransformationMatrix(lightPos, dir);
    // Offset by 0.5 to get to texture coordinates
    lightSource->setModelMatrix(glm::translate(vec3(0.5f)));
    lightSource->setWorldMatrix(transformationMatrix);

    lightSource->setSize(lightSize_.get());
    vec3 diffuseLight = lightDiffuse_.get().xyz();
    lightSource->setIntensity(lightPowerProp_.get()*diffuseLight);
    lightSource->setDirection(dir);
    lightSource->setConeRadiusAngle(lightConeRadiusAngle_.get());
    lightSource->setConeFallOffAngle(lightFallOffAngle_.get());
}

} // namespace
