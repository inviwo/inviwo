/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/viewevent.h>
#include <inviwo/core/util/foreach.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>

#include <limits>

namespace inviwo {

const std::string CameraProperty::classIdentifier = "org.inviwo.CameraProperty";
std::string CameraProperty::getClassIdentifier() const { return classIdentifier; }

CameraProperty::CameraProperty(const std::string& identifier, const std::string& displayName,
                               std::function<std::optional<mat4>()> getBoundingBox, vec3 eye,
                               vec3 center, vec3 lookUp, InvalidationLevel invalidationLevel,
                               PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, invalidationLevel, semantics}
    , cameraType_("cameraType", "Camera Type",
                  InviwoApplication::getPtr()->getCameraFactory()->getKeys(), 0)
    , cameraActions_("actions", "Actions", buttons())
    , lookFrom_("lookFrom", "Look from", eye, -vec3(100.0f), vec3(100.0f), vec3(0.1f),
                InvalidationLevel::InvalidOutput, PropertySemantics{"SphericalSpinBox"})
    , lookTo_("lookTo", "Look to", center, -vec3(100.0f), vec3(100.0f), vec3(0.1f),
              InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , lookUp_("lookUp", "Look up", lookUp, -vec3(1.f), vec3(1.f), vec3(0.1f),
              InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , aspectRatio_("aspectRatio", "Aspect Ratio", 1.0f, 0.0f, std::numeric_limits<float>::max(),
                   0.01f, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , nearPlane_("near", "Near Plane", 0.1f, 0.001f, 10.f, 0.001f)
    , farPlane_("far", "Far Plane", 100.0f, 1.0f, 1000.0f, 1.0f)

    , settings_("settings", "Settings")
    , updateNearFar_("updateNearFar", "Update Near/Far Distances", true)
    , updateLookRanges_("updateLookRanges", "Update Look-to/-from Ranges", true)
    , fittingRatio_("fittingRatio", "Fitting Ratio", 1.05f, 0, 2, 0.01f)

    , setNearFarButton_("setNearFarButton", "Set Near/Far Distances", [this] { setNearFar(); })
    , setLookRangesButton_("setLookRangesButton", "Set Look-to/-from Ranges",
                           [this] { setLookRange(); })

    , camera_{}
    , getBoundingBox_{std::move(getBoundingBox)} {

    cameraType_.setSelectedIdentifier("PerspectiveCamera");
    cameraType_.setCurrentStateAsDefault();
    cameraType_.onChange([&]() {
        changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_));
    });

    // Make sure that the Camera is in sync with the property values.
    lookFrom_.onChange([&]() { camera_->setLookFrom(lookFrom_); });
    lookTo_.onChange([&]() { camera_->setLookTo(lookTo_); });
    lookUp_.onChange([&]() { camera_->setLookUp(lookUp_); });
    aspectRatio_.onChange([&]() { camera_->setAspectRatio(aspectRatio_); });
    nearPlane_.onChange([&]() { camera_->setNearPlaneDist(nearPlane_); });
    farPlane_.onChange([&]() { camera_->setFarPlaneDist(farPlane_); });

    aspectRatio_.setReadOnly(true);
    aspectRatio_.setCurrentStateAsDefault();

    addProperties(cameraType_, cameraActions_, lookFrom_, lookTo_, lookUp_, aspectRatio_,
                  nearPlane_, farPlane_, settings_);
    settings_.addProperties(setNearFarButton_, setLookRangesButton_, updateNearFar_,
                            updateLookRanges_, fittingRatio_);
    settings_.setCollapsed(true);

    auto cameraFitVisible = [this]() {
        util::for_each_argument(
            [&](auto& p) {
                p.setVisible(getBoundingBox_ && cameraType_ == "PerspectiveCamera");
                p.setCurrentStateAsDefault();
            },
            cameraActions_, settings_, setNearFarButton_, setLookRangesButton_, updateNearFar_,
            updateLookRanges_, fittingRatio_);
    };

    cameraType_.onChange(cameraFitVisible);
    cameraFitVisible();

    changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_));
}

