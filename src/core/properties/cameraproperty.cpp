/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/camerafactory.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/interaction/events/mouseevent.h>

namespace inviwo {

PropertyClassIdentifier(CameraProperty, "org.inviwo.CameraProperty");

CameraProperty::CameraProperty(std::string identifier, std::string displayName, vec3 eye,
                               vec3 center, vec3 lookUp, Inport* inport,
                               InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , cameraType_("cameraType", "Camera Type",
                  []() {
                      std::vector<OptionPropertyStringOption> options;
                      for (auto& key : InviwoApplication::getPtr()->getCameraFactory()->getKeys()) {
                          options.emplace_back(key, key, key);
                      }
                      return options;
                  }(),
                  0)
    , lookFrom_("lookFrom", "Look from", eye, -vec3(100.0f), vec3(100.0f), vec3(0.1f),
                InvalidationLevel::InvalidOutput, PropertySemantics("Spherical"))
    , lookTo_("lookTo", "Look to", center, -vec3(100.0f), vec3(100.0f), vec3(0.1f))
    , lookUp_("lookUp", "Look up", lookUp, -vec3(100.0f), vec3(100.0f), vec3(0.1f))
    , aspectRatio_("aspectRatio", "Aspect Ratio", 1.0f, 0.01f, 100.0f, 0.01f)
    , nearPlane_("near", "Near Plane", 0.1f, 0.001f, 10.f, 0.001f)
    , farPlane_("far", "Far Plane", 100.0f, 1.0f, 1000.0f, 1.0f)
    , adjustCameraOnDataChange_("fitToBasis_", "Adjust camera on data change", true,
                                InvalidationLevel::Valid)



    , mouseChangeFocusPoint_("mouseChangeFocusPoint", "Change Focus Point",
        new MouseEvent(MouseEvent::MOUSE_BUTTON_LEFT, InteractionEvent::MODIFIER_NONE,
            MouseEvent::MOUSE_STATE_DOUBLE_CLICK),
        new Action(this, &CameraProperty::changeFocusPoint))

    , camera_()
    , inport_(inport)
    , data_(nullptr)
    , prevDataToWorldMatrix_(0) {
    // Make sure that the Camera) is
    // in sync with the property values.
    cameraType_.setSelectedIdentifier("PerspectiveCamera");
    cameraType_.setCurrentStateAsDefault();
    addProperty(cameraType_);
    cameraType_.onChange([&]() {
        changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_.get()));
    });

    lookFrom_.onChange([&]() { camera_->setLookFrom(lookFrom_.get()); });
    lookTo_.onChange([&]() { camera_->setLookTo(lookTo_.get()); });
    lookUp_.onChange([&]() { camera_->setLookUp(lookUp_.get()); });
    addProperty(lookFrom_);
    addProperty(lookTo_);
    addProperty(lookUp_);
    aspectRatio_.onChange([&]() { camera_->setAspectRatio(aspectRatio_.get()); });
    nearPlane_.onChange([&]() { camera_->setNearPlaneDist(nearPlane_.get()); });
    farPlane_.onChange([&]() { camera_->setFarPlaneDist(farPlane_.get()); });
    addProperty(aspectRatio_);
    addProperty(nearPlane_);
    addProperty(farPlane_);
    addProperty(mouseChangeFocusPoint_);

    adjustCameraOnDataChange_.onChange([&]() { resetAdjustCameraToData(); });
    addProperty(adjustCameraOnDataChange_);

    changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_.get()));

    if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);
}

CameraProperty::CameraProperty(const CameraProperty& rhs)
    : CompositeProperty(rhs)
    , cameraType_(rhs.cameraType_)
    , lookFrom_(rhs.lookFrom_)
    , lookTo_(rhs.lookTo_)
    , lookUp_(rhs.lookUp_)
    , aspectRatio_(rhs.aspectRatio_)
    , nearPlane_(rhs.nearPlane_)
    , farPlane_(rhs.farPlane_)
    , adjustCameraOnDataChange_(rhs.adjustCameraOnDataChange_)
    , mouseChangeFocusPoint_(rhs.mouseChangeFocusPoint_)
    , camera_()
    , inport_(rhs.inport_)
    , data_(nullptr)
    , prevDataToWorldMatrix_(0) 

