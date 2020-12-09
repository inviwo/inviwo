/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/util/glm.h>

#include <optional>

namespace inviwo {

namespace cameradefaults {
constexpr vec3 lookFrom = vec3(0.0f, 0.0f, 2.0f);
constexpr vec3 lookTo = vec3(0.0f);
constexpr vec3 lookUp = vec3(0.0f, 1.0f, 0.0f);
constexpr float nearPlane = 0.1f;
constexpr float farPlane = 100.0f;
constexpr float fieldOfView = 38.f;
constexpr float aspectRatio = 1.f;
constexpr float width = 10.f;
}  // namespace cameradefaults

class CameraProperty;

/**
 * \ingroup datastructures
 *
 * \brief Base class for cameras.
 * Override this class to set your own projection matrix.
 * @see PerspectiveCamera
 * @see OrthographicCamera
 */
class IVW_CORE_API Camera : public Serializable {
public:
    /**
     * @brief Create a camera
     *
     * Default parameters creates a right handed coordinate system
     * with camera looking towards the negative z-axis.
     * with X = (1, 0, 0), Y = (0, 1, 0), Z = (0, 0, -1)
     *
     * @param lookFrom Camera position (eye)
     * @param lookTo Camera focus point (center)
     * @param lookUp Camera up direction
     * @param nearPlane Camera near clip-plane
     * @param farPlane Camera far clip-plane
     * @param aspectRatio Camera aspect ratio
     */
    Camera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane, float farPlane,
           float aspectRatio);
    virtual ~Camera() = default;
    Camera(const Camera& other) = default;
    Camera& operator=(const Camera& other) = default;
    virtual Camera* clone() const = 0;
    virtual std::string getClassIdentifier() const = 0;

    virtual void updateFrom(const Camera& source);
    virtual void configureProperties(CameraProperty& cameraProperty, bool attach);

    const vec3& getLookFrom() const;
    virtual void setLookFrom(vec3 val);

    const vec3& getLookTo() const;
    virtual void setLookTo(vec3 val);

    const vec3& getLookUp() const;
    virtual void setLookUp(vec3 val);

    virtual float getAspectRatio() const;
    virtual void setAspectRatio(float val);

    virtual void zoom(float factor, std::optional<mat4> boundingBox) = 0;

    /**
     * \brief Get unnormalized direction of camera: lookTo - lookFrom
     */
    vec3 getDirection() const;

    /**
     * \brief Set distance to the near plane from lookFrom.
     */
    virtual void setNearPlaneDist(float distance);
    float getNearPlaneDist() const;

    /**
     * \brief Set distance to the far plane from lookFrom.
     */
    virtual void setFarPlaneDist(float distance);
    float getFarPlaneDist() const;

    const mat4& getViewMatrix() const;
    const mat4& getProjectionMatrix() const;
    const mat4& getInverseViewMatrix() const;
    const mat4& getInverseProjectionMatrix() const;

    /**
     * \brief Convert from normalized device coordinates (xyz in [-1 1]) to world coordinates.
     * @param ndcCoords Coordinates in [-1 1]
     * @return World space position
     */
    vec3 getWorldPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const;

    /**
     * \brief Convert from normalized device coordinates (xyz in [-1 1]) to clip coordinates,
     * where z value of -1 correspond to the near plane and 1 to the far plane.
     * Coordinates outside of the [-1 1]^3 range will be clipped.
     *
     * @param ndcCoords xyz clip-coordinates in [-1 1]^3, and the clip w-coordinate used for
     * perspective division.
     * @return Clip space position
     */
    virtual vec4 getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const;

    vec3 getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth(
        const vec2& normalizedScreenCoord) const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    friend IVW_CORE_API bool operator==(const Camera& lhs, const Camera& rhs) {
        return lhs.equal(rhs);
    }
    friend IVW_CORE_API bool operator!=(const Camera& lhs, const Camera& rhs) {
        return !(lhs == rhs);
    }

protected:
    virtual bool equal(const Camera& other) const = 0;
    bool equalTo(const Camera& other) const;

    /**
     * \brief Calculate and return the projection matrix for the camera.
     *
     * Implement this function to provide your own projection computation functionality.
     * For example orthogonal or perspective projection.
     * This function will be called when the projection matrix is invalid.
     * @see PerspectiveCamera
     * @see OrthographicCamera
     */
    virtual mat4 calculateProjectionMatrix() const = 0;

    virtual mat4 calculateViewMatrix() const;

    void invalidateViewMatrix();
    void invalidateProjectionMatrix();

    vec3 lookFrom_;
    vec3 lookTo_;
    vec3 lookUp_;

    float nearPlaneDist_;  ///< Distance to the near plane from lookFrom.
    float farPlaneDist_;   ///< Distance to the far plane from lookFrom.

    float aspectRatio_;

    // Make mutable to allow then to be changed even though they are called from const function.
    // This allows us to perform lazy evaluation.
    mutable bool invalidViewMatrix_;
    mutable bool invalidProjectionMatrix_;
    mutable mat4 viewMatrix_;
    mutable mat4 projectionMatrix_;
    mutable mat4 inverseViewMatrix_;
    mutable mat4 inverseProjectionMatrix_;
    CameraProperty* camprop_ = nullptr;
};

// Implementation details
inline const vec3& Camera::getLookFrom() const { return lookFrom_; }
inline const vec3& Camera::getLookTo() const { return lookTo_; }
inline const vec3& Camera::getLookUp() const { return lookUp_; }
inline vec3 Camera::getDirection() const { return lookTo_ - lookFrom_; }
inline float Camera::getNearPlaneDist() const { return nearPlaneDist_; }
inline float Camera::getFarPlaneDist() const { return farPlaneDist_; }
inline float Camera::getAspectRatio() const { return aspectRatio_; }

}  // namespace inviwo
