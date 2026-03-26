/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/algorithm/camerautils.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/interaction/events/eventlistener.h>
#include <inviwo/core/interaction/trackballobject.h>
#include <inviwo/core/util/glm.h>

#include <functional>
#include <optional>

namespace inviwo {

class Inport;
class CameraFactory;
/**
 * @ingroup properties
 * A property wrapping the Camera data structure
 * @see Camera
 */
class IVW_CORE_API CameraProperty : public CompositeProperty, public TrackballObject {
public:
    using value_type = Camera;
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.CameraProperty"};

    CameraProperty(std::string_view identifier, std::string_view displayName, Document help,
                   std::function<std::optional<mat4>()> getBoundingBox,
                   dvec3 eye = dvec3(0.0, 0.0, 2.0), dvec3 center = dvec3(0.0),
                   dvec3 lookUp = dvec3(0.0, 1.0, 0.0),
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

    CameraProperty(std::string_view identifier, std::string_view displayName,
                   std::function<std::optional<mat4>()> getBoundingBox,
                   dvec3 eye = dvec3(0.0, 0.0, 2.0), dvec3 center = dvec3(0.0),
                   dvec3 lookUp = dvec3(0.0, 1.0, 0.0),
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

    CameraProperty(std::string_view identifier, std::string_view displayName,
                   dvec3 eye = dvec3(0.0, 0.0, 2.0), dvec3 center = dvec3(0.0),
                   dvec3 lookUp = dvec3(0.0, 1.0, 0.0), Inport* inport = nullptr,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

    CameraProperty(const CameraProperty& rhs);

    operator const Camera&() const;

    virtual CameraProperty* clone() const override;
    virtual ~CameraProperty();

    virtual Camera& get();
    virtual const Camera& get() const;
    virtual void set(const Property* srcProperty) override;

    CameraProperty& setCamera(std::string_view cameraIdentifier);
    CameraProperty& setCamera(std::unique_ptr<Camera> camera);

    virtual dvec3 getLookFrom() const override;
    virtual TrackballObject& setLookFrom(dvec3 lookFrom) override;
    virtual dvec3 getLookTo() const override;
    virtual TrackballObject& setLookTo(dvec3 lookTo) override;
    virtual dvec3 getLookUp() const override;
    virtual TrackballObject& setLookUp(dvec3 lookUp) override;

    dvec3 getLookRight() const;

    /**
     * @brief Get unnormalized direction of camera: lookTo - lookFrom
     */
    dvec3 getDirection() const;

    CameraProperty& setAspectRatio(float aspectRatio);
    float getAspectRatio() const;

    /**
     * Sets given camera properties while respecting their min/max ranges.
     * Locks and unlocks processor network before and after changing property values.
     * @note Parameters will be capped by their min/max.
     */
    virtual TrackballObject& setLook(dvec3 lookFrom, dvec3 lookTo, dvec3 lookUp) override;  // NOLINT

    virtual float getNearPlaneDist() const override;
    virtual float getFarPlaneDist() const override;

    CameraProperty& setNearPlaneDist(float v);
    CameraProperty& setFarPlaneDist(float v);

    /**
     * Set near and far plane distance values and adjust their min/max ranges.
     * Adjusts the min/max ranges of the properties to e.g. 0.1/10 times the given value.
     * Locks and unlocks processor network before and after changing property values.
     */
    CameraProperty& setNearFarPlaneDist(float nearPlaneDist, float farPlaneDist,
                                        float minMaxRatio = 10.f);

    virtual dvec3 getLookFromMinValue() const override;
    virtual dvec3 getLookFromMaxValue() const override;

    virtual dvec3 getLookToMinValue() const override;
    virtual dvec3 getLookToMaxValue() const override;

    virtual void zoom(const ZoomOptions& opts) override;

    /**
     * @brief Convert from normalized device coordinates (xyz in [-1 1]) to world coordinates.
     * @param ndcCoords Coordinates in [-1 1]
     * @return World space position
     */
    virtual dvec3 getWorldPosFromNormalizedDeviceCoords(const dvec3& ndcCoords) const override;

    /**
     * @brief Convert from normalized device coordinates (xyz in [-1 1]) to clip coordinates.
     * @param ndcCoords xyz clip-coordinates in [-1 1]^3, and the clip w-coordinate used for
     * perspective division.
     * @return Clip space position
     */
    dvec4 getClipPosFromNormalizedDeviceCoords(const dvec3& ndcCoords) const;

    virtual dvec3 getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
        const dvec2& normalizedScreenCoord) const override;

    const dmat4& viewMatrix() const;
    const dmat4& projectionMatrix() const;
    const dmat4& inverseViewMatrix() const;
    const dmat4& inverseProjectionMatrix() const;

    void invokeEvent(Event* event) override;

    Property* getCameraProperty(const std::string& identifier) const;
    void addCamerapProperty(std::unique_ptr<Property> camprop);

    virtual CameraProperty& setCurrentStateAsDefault() override;
    virtual CameraProperty& resetToDefaultState() override;
    virtual bool isDefaultState() const override;
    virtual bool needsSerialization() const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /**
     * Fit the current bounding box into view, keeping the view direction
     * If updateNearFar is set it will also adjust near and far.
     * If updateLookRanges is set it will also adjust the range of look from
     * @see camerautil::setCameraView
     * @pre There must be a bounding box set
     */
    void fitData();

    /**
     * Fit the current bounding box into view and set the view direction to the specified side
     * If updateNearFar is set it will also adjust near and far.
     * If updateLookRanges is set it will also adjust the range of look from
     * @see camerautil::setCameraView
     * @pre There must be a bounding box set
     */
    void setView(camerautil::Side side);

    /**
     * Flip the direction of the up vector
     */
    void flipUp();

    /**
     * Flip the view vector, look at the backside.
     */
    void flipView();

    /**
     * Roll the view
     */
    void roll(float radians);

    /**
     * Adjust the near and far values for the current bounding box
     * @pre There must be a bounding box set
     */
    void setNearFar();

    /**
     * Adjust the look from ranges for the current bounding box
     * @pre There must be a bounding box set
     */
    void setLookRange();

private:
    CameraFactory* factory_;
    OptionPropertyString cameraType_;
    std::vector<Property*> cameraProperties_;
    std::vector<std::unique_ptr<Property>> ownedCameraProperties_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Camera> defaultCamera_;
    ButtonGroupProperty cameraActions_;

public:
    // These properties enable linking of individual
    // camera properties but requires them to be synced
    // with the camera.
    // Use NetworkLock if editing multiple properties at the same time

    DoubleVec3RefProperty lookFrom_;
    DoubleVec3RefProperty lookTo_;
    DoubleVec3RefProperty lookUp_;
    FloatRefProperty aspectRatio_;
    FloatRefProperty nearPlane_;
    FloatRefProperty farPlane_;

private:
    bool changeCamera(const std::string& name);
    void hideConfiguredProperties();

    std::vector<ButtonGroupProperty::Button> buttons();
    void updateFittingVisibility();

    std::function<std::optional<mat4>()> getBoundingBox_;
    bool aspectSupplier_ = false;
};

}  // namespace inviwo
