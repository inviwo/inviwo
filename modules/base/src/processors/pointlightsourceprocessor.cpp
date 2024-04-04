/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <modules/base/processors/pointlightsourceprocessor.h>

#include <inviwo/core/datastructures/camera/camera.h>             // for mat4
#include <inviwo/core/datastructures/camera/cameratools.h>        // for perspectiveZoom
#include <inviwo/core/datastructures/light/baselightsource.h>     // for LightSource, getLightTr...
#include <inviwo/core/datastructures/light/pointlight.h>          // for PointLight
#include <inviwo/core/interaction/events/event.h>                 // for Event
#include <inviwo/core/interaction/events/gestureevent.h>          // for GestureEvent
#include <inviwo/core/interaction/events/gesturestate.h>          // for GestureType, GestureTyp...
#include <inviwo/core/interaction/events/mousebuttons.h>          // for MouseButton, MouseButto...
#include <inviwo/core/interaction/events/mouseevent.h>            // for MouseEvent
#include <inviwo/core/interaction/interactionhandler.h>           // for InteractionHandler
#include <inviwo/core/interaction/trackball.h>                    // for Trackball
#include <inviwo/core/interaction/trackballobject.h>              // for TrackballObject::Bounded
#include <inviwo/core/ports/dataoutport.h>                        // for DataOutport
#include <inviwo/core/ports/outportiterable.h>                    // for OutportIterableImpl<>::...
#include <inviwo/core/processors/processor.h>                     // for Processor
#include <inviwo/core/processors/processorinfo.h>                 // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                // for CodeState, CodeState::E...
#include <inviwo/core/processors/processortags.h>                 // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                  // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>             // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>             // for InvalidationLevel, Inva...
#include <inviwo/core/properties/optionproperty.h>                // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>               // for FloatVec2Property, Floa...
#include <inviwo/core/properties/positionproperty.h>              // for PositionProperty, Posit...
#include <inviwo/core/properties/propertysemantics.h>             // for PropertySemantics, Prop...
#include <inviwo/core/util/glmvec.h>                              // for vec3, vec2
#include <inviwo/core/util/intersection/rayplaneintersection.h>   // for rayPlaneIntersection
#include <inviwo/core/util/intersection/raysphereintersection.h>  // for raySphereIntersection

#include <cmath>        // for sqrt
#include <functional>   // for __base
#include <limits>       // for numeric_limits<>::type
#include <optional>     // for nullopt
#include <string_view>  // for string_view
#include <utility>      // for pair

#include <fmt/core.h>                    // for format
#include <glm/ext/matrix_transform.hpp>  // for translate
#include <glm/geometric.hpp>             // for normalize, dot, length
#include <glm/gtx/transform.hpp>         // for translate
#include <glm/vec2.hpp>                  // for operator*, operator-, vec
#include <glm/vec3.hpp>                  // for operator*, operator-, vec
#include <glm/vec4.hpp>                  // for operator*, operator+

namespace inviwo {
class Deserializer;
class Serializer;

const std::string PointLightTrackball::classIdentifier = "org.inviwo.PointLightTrackball";
std::string PointLightTrackball::getClassIdentifier() const { return classIdentifier; }

const ProcessorInfo PointLightSourceProcessor::processorInfo_{
    "org.inviwo.Pointlightsource",  // Class identifier
    "Point light source",           // Display name
    "Light source",                 // Category
    CodeState::Experimental,        // Code state
    Tags::CPU,                      // Tags
};
const ProcessorInfo PointLightSourceProcessor::getProcessorInfo() const { return processorInfo_; }

PointLightSourceProcessor::PointLightSourceProcessor()
    : Processor()
    , outport_("PointLightSource")
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid)
    , lightPosition_("lightPosition", "Light Source Position", vec3(-2.f, -50.f, 90.f),
                     CoordinateSpace::World, &camera_, PropertySemantics::LightPosition)
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)", 50.f, 0.f, 100.f)
    , lightSize_("lightSize", "Light radius", 1.5f, 0.0f, 3.0f)
    , lightDiffuse_("lightDiffuse", "Color", vec3(1.0f))
    , lightEnabled_("lightEnabled", "Enabled", true)
    , lightScreenPosEnabled_("lightScreenPosEnabled", "Screen Pos Enabled", false)
    , lightScreenPos_("lightScreenPos", "Light Screen Pos", vec2(0.7f), vec2(0.f), vec2(1.f))
    , interactionEvents_("interactionEvents", "Interaction Events")
    , lightInteractionHandler_(&lightPosition_, &camera_, &lightScreenPosEnabled_, &lightScreenPos_)
    , lightSource_(std::make_shared<PointLight>()) {

    addPort(outport_);

    lighting_.addProperties(lightDiffuse_, lightPowerProp_, lightSize_, lightEnabled_,
                            lightScreenPosEnabled_, lightScreenPos_);

    addProperties(lightPosition_, lighting_, camera_, interactionEvents_);

    lightScreenPos_.setVisible(false);
    lightScreenPosEnabled_.onChange(
        [this]() { lightScreenPos_.setVisible(lightScreenPosEnabled_.get()); });

    camera_.setVisible(false);

    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightDiffuse_.setCurrentStateAsDefault();

    lightScreenPos_.onChange(
        [this]() { lightInteractionHandler_.setLightPosFromScreenCoords(lightScreenPos_.get()); });

    interactionEvents_.addOption("off", "Handle None", 0);
    interactionEvents_.addOption("on", "Handle All", 1);
    interactionEvents_.addOption("onlytrackball", "Handle Trackball Related Only", 2);
    interactionEvents_.addOption("onlyscreencorrds", "Handle Light Pos From Screen Coords Only", 3);
    interactionEvents_.setSelectedValue(0);
    interactionEvents_.setCurrentStateAsDefault();

    interactionEvents_.onChange([this]() {
        if (interactionEvents_.get() > 0) {
            addInteractionHandler(&lightInteractionHandler_);
        } else {
            removeInteractionHandler(&lightInteractionHandler_);
        }
        lightInteractionHandler_.setHandleEventsOptions(interactionEvents_.get());
    });
}