{
    // Make sure that the Camera) is
    // in sync with the property values.
    addProperty(cameraType_);
    cameraType_.onChange([&]() {
        changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_.get()));
    });

    lookFrom_.onChange([&]() { camera_->setLookFrom(lookFrom_.get()); });
    lookTo_.onChange([&]() { camera_->setLookTo(lookTo_.get()); });
    lookUp_.onChange([&]() { camera_->setLookUp(lookUp_.get()); });
    addProperty(lookFrom_);
    addProperty(lookTo_);
    addProperty(lookUp_);
    aspectRatio_.onChange([&]() { camera_->setAspectRatio(aspectRatio_.get()); });
    nearPlane_.onChange([&]() { camera_->setNearPlaneDist(nearPlane_.get()); });
    farPlane_.onChange([&]() { camera_->setFarPlaneDist(farPlane_.get()); });
    addProperty(aspectRatio_);
    addProperty(nearPlane_);
    addProperty(farPlane_);
    addProperty(mouseChangeFocusPoint_);

    adjustCameraOnDataChange_.onChange([&]() { resetAdjustCameraToData(); });
    addProperty(adjustCameraOnDataChange_);

    changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_.get()));

    if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);

    inportChanged();
}

void CameraProperty::changeCamera(std::unique_ptr<Camera> newCamera) {
    NetworkLock lock(this);
    camera_ = std::move(newCamera);
    camera_->setLookFrom(lookFrom_.get());
    camera_->setLookTo(lookTo_.get());
    camera_->setLookUp(lookUp_.get());
    camera_->setAspectRatio(aspectRatio_.get());
    camera_->setNearPlaneDist(nearPlane_.get());
    camera_->setFarPlaneDist(farPlane_.get());
    camera_->configureProperties(this);
}

CameraProperty& CameraProperty::operator=(const CameraProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        changeCamera(std::unique_ptr<Camera>(that.camera_->clone()));

        if (inport_) inport_->removeOnChange(this);
        inport_ = that.inport_;
        if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);
        data_ = nullptr;
        prevDataToWorldMatrix_ = mat4(0);
        updatePropertyFromValue();

        inportChanged();
    }
    return *this;
}

const Camera& CameraProperty::get() const { return *camera_; }
Camera& CameraProperty::get() { return *camera_; }

void CameraProperty::set(const Property* srcProperty) {
    if (const auto cameraSrcProp = dynamic_cast<const CameraProperty*>(srcProperty)) {
        if (!camera_->update(cameraSrcProp->camera_.get())) {
            // update failed, make a clone
            changeCamera(std::unique_ptr<Camera>(cameraSrcProp->camera_->clone()));
        }
        CompositeProperty::set(static_cast<const CompositeProperty*>(srcProperty));
    }
}

CameraProperty::operator const Camera&() const { return *camera_; }

CameraProperty* CameraProperty::clone() const { return new CameraProperty(*this); }

void CameraProperty::updatePropertyFromValue() {
    NetworkLock lock(this);
    lookFrom_ = camera_->getLookFrom();
    lookTo_ = camera_->getLookTo();
    lookUp_ = camera_->getLookUp();
    aspectRatio_ = camera_->getAspectRatio();
    nearPlane_ = camera_->getNearPlaneDist();
    farPlane_ = camera_->getFarPlaneDist();

    propertyModified();
}

void CameraProperty::resetCamera() {
    NetworkLock lock(this);

    lookFrom_.resetToDefaultState();
    lookTo_.resetToDefaultState();
    lookUp_.resetToDefaultState();

    // Update template value
    camera_->setLookFrom(lookFrom_.get());
    camera_->setLookTo(lookTo_.get());
    camera_->setLookUp(lookUp_.get());
}

