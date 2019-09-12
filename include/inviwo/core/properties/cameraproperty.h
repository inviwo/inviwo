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

#ifndef IVW_CAMERAPROPERTY_H
#define IVW_CAMERAPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/algorithm/camerautils.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/interaction/events/eventlistener.h>
#include <inviwo/core/interaction/trackballobject.h>

#include <functional>
#include <optional>

namespace inviwo {

class Inport;

/**
 * \ingroup properties
 * A property wrapping the Camera data structure
 * @see Camera
 */
class IVW_CORE_API CameraProperty : public CompositeProperty, public TrackballObject {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    CameraProperty(const std::string& identifier, const std::string& displayName,
                   std::function<std::optional<mat4>()> getBoundingBox,
                   vec3 eye = vec3(0.0f, 0.0f, 2.0f), vec3 center = vec3(0.0f),
                   vec3 lookUp = vec3(0.0f, 1.0f, 0.0f),
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

    CameraProperty(const std::string& identifier, const std::string& displayName,
                   vec3 eye = vec3(0.0f, 0.0f, 2.0f), vec3 center = vec3(0.0f),
                   vec3 lookUp = vec3(0.0f, 1.0f, 0.0f), Inport* inport = nullptr,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);

    CameraProperty(const CameraProperty& rhs);

    operator const Camera&() const;

    virtual CameraProperty* clone() const override;
    virtual ~CameraProperty();

    virtual Camera& get();
    virtual const Camera& get() const;
    virtual void set(const Property* srcProperty) override;

    /**
     * Reset camera position, direction to default state.
     */
    void resetCamera();

    virtual CameraProperty& setCurrentStateAsDefault() override;
    virtual CameraProperty& resetToDefaultState() override;

    virtual const vec3& getLookFrom() const override;
    virtual void setLookFrom(vec3 lookFrom) override;
    virtual const vec3& getLookTo() const override;
    virtual void setLookTo(vec3 lookTo) override;
    virtual const vec3& getLookUp() const override;
    virtual void setLookUp(vec3 lookUp) override;

    vec3 getLookRight() const;

    void setAspectRatio(float aspectRatio);
    float getAspectRatio() const;
    /**
     * Sets given camera properties while respecting their min/max ranges.
     * Locks and unlocks processor network before and after changing property values.
     * @note Parameters will be capped by their min/max.
     */
    virtual void setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp) override;

    virtual float getNearPlaneDist() const override;
    virtual float getFarPlaneDist() const override;

    void setNearPlaneDist(float v);
    void setFarPlaneDist(float v);
    /**
     * Set near and far plane distance values and adjust their min/max ranges.
     * Adjusts the min/max ranges of the properties to e.g. 0.1/10 times the given value.
     * Locks and unlocks processor network before and after changing property values.
     */
    void setNearFarPlaneDist(float nearPlaneDist, float farPlaneDist, float minMaxRatio = 10.f);

    virtual vec3 getLookFromMinValue() const override;
    virtual vec3 getLookFromMaxValue() const override;

    virtual vec3 getLookToMinValue() const override;
    virtual vec3 getLookToMaxValue() const override;

    /**
     * \brief Convert from normalized device coordinates (xyz in [-1 1]) to world coordinates.
     * @param ndcCoords Coordinates in [-1 1]
     * @return World space position
     */
    virtual vec3 getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const override;

    /**
     * \brief Convert from normalized device coordinates (xyz in [-1 1]) to clip coordinates.
     * @param ndcCoords xyz clip-coordinates in [-1 1]^3, and the clip w-coordinate used for
     * perspective division.
     * @return Clip space position
     */
    vec4 getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const;

    virtual vec3 getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
        const vec2& normalizedScreenCoord) const override;

    const mat4& viewMatrix() const;
    const mat4& projectionMatrix() const;
    const mat4& inverseViewMatrix() const;
    const mat4& inverseProjectionMatrix() const;

    void invokeEvent(Event* event) override;

    // These properties enable linking of individual
    // camera properties but requires them to be synced
    // with the camera.
    // Use NetworkLock if editing multiple properties at the same time
    OptionPropertyString cameraType_;
    ButtonGroupProperty cameraActions_;
    FloatVec3Property lookFrom_;
    FloatVec3Property lookTo_;
    FloatVec3Property lookUp_;
    FloatProperty aspectRatio_;
    FloatProperty nearPlane_;
    FloatProperty farPlane_;

private:
    std::vector<ButtonGroupProperty::Button> buttons();
    void setView(::inviwo::camerautil::Side side);
    void fitData();
    void flipUp();
    void setNearFar();
    void setLookRange();

    CompositeProperty settings_;
    BoolProperty updateNearFar_;
    BoolProperty updateLookRanges_;
    FloatProperty fittingRatio_;
    ButtonProperty setNearFarButton_;
    ButtonProperty setLookRangesButton_;

    void changeCamera(std::unique_ptr<Camera> newCamera);
    std::unique_ptr<Camera> camera_;
    std::function<std::optional<mat4>()> getBoundingBox_;
    bool aspectSupplier_ = false;
};

}  // namespace inviwo

#endif  // IVW_CAMERAPROPERTY_H