PointLightSourceProcessor::~PointLightSourceProcessor() = default;

void PointLightSourceProcessor::process() {
    updatePointLightSource(lightSource_.get());
    outport_.setData(lightSource_);
}

void PointLightSourceProcessor::updatePointLightSource(PointLight* lightSource) {
    const vec3 lightPos = lightPosition_.get(CoordinateSpace::World);
    const vec3 dir = -glm::normalize(lightPosition_.getWorldSpaceDirection());

    mat4 transformationMatrix = getLightTransformationMatrix(lightPos, dir);
    // Offset by 0.5 to get to texture coordinates
    lightSource->setModelMatrix(glm::translate(vec3(0.5f)));
    lightSource->setWorldMatrix(transformationMatrix);

    lightSource->setSize(vec2(lightSize_.get()));
    lightSource->setIntensity(lightPowerProp_.get() * lightDiffuse_.get());
    lightSource->setEnabled(lightEnabled_.get());
}

PointLightInteractionHandler::PointLightInteractionHandler(PositionProperty* pl,
                                                           CameraProperty* cam,
                                                           BoolProperty* screenPosEnabled,
                                                           FloatVec2Property* screenPos)
    : InteractionHandler()
    , lightPosition_(pl)
    , camera_(cam)
    , screenPosEnabled_(screenPosEnabled)
    , screenPos_(screenPos)
    , lookUp_(camera_->getLookUp())
    , lookTo_(0.f)
    , trackball_(this)
    , interactionEventOption_(0) {
    // static_cast<TrackballObservable*>(&trackball_)->addObserver(this);
    camera_->onChange([this]() { onCameraChanged(); });
}

PointLightInteractionHandler::~PointLightInteractionHandler() = default;

vec3 PointLightInteractionHandler::getLookTo() const { return lookTo_; }

vec3 PointLightInteractionHandler::getLookFrom() const { return lightPosition_->get(); }

vec3 PointLightInteractionHandler::getLookUp() const { return lookUp_; }

PointLightInteractionHandler& PointLightInteractionHandler::setLookTo(vec3 lookTo) {
    lookTo_ = lookTo;
    return *this;
}

PointLightInteractionHandler& PointLightInteractionHandler::setLookFrom(vec3 lookFrom) {
    lightPosition_->updatePosition(lookFrom, CoordinateSpace::World);
    return *this;
}

PointLightInteractionHandler& PointLightInteractionHandler::setLookUp(vec3 lookUp) {
    lookUp_ = lookUp;
    return *this;
}

vec3 PointLightInteractionHandler::getLookFromMinValue() const {
    return camera_->lookFrom_.getMinValue();
}

vec3 PointLightInteractionHandler::getLookFromMaxValue() const {
    return camera_->lookFrom_.getMaxValue();
}

vec3 PointLightInteractionHandler::getLookToMinValue() const {
    return camera_->lookTo_.getMinValue();
}

vec3 PointLightInteractionHandler::getLookToMaxValue() const {
    return camera_->lookTo_.getMaxValue();
}

