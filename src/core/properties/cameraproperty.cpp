/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
#include <inviwo/core/ports/geometryport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/geometry/geometry.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/util/glmstreamoperators.h>

namespace inviwo {

PropertyClassIdentifier(CameraProperty, "org.inviwo.CameraProperty");

CameraProperty::CameraProperty(std::string identifier, std::string displayName, vec3 eye,
                               vec3 center, vec3 lookUp, Inport* inport,
                               InvalidationLevel invalidationLevel,
                               PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , EventListener()
    , lookFrom_("lookFrom", "Look from", eye, -vec3(10.0f), vec3(10.0f), vec3(0.1f),
                VALID, PropertySemantics("Spherical"))
    , lookTo_("lookTo", "Look to", center, -vec3(10.0f), vec3(10.0f), vec3(0.1f),
              VALID)
    , lookUp_("lookUp", "Look up", lookUp, -vec3(10.0f), vec3(10.0f), vec3(0.1f),
              VALID)
    , fovy_("fov", "FOV", 60.0f, 30.0f, 360.0f, 0.1f, VALID)
    , aspectRatio_("aspectRatio", "Aspect Ratio", 1.0f, 0.01f, 100.0f, 0.01f, VALID)
    , farPlane_("far", "Far Plane", 100.0f, 1.0f, 1000.0f, 1.0f, VALID)
    , nearPlane_("near", "Near Plane", 0.1f, 0.001f, 10.f, 0.001f, VALID)
    , fitToBasis_("fitToBasis_", "Fit to basis", true, VALID)
    , lockInvalidation_(false)
    , inport_(inport)
    , data_(NULL)
    , oldBasis_(0) {

    lookFrom_.onChange(this, &CameraProperty::updateViewMatrix);
    lookTo_.onChange(this, &CameraProperty::updateViewMatrix);
    lookUp_.onChange(this, &CameraProperty::updateViewMatrix);
    addProperty(lookFrom_);
    addProperty(lookTo_);
    addProperty(lookUp_);
    fovy_.onChange(this, &CameraProperty::updateProjectionMatrix);
    aspectRatio_.onChange(this, &CameraProperty::updateProjectionMatrix);
    nearPlane_.onChange(this, &CameraProperty::updateProjectionMatrix);
    farPlane_.onChange(this, &CameraProperty::updateProjectionMatrix);
    addProperty(fovy_);
    addProperty(aspectRatio_);
    addProperty(nearPlane_);
    addProperty(farPlane_);
    
    fitToBasis_.onChange(this, &CameraProperty::fitReset);
    addProperty(fitToBasis_);

    lockInvalidation();
    updateViewMatrix();
    updateProjectionMatrix();
    unlockInvalidation();

    if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);
}

CameraProperty::CameraProperty(const CameraProperty& rhs)
    : CompositeProperty(rhs)
    , EventListener(rhs)
    , lookFrom_(rhs.lookFrom_)
    , lookTo_(rhs.lookTo_)
    , lookUp_(rhs.lookUp_)
    , fovy_(rhs.fovy_)
    , aspectRatio_(rhs.aspectRatio_)
    , farPlane_(rhs.farPlane_)
    , nearPlane_(rhs.nearPlane_)
    , fitToBasis_(rhs.fitToBasis_)
    , lockInvalidation_(false)
    , inport_(rhs.inport_)
    , data_(NULL)
    , oldBasis_(0) {

    if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);

    inportChanged();
}

CameraProperty& CameraProperty::operator=(const CameraProperty& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        EventListener::operator=(that);
        lookFrom_ = that.lookFrom_;
        lookTo_ = that.lookTo_;
        lookUp_ = that.lookUp_;
        fovy_ = that.fovy_;
        aspectRatio_ = that.aspectRatio_;
        nearPlane_ = that.nearPlane_;
        farPlane_ = that.farPlane_;
        fitToBasis_ = that.fitToBasis_;
        
        if(inport_) inport_->removeOnChange(this);
        inport_ = that.inport_;
        if (inport_) inport_->onChange(this, &CameraProperty::inportChanged);
        data_ = NULL;
        oldBasis_ = mat3(0);

        inportChanged();
    }
    return *this;
}

