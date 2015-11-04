/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "directionallightsourceprocessor.h"
#include <inviwo/core/datastructures/light/directionallight.h>

namespace inviwo {

const ProcessorInfo DirectionalLightSourceProcessor::processorInfo_{
    "org.inviwo.Directionallightsource",  // Class identifier
    "Directional light source",           // Display name
    "Light source",                       // Category
    CodeState::Experimental,              // Code state
    Tags::CPU,                            // Tags
};
const ProcessorInfo DirectionalLightSourceProcessor::getProcessorInfo() const {
    return processorInfo_;
}

DirectionalLightSourceProcessor::DirectionalLightSourceProcessor()
    : Processor()
    , outport_("DirectionalLightSource")
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid)
    , lightPosition_(
          "lightPosition", "Light Source Position",
          FloatVec3Property("position", "Position", vec3(100.f), vec3(-100.f), vec3(100.f)),
          &camera_.get())
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)", 50.f, 0.f, 100.f)
    , lightDiffuse_("lightDiffuse", "Color", vec3(1.0f))
    , lightEnabled_("lightEnabled", "Enabled", true)
    , lightSource_{std::make_shared<DirectionalLight>()} {
    addPort(outport_);

    addProperty(lightPosition_);
    lighting_.addProperty(lightDiffuse_);
    lighting_.addProperty(lightPowerProp_);
    lighting_.addProperty(lightEnabled_);
    addProperty(lighting_);
    addProperty(camera_);

    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightDiffuse_.setCurrentStateAsDefault();
}


void DirectionalLightSourceProcessor::process() {
    updateDirectionalLightSource(lightSource_.get());
    outport_.setData(lightSource_);
}

void DirectionalLightSourceProcessor::updateDirectionalLightSource(DirectionalLight* lightSource) {
    vec3 lightPos = lightPosition_.get();
    vec3 dir;
    switch (
        static_cast<PositionProperty::Space>(lightPosition_.referenceFrame_.getSelectedValue())) {
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
    lightSource->setSize(vec2(1.f));

    lightSource->setIntensity(lightPowerProp_.get() * lightDiffuse_.get());
    lightSource->setDirection(dir);
    lightSource->setEnabled(lightEnabled_.get());
}

}  // namespace

