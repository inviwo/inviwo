/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/base/processors/diffuselightsourceprocessor.h>

#include <inviwo/core/algorithm/markdown.h>                    // for operator""_help, operator"...
#include <inviwo/core/datastructures/camera/camera.h>          // for mat4
#include <inviwo/core/datastructures/light/baselightsource.h>  // for LightSource, getLightTrans...
#include <inviwo/core/datastructures/light/diffuselight.h>     // for DiffuseLight
#include <inviwo/core/ports/dataoutport.h>                     // for DataOutport
#include <inviwo/core/ports/outportiterable.h>                 // for OutportIterableImpl<>::con...
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Expe...
#include <inviwo/core/processors/processortags.h>              // for Tags, Tags::CPU
#include <inviwo/core/properties/cameraproperty.h>             // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>          // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>         // for ConstraintBehavior, Constr...
#include <inviwo/core/properties/invalidationlevel.h>          // for InvalidationLevel, Invalid...
#include <inviwo/core/properties/optionproperty.h>             // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>            // for FloatVec3Property, ordinal...
#include <inviwo/core/properties/positionproperty.h>           // for PositionProperty, Position...
#include <inviwo/core/properties/propertysemantics.h>          // for PropertySemantics, Propert...
#include <inviwo/core/util/glmvec.h>                           // for vec3, vec2

#include <functional>   // for __base
#include <string_view>  // for string_view

#include <fmt/core.h>                    // for format
#include <glm/ext/matrix_transform.hpp>  // for translate
#include <glm/geometric.hpp>             // for normalize
#include <glm/gtx/transform.hpp>         // for translate
#include <glm/vec2.hpp>                  // for vec
#include <glm/vec3.hpp>                  // for operator*, operator-, vec
#include <glm/vec4.hpp>                  // for operator*, operator+

namespace inviwo {

const ProcessorInfo DiffuseLightSourceProcessor::processorInfo_{
    "org.inviwo.Diffuselightsource",  // Class identifier
    "Diffuse light source",           // Display name
    "Light source",                   // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                        // Tags
    R"(Produces a planar area light source, spreading light in all directions from the plane.
    The direction of the plane will be computed as glm::normalize(vec3(0) - lightPos)
    when specified in world space and normalize(camera_.getLookTo() - lightPos) when specified in
    view space.)"_unindentHelp};

const ProcessorInfo DiffuseLightSourceProcessor::getProcessorInfo() const { return processorInfo_; }

DiffuseLightSourceProcessor::DiffuseLightSourceProcessor()
    : Processor()
    , outport_("DiffuseLightSource", "Planar area light source"_help)
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid)
    , lightPosition_("lightPosition", "Light Source Position", "Center point of the plane"_help,
                     FloatVec3Property("position", "Position", vec3(1.f, 0.65f, 0.65f), vec3(-10.f),
                                       vec3(10.f)),
                     &camera_)
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)", "Increases/decreases light strength"_help,
                      50.f, {0.f, ConstraintBehavior::Immutable},
                      {100.f, ConstraintBehavior::Immutable})
    , lightSize_(
          "lightSize", "Light size",
          util::ordinalLength(vec2(1.5f, 1.5f), 3.0f).set("Width and height in world space"_help))
    , lightDiffuse_("lightDiffuse", "Color",
                    "Flux density per solid angle, W*s*r^-1 (intensity)"_help, vec3(1.0f)) {

    addPort(outport_);
    addProperty(lightPosition_);
    lighting_.addProperty(lightDiffuse_);
    lighting_.addProperty(lightPowerProp_);
    lighting_.addProperty(lightSize_);
    addProperty(lighting_);
    addProperty(camera_);

    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightSource_ = std::make_shared<DiffuseLight>();
}

void DiffuseLightSourceProcessor::process() {
    updateLightSource(lightSource_.get());
    outport_.setData(lightSource_);
}

void DiffuseLightSourceProcessor::updateLightSource(DiffuseLight* lightSource) {
    vec3 lightPos = lightPosition_.get();
    vec3 dir;
    switch (
        static_cast<PositionProperty::Space>(lightPosition_.referenceFrame_.getSelectedValue())) {
        case PositionProperty::Space::VIEW: {
            dir = glm::normalize(camera_.getLookTo() - lightPos);
            break;
        }
        case PositionProperty::Space::WORLD:
            [[fallthrough]];
        default: {
            dir = glm::normalize(vec3(0.f) - lightPos);
            break;
        }
    }

    mat4 transformationMatrix = getLightTransformationMatrix(lightPos, dir);
    // Offset by 0.5 to get to texture coordinates
    lightSource->setModelMatrix(glm::translate(vec3(0.5f)));
    lightSource->setWorldMatrix(transformationMatrix);
    lightSource->setSize(lightSize_.get());
    lightSource->setIntensity(lightPowerProp_.get() * lightDiffuse_.get());
    lightSource->setNormal(dir);
}

}  // namespace inviwo
