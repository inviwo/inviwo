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

#ifndef IVW_CAMERAPROPERTY_H
#define IVW_CAMERAPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/interaction/events/eventlistener.h>

namespace inviwo {

template <unsigned int N>
class SpatialEntity;

class Inport;

/**
* \class CameraProperty
* 
* Besides giving access to a perspective camera in the GUI 
* it also enables linking individual camera properties.
* @see PerspectiveCamera
*/
class IVW_CORE_API CameraProperty : public CompositeProperty {

public:
    InviwoPropertyInfo();

    CameraProperty(std::string identifier,
                   std::string displayName,
                   vec3 eye = vec3(0.0f, 0.0f, -2.0f),
                   vec3 center = vec3(0.0f),
                   vec3 lookUp = vec3(0.0f, 1.0f, 0.0f),
                   Inport* inport = nullptr,
                   InvalidationLevel=INVALID_RESOURCES,
                   PropertySemantics semantics = PropertySemantics::Default);
    
    CameraProperty(const CameraProperty& rhs);
    CameraProperty& operator=(const CameraProperty& that);
    CameraProperty& operator=(const PerspectiveCamera& value);

    //virtual operator PerspectiveCamera&() { return value_; }; // Do not allow user to get non-const reference since no notification mechanism exist.
    virtual operator const PerspectiveCamera&() const;

    virtual CameraProperty* clone() const;
    virtual ~CameraProperty() = default;

    virtual PerspectiveCamera& get();
    virtual const PerspectiveCamera& get() const;
    virtual void set(const PerspectiveCamera& value);
    virtual void set(const Property* srcProperty) override;

    virtual void resetToDefaultState() override;


    /** 
     * Reset camera position, direction and field of view to default state.
     */
    void resetCamera();

    const vec3& getLookFrom() const;
    void setLookFrom(vec3 lookFrom);
    const vec3& getLookTo() const;
    void setLookTo(vec3 lookTo);
    const vec3& getLookUp() const;
    void setLookUp(vec3 lookUp);
    vec3 getLookRight() const;

    float getFovy() const;
    void setFovy(float fovy);

    void setAspectRatio(float aspectRatio);
    float getAspectRatio() const;

    void setLook(vec3 lookFrom, vec3 lookTo, vec3 lookUp);

    float getNearPlaneDist() const;
    float getFarPlaneDist() const;

    void setNearPlaneDist(float v);
    void setFarPlaneDist(float v);

    vec3 getLookFromMinValue() const;
    vec3 getLookFromMaxValue() const;

    vec3 getLookToMinValue() const;
    vec3 getLookToMaxValue() const;

    /** 
     * \brief Convert from normalized device coordinates (xyz in [-1 1]) to world coordinates.
     * 
     * @param ndcCoords Coordinates in [-1 1]
     * @return World space position
     */
    vec3 getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const;

    /**
    * \brief Convert from normalized device coordinates (xyz in [-1 1]) to clip coordinates.
    *
    * @param ndcCoords xyz clip-coordinates in [-1 1]^3, and the clip w-coordinate used for perspective division.   
    * @return Clip space position
    */
    vec4 getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const;

    const mat4& viewMatrix() const;
    const mat4& projectionMatrix() const;
    const mat4& inverseViewMatrix() const;
    const mat4& inverseProjectionMatrix() const;

    void setProjectionMatrix(float fovy, float aspect, float farPlane, float nearPlane);

    void invokeEvent(Event* event);

    // Local camera invalidation
    // Use lock and unlock to set several camera properties without causing evaluation,
    // then call invalidateCamera().
    void invalidateCamera();
    void lockInvalidation();
    void unlockInvalidation();
    bool isInvalidationLocked();

    void setInport(Inport* inport);
    void fitWithBasis(const mat3& basis);
    void fitReset();
    void inportChanged();

private:

    // These functions make sure that the 
    // template value (PerspectiveCamera) is 
    // in sync with the property values.
    void lookFromChangedFromProperty();
    void lookToChangedFromProperty();
    void lookUpChangedFromProperty();
    void verticalFieldOfViewChangedFromProperty();
    void aspectRatioChangedFromProperty();
    void nearPlaneChangedFromProperty();
    void farPlaneChangedFromProperty();

    void updatePropertyFromValue();

    PerspectiveCamera value_;
    // These properties enable linking of individual 
    // camera properties but requires them to be synced 
    // with the template value_ (PerspectiveCamera).
    FloatVec3Property lookFrom_;
    FloatVec3Property lookTo_;
    FloatVec3Property lookUp_;

    FloatProperty fovy_;
    FloatProperty aspectRatio_;
    FloatProperty nearPlane_;
    FloatProperty farPlane_;


    BoolProperty fitToBasis_;

    bool lockInvalidation_;

    Inport* inport_; ///< Allows the camera to be positioned relative to new data (VolumeInport, MeshInport)
    const SpatialEntity<3>* data_; //< non-owning reference;
    mat3 oldBasis_;
};

} // namespace

#endif // IVW_CAMERAPROPERTY_H
