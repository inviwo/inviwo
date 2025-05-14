/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/camera/plotcamera.h>

#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <inviwo/core/datastructures/camera/cameratools.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/datastructures/camera/orthographiccamera.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>

namespace inviwo {

PlotCamera::PlotCamera(vec3 lookFrom, vec3 lookTo, vec3 lookUp, float nearPlane, float farPlane,
                       float aspectRatio, vec2 size)
    : Camera(lookFrom, lookTo, lookUp, nearPlane, farPlane, aspectRatio), size_{size} {}

PlotCamera::PlotCamera(const PlotCamera&) = default;

PlotCamera& PlotCamera::operator=(const PlotCamera&) = default;

PlotCamera::PlotCamera(PlotCamera&& other) noexcept = default;

PlotCamera& PlotCamera::operator=(PlotCamera&& other) noexcept = default;

PlotCamera* PlotCamera::clone() const { return new PlotCamera(*this); }

std::string_view PlotCamera::getClassIdentifier() const { return classIdentifier; }

void PlotCamera::setSize(vec2 size) {
    if (size_ != size) {
        size_ = size;
        invalidateProjectionMatrix();
        if (camprop_) {
            if (auto* p = util::getCameraWidthProperty(*camprop_)) {
                p->propertyModified();
            }
        }
    }
}

void PlotCamera::zoom(const ZoomOptions& opts) {
    if (opts.origin) {

        const auto up = getLookUp();
        const auto dir = -glm::normalize(getDirection());
        const auto right = glm::cross(up, dir);
        const auto basis = mat3(right, up, dir);

        const auto translate = glm::translate(vec3{0.5f * size_ * opts.origin.value(), 0.f});
        const auto scale = glm::scale(vec3{1.0f - opts.factor, 1.f});
        const auto m = translate * scale * glm::inverse(translate);
        const auto offset = basis * vec3{m * vec4{0.f, 0.f, 0.f, 1.f}};
        setLook(getLookFrom() + offset, getLookTo() + offset, getLookUp());
    }
    setSize(size_ * (1.0f - opts.factor));
}

void PlotCamera::updateFrom(const Camera& source) {
    Camera::updateFrom(source);
    if (const auto* plc = dynamic_cast<const PlotCamera*>(&source)) {
        setSize(plc->getSize());
    } else if (const auto* oc = dynamic_cast<const OrthographicCamera*>(&source)) {
        setSize(vec2{oc->getWidth(), oc->getWidth() / getAspectRatio()});
    } else if (const auto* pc = dynamic_cast<const PerspectiveCamera*>(&source)) {
        const auto width = util::fovyToWidth(
            pc->getFovy(), glm::distance(getLookTo(), getLookFrom()), getAspectRatio());
        setSize(vec2{width, width / getAspectRatio()});
    } else if (const auto* sc = dynamic_cast<const SkewedPerspectiveCamera*>(&source)) {
        const auto width = util::fovyToWidth(
            sc->getFovy(), glm::distance(getLookTo(), getLookFrom()), getAspectRatio());
        setSize(vec2{width, width / getAspectRatio()});
    }
}

void PlotCamera::configureProperties(CameraProperty& cp, bool attach) {
    Camera::configureProperties(cp, attach);

    const auto get = [this]() { return getSize(); };
    const auto set = [this](const vec2& val) {
        if (size_ != val) {
            size_ = val;
            invalidateProjectionMatrix();
        }
    };

    if (attach) {
        auto* sizeProp = dynamic_cast<FloatVec2RefProperty*>(cp.getCameraProperty("size"));
        if (sizeProp) {
            sizeProp->setGetAndSet(get, set);
        } else {
            auto newSize = std::make_unique<FloatVec2RefProperty>(
                "size", "Size", "The viewport size in world space"_help, get, set,
                std::pair<vec2, ConstraintBehavior>{vec2{0.0f}, ConstraintBehavior::Immutable},
                std::pair<vec2, ConstraintBehavior>{vec2{1000.0f}, ConstraintBehavior::Ignore},
                vec2{0.1f});
            sizeProp = newSize.get();
            cp.addCamerapProperty(std::move(newSize));
        }
        sizeProp->setVisible(true);
    } else if (auto* sizeProp = dynamic_cast<FloatVec2RefProperty*>(cp.getCameraProperty("size"))) {
        sizeProp->setGetAndSet([val = sizeProp->get()]() { return val; }, [](const vec2&) {});
    }
}

bool PlotCamera::equal(const Camera& other) const {
    if (const auto* rhs = dynamic_cast<const PlotCamera*>(&other)) {
        return equalTo(other) && size_ == rhs->size_;
    } else {
        return false;
    }
}

mat4 PlotCamera::calculateProjectionMatrix() const {
    const float halfWidth = 0.5f * size_.x;
    const float halfHeight = 0.5f * size_.y;
    return glm::ortho(-halfWidth, +halfWidth, -halfHeight, +halfHeight, nearPlaneDist_,
                      farPlaneDist_);
}

vec4 PlotCamera::getClipPosFromNormalizedDeviceCoords(const vec3& ndcCoords) const {
    return vec4{ndcCoords, 1.0f};
}

void PlotCamera::serialize(Serializer& s) const {
    Camera::serialize(s);
    s.serialize("size", size_);
}
void PlotCamera::deserialize(Deserializer& d) {
    d.deserialize("size", size_);
    Camera::deserialize(d);
}
}  // namespace inviwo
