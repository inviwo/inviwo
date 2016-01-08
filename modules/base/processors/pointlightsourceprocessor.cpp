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

#include "pointlightsourceprocessor.h"
#include <inviwo/core/util/intersection/rayplaneintersection.h>
#include <inviwo/core/util/intersection/raysphereintersection.h>
#include <inviwo/core/datastructures/light/pointlight.h>

namespace inviwo {

PropertyClassIdentifier(PointLightTrackball, "org.inviwo.PointLightTrackball");

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
    , lightPosition_("lightPosition", "Light Source Position",
                     FloatVec3Property("position", "Position", vec3(-2.f, -50.f, 90.f),
                                       vec3(-100.f), vec3(100.f)),
                     &camera_)
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
    addProperty(lightPosition_);
    lighting_.addProperty(lightDiffuse_);
    lighting_.addProperty(lightPowerProp_);
    lighting_.addProperty(lightSize_);
    lighting_.addProperty(lightEnabled_);
    lighting_.addProperty(lightScreenPosEnabled_);
    lighting_.addProperty(lightScreenPos_);
    addProperty(lighting_);

    lightScreenPos_.setVisible(false);
    lightScreenPosEnabled_.onChange(
        [this]() { lightScreenPos_.setVisible(lightScreenPosEnabled_.get()); });

    addProperty(camera_);
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
    addProperty(interactionEvents_);

    interactionEvents_.onChange(this, &PointLightSourceProcessor::handleInteractionEventsChanged);
}

PointLightSourceProcessor::~PointLightSourceProcessor() {
    removeInteractionHandler(&lightInteractionHandler_);
}

void PointLightSourceProcessor::process() {
    updatePointLightSource(lightSource_.get());
    outport_.setData(lightSource_);
}

void PointLightSourceProcessor::updatePointLightSource(PointLight* lightSource) {
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

    lightSource->setSize(vec2(lightSize_.get()));
    lightSource->setIntensity(lightPowerProp_.get() * lightDiffuse_.get());
    lightSource->setEnabled(lightEnabled_.get());
}

void PointLightSourceProcessor::handleInteractionEventsChanged() {
    if (interactionEvents_.get() > 0) {
        addInteractionHandler(&lightInteractionHandler_);
    } else {
        removeInteractionHandler(&lightInteractionHandler_);
    }
    lightInteractionHandler_.setHandleEventsOptions(interactionEvents_.get());
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
    camera_->onChange(this, &PointLightInteractionHandler::onCameraChanged);
}

PointLightInteractionHandler::~PointLightInteractionHandler() {}

void PointLightInteractionHandler::serialize(Serializer& s) const {}

void PointLightInteractionHandler::deserialize(Deserializer& d) {}

const vec3& PointLightInteractionHandler::getLookFrom() const { return lightPosition_->get(); }

const vec3& PointLightInteractionHandler::getLookUp() const { return lookUp_; }

void PointLightInteractionHandler::setLookTo(vec3 lookTo) { lookTo_ = lookTo; }

void PointLightInteractionHandler::setLookFrom(vec3 lookFrom) { lightPosition_->set(lookFrom); }

void PointLightInteractionHandler::setLookUp(vec3 lookUp) { lookUp_ = lookUp; }

inviwo::vec3 PointLightInteractionHandler::getLookFromMinValue() const {
    return lightPosition_->position_.getMinValue();
}

inviwo::vec3 PointLightInteractionHandler::getLookFromMaxValue() const {
    return lightPosition_->position_.getMaxValue();
}

inviwo::vec3 PointLightInteractionHandler::getLookToMinValue() const {
    return vec3(-std::numeric_limits<float>::max());
}

inviwo::vec3 PointLightInteractionHandler::getLookToMaxValue() const {
    return vec3(std::numeric_limits<float>::max());
}

void PointLightInteractionHandler::setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) {
    lightPosition_->set(lookFrom);
    lookTo_ = lookTo;
    lookUp = lookUp;
}

float PointLightInteractionHandler::getNearPlaneDist() const { return camera_->getNearPlaneDist(); }

float PointLightInteractionHandler::getFarPlaneDist() const { return camera_->getFarPlaneDist(); }

inviwo::vec3 PointLightInteractionHandler::getWorldPosFromNormalizedDeviceCoords(
    const vec3& ndcCoords) const {
    return camera_->getWorldPosFromNormalizedDeviceCoords(ndcCoords);
}

inviwo::vec3 PointLightInteractionHandler::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
    const vec2& normalizedScreenCoord) const {
    return camera_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord);
}

std::string PointLightInteractionHandler::getClassIdentifier() const {
    return "org.inviwo.PointLightInteractionHandler";
}

const Camera& PointLightInteractionHandler::getCamera() { return camera_->get(); }

void PointLightInteractionHandler::invokeEvent(Event* event) {
    // if(event->hasBeenUsed())
    //    return;

    if (screenPosEnabled_->get()) setLightPosFromScreenCoords(screenPos_->get());

    if (interactionEventOption_ == 1 || interactionEventOption_ == 3) {
        GestureEvent* gestureEvent = dynamic_cast<GestureEvent*>(event);
        if (gestureEvent) {
            if (gestureEvent->type() == GestureEvent::PAN) {
                setLightPosFromScreenCoords(gestureEvent->screenPosNormalized());
                gestureEvent->markAsUsed();
                return;
            }
        }
        MouseEvent* mouseEvent = dynamic_cast<MouseEvent*>(event);
        if (mouseEvent) {
            int button = mouseEvent->button();
            if (button == MouseEvent::MOUSE_BUTTON_MIDDLE) {
                // setLightPosFromScreenCoords(mouseEvent->posNormalized());
                mouseEvent->markAsUsed();
                screenPos_->set(mouseEvent->posNormalized());
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
    float t0 = 0, t1 = std::numeric_limits<float>::max();
    float lightRadius = glm::length(lightPosition_->get());

    if (raySphereIntersection(vec3(0.f), sceneRadius, rayOrigin, rayDir, &t0, &t1)) {
        lightPosition_->set(glm::normalize(rayOrigin + t1 * rayDir) * lightRadius);
    } else {
        // Project it to the rim of the sphere
        t0 = 0;
        t1 = std::numeric_limits<float>::max();
        if (rayPlaneIntersection(camera_->getLookTo(),
                                 glm::normalize(camera_->getLookTo() - camera_->getLookFrom()),
                                 rayOrigin, rayDir, &t0, &t1)) {
            // Project it to the edge of the sphere
            lightPosition_->set(glm::normalize(rayOrigin + t1 * rayDir) * lightRadius);
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

const vec3& PointLightInteractionHandler::getLookTo() const { return lookTo_; }

PointLightTrackball::PointLightTrackball(PointLightInteractionHandler* p) : Trackball(p) {}

}  // namespace