// It seems like it is a job for the code managing interaction to consider the boundaries.
// Need to change that code before clamping values.
// void CameraProperty::setLookFrom(vec3 lookFrom) { lookFrom_.set(glm::clamp(lookFrom,
// lookFrom_.getMinValue(), lookFrom_.getMaxValue())); }
// void CameraProperty::setLookTo(vec3 lookTo) { lookTo_.set(glm::clamp(lookTo,
// lookTo_.getMinValue(), lookTo_.getMaxValue())); }
void CameraProperty::setLookFrom(vec3 lookFrom) { lookFrom_.set(lookFrom); }

void CameraProperty::setLookTo(vec3 lookTo) { lookTo_.set(lookTo); }

void CameraProperty::setLookUp(vec3 lookUp) { lookUp_.set(lookUp); }

void CameraProperty::setAspectRatio(float aspectRatio) {
    aspectRatio_.set(
        glm::clamp(aspectRatio, aspectRatio_.getMinValue(), aspectRatio_.getMaxValue()));
}
float CameraProperty::getAspectRatio() const { return camera_->getAspectRatio(); }

void CameraProperty::setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) {
    NetworkLock lock(this);
    setLookFrom(lookFrom);
    setLookTo(lookTo);
    setLookUp(lookUp);
}

float CameraProperty::getNearPlaneDist() const { return nearPlane_.get(); }

float CameraProperty::getFarPlaneDist() const { return farPlane_.get(); }

void CameraProperty::setNearPlaneDist(float v) {
    nearPlane_.set(glm::clamp(v, nearPlane_.getMinValue(), nearPlane_.getMaxValue()));
}

void CameraProperty::setFarPlaneDist(float v) {
    farPlane_.set(glm::clamp(v, farPlane_.getMinValue(), farPlane_.getMaxValue()));
}

inviwo::vec3 CameraProperty::getLookFromMinValue() const { return lookFrom_.getMinValue(); }

inviwo::vec3 CameraProperty::getLookFromMaxValue() const { return lookFrom_.getMaxValue(); }

inviwo::vec3 CameraProperty::getLookToMinValue() const { return lookTo_.getMinValue(); }

inviwo::vec3 CameraProperty::getLookToMaxValue() const { return lookTo_.getMaxValue(); }

// XYZ between -1 -> 1
vec3 CameraProperty::getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    return get().getWorldPosFromNormalizedDeviceCoords(ndcCoords);
}

vec4 CameraProperty::getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    return get().getClipPosFromNormalizedDeviceCoords(ndcCoords);
}

vec3 CameraProperty::getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
    const vec2& normalizedScreenCoord) const {
    return camera_->getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(normalizedScreenCoord);
}

void CameraProperty::invokeEvent(Event* event) {
    if (auto resizeEvent = dynamic_cast<ResizeEvent*>(event)) {
        uvec2 canvasSize = resizeEvent->size();
        // Do not set aspect ratio if canvas size is 0 in any dimension.
        if (canvasSize.x > 0 && canvasSize.y > 0) {
            const double width{static_cast<double>(canvasSize[0])};
            const double height{static_cast<double>(canvasSize[1])};
            setAspectRatio(static_cast<float>(width / height));
        }
    }
    else {
        PropertyOwner::invokeEvent(event);
    }
}

void CameraProperty::setInport(Inport* inport) {
    if (inport_ != inport) inport->onChange(this, &CameraProperty::inportChanged);

    inport_ = inport;
}

