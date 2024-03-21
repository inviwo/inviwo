/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <inviwo/core/properties/positionproperty.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/foreach.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

const std::string PositionProperty::classIdentifier = "org.inviwo.PositionProperty";
std::string PositionProperty::getClassIdentifier() const { return classIdentifier; }

PositionProperty::PositionProperty(std::string_view identifier, std::string_view displayName,
                                   Document help, const vec3& position,
                                   CoordinateSpace coordinateSpace, CameraProperty* camera,
                                   InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, std::move(help), invalidationLevel, semantics}
    , camera_{camera}
    , previousReferenceSpace_{coordinateSpace}
    , referenceSpace_("referenceFrame", "Space",
                      "Reference coordinate space for the position below."_help,
                      {{"world", "World coordinates", CoordinateSpace::World},
                       {"view", "View coordinates", CoordinateSpace::View},
                       {"clip", "Clip coordinates", CoordinateSpace::Clip}})
    , transformPosition_{"transformPosition", "Transform Position to Reference Space",
                         "When the coordinate space is changed and this property is checked, the "
                         "position will be transformed to the new space keeping it in the same "
                         "spatial position."_help,
                         true, InvalidationLevel::Valid}
    , position_{"position", "Position", util::ordinalSymmetricVector(position, 10.0f)}

    , output_{"output", "Output"}
    , worldPos_{"worldPosition", "World Position",
                [&]() -> vec3 { return get(CoordinateSpace::World); },
                [&](const vec3& pos) { set(pos, CoordinateSpace::World); },
                util::ordinalRefSymmetricVector<vec3>(100.0f)
                    .set("Position in world space coordinates"_help)
                    .set(PropertySemantics::Text)
                    .set(ReadOnly::Yes)}
    , viewPos_{"viewPosition", "View Position",
               [&]() -> vec3 { return get(CoordinateSpace::View); },
               [&](const vec3& pos) { set(pos, CoordinateSpace::View); },
               util::ordinalRefSymmetricVector<vec3>(100.0f)
                   .set("Position in view space coordinates of the camera"_help)
                   .set(PropertySemantics::Text)
                   .set(ReadOnly::Yes)}
    , clipPos_{"clipPosition", "Clip Position",
               [&]() -> vec3 { return get(CoordinateSpace::Clip); },
               [&](const vec3& pos) { set(pos, CoordinateSpace::Clip); },
               util::ordinalRefSymmetricVector<vec3>(1.0f)
                   .set("Position in clip space coordinates [-1,1]"_help)
                   .set(PropertySemantics::Text)
                   .set(ReadOnly::Yes)}
    , screenPos_{"screenPosition", "Screen Position (normalized)",
                 [&]() -> vec3 { return get(CoordinateSpace::Clip) * 0.5f + 0.5f; },
                 [&](const vec3& pos) { set(pos * 2.0f - 1.0f, CoordinateSpace::Clip); },
                 OrdinalRefPropertyState<vec3>{.min = vec3{0.0f},
                                               .minConstraint = ConstraintBehavior::Ignore,
                                               .max = vec3{1.0f},
                                               .maxConstraint = ConstraintBehavior::Ignore}
                     .set("Position in clip space coordinates [-1,1]"_help)
                     .set(PropertySemantics::Text)
                     .set(ReadOnly::Yes)} {

    referenceSpace_.setSelectedValue(coordinateSpace);
    referenceSpace_.setCurrentStateAsDefault();

    output_.addProperties(worldPos_, viewPos_, clipPos_, screenPos_);
    output_.setCollapsed(true);
    addProperties(referenceSpace_, transformPosition_, position_, output_);

    util::for_each_argument([&](auto& p) { outputProperties_.push_back(&p); }, worldPos_, viewPos_,
                            clipPos_, screenPos_);

    referenceSpace_.onChange([this]() { referenceSpaceChanged(); });
    position_.onChange([this]() { invalidateOutputProperties(); });
    if (camera_) {
        camera_->onChange([this]() { invalidateOutputProperties(); });
    }
}

PositionProperty::PositionProperty(std::string_view identifier, std::string_view displayName,
                                   const vec3& position, CoordinateSpace coordinateSpace,
                                   CameraProperty* camera, InvalidationLevel invalidationLevel,
                                   PropertySemantics semantics)
    : PositionProperty(identifier, displayName, Document{}, position, coordinateSpace, camera,
                       invalidationLevel, semantics) {}

