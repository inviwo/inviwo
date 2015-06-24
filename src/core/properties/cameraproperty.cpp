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
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/events/resizeevent.h>

namespace inviwo {

PropertyClassIdentifier(CameraProperty, "org.inviwo.CameraProperty");

CameraProperty::CameraProperty(std::string identifier, std::string displayName, vec3 eye,
                               vec3 center, vec3 lookUp, Inport* inport,
                               InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , value_(eye, center, lookUp, 0.1f, 100.f)
    , lookFrom_("lookFrom", "Look from", eye, -vec3(10.0f), vec3(10.0f), vec3(0.1f), VALID,
                PropertySemantics("Spherical"))
    , lookTo_("lookTo", "Look to", center, -vec3(10.0f), vec3(10.0f), vec3(0.1f), VALID)
    , lookUp_("lookUp", "Look up", lookUp, -vec3(10.0f), vec3(10.0f), vec3(0.1f), VALID)
    , fovy_("fov", "FOV", 60.0f, 30.0f, 360.0f, 0.1f, VALID)
    , aspectRatio_("aspectRatio", "Aspect Ratio", 1.0f, 0.01f, 100.0f, 0.01f, VALID)
    , nearPlane_("near", "Near Plane", 0.1f, 0.001f, 10.f, 0.001f, VALID)
    , farPlane_("far", "Far Plane", 100.0f, 1.0f, 1000.0f, 1.0f, VALID)
    , fitToBasis_("fitToBasis_", "Fit to basis", true, VALID)
    , lockInvalidation_(false)
    , inport_(inport)
    , data_(nullptr)
    , oldBasis_(0) {
    lookFrom_.onChange(this, &CameraProperty::lookFromChangedFromProperty);
    lookTo_.onChange(this, &CameraProperty::lookToChangedFromProperty);
    lookUp_.onChange(this, &CameraProperty::lookUpChangedFromProperty);
    addProperty(lookFrom_);
    addProperty(lookTo_);
    addProperty(lookUp_);
    fovy_.onChange(this, &CameraProperty::verticalFieldOfViewChangedFromProperty);
    aspectRatio_.onChange(this, &CameraProperty::aspectRatioChangedFromProperty);
    nearPlane_.onChange(this, &CameraProperty::nearPlaneChangedFromProperty);
    farPlane_.onChange(this, &CameraProperty::farPlaneChangedFromProperty);
    addProperty(fovy_);
    addProperty(aspectRatio_);
    addProperty(nearPlane_);
    addProperty(farPlane_);

    fitToBasis_.onChange(this, &CameraProperty::fitReset);
    addProperty(fitToBasis_);

    if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);
}

CameraProperty::CameraProperty(const CameraProperty& rhs)
    : CompositeProperty(rhs)
    , value_(rhs.value_)
    , lookFrom_(rhs.lookFrom_)
    , lookTo_(rhs.lookTo_)
    , lookUp_(rhs.lookUp_)
    , fovy_(rhs.fovy_)
    , aspectRatio_(rhs.aspectRatio_)
    , nearPlane_(rhs.nearPlane_)
    , farPlane_(rhs.farPlane_)
    , fitToBasis_(rhs.fitToBasis_)
    , lockInvalidation_(false)
    , inport_(rhs.inport_)
    , data_(nullptr)
    , oldBasis_(0) {
    lookFrom_.onChange(this, &CameraProperty::lookFromChangedFromProperty);
    lookTo_.onChange(this, &CameraProperty::lookToChangedFromProperty);
    lookUp_.onChange(this, &CameraProperty::lookUpChangedFromProperty);
    addProperty(lookFrom_);
    addProperty(lookTo_);
    addProperty(lookUp_);
    fovy_.onChange(this, &CameraProperty::verticalFieldOfViewChangedFromProperty);
    aspectRatio_.onChange(this, &CameraProperty::aspectRatioChangedFromProperty);
    nearPlane_.onChange(this, &CameraProperty::nearPlaneChangedFromProperty);
    farPlane_.onChange(this, &CameraProperty::farPlaneChangedFromProperty);
    addProperty(fovy_);
    addProperty(aspectRatio_);
    addProperty(nearPlane_);
    addProperty(farPlane_);

    fitToBasis_.onChange(this, &CameraProperty::fitReset);
    addProperty(fitToBasis_);

    if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);

    inportChanged();
}

CameraProperty& CameraProperty::operator=(const PerspectiveCamera& value) {
    if (value_ != value) {
        value_ = value;
        updatePropertyFromValue();
    }
    return *this;
}

CameraProperty& CameraProperty::operator=(const CameraProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        value_ = that.value_;

        if (inport_) inport_->removeOnChange(this);
        inport_ = that.inport_;
        if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);
        data_ = nullptr;
        oldBasis_ = mat3(0);
        updatePropertyFromValue();

        inportChanged();
    }
    return *this;
}