void CameraProperty::adjustCameraToData(const mat4& prevDataToWorldMatrix,
                                        const mat4& newDataToWorldMatrix) {
    if (newDataToWorldMatrix != prevDataToWorldMatrix_) {
        // Transform to data space of old basis and then to world space in new basis.
        auto toNewSpace = newDataToWorldMatrix * glm::inverse(prevDataToWorldMatrix_);

        auto newLookFrom = (toNewSpace * vec4(lookFrom_.get(), 1.f)).xyz();
        auto newLookTo = (toNewSpace * vec4(lookTo_.get(), 1.f)).xyz();
        float depthRatio =
            glm::length(newLookTo - newLookFrom) / glm::length(camera_->getDirection());
        NetworkLock lock(this);
        // Compute temporary values such that they do not get clamped when set
        auto nearPlane = nearPlane_ * depthRatio;
        auto farPlane = farPlane_ * depthRatio;
        farPlane_.setMaxValue(farPlane_.getMaxValue() * depthRatio);
        farPlane_.set(farPlane);
        nearPlane_.setMaxValue(nearPlane_.getMaxValue() * depthRatio);
        nearPlane_.set(nearPlane);

        lookFrom_.setMinValue((toNewSpace * vec4(lookFrom_.getMinValue(), 1.f)).xyz());
        lookFrom_.setMaxValue((toNewSpace * vec4(lookFrom_.getMaxValue(), 1.f)).xyz());
        lookTo_.setMinValue((toNewSpace * vec4(lookTo_.getMinValue(), 1.f)).xyz());
        lookTo_.setMaxValue((toNewSpace * vec4(lookTo_.getMaxValue(), 1.f)).xyz());

        setLookFrom(newLookFrom);
        setLookTo(newLookTo);

        prevDataToWorldMatrix_ = newDataToWorldMatrix;
    }
}

void CameraProperty::resetAdjustCameraToData() {
    data_ = nullptr;
    prevDataToWorldMatrix_ = mat4(0.0f);
    if (adjustCameraOnDataChange_) {
        inportChanged();
    }
}

void CameraProperty::inportChanged() {
    if (!adjustCameraOnDataChange_) return;

    VolumeInport* volumeInport = dynamic_cast<VolumeInport*>(inport_);
    MeshInport* meshInport = dynamic_cast<MeshInport*>(inport_);

    // using SpatialEntity since Geometry is not derived from data
    const SpatialEntity<3>* data = nullptr;

    if (volumeInport) {
        data = volumeInport->getData().get();
    } else if (meshInport) {
        data = meshInport->getData().get();
    }

    if (data_ == nullptr && prevDataToWorldMatrix_ == mat4(0.0f)) {  // first time only
        if (volumeInport && volumeInport->hasData()) {
            prevDataToWorldMatrix_ =
                volumeInport->getData()->getCoordinateTransformer().getDataToWorldMatrix();
        } else if (meshInport && meshInport->hasData()) {
            prevDataToWorldMatrix_ =
                meshInport->getData()->getCoordinateTransformer().getDataToWorldMatrix();
        }
    } else if (data && data_ != data) {
        adjustCameraToData(prevDataToWorldMatrix_,
                           data->getCoordinateTransformer().getDataToWorldMatrix());
    }

    data_ = data;
}

const vec3& CameraProperty::getLookFrom() const { return camera_->getLookFrom(); }

const vec3& CameraProperty::getLookTo() const { return camera_->getLookTo(); }

const vec3& CameraProperty::getLookUp() const { return camera_->getLookUp(); }

vec3 CameraProperty::getLookRight() const {
    return glm::cross(glm::normalize(camera_->getDirection()), camera_->getLookUp());
}

const mat4& CameraProperty::viewMatrix() const { return camera_->viewMatrix(); }

const mat4& CameraProperty::projectionMatrix() const { return camera_->projectionMatrix(); }

const mat4& CameraProperty::inverseViewMatrix() const { return camera_->inverseViewMatrix(); }

const mat4& CameraProperty::inverseProjectionMatrix() const {
    return camera_->inverseProjectionMatrix();
}

void CameraProperty::changeFocusPoint(Event* event) {
    if (auto mouseEvent = dynamic_cast<MouseEvent*>(event)) {
        vec2 p = mouseEvent->posNormalized();
        float d = mouseEvent->depth();
        if (std::abs(d - 1) < glm::epsilon<float>()) {
            return;
        }
        p.y = 1 - p.y;
        p = p * 2.0f - 1.0f;
        vec4 viewPos(p.x,p.y, d , 1.0f);

        auto point = (inverseViewMatrix() * inverseProjectionMatrix()) * viewPos;
        auto newLookTo = point.xyz() / point.w;

        auto newLookFrom = lookFrom_.get() + (newLookTo - lookTo_.get());

        setLookTo(newLookTo);
        setLookFrom(newLookFrom);
    }
    event->markAsUsed();
}

}  // namespace