CameraProperty::CameraProperty(const std::string& identifier, const std::string& displayName,
                               vec3 eye, vec3 center, vec3 lookUp, Inport* inport,
                               InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CameraProperty(identifier, displayName,
                     [&]() -> std::function<std::optional<mat4>()> {
                         if (auto vp = dynamic_cast<VolumeInport*>(inport)) {
                             return util::boundingBox(*vp);
                         } else if (auto mp = dynamic_cast<MeshInport*>(inport)) {
                             return util::boundingBox(*mp);
                         } else {
                             return nullptr;
                         }
                     }(),
                     eye, center, lookUp, invalidationLevel, semantics) {}

CameraProperty::CameraProperty(const CameraProperty& rhs)
    : CompositeProperty(rhs)
    , cameraType_(rhs.cameraType_)
    , cameraActions_{rhs.cameraActions_, buttons()}
    , lookFrom_(rhs.lookFrom_)
    , lookTo_(rhs.lookTo_)
    , lookUp_(rhs.lookUp_)
    , aspectRatio_(rhs.aspectRatio_)
    , nearPlane_(rhs.nearPlane_)
    , farPlane_(rhs.farPlane_)

    , settings_{rhs.settings_}
    , updateNearFar_{rhs.updateNearFar_}
    , updateLookRanges_{rhs.updateLookRanges_}
    , fittingRatio_{rhs.fittingRatio_}
    , setNearFarButton_{rhs.setNearFarButton_, [this] { setNearFar(); }}
    , setLookRangesButton_{rhs.setLookRangesButton_, [this] { setLookRange(); }}

    , camera_()
    , getBoundingBox_(rhs.getBoundingBox_) {

    // Make sure that the Camera) is
    // in sync with the property values.
    cameraType_.onChange([&]() {
        changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_));
    });
    lookFrom_.onChange([&]() { camera_->setLookFrom(lookFrom_); });
    lookTo_.onChange([&]() { camera_->setLookTo(lookTo_); });
    lookUp_.onChange([&]() { camera_->setLookUp(lookUp_); });
    aspectRatio_.onChange([&]() { camera_->setAspectRatio(aspectRatio_); });
    nearPlane_.onChange([&]() { camera_->setNearPlaneDist(nearPlane_); });
    farPlane_.onChange([&]() { camera_->setFarPlaneDist(farPlane_); });

    {
        // Make sure we put these properties before any owned properties, added from the
        // CompositeProperty base class
        size_t i = 0;
        insertProperty(i++, cameraType_);
        insertProperty(i++, cameraActions_);
        insertProperty(i++, lookFrom_);
        insertProperty(i++, lookTo_);
        insertProperty(i++, lookUp_);
        insertProperty(i++, aspectRatio_);
        insertProperty(i++, nearPlane_);
        insertProperty(i++, farPlane_);
    }
    addProperty(settings_);  // We want settings to be last
    settings_.addProperties(setNearFarButton_, setLookRangesButton_, updateNearFar_,
                            updateLookRanges_, fittingRatio_);

    auto cameraFitVisible = [this]() {
        util::for_each_argument(
            [&](auto& p) {
                p.setVisible(getBoundingBox_ && cameraType_ == "PerspectiveCamera");
                p.setCurrentStateAsDefault();
            },
            cameraActions_, settings_, setNearFarButton_, setLookRangesButton_, updateNearFar_,
            updateLookRanges_, fittingRatio_);
    };

    cameraType_.onChange(cameraFitVisible);
    cameraFitVisible();

    changeCamera(InviwoApplication::getPtr()->getCameraFactory()->create(cameraType_.get()));
}

CameraProperty::~CameraProperty() = default;

void CameraProperty::changeCamera(std::unique_ptr<Camera> newCamera) {
    NetworkLock lock(this);
    if (camera_) camera_->configureProperties(this, Camera::Config::Hide);
    camera_ = std::move(newCamera);
    camera_->setLookFrom(lookFrom_);
    camera_->setLookTo(lookTo_);
    camera_->setLookUp(lookUp_);
    camera_->setAspectRatio(aspectRatio_);
    camera_->setNearPlaneDist(nearPlane_);
    camera_->setFarPlaneDist(farPlane_);
    camera_->configureProperties(this, Camera::Config::Show);
}

