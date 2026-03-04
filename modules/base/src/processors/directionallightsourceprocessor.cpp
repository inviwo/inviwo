/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <modules/base/processors/directionallightsourceprocessor.h>

#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/light/baselightsource.h>
#include <inviwo/core/datastructures/light/directionallight.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/constraintbehavior.h>
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

const ProcessorInfo DirectionalLightSourceProcessor::processorInfo_{
    "org.inviwo.Directionallightsource",  // Class identifier
    "Directional light source",           // Display name
    "Light source",                       // Category
    CodeState::Experimental,              // Code state
    Tags::CPU,                            // Tags
    R"(Produces a light source with parallel light rays, spreading light in the direction from an
    infinite plane. The direction of the plane will be computed as
    glm::normalize(vec3(0) - lightPos) when specified in world space and
    normalize(camera_.getLookTo() - lightPos) when specified in view space.)"_unindentHelp};

const ProcessorInfo& DirectionalLightSourceProcessor::getProcessorInfo() const {
    return processorInfo_;
}

DirectionalLightSourceProcessor::DirectionalLightSourceProcessor()
    : Processor()
    , outport_("DirectionalLightSource", "Directional light source"_help)
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid)
    , lightPosition_("lightPosition", "Light Source Position", "Origin of the light source"_help,
                     vec3(100.f), CoordinateSpace::World, &camera_,
                     PropertySemantics::LightPosition)
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)", "Increases/decreases light strength"_help,
                      50.f, {0.f, ConstraintBehavior::Immutable},
                      {100.f, ConstraintBehavior::Immutable})
    , lightDiffuse_("lightDiffuse", "Color",
                    "Flux density per solid angle, W*s*r^-1 (intensity)"_help, vec3(1.0f))
    , lightEnabled_("lightEnabled", "Enabled", "Turns light on or off"_help, true)
    , lightSource_{std::make_shared<DirectionalLight>()} {

    addPort(outport_);

    lighting_.addProperties(lightDiffuse_, lightPowerProp_, lightEnabled_);
    addProperties(lightPosition_, lighting_, camera_);

    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightDiffuse_.setCurrentStateAsDefault();
}

void DirectionalLightSourceProcessor::process() {
    updateDirectionalLightSource(lightSource_.get());
    outport_.setData(lightSource_);
}

void DirectionalLightSourceProcessor::updateDirectionalLightSource(DirectionalLight* lightSource) {
    const vec3 lightPos = lightPosition_.get(CoordinateSpace::World);
    const vec3 dir = -lightPosition_.getDirection(CoordinateSpace::World);

    mat4 transformationMatrix = getLightTransformationMatrix(lightPos, dir);

    // Offset by 0.5 to get to texture coordinates
    lightSource->setModelMatrix(glm::translate(vec3(0.5f)));
    lightSource->setWorldMatrix(transformationMatrix);
    lightSource->setSize(vec2(1.f));

    lightSource->setIntensity(lightPowerProp_.get() * lightDiffuse_.get());
    lightSource->setDirection(dir);
    lightSource->setEnabled(lightEnabled_.get());
}

}  // namespace inviwo