const PerspectiveCamera& CameraProperty::get() const {
    return value_;
}

PerspectiveCamera& CameraProperty::get() {
    return value_;
}

void CameraProperty::set(const PerspectiveCamera& value) { *this = value; }

void CameraProperty::set(const Property* srcProperty) {
    if (const auto cameraSrcProp = dynamic_cast<const CameraProperty*>(srcProperty)) {
        NetworkLock lock;
        value_ = cameraSrcProp->value_;
        CompositeProperty::set(static_cast<const CompositeProperty*>(srcProperty));
    }
}

CameraProperty::operator const PerspectiveCamera&() const {
    return value_;
}

CameraProperty* CameraProperty::clone() const { return new CameraProperty(*this); }

void CameraProperty::updatePropertyFromValue() {
    bool lock = isInvalidationLocked();
    if (!lock) lockInvalidation();

    lookFrom_ = value_.getLookFrom();
    lookTo_ = value_.getLookTo();
    lookUp_ = value_.getLookUp();
    fovy_ = value_.getFovy();
    aspectRatio_ = value_.getAspectRatio();
    nearPlane_ = value_.getNearPlaneDist();
    farPlane_ = value_.getFarPlaneDist();

    if (!lock) unlockInvalidation();
    propertyModified();
}

void CameraProperty::resetToDefaultState() {
    // Override CompositeProperty function to avoid
    // invalidation before value_ (perspective camera) has been set.
    for (auto& elem : properties_) {
        elem->resetToDefaultState();
    }
    value_ = PerspectiveCamera(lookFrom_.get(), lookTo_.get(), lookUp_.get(), nearPlane_.get(),
                               farPlane_.get(), fovy_.get(), aspectRatio_.get());
    Property::resetToDefaultState();
}

void CameraProperty::resetCamera() {
    bool lock = isInvalidationLocked();
    if (!lock) lockInvalidation();

    lookFrom_.resetToDefaultState();
    lookTo_.resetToDefaultState();
    lookUp_.resetToDefaultState();
    fovy_.resetToDefaultState();

    // Update template value
    get().setLookFrom(lookFrom_.get());
    get().setLookTo(lookTo_.get());
    get().setLookUp(lookUp_.get());
    get().setFovy(fovy_.get());

    if (!lock) unlockInvalidation();

    invalidateCamera();
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

void CameraProperty::setFovy(float fovy) {
    fovy_.set(glm::clamp(fovy, fovy_.getMinValue(), fovy_.getMaxValue()));
}

void CameraProperty::setAspectRatio(float aspectRatio) {
    aspectRatio_.set(
        glm::clamp(aspectRatio, aspectRatio_.getMinValue(), aspectRatio_.getMaxValue()));
}

void CameraProperty::setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) {
    bool lock = isInvalidationLocked();

    if (!lock) lockInvalidation();

    setLookFrom(lookFrom);
    setLookTo(lookTo);
    setLookUp(lookUp);

    if (!lock) unlockInvalidation();

    invalidateCamera();
}

float CameraProperty::getNearPlaneDist() const { return nearPlane_.get(); }

float CameraProperty::getFarPlaneDist() const { return farPlane_.get(); }

void CameraProperty::setNearPlaneDist(float v) {
    nearPlane_.set(glm::clamp(v, nearPlane_.getMinValue(), nearPlane_.getMaxValue()));
}

void CameraProperty::setFarPlaneDist(float v) {
    farPlane_.set(glm::clamp(v, farPlane_.getMinValue(), farPlane_.getMaxValue()));
}

inviwo::vec3 CameraProperty::getLookFromMinValue() const {
    return lookFrom_.getMinValue();
}

inviwo::vec3 CameraProperty::getLookFromMaxValue() const {
    return lookFrom_.getMaxValue();
}

inviwo::vec3 CameraProperty::getLookToMinValue() const {
    return lookTo_.getMinValue();
}

inviwo::vec3 CameraProperty::getLookToMaxValue() const {
    return lookTo_.getMaxValue();
}

// XYZ between -1 -> 1
vec3 CameraProperty::getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    return get().getWorldPosFromNormalizedDeviceCoords(ndcCoords);
}

vec4 CameraProperty::getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    return get().getClipPosFromNormalizedDeviceCoords(ndcCoords);
}

void CameraProperty::setProjectionMatrix(float fovy, float aspect, float nearPlane,
                                         float farPlane) {
    bool lock = isInvalidationLocked();

    if (!lock) lockInvalidation();

    setFovy(fovy);
    setAspectRatio(aspect);
    setFarPlaneDist(farPlane);
    setNearPlaneDist(nearPlane);

    if (!lock) unlockInvalidation();

    invalidateCamera();
}

void CameraProperty::invalidateCamera() {
    if (!isInvalidationLocked()) {
        CompositeProperty::invalidate(INVALID_OUTPUT, this);
    }
}

