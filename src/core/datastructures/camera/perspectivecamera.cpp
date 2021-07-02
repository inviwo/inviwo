/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <inviwo/core/datastructures/camera/cameratools.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>

#include <memory>
#include <utility>

namespace inviwo {

PerspectiveCamera::PerspectiveCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane,
                                     float farPlane, float fieldOfView, float aspectRatio)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane, aspectRatio), fovy_(fieldOfView) {}

PerspectiveCamera::PerspectiveCamera(const PerspectiveCamera& other) = default;

PerspectiveCamera& PerspectiveCamera::operator=(const PerspectiveCamera& other) = default;

PerspectiveCamera* PerspectiveCamera::clone() const { return new PerspectiveCamera(*this); }

std::string PerspectiveCamera::getClassIdentifier() const { return classIdentifier; }

const std::string PerspectiveCamera::classIdentifier = "PerspectiveCamera";

void PerspectiveCamera::updateFrom(const Camera& source) {
    Camera::updateFrom(source);
    if (auto pc = dynamic_cast<const PerspectiveCamera*>(&source)) {
        setFovy(pc->getFovy());
    } else if (auto oc = dynamic_cast<const OrthographicCamera*>(&source)) {
        const auto dist = util::widthToViewDist(oc->getWidth(), getFovy(), getAspectRatio());
        setLookFrom(getLookTo() + dist * glm::normalize(getLookFrom() - getLookTo()));
    } else if (auto sc = dynamic_cast<const SkewedPerspectiveCamera*>(&source)) {
        setFovy(sc->getFovy());
    }
}

void PerspectiveCamera::setFovy(float val) {
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

void PerspectiveCamera::zoom(float factor, std::optional<mat4> boundingBox) {
    setLookFrom(util::perspectiveZoom(*this, factor, boundingBox));
}

void PerspectiveCamera::configureProperties(CameraProperty& cp, bool attach) {
    Camera::configureProperties(cp, attach);
    if (attach) {
        util::updateOrCreateCameraFovProperty(
            cp, [this]() { return getFovy(); },
            [this](const float& val) {
                if (fovy_ != val) {
                    fovy_ = val;
                    invalidateProjectionMatrix();
                }
            });

    } else if (auto fov = util::getCameraFovProperty(cp)) {
        fov->setGetAndSet([val = fov->get()]() { return val; }, [](const float&) {});
    }
}

bool PerspectiveCamera::equal(const Camera& other) const {
    if (auto rhs = dynamic_cast<const PerspectiveCamera*>(&other)) {
        return equalTo(other) && fovy_ == rhs->fovy_;
    } else {
        return false;
    }
}

void PerspectiveCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("fov", fovy_);
}
void PerspectiveCamera::deserialize(Deserializer& d) {
    d.deserialize("fov", fovy_);
    Camera::deserialize(d);
}

}  // namespace inviwo