CameraProperty* CameraProperty::clone() const {
    return new CameraProperty(*this);
}

CameraProperty::~CameraProperty() {}

void CameraProperty::resetCamera() {
    bool lock = isInvalidationLocked();
    if (!lock) lockInvalidation();

    lookFrom_.resetToDefaultState();
    lookTo_.resetToDefaultState();
    lookUp_.resetToDefaultState();
    fovy_.resetToDefaultState();

    if (!lock) unlockInvalidation();

    invalidate();
}

void CameraProperty::setCamera(const CameraProperty* cam){
    setLook(cam->getLookFrom(), cam->getLookTo(), cam->getLookUp());
    setFovy(cam->getFovy());
}

void CameraProperty::setLookFrom(vec3 lookFrom) { lookFrom_.set(lookFrom); }

void CameraProperty::setLookTo(vec3 lookTo) { lookTo_.set(lookTo); }

void CameraProperty::setLookUp(vec3 lookUp) { lookUp_.set(lookUp); }

void CameraProperty::setFovy(float fovy) { fovy_.set(fovy); }

void CameraProperty::setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) {
    bool lock = isInvalidationLocked();

    if (!lock) lockInvalidation();

    lookFrom_.set(lookFrom);
    lookTo_.set(lookTo);
    lookUp_.set(lookUp);

    if (!lock) unlockInvalidation();

    invalidate();
}

float CameraProperty::getNearPlaneDist() const { return nearPlane_.get(); }

float CameraProperty::getFarPlaneDist() const { return farPlane_.get(); }

// XYZ between -1 -> 1
vec3 CameraProperty::getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    float clipW = projectionMatrix_[2][3] /
                  (ndcCoords.z - (projectionMatrix_[2][2] / projectionMatrix_[3][2]));
    vec4 clipCoords = vec4(ndcCoords * clipW, clipW);
    vec4 eyeCoords = inverseProjectionMatrix() * clipCoords;
    vec4 worldCoords = inverseViewMatrix() * eyeCoords;
    worldCoords /= worldCoords.w;
    return worldCoords.xyz();
}

void CameraProperty::setProjectionMatrix(float fovy, float aspect, float nearPlane,
                                         float farPlane) {
    fovy_.set(fovy);
    aspectRatio_.set(aspect);
    farPlane_.set(farPlane);
    nearPlane_.set(nearPlane);
    updateProjectionMatrix();
}

void CameraProperty::updateProjectionMatrix() {
    projectionMatrix_ = glm::perspective(glm::radians(fovy_.get()), aspectRatio_.get(),
                                         nearPlane_.get(), farPlane_.get());
    inverseProjectionMatrix_ = glm::inverse(projectionMatrix_);
    invalidate();
}

void CameraProperty::updateViewMatrix() {
    lookRight_ = glm::normalize(glm::cross(lookTo_.get() - lookFrom_.get(), lookUp_.get()));
    viewMatrix_ = glm::lookAt(lookFrom_.get(), lookTo_.get(), lookUp_.get());
    inverseViewMatrix_ = glm::inverse(viewMatrix_);
    invalidate();
}

void CameraProperty::invalidate() {
    if (!isInvalidationLocked()) Property::propertyModified();
}

void CameraProperty::invalidate(InvalidationLevel invalidationLevel,
                                Property* modifiedProperty) {
    CompositeProperty::invalidate(invalidationLevel, modifiedProperty);
}