PositionProperty::PositionProperty(const PositionProperty& rhs)
    : CompositeProperty{rhs}
    , camera_{rhs.camera_}
    , previousReferenceSpace_{rhs.previousReferenceSpace_}
    , referenceSpace_{rhs.referenceSpace_}
    , transformPosition_{rhs.transformPosition_}
    , position_{rhs.position_}

    , output_{rhs.output_}
    , worldPos_{rhs.worldPos_}
    , viewPos_{rhs.viewPos_}
    , clipPos_{rhs.clipPos_}
    , screenPos_{rhs.screenPos_} {

    output_.addProperties(worldPos_, viewPos_, clipPos_, screenPos_);
    addProperties(referenceSpace_, position_, output_);

    util::for_each_argument([&](auto& p) { outputProperties_.push_back(&p); }, worldPos_, viewPos_,
                            clipPos_, screenPos_);

    referenceSpace_.onChange([this]() { referenceSpaceChanged(); });
    position_.onChange([this]() { invalidateOutputProperties(); });
    if (camera_) {
        camera_->onChange([this]() { invalidateOutputProperties(); });
    }
}

PositionProperty* PositionProperty::clone() const { return new PositionProperty(*this); }

vec3 PositionProperty::get(CoordinateSpace space) const {
    return convertFromWorld(convertToWorld(position_, referenceSpace_), space);
}

void PositionProperty::set(const vec3& pos, CoordinateSpace space) {
    NetworkLock lock(this);
    referenceSpace_.set(space);
    position_.set(pos);
}

void PositionProperty::updatePosition(const vec3& pos, CoordinateSpace sourceSpace) {
    position_.set(convertFromWorld(convertToWorld(pos, sourceSpace), referenceSpace_));
}

CoordinateSpace PositionProperty::getCoordinateSpace() const { return referenceSpace_; }

vec3 PositionProperty::getWorldSpaceDirection() const {
    if (camera_ &&
        (referenceSpace_ == CoordinateSpace::View || referenceSpace_ == CoordinateSpace::Clip)) {
        return convertToWorld(position_, referenceSpace_) - camera_->getLookTo();
    }
    return position_;
}

vec3 PositionProperty::convertFromWorld(const vec3& pos, CoordinateSpace targetSpace) const {
    switch (targetSpace) {
        case CoordinateSpace::World:
            return pos;
        case CoordinateSpace::View:
            if (camera_) {
                return vec3{camera_->viewMatrix() * vec4{pos, 1.0f}};
            } else {
                return pos;
            }
            break;
        case CoordinateSpace::Clip:
            if (camera_) {
                vec4 clipCoords =
                    camera_->projectionMatrix() * camera_->viewMatrix() * vec4{pos, 1.0f};
                clipCoords /= clipCoords.w;
                return vec3{clipCoords};
            } else {
                return pos;
            }
            break;
        default:
            throw Exception(IVW_CONTEXT, "Unsupported coordinate space '{}'", targetSpace);
            break;
    }
    return vec3{0.0f};
}

vec3 PositionProperty::convertToWorld(const vec3& pos, CoordinateSpace sourceSpace) const {
    switch (sourceSpace) {
        case CoordinateSpace::World:
            return pos;
        case CoordinateSpace::View:
            if (camera_) {
                return vec3{camera_->inverseViewMatrix() * vec4{pos, 1.0f}};
            } else {
                return pos;
            }
            break;
        case CoordinateSpace::Clip:
            if (camera_) {
                return camera_->getWorldPosFromNormalizedDeviceCoords(pos);
            } else {
                return pos;
            }
            break;
        default:
            throw Exception(IVW_CONTEXT, "Unsupported coordinate space '{}'", sourceSpace);
            break;
    }
    return vec3{0.0f};
}

void PositionProperty::setPositionSemantics(PropertySemantics semantics) {
    position_.setSemantics(semantics);
}

void PositionProperty::deserialize(Deserializer& d) {
    previousReferenceSpace_.reset();
    CompositeProperty::deserialize(d);
    previousReferenceSpace_ = referenceSpace_;
}

void PositionProperty::invalidateOutputProperties() {
    for (auto p : outputProperties_) {
        p->propertyModified();
    }
}

void PositionProperty::referenceSpaceChanged() {
    NetworkLock lock(this);
    if (transformPosition_ && previousReferenceSpace_.has_value()) {
        position_.set(
            convertFromWorld(convertToWorld(position_, *previousReferenceSpace_), referenceSpace_));
    } else {
        invalidateOutputProperties();
    }
    previousReferenceSpace_ = referenceSpace_;
}

}  // namespace inviwo
