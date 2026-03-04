/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/base/processors/spotlightsourceprocessor.h>

#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/light/baselightsource.h>
#include <inviwo/core/datastructures/light/spotlight.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/positionproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/glmvec.h>

#include <functional>
#include <string_view>

#include <fmt/core.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inviwo {

const ProcessorInfo SpotLightSourceProcessor::processorInfo_{
    "org.inviwo.Spotlightsource",  // Class identifier
    "Spot light source",           // Display name
    "Light source",                // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
    R"(Produces a spot light source, spreading light in the shape of a cone.
    The direction of the cone will be computed as glm::normalize(vec3(0) - lightPos)
    when specified in world space and normalize(camera_.getLookTo() - lightPos) when specified in
    view space.)"_unindentHelp,
};
const ProcessorInfo& SpotLightSourceProcessor::getProcessorInfo() const { return processorInfo_; }

SpotLightSourceProcessor::SpotLightSourceProcessor()
    : Processor()
    , outport_("SpotLightSource")
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid)
    , lightPosition_("lightPosition", "Light Source Position", "Start point of the cone."_help,
                     vec3(100.f), CoordinateSpace::World, &camera_,
                     PropertySemantics::LightPosition)
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)",
                      util::ordinalLength(50.f).set("Increases/decreases light strength"_help))
    , lightSize_("lightSize", "Light size", vec2(1.5f, 1.5f), vec2(0.0f, 0.0f), vec2(3.0f, 3.0f))
    , lightDiffuse_("lightDiffuse", "Color",
                    "Flux density per solid angle, W*s*r^-1 (intensity)"_help, vec3(1.0f))
    , lightConeRadiusAngle_("lightConeRadiusAngle", "Light Cone Radius Angle",
                            "Cone radius angle of the light source"_help, 30.f,
                            {1.f, ConstraintBehavior::Immutable},
                            {90.f, ConstraintBehavior::Immutable})
    , lightFallOffAngle_(
          "lightFallOffAngle", "Light Fall Off Angle", "Fall off angle of the light source"_help,
          5.f, {0.f, ConstraintBehavior::Immutable}, {30.f, ConstraintBehavior::Immutable})
    , lightSource_{std::make_shared<SpotLight>()} {

    addPort(outport_);

    lighting_.addProperties(lightConeRadiusAngle_, lightFallOffAngle_, lightDiffuse_,
                            lightPowerProp_, lightSize_);

    addProperties(lightPosition_, lighting_, camera_);

    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightDiffuse_.setCurrentStateAsDefault();
}

void SpotLightSourceProcessor::process() {
    updateSpotLightSource(lightSource_.get());
    outport_.setData(lightSource_);
}

void SpotLightSourceProcessor::updateSpotLightSource(SpotLight* lightSource) {
    const vec3 lightPos = lightPosition_.get(CoordinateSpace::World);
    const vec3 dir = -lightPosition_.getDirection(CoordinateSpace::World);

    mat4 transformationMatrix = getLightTransformationMatrix(lightPos, dir);
    // Offset by 0.5 to get to texture coordinates
    lightSource->setModelMatrix(glm::translate(vec3(0.5f)));
    lightSource->setWorldMatrix(transformationMatrix);

    lightSource->setSize(lightSize_.get());
    lightSource->setIntensity(lightPowerProp_.get() * lightDiffuse_.get());
    lightSource->setDirection(dir);
    lightSource->setConeRadiusAngle(lightConeRadiusAngle_.get());
    lightSource->setConeFallOffAngle(lightFallOffAngle_.get());
}

}  // namespace inviwo