void CameraProperty::invokeEvent(Event* event) {
    ResizeEvent* resizeEvent = dynamic_cast<ResizeEvent*>(event);

    if (resizeEvent) {
        uvec2 canvasSize = resizeEvent->size();
        float width = (float)canvasSize[0];
        float height = (float)canvasSize[1];
        setProjectionMatrix(fovy_.get(), width / height, nearPlane_.get(), farPlane_.get());
    }
}

void CameraProperty::serialize(IvwSerializer& s) const {
    CompositeProperty::serialize(s);
}

void CameraProperty::deserialize(IvwDeserializer& d) {
    CompositeProperty::deserialize(d);
    lockInvalidation();
    updateViewMatrix();
    updateProjectionMatrix();
    unlockInvalidation();
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
    farPlane_.set(newFarPlane);
    vec3 oldOffset = lookFrom_.get() - lookTo_.get();
    vec3 newPos = lookTo_.get() + (oldOffset * ratio);
    lookFrom_.setMinValue(lookFrom_.getMinValue() * ratio);
    lookFrom_.setMaxValue(lookFrom_.getMaxValue() * ratio);
    lookTo_.setMinValue(lookTo_.getMinValue() * ratio);
    lookTo_.setMaxValue(lookTo_.getMaxValue() * ratio);
    lookFrom_.set(newPos);
    updateViewMatrix();
    updateProjectionMatrix();
    unlockInvalidation();
    oldBasis_ = basis;
}

void CameraProperty::fitReset() {
    data_ = NULL;
    oldBasis_ = mat3(0.0f);
    if (fitToBasis_) {
        inportChanged();
    }
}

void CameraProperty::inportChanged() {
    if (!fitToBasis_) return;

    VolumeInport* volumeInport = dynamic_cast<VolumeInport*>(inport_);
    GeometryInport* geometryInport = dynamic_cast<GeometryInport*>(inport_);
    const SpatialEntity<3>* data = NULL;  // using SpatialEntity since Geometry is not derived from
                                          // data

    if (volumeInport) {
        data = volumeInport->getData();
    } else if (geometryInport) {
        data = geometryInport->getData();
    }

    if (data_ == NULL && oldBasis_ == mat3(0.0f)) {  // first time only
        if (volumeInport && volumeInport->hasData()) {
            oldBasis_ = volumeInport->getData()->getBasis();
        } else if (geometryInport && geometryInport->hasData()) {
            oldBasis_ = geometryInport->getData()->getBasis();
        }
    } else if (data && data_ != data ) {
        fitWithBasis(data->getBasis());
    }

    data_ = data;
}

vec3& CameraProperty::getLookFrom() {
    return lookFrom_.get();
}

const vec3& CameraProperty::getLookFrom() const {
    return lookFrom_.get();
}

vec3& CameraProperty::getLookTo() {
    return lookTo_.get();
}

const vec3& CameraProperty::getLookTo() const {
    return lookTo_.get();
}

vec3& CameraProperty::getLookUp() {
    return lookUp_.get();
}

const vec3& CameraProperty::getLookUp() const {
    return lookUp_.get();
}

vec3 CameraProperty::getLookRight() const {
    return lookRight_;
}

float CameraProperty::getFovy() const {
    return fovy_.get();
}

float CameraProperty::getAspectRatio() const {
    return aspectRatio_.get();
}

const mat4& CameraProperty::viewMatrix() const {
    return viewMatrix_;
}

const mat4& CameraProperty::projectionMatrix() const {
    return projectionMatrix_;
}

const mat4& CameraProperty::inverseViewMatrix() const {
    return inverseViewMatrix_;
}

const mat4& CameraProperty::inverseProjectionMatrix() const {
    return inverseProjectionMatrix_;
}

void CameraProperty::lockInvalidation() {
    lockInvalidation_ = true;
}

void CameraProperty::unlockInvalidation() {
    lockInvalidation_ = false;
}

bool CameraProperty::isInvalidationLocked() {
    return lockInvalidation_;
}

}  // namespace