const Camera& CameraProperty::get() const { return *camera_; }
Camera& CameraProperty::get() { return *camera_; }

void CameraProperty::set(const Property* srcProperty) {
    if (const auto cameraSrcProp = dynamic_cast<const CameraProperty*>(srcProperty)) {
        if (!camera_->update(cameraSrcProp->camera_.get())) {
            // update failed, make a clone
            changeCamera(std::unique_ptr<Camera>(cameraSrcProp->camera_->clone()));
        }

        for (auto dest : getProperties()) {
            if (!aspectSupplier_ || dest->getIdentifier() != aspectRatio_.getIdentifier()) {
                if (auto src = cameraSrcProp->getPropertyByIdentifier(dest->getIdentifier())) {
                    dest->set(src);
                }
            }
        }

        if (aspectSupplier_) {
            camera_->setAspectRatio(aspectRatio_);
        }

        propertyModified();
    }
}

CameraProperty::operator const Camera&() const { return *camera_; }

CameraProperty* CameraProperty::clone() const { return new CameraProperty(*this); }

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

CameraProperty& CameraProperty::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    for (auto& elem : properties_) {
        elem->setCurrentStateAsDefault();
    }
    return *this;
}

CameraProperty& CameraProperty::resetToDefaultState() {
    NetworkLock lock(this);
    for (auto& elem : properties_) {
        if (elem != &aspectRatio_) {  // We never want to reset the aspect
            elem->resetToDefaultState();
        }
    }
    return *this;
}

void CameraProperty::setLookFrom(vec3 lookFrom) { lookFrom_.set(lookFrom); }

void CameraProperty::setLookTo(vec3 lookTo) { lookTo_.set(lookTo); }

void CameraProperty::setLookUp(vec3 lookUp) { lookUp_.set(lookUp); }

void CameraProperty::setAspectRatio(float aspectRatio) { aspectRatio_.set(aspectRatio); }
float CameraProperty::getAspectRatio() const { return camera_->getAspectRatio(); }

void CameraProperty::setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) {
    NetworkLock lock(this);
    setLookFrom(lookFrom);
    setLookTo(lookTo);
    setLookUp(lookUp);
}

float CameraProperty::getNearPlaneDist() const { return nearPlane_.get(); }

float CameraProperty::getFarPlaneDist() const { return farPlane_.get(); }

void CameraProperty::setNearPlaneDist(float v) { nearPlane_.set(v); }

void CameraProperty::setFarPlaneDist(float v) { farPlane_.set(v); }

void CameraProperty::setNearFarPlaneDist(float nearPlaneDist, float farPlaneDist,
                                         float minMaxRatio) {
    NetworkLock lock(this);

    nearPlane_.set(nearPlaneDist, std::min(nearPlane_.getMinValue(), nearPlaneDist / minMaxRatio),
                   std::max(nearPlane_.getMaxValue(), nearPlaneDist * minMaxRatio),
                   nearPlane_.getIncrement());

    farPlane_.set(farPlaneDist, std::min(farPlane_.getMinValue(), farPlaneDist / minMaxRatio),
                  std::max(farPlane_.getMaxValue(), farPlaneDist * minMaxRatio),
                  farPlane_.getIncrement());
}

vec3 CameraProperty::getLookFromMinValue() const { return lookFrom_.getMinValue(); }

vec3 CameraProperty::getLookFromMaxValue() const { return lookFrom_.getMaxValue(); }

vec3 CameraProperty::getLookToMinValue() const { return lookTo_.getMinValue(); }

