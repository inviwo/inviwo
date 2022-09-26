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

#include <inviwo/sgct/datastructures/sgctcamera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <inviwo/core/datastructures/camera/cameratools.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>

#include <sgct/sgct.h>

namespace inviwo {

SGCTCamera::SGCTCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane, float farPlane,
                       float fieldOfView, float aspectRatio)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane, aspectRatio), fovy_(fieldOfView) {}

SGCTCamera::SGCTCamera(const SGCTCamera& other) = default;

SGCTCamera& SGCTCamera::operator=(const SGCTCamera& other) = default;

SGCTCamera* SGCTCamera::clone() const { return new SGCTCamera(*this); }

std::string SGCTCamera::getClassIdentifier() const { return classIdentifier; }

const std::string SGCTCamera::classIdentifier = "SGCTCamera";

void SGCTCamera::updateFrom(const Camera& source) {
    Camera::updateFrom(source);

    if (auto sc = dynamic_cast<const SGCTCamera*>(&source)) {
        setFovy(sc->getFovy());

        extProj_ = sc->extProj_;
        extView_ = sc->extView_;
        extModel_ = sc->extModel_;
        invalidateProjectionMatrix();
        invalidateViewMatrix();

    } else if (auto pc = dynamic_cast<const PerspectiveCamera*>(&source)) {
        setFovy(pc->getFovy());
    } else if (auto spc = dynamic_cast<const SkewedPerspectiveCamera*>(&source)) {
        setFovy(spc->getFovy());
    } else if (auto oc = dynamic_cast<const OrthographicCamera*>(&source)) {
        const auto dist = util::widthToViewDist(oc->getWidth(), getFovy(), getAspectRatio());
        setLookFrom(getLookTo() + dist * glm::normalize(getLookFrom() - getLookTo()));
    }
}

void SGCTCamera::setFovy(float val) {
    if (fovy_ != val) {
        fovy_ = val;
        invalidateProjectionMatrix();
        if (camprop_) {
            if (auto p = util::getCameraFovProperty(*camprop_)) {
                p->propertyModified();
            }
        }
    }
}

void SGCTCamera::configureProperties(CameraProperty& cp, bool attach) {
    Camera::configureProperties(cp, attach);
    if (attach) {
        util::updateOrCreateCameraFovProperty(
            cp, [this]() { return getFovy(); }, [this](const float& val) { setFovy(val); });

    } else if (auto fov = util::getCameraFovProperty(cp)) {
        fov->setGetAndSet([val = fov->get()]() { return val; }, [](const float&) {});
    }
}

bool SGCTCamera::equal(const Camera& other) const {
    if (auto rhs = dynamic_cast<const SGCTCamera*>(&other)) {
        return equalTo(other) && fovy_ == rhs->fovy_ && extProj_ == rhs->extProj_ &&
               extModel_ == rhs->extModel_ && extView_ == rhs->extView_;
    } else {
        return false;
    }
}

void SGCTCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("fov", fovy_);
}
void SGCTCamera::deserialize(Deserializer& d) {
    d.deserialize("fov", fovy_);
    Camera::deserialize(d);
}

void SGCTCamera::setExternal(const sgct::RenderData& renderData) {
    mat4 proj = glm::make_mat4(renderData.projectionMatrix.values);
    mat4 view = glm::make_mat4(renderData.viewMatrix.values);
    mat4 model = glm::make_mat4(renderData.modelMatrix.values);

    bool modified = true; // true here to force eval every frame, for testing

    if (extProj_ != proj) {
        extProj_ = proj;
        invalidateProjectionMatrix();
        modified = true;
    }
    if (extView_ != view) {
        extView_ = view;
        invalidateViewMatrix();
        modified = true;
    }
    if (extModel_ != model) {
        extModel_ = model;
        getInverseViewMatrix();
        modified = true;
    }

    setFovy(renderData.viewport.horizontalFieldOfViewDegrees());
    setAspectRatio(renderData.window.aspectRatio());

    if (camprop_ && modified) {
        camprop_->getProcessor()->invalidate(InvalidationLevel::InvalidOutput);
    }
}

void SGCTCamera::zoom(float factor, std::optional<mat4> boundingBox) {
    setLookFrom(util::perspectiveZoom(*this, factor, boundingBox));
}

mat4 SGCTCamera::calculateViewMatrix() const {
    return extView_ * extModel_ * glm::lookAt(lookFrom_, lookTo_, lookUp_);
}

mat4 SGCTCamera::calculateProjectionMatrix() const {
    auto old = glm::perspective(glm::radians(fovy_), aspectRatio_, nearPlaneDist_, farPlaneDist_);
    return extProj_;
}

}  // namespace inviwo