PointLightInteractionHandler& PointLightInteractionHandler::setLook(vec3 lookFrom, vec3 lookTo,
                                                                    vec3 lookUp) {
    lightPosition_->updatePosition(lookFrom, CoordinateSpace::World);
    lookTo_ = lookTo;
    lookUp_ = lookUp;
    return *this;
}

float PointLightInteractionHandler::getNearPlaneDist() const { return camera_->getNearPlaneDist(); }

float PointLightInteractionHandler::getFarPlaneDist() const { return camera_->getFarPlaneDist(); }

void PointLightInteractionHandler::zoom(float factor, Bounded) {
    setLookFrom(util::perspectiveZoom(*this, factor, std::nullopt));
}

vec3 PointLightInteractionHandler::getWorldPosFromNormalizedDeviceCoords(
    const vec3& ndcCoords) const {
    return camera_->getWorldPosFromNormalizedDeviceCoords(ndcCoords);
}

vec3 PointLightInteractionHandler::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
    const vec2& normalizedScreenCoord) const {
    return camera_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord);
}

std::string PointLightInteractionHandler::getClassIdentifier() const {
    return "org.inviwo.PointLightInteractionHandler";
}

const Camera& PointLightInteractionHandler::getCamera() { return camera_->get(); }

void PointLightInteractionHandler::invokeEvent(Event* event) {
    if (screenPosEnabled_->get()) setLightPosFromScreenCoords(screenPos_->get());

    if (interactionEventOption_ == 1 || interactionEventOption_ == 3) {
        if (auto gestureEvent = dynamic_cast<GestureEvent*>(event)) {
            if (gestureEvent->type() == GestureType::Pan) {
                setLightPosFromScreenCoords(gestureEvent->screenPosNormalized());
                gestureEvent->markAsUsed();
                return;
            }
        } else if (auto mouseEvent = dynamic_cast<MouseEvent*>(event)) {
            if (mouseEvent->button() == MouseButton::Middle) {
                // setLightPosFromScreenCoords(mouseEvent->posNormalized());
                mouseEvent->markAsUsed();
                screenPos_->set(static_cast<vec2>(mouseEvent->posNormalized()));
                return;
            }
        }
    }

    if (interactionEventOption_ == 1 || interactionEventOption_ == 2) trackball_.invokeEvent(event);
}

void PointLightInteractionHandler::setHandleEventsOptions(int option) {
    interactionEventOption_ = option;
}

void PointLightInteractionHandler::setLightPosFromScreenCoords(const vec2& normalizedScreenCoord) {
    vec2 deviceCoord(2.f * (normalizedScreenCoord - 0.5f));
    // Flip vertical axis since mouse event y position starts at top of screen
    deviceCoord.y *= -1.f;
    // Use half distance between look from and look to positions as scene radius.
    float sceneRadius = 0.5f * glm::length(camera_->getLookTo() - camera_->getLookFrom());
    vec3 rayOrigin = camera_->getWorldPosFromNormalizedDeviceCoords(vec3(deviceCoord, 0.f));
    vec3 rayDir = glm::normalize(
        camera_->getWorldPosFromNormalizedDeviceCoords(vec3(deviceCoord, 1.f)) - rayOrigin);

    float lightRadius = glm::length(lightPosition_->get());
    auto res = raySphereIntersection(vec3(0.f), sceneRadius, rayOrigin, rayDir, 0.0f,
                                     std::numeric_limits<float>::max());
    if (res.first) {
        lightPosition_->updatePosition(
            glm::normalize(rayOrigin + res.second * rayDir) * lightRadius, CoordinateSpace::World);
    } else {
        auto res2 = rayPlaneIntersection(
            camera_->getLookTo(), glm::normalize(camera_->getLookTo() - camera_->getLookFrom()),
            rayOrigin, rayDir, 0.0f, std::numeric_limits<float>::max());
        if (res2.first) {
            // Project it to the edge of the sphere
            lightPosition_->updatePosition(
                glm::normalize(rayOrigin + res2.second * rayDir) * lightRadius,
                CoordinateSpace::World);
        }
    }
    // Ensure that up vector is same as camera afterwards
    setLookUp(camera_->getLookUp());
}

void PointLightInteractionHandler::onCameraChanged() {
    // This makes sure that the interaction with the light source is consistent with the direction
    // of the camera
    setLookUp(camera_->getLookUp());
}

PointLightTrackball::PointLightTrackball(PointLightInteractionHandler* p) : Trackball(p) {}

}  // namespace inviwo