vec3 CameraProperty::getLookToMaxValue() const { return lookTo_.getMaxValue(); }

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
    if (auto resizeEvent = event->getAs<ResizeEvent>()) {
        aspectSupplier_ = true;
        const auto canvasSize = resizeEvent->size();
        // Do not set aspect ratio if canvas size is 0 in any dimension.
        if (canvasSize.x > 0 && canvasSize.y > 0) {
            const double width{static_cast<double>(canvasSize[0])};
            const double height{static_cast<double>(canvasSize[1])};
            setAspectRatio(static_cast<float>(width / height));
        }
    } else if (auto ve = event->getAs<ViewEvent>(); ve && getBoundingBox_) {
        std::visit(util::overloaded{[&](camerautil::Side side) { setView(side); },
                                    [&](ViewEvent::FlipUp) { flipUp(); },
                                    [&](ViewEvent::FitData) { fitData(); }},
                   ve->getAction());
        ve->markAsUsed();
    } else {
        CompositeProperty::invokeEvent(event);
    }
}

const vec3& CameraProperty::getLookFrom() const { return camera_->getLookFrom(); }

const vec3& CameraProperty::getLookTo() const { return camera_->getLookTo(); }

const vec3& CameraProperty::getLookUp() const { return camera_->getLookUp(); }

vec3 CameraProperty::getLookRight() const {
    return glm::cross(glm::normalize(camera_->getDirection()), camera_->getLookUp());
}

const mat4& CameraProperty::viewMatrix() const { return camera_->getViewMatrix(); }

const mat4& CameraProperty::projectionMatrix() const { return camera_->getProjectionMatrix(); }

const mat4& CameraProperty::inverseViewMatrix() const { return camera_->getInverseViewMatrix(); }

const mat4& CameraProperty::inverseProjectionMatrix() const {
    return camera_->getInverseProjectionMatrix();
}

std::vector<ButtonGroupProperty::Button> CameraProperty::buttons() {
    return {
        {{std::nullopt, ":svgicons/view-fit-to-data.svg", "Fit data in view",
          [this] { fitData(); }},
         {std::nullopt, ":svgicons/view-x-m.svg", "View data from X-",
          [this] { setView(camerautil::Side::XNegative); }},
         {std::nullopt, ":svgicons/view-x-p.svg", "View data from X+",
          [this] { setView(camerautil::Side::XPositive); }},
         {std::nullopt, ":svgicons/view-y-m.svg", "View data from Y-",
          [this] { setView(camerautil::Side::YNegative); }},
         {std::nullopt, ":svgicons/view-y-p.svg", "View data from Y+",
          [this] { setView(camerautil::Side::YPositive); }},
         {std::nullopt, ":svgicons/view-z-m.svg", "View data from Z-",
          [this] { setView(camerautil::Side::ZNegative); }},
         {std::nullopt, ":svgicons/view-z-p.svg", "View data from Z+",
          [this] { setView(camerautil::Side::ZPositive); }},
         {std::nullopt, ":svgicons/view-flip.svg", "Flip the up vector", [this] { flipUp(); }}}};
}

void CameraProperty::setView(camerautil::Side side) {
    if (getBoundingBox_) {
        if (auto bb = getBoundingBox_()) {
            using namespace camerautil;
            setCameraView(*this, *bb, side, fittingRatio_,
                          updateNearFar_ ? UpdateNearFar::Yes : UpdateNearFar::No,
                          updateLookRanges_ ? UpdateLookRanges::Yes : UpdateLookRanges::No);
        }
    }
}

void CameraProperty::fitData() {
    if (getBoundingBox_) {
        if (auto bb = getBoundingBox_()) {
            using namespace camerautil;
            setCameraView(*this, *bb, fittingRatio_,
                          updateNearFar_ ? UpdateNearFar::Yes : UpdateNearFar::No,
                          updateLookRanges_ ? UpdateLookRanges::Yes : UpdateLookRanges::No);
        }
    }
}

void CameraProperty::flipUp() { setLookUp(-getLookUp()); }

void CameraProperty::setNearFar() {
    if (getBoundingBox_) {
        if (auto bb = getBoundingBox_()) {
            camerautil::setCameraNearFar(*this, *bb);
        }
    }
}

void CameraProperty::setLookRange() {
    if (getBoundingBox_) {
        if (auto bb = getBoundingBox_()) {
            camerautil::setCameraLookRanges(*this, *bb);
        }
    }
}

}  // namespace inviwo
