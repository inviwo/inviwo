/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/properties/cameraproperty.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/camera/camerafactory.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/processors/processor.h>
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
    , factory_{InviwoApplication::getPtr()->getCameraFactory()}
    , cameraType_("cameraType", "Camera Type", factory_->getKeys(),
                  [&]() {
                      auto keys = factory_->getKeys();
                      auto it = std::find(keys.begin(), keys.end(), "PerspectiveCamera");
                      return it != keys.end() ? std::distance(keys.begin(), it) : 0;
                  }())
    , camera_{factory_->create(cameraType_)}
    , defaultCamera_{}
    , cameraActions_("actions", "Actions", buttons(), InvalidationLevel::Valid)
    , lookFrom_(
          "lookFrom", "Look from", []() { return vec3{}; }, [](const vec3&) {},
          {-vec3(100.0f), ConstraintBehavior::Ignore}, {vec3(100.0f), ConstraintBehavior::Ignore},
          vec3(0.1f), InvalidationLevel::InvalidOutput, PropertySemantics{"SphericalSpinBox"})
    , lookTo_(
          "lookTo", "Look to", []() { return vec3{}; }, [](const vec3&) {},
          {-vec3(100.0f), ConstraintBehavior::Ignore}, {vec3(100.0f), ConstraintBehavior::Ignore},
          vec3(0.1f), InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , lookUp_(
          "lookUp", "Look up", []() { return vec3(1.0f, 1.0f, 1.0f); }, [](const vec3&) {},
          {-vec3(1.0f), ConstraintBehavior::Immutable}, {vec3(1.0f), ConstraintBehavior::Immutable},
          vec3(0.1f), InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , aspectRatio_(
          "aspectRatio", "Aspect Ratio", []() { return 1.0f; }, [](const float&) {},
          {0.0f, ConstraintBehavior::Immutable},
          {std::numeric_limits<float>::max(), ConstraintBehavior::Immutable}, 0.01f,
          InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , nearPlane_(
          "near", "Near Plane", []() { return 0.001f; }, [](const float&) {},
          {0.001f, ConstraintBehavior::Ignore}, {10.0f, ConstraintBehavior::Ignore}, 0.001f)
    , farPlane_(
          "far", "Far Plane", []() { return 1.0f; }, [](const float&) {},
          {1.0f, ConstraintBehavior::Ignore}, {1000.0f, ConstraintBehavior::Ignore}, 1.0f)

    , settings_("settings", "Settings")
    , updateNearFar_("updateNearFar", "Update Near/Far Distances", true)
    , updateLookRanges_("updateLookRanges", "Update Look-to/-from Ranges", true)
    , fittingRatio_("fittingRatio", "Fitting Ratio", 1.05f, 0, 2, 0.01f)

    , setNearFarButton_("setNearFarButton", "Set Near/Far Distances", [this] { setNearFar(); })
    , setLookRangesButton_("setLookRangesButton", "Set Look-to/-from Ranges",
                           [this] { setLookRange(); })

    , getBoundingBox_{std::move(getBoundingBox)} {

    aspectRatio_.setReadOnly(true).setCurrentStateAsDefault();

    setNearFarButton_.setSerializationMode(PropertySerializationMode::None);
    setLookRangesButton_.setSerializationMode(PropertySerializationMode::None);

    settings_.setCollapsed(true).addProperties(setNearFarButton_, setLookRangesButton_,
                                               updateNearFar_, updateLookRanges_, fittingRatio_);
    settings_.setCurrentStateAsDefault();

    addProperties(cameraType_, cameraActions_, lookFrom_, lookTo_, lookUp_, aspectRatio_,
                  nearPlane_, farPlane_, settings_);
    util::for_each_argument([this](auto& arg) { cameraProperties_.push_back(&arg); }, lookFrom_,
                            lookTo_, lookUp_, aspectRatio_, nearPlane_, farPlane_);

    cameraActions_.setSerializationMode(PropertySerializationMode::None);

    camera_->configureProperties(*this, true);
    cameraType_.onChange([this]() {
        changeCamera(cameraType_);
        updateFittingVisibility();
    });

    updateFittingVisibility();

    setLook(eye, center, lookUp);
    defaultCamera_.reset(camera_->clone());
}

CameraProperty::CameraProperty(const std::string& identifier, const std::string& displayName,
                               vec3 eye, vec3 center, vec3 lookUp, Inport* inport,
                               InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CameraProperty(
          identifier, displayName,
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
    , factory_{rhs.factory_}
    , cameraType_(rhs.cameraType_)
    , camera_{factory_->create(cameraType_)}
    , defaultCamera_{}
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

    , getBoundingBox_(rhs.getBoundingBox_) {

    settings_.addProperties(setNearFarButton_, setLookRangesButton_, updateNearFar_,
                            updateLookRanges_, fittingRatio_);
    addProperties(cameraType_, cameraActions_, lookFrom_, lookTo_, lookUp_, aspectRatio_,
                  nearPlane_, farPlane_, settings_);
    util::for_each_argument([this](auto& arg) { cameraProperties_.push_back(&arg); }, lookFrom_,
                            lookTo_, lookUp_, aspectRatio_, nearPlane_, farPlane_);

    camera_->configureProperties(*this, true);
    cameraType_.onChange([this]() {
        changeCamera(cameraType_);
        updateFittingVisibility();
    });
    updateFittingVisibility();

    defaultCamera_.reset(camera_->clone());
}

CameraProperty::~CameraProperty() = default;

const Camera& CameraProperty::get() const { return *camera_; }
Camera& CameraProperty::get() { return *camera_; }

void CameraProperty::set(const Property* srcProperty) {
    if (const auto src = dynamic_cast<const CameraProperty*>(srcProperty)) {
        NetworkLock lock(this);
        const auto aspect = getAspectRatio();

        cameraType_.set(&src->cameraType_);
        camera_->updateFrom(*src->camera_);

        if (aspectSupplier_) {  // restore the aspect if we are a supplier.
            setAspectRatio(aspect);
        }

        // no need to call propertyModified here since updateFrom will call propertyModified for any
        // modified sub property
    }
}

CameraProperty::operator const Camera&() const { return *camera_; }

CameraProperty* CameraProperty::clone() const { return new CameraProperty(*this); }

bool CameraProperty::changeCamera(const std::string& name) {
    if (name != camera_->getClassIdentifier()) {
        NetworkLock lock(this);
        auto newCamera = factory_->create(name);
        newCamera->updateFrom(*camera_);
        hideConfiguredProperties();
        newCamera->configureProperties(*this, true);
        camera_ = std::move(newCamera);

        return true;
    } else {
        return false;
    }
}

void CameraProperty::hideConfiguredProperties() {
    for (auto& p : ownedCameraProperties_) p->setVisible(false);
}

CameraProperty& CameraProperty::setCamera(const std::string& cameraIdentifier) {
    cameraType_.setSelectedIdentifier(cameraIdentifier);
    return *this;
}

CameraProperty& CameraProperty::setCamera(std::unique_ptr<Camera> newCamera) {
    if (newCamera) {
        NetworkLock lock(this);
        hideConfiguredProperties();
        newCamera->configureProperties(*this, true);
        camera_ = std::move(newCamera);

        cameraType_.setSelectedIdentifier(camera_->getClassIdentifier());
    }
    return *this;
}

CameraProperty& CameraProperty::setLookFrom(vec3 lookFrom) {
    lookFrom_.set(lookFrom);
    return *this;
}

CameraProperty& CameraProperty::setLookTo(vec3 lookTo) {
    lookTo_.set(lookTo);
    return *this;
}

CameraProperty& CameraProperty::setLookUp(vec3 lookUp) {
    lookUp_.set(lookUp);
    return *this;
}

CameraProperty& CameraProperty::setAspectRatio(float aspectRatio) {
    aspectRatio_.set(aspectRatio);
    return *this;
}
float CameraProperty::getAspectRatio() const { return camera_->getAspectRatio(); }

CameraProperty& CameraProperty::setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) {
    NetworkLock lock(this);
    setLookFrom(lookFrom);
    setLookTo(lookTo);
    setLookUp(lookUp);
    return *this;
}

float CameraProperty::getNearPlaneDist() const { return nearPlane_.get(); }

float CameraProperty::getFarPlaneDist() const { return farPlane_.get(); }

CameraProperty& CameraProperty::setNearPlaneDist(float v) {
    nearPlane_.set(v);
    return *this;
}

CameraProperty& CameraProperty::setFarPlaneDist(float v) {
    farPlane_.set(v);
    return *this;
}

CameraProperty& CameraProperty::setNearFarPlaneDist(float nearPlaneDist, float farPlaneDist,
                                                    float minMaxRatio) {
    NetworkLock lock(this);

    nearPlane_.set(nearPlaneDist, std::min(nearPlane_.getMinValue(), nearPlaneDist / minMaxRatio),
                   std::max(nearPlane_.getMaxValue(), nearPlaneDist * minMaxRatio),
                   nearPlane_.getIncrement());

    farPlane_.set(farPlaneDist, std::min(farPlane_.getMinValue(), farPlaneDist / minMaxRatio),
                  std::max(farPlane_.getMaxValue(), farPlaneDist * minMaxRatio),
                  farPlane_.getIncrement());
    return *this;
}

vec3 CameraProperty::getLookFromMinValue() const { return lookFrom_.getMinValue(); }

vec3 CameraProperty::getLookFromMaxValue() const { return lookFrom_.getMaxValue(); }

vec3 CameraProperty::getLookToMinValue() const { return lookTo_.getMinValue(); }

vec3 CameraProperty::getLookToMaxValue() const { return lookTo_.getMaxValue(); }

void CameraProperty::zoom(float factor, Bounded bounded) {
    camera_->zoom(factor,
                  bounded == Bounded::Yes && getBoundingBox_ ? getBoundingBox_() : std::nullopt);
}

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

Property* CameraProperty::getCameraProperty(const std::string& identifier) const {
    const auto it =
        std::find_if(cameraProperties_.begin(), cameraProperties_.end(),
                     [&](auto property) { return property->getIdentifier() == identifier; });
    if (it != cameraProperties_.end()) {
        return *it;
    } else {
        return nullptr;
    }
}

void CameraProperty::addCamerapProperty(std::unique_ptr<Property> camprop) {
    cameraProperties_.push_back(camprop.get());
    insertProperty(size() - 1, camprop.get(), false);
    ownedCameraProperties_.emplace_back(std::move(camprop));
}

CameraProperty& CameraProperty::setCurrentStateAsDefault() {
    defaultCamera_.reset(camera_->clone());
    Property::setCurrentStateAsDefault();
    for (auto& elem : properties_) {
        elem->setCurrentStateAsDefault();
    }
    return *this;
}

CameraProperty& CameraProperty::resetToDefaultState() {
    NetworkLock lock(this);
    setCamera(std::unique_ptr<Camera>(defaultCamera_->clone()));
    for (auto& elem : properties_) {
        if (elem != &aspectRatio_) {  // We never want to reset the aspect
            elem->resetToDefaultState();
        }
    }
    return *this;
}

bool CameraProperty::isDefaultState() const {
    // We always consider the camera to be changed. It will almost always be true and handling the
    // deserialization will be expensive if we have to take the default camera into account
    return false;
}

bool CameraProperty::needsSerialization() const {
    return serializationMode_ != PropertySerializationMode::None;
}

void CameraProperty::serialize(Serializer& s) const {
    CompositeProperty::serialize(s);
    if (serializationMode_ != PropertySerializationMode::None) {
        s.serialize("Camera", camera_);
    }
}

void CameraProperty::deserialize(Deserializer& d) {
    if (serializationMode_ != PropertySerializationMode::None) {
        camera_->configureProperties(*this, false);
        d.deserialize("Camera", camera_);
        hideConfiguredProperties();
        camera_->configureProperties(*this, true);
    }
    CompositeProperty::deserialize(d);
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

void CameraProperty::updateFittingVisibility() {
    util::for_each_argument(
        [&](auto& p) {
            p.setVisible(getBoundingBox_ && cameraType_ == "PerspectiveCamera");
            p.setCurrentStateAsDefault();
        },
        cameraActions_, settings_, setNearFarButton_, setLookRangesButton_, updateNearFar_,
        updateLookRanges_, fittingRatio_);
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
