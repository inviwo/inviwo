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

ProcessorClassIdentifier(PointLightSourceProcessor, "org.inviwo.Point light source");
ProcessorDisplayName(PointLightSourceProcessor,  "Point light source");
ProcessorTags(PointLightSourceProcessor, Tags::CPU);
ProcessorCategory(PointLightSourceProcessor, "Light source");
ProcessorCodeState(PointLightSourceProcessor, CODE_STATE_EXPERIMENTAL);

PointLightSourceProcessor::PointLightSourceProcessor()
    : Processor()
    , outport_("PointLightSource")
    , lighting_("lighting", "Light Parameters")
    , lightPowerProp_("lightPower", "Light power (%)", 50.f, 0.f, 100.f)
    , lightSize_("lightSize", "Light radius", 1.5f, 0.0f, 3.0f)
    , lightDiffuse_("lightDiffuse", "Color", vec4(1.0f))
    , lightPosition_("lightPosition", "Light Source Position", vec3(-2.f, -50.f, 90.f),
                     vec3(-100.f), vec3(100.f))
    , lightEnabled_("lightEnabled", "Enabled", true)
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), nullptr, VALID)
    , interactionEvents_("interactionEvents", "Interaction Events") 
{
    addPort(outport_);
    lighting_.addProperty(lightPosition_);
    lighting_.addProperty(lightDiffuse_);
    lighting_.addProperty(lightPowerProp_);
    lighting_.addProperty(lightSize_);
    lighting_.addProperty(lightEnabled_);
    addProperty(lighting_);
    
    addProperty(camera_);
    camera_.setVisible(false);
    
    lightPosition_.setSemantics(PropertySemantics::LightPosition);
    lightDiffuse_.setSemantics(PropertySemantics::Color);
    lightSource_ = new PointLight();
    lightInteractionHandler_ = new PointLightInteractionHandler(&lightPosition_, &camera_);

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
    delete lightSource_;
    removeInteractionHandler(lightInteractionHandler_);
    delete lightInteractionHandler_;
    const std::vector<InteractionHandler*>& interactionHandlers = getInteractionHandlers();
    for (auto handler : interactionHandlers) {
        removeInteractionHandler(handler);
        LogWarn("Interaction handler was not removed and deleted");
    }

}

void PointLightSourceProcessor::process() {
    updatePointLightSource(lightSource_);
    outport_.setData(lightSource_, false);
}

void PointLightSourceProcessor::updatePointLightSource(PointLight* lightSource) {
    vec3 lightPos = lightPosition_.get();
    vec3 dir = glm::normalize(camera_.getLookTo()-lightPos);
    mat4 transformationMatrix = getLightTransformationMatrix(lightPos, dir);
    // Offset by 0.5 to get to texture coordinates
    lightSource->setModelMatrix(glm::translate(vec3(0.5f)));
    lightSource->setWorldMatrix(transformationMatrix);

    lightSource->setSize(vec2(lightSize_.get()));
    vec3 diffuseLight = lightDiffuse_.get().xyz();
    lightSource->setIntensity(lightPowerProp_.get()*diffuseLight);
    lightSource->setEnabled(lightEnabled_.get());
}



void PointLightSourceProcessor::handleInteractionEventsChanged() {
    if (interactionEvents_.get() > 0) {
        if (!hasInteractionHandler())
            addInteractionHandler(lightInteractionHandler_);

        
    } else {
        removeInteractionHandler(lightInteractionHandler_);
    }

    lightInteractionHandler_->setHandleEventsOptions(interactionEvents_.get());
}

PointLightSourceProcessor::PointLightInteractionHandler::PointLightInteractionHandler(FloatVec3Property* pl, CameraProperty* cam) 
    : InteractionHandler()
    , lightPosition_(pl)
    , camera_(cam)    
    , lookUp_(camera_->getLookUp())
    , lookTo_(0.f)
    , trackball_(&(pl->get()), &lookTo_, &lookUp_)
    , interactionEventOption_(0)
{
    static_cast<TrackballObservable*>(&trackball_)->addObserver(this);
    camera_->onChange(this, &PointLightInteractionHandler::onCameraChanged); 
}

void PointLightSourceProcessor::PointLightInteractionHandler::invokeEvent(Event* event) {
    //if(event->hasBeenUsed())
    //    return;

    if (interactionEventOption_ == 1 || interactionEventOption_ == 3){
        GestureEvent* gestureEvent = dynamic_cast<GestureEvent*>(event);
        if (gestureEvent) {
            if (gestureEvent->type() == GestureEvent::PAN){
                setLightPosFromScreenCoords(gestureEvent->screenPosNormalized());
                gestureEvent->markAsUsed();
                return;
            }
        }
        MouseEvent* mouseEvent = dynamic_cast<MouseEvent*>(event);
        if (mouseEvent) {
            int button = mouseEvent->button();
            if (button == MouseEvent::MOUSE_BUTTON_MIDDLE) {
                setLightPosFromScreenCoords(mouseEvent->posNormalized());
                mouseEvent->markAsUsed();
                return;
            }
        }
    }

    if (interactionEventOption_ == 1 || interactionEventOption_ == 2)
        trackball_.invokeInteractionEvent(event);
}

void PointLightSourceProcessor::PointLightInteractionHandler::setHandleEventsOptions(int option){
    interactionEventOption_ = option;
}

void PointLightSourceProcessor::PointLightInteractionHandler::setLightPosFromScreenCoords(const vec2& normalizedScreenCoord)
{
    vec2 deviceCoord(2.f*(normalizedScreenCoord-0.5f)); 
    // Flip vertical axis since mouse event y position starts at top of screen
    deviceCoord.y *= -1.f;
    // Use half distance between look from and look to positions as scene radius.
    float sceneRadius = 0.5f*glm::length(camera_->getLookTo()-camera_->getLookFrom());
    vec3 rayOrigin = camera_->getWorldPosFromNormalizedDeviceCoords(vec3(deviceCoord, 0.f)); 
    vec3 rayDir = glm::normalize(camera_->getWorldPosFromNormalizedDeviceCoords(vec3(deviceCoord, 1.f))-rayOrigin); 
    float t0 = 0, t1 = std::numeric_limits<float>::max();
    float lightRadius = glm::length(lightPosition_->get());

    if (raySphereIntersection(vec3(0.f), sceneRadius, rayOrigin, rayDir, &t0, &t1)) {
        lightPosition_->set(glm::normalize(rayOrigin + t1*rayDir)*lightRadius);
    } else {

        // Project it to the rim of the sphere
        t0 = 0; t1 = std::numeric_limits<float>::max();
        if (rayPlaneIntersection(camera_->getLookTo(), glm::normalize(camera_->getLookTo()-camera_->getLookFrom()), rayOrigin, rayDir, &t0, &t1)) {
            // Project it to the edge of the sphere
            lightPosition_->set(glm::normalize(rayOrigin + t1*rayDir)*lightRadius);
        }
    }
    // Ensure that up vector is same as camera afterwards
    *(trackball_.getLookUp()) = camera_->getLookUp();
}

void PointLightSourceProcessor::PointLightInteractionHandler::onAllTrackballChanged(const Trackball* trackball) {
    lightPosition_->propertyModified();
}

void PointLightSourceProcessor::PointLightInteractionHandler::onLookFromChanged(const Trackball* trackball) {
    lightPosition_->propertyModified();
}

void PointLightSourceProcessor::PointLightInteractionHandler::onCameraChanged() {
    // This makes sure that the interaction with the light source is consistent with the direction of the camera
    *(trackball_.getLookUp()) = camera_->getLookUp();
}

} // namespace