void CameraProperty::invokeEvent(Event* event) {
    ResizeEvent* resizeEvent = dynamic_cast<ResizeEvent*>(event);

    if (resizeEvent) {
        uvec2 canvasSize = resizeEvent->size();
        float width = (float)canvasSize[0];
        float height = (float)canvasSize[1];
        setAspectRatio(width / height);
    }
}

void CameraProperty::setInport(Inport* inport) {
    if (inport_ != inport) inport->onChange(this, &CameraProperty::inportChanged);

    inport_ = inport;
}

void CameraProperty::fitWithBasis(const mat3& basis) {
    float newSize = glm::length(basis * vec3(1, 1, 1));
    float oldSize = glm::length(oldBasis_ * vec3(1, 1, 1));
    float ratio = newSize / oldSize;

    if (ratio == 1) return;

    lockInvalidation();
    float newFarPlane = farPlane_.get() * ratio;
    farPlane_.setMaxValue(farPlane_.getMaxValue() * ratio);
    setFarPlaneDist(newFarPlane);
    vec3 oldOffset = lookFrom_.get() - lookTo_.get();
    vec3 newPos = lookTo_.get() + (oldOffset * ratio);
    lookFrom_.setMinValue(lookFrom_.getMinValue() * ratio);
    lookFrom_.setMaxValue(lookFrom_.getMaxValue() * ratio);
    lookTo_.setMinValue(lookTo_.getMinValue() * ratio);
    lookTo_.setMaxValue(lookTo_.getMaxValue() * ratio);
    setLookFrom(newPos);

    unlockInvalidation();
    oldBasis_ = basis;
}

void CameraProperty::fitReset() {
    data_ = nullptr;
    oldBasis_ = mat3(0.0f);
    if (fitToBasis_) {
        inportChanged();
    }
}

void CameraProperty::inportChanged() {
    if (!fitToBasis_) return;

    VolumeInport* volumeInport = dynamic_cast<VolumeInport*>(inport_);
    MeshInport* meshInport = dynamic_cast<MeshInport*>(inport_);

    // using SpatialEntity since Geometry is not derived from data
    const SpatialEntity<3>* data = nullptr;

    if (volumeInport) {
        data = volumeInport->getData();
    } else if (meshInport) {
        data = meshInport->getData();
    }

    if (data_ == nullptr && oldBasis_ == mat3(0.0f)) {  // first time only
        if (volumeInport && volumeInport->hasData()) {
            oldBasis_ = volumeInport->getData()->getBasis();
        } else if (meshInport && meshInport->hasData()) {
            oldBasis_ = meshInport->getData()->getBasis();
        }
    } else if (data && data_ != data) {
        fitWithBasis(data->getBasis());
    }

    data_ = data;
}

void CameraProperty::lookFromChangedFromProperty() {
    value_.setLookFrom(lookFrom_.get());
    invalidateCamera();
}

void CameraProperty::lookToChangedFromProperty() {
    value_.setLookTo(lookTo_.get());
    invalidateCamera();
}

void CameraProperty::lookUpChangedFromProperty() {
    value_.setLookUp(lookUp_.get());
    invalidateCamera();
}

void CameraProperty::verticalFieldOfViewChangedFromProperty() {
    value_.setFovy(fovy_.get());
    invalidateCamera();
}

void CameraProperty::aspectRatioChangedFromProperty() {
    value_.setAspectRatio(aspectRatio_.get());
    invalidateCamera();
}

void CameraProperty::nearPlaneChangedFromProperty() {
    value_.setNearPlaneDist(nearPlane_.get());
    invalidateCamera();
}

void CameraProperty::farPlaneChangedFromProperty() {
    value_.setFarPlaneDist(farPlane_.get());
    invalidateCamera();
}

const vec3& CameraProperty::getLookFrom() const { return value_.getLookFrom(); }

const vec3& CameraProperty::getLookTo() const { return value_.getLookTo(); }

const vec3& CameraProperty::getLookUp() const { return value_.getLookUp(); }

vec3 CameraProperty::getLookRight() const {
    return glm::cross(glm::normalize(get().getDirection()), get().getLookUp());
}

float CameraProperty::getFovy() const { return value_.getFovy(); }

float CameraProperty::getAspectRatio() const { return value_.getAspectRatio(); }

const mat4& CameraProperty::viewMatrix() const { return value_.viewMatrix(); }

const mat4& CameraProperty::projectionMatrix() const { return value_.projectionMatrix(); }

const mat4& CameraProperty::inverseViewMatrix() const { return value_.inverseViewMatrix(); }

const mat4& CameraProperty::inverseProjectionMatrix() const {
    return value_.inverseProjectionMatrix();
}

void CameraProperty::lockInvalidation() { lockInvalidation_ = true; }

void CameraProperty::unlockInvalidation() { lockInvalidation_ = false; }

bool CameraProperty::isInvalidationLocked() { return lockInvalidation_; }

}  // namespace
