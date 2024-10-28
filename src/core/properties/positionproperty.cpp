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
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>

namespace inviwo {

const std::string PositionProperty::classIdentifier = "org.inviwo.PositionProperty";
std::string_view PositionProperty::getClassIdentifier() const { return classIdentifier; }

PositionProperty::PositionProperty(std::string_view identifier, std::string_view displayName,
                                   Document help, const vec3& position,
                                   CoordinateSpace coordinateSpace, CameraProperty* camera,
                                   PropertySemantics positionSemantics,
                                   InvalidationLevel invalidationLevel)
    : CompositeProperty{identifier, displayName, std::move(help), invalidationLevel}
    , camera_{camera}
    , previousReferenceSpace_{coordinateSpace}
    , referenceSpace_{"referenceFrame", "Space",
                      OptionPropertyState<CoordinateSpace>{
                          .options = {{"world", "World coordinates", CoordinateSpace::World},
                                      {"view", "View coordinates", CoordinateSpace::View},
                                      {"clip", "Clip coordinates", CoordinateSpace::Clip}}}
                          .set("Reference coordinate space for the position below."_help)
                          .setSelectedValue(coordinateSpace)}
    , position_{"position", "Position",
                util::ordinalSymmetricVector(position, 10.0f)
                    .set("Spatial position in the reference coordinate space"_help)
                    .set(positionSemantics)}
    , coordinateOffsetMode_("coordinateOffsetMode", "Offset",
                            "Preset for the positional offset below."_help,
                            {{"none", "No offset", CoordinateOffset::None},
                             {"cameraLookAt", "Camera look-at", CoordinateOffset::CameraLookAt},
                             {"custom", "Custom", CoordinateOffset::Custom}})
    , coordinateOffset_{"coordinateOffset", "Offset",
                        util::ordinalSymmetricVector(vec3{0.0f}, 10.0f)
                            .set("The offset is applied to the position in the same reference "
                                 "space. This "
                                 "allows for example to move a light source around a volume or "
                                 "mesh while "
                                 "being centered in the middle of the volume."_help)}

    , output_{"output", "Output"}
    , worldPos_{"worldPosition", "World Position",
                [&]() -> vec3 { return get(CoordinateSpace::World); }, [&](const vec3&) {},
                util::ordinalRefSymmetricVector<vec3>(100.0f)
                    .set("Position in world space coordinates"_help)
                    .set(PropertySemantics::Text)
                    .set(ReadOnly::Yes)}
    , viewPos_{"viewPosition", "View Position",
               [&]() -> vec3 { return get(CoordinateSpace::View); }, [&](const vec3&) {},
               util::ordinalRefSymmetricVector<vec3>(100.0f)
                   .set("Position in view space coordinates of the camera"_help)
                   .set(PropertySemantics::Text)
                   .set(ReadOnly::Yes)}
    , clipPos_{"clipPosition", "Clip Position",
               [&]() -> vec3 { return get(CoordinateSpace::Clip); }, [&](const vec3&) {},
               util::ordinalRefSymmetricVector<vec3>(1.0f)
                   .set("Position in clip space coordinates [-1,1]"_help)
                   .set(PropertySemantics::Text)
                   .set(ReadOnly::Yes)}
    , screenPos_{"screenPosition", "Screen Position (normalized)",
                 [&]() -> vec3 { return get(CoordinateSpace::Clip) * 0.5f + 0.5f; },
                 [&](const vec3&) {},
                 OrdinalRefPropertyState<vec3>{.min = vec3{0.0f},
                                               .minConstraint = ConstraintBehavior::Ignore,
                                               .max = vec3{1.0f},
                                               .maxConstraint = ConstraintBehavior::Ignore}
                     .set("Position in clip space coordinates [-1,1]"_help)
                     .set(PropertySemantics::Text)
                     .set(ReadOnly::Yes)} {

    output_.addProperties(worldPos_, viewPos_, clipPos_, screenPos_);
    output_.setCollapsed(true);
    output_.setCurrentStateAsDefault();
    addProperties(referenceSpace_, position_, coordinateOffsetMode_, coordinateOffset_, output_);

    referenceSpace_.onChange([this]() { referenceSpaceChanged(); });
    position_.onChange([this]() { invalidateOutputProperties(); });
    coordinateOffsetMode_.onChange([this]() { invalidateOutputProperties(); });
    coordinateOffset_.onChange([this]() { invalidateOutputProperties(); });
    coordinateOffset_.readonlyDependsOn(
        coordinateOffsetMode_, [](auto& prop) { return prop.get() != CoordinateOffset::Custom; });
    if (camera_) {
        camera_->onChange([this]() { invalidateOutputProperties(); });
    }
}

PositionProperty::PositionProperty(std::string_view identifier, std::string_view displayName,
                                   const vec3& position, CoordinateSpace coordinateSpace,
                                   CameraProperty* camera, PropertySemantics positionSemantics,
                                   InvalidationLevel invalidationLevel)
    : PositionProperty(identifier, displayName, Document{}, position, coordinateSpace, camera,
                       positionSemantics, invalidationLevel) {}

PositionProperty::PositionProperty(const PositionProperty& rhs)
    : CompositeProperty{rhs}
    , camera_{rhs.camera_}
    , previousReferenceSpace_{rhs.previousReferenceSpace_}
    , referenceSpace_{rhs.referenceSpace_}
    , position_{rhs.position_}
    , coordinateOffsetMode_{rhs.coordinateOffsetMode_}
    , coordinateOffset_{rhs.coordinateOffset_}

    , output_{rhs.output_}
    , worldPos_{rhs.worldPos_}
    , viewPos_{rhs.viewPos_}
    , clipPos_{rhs.clipPos_}
    , screenPos_{rhs.screenPos_} {

    output_.addProperties(worldPos_, viewPos_, clipPos_, screenPos_);
    addProperties(referenceSpace_, position_, coordinateOffsetMode_, coordinateOffset_, output_);

    referenceSpace_.onChange([this]() { referenceSpaceChanged(); });
    position_.onChange([this]() { invalidateOutputProperties(); });
    coordinateOffsetMode_.onChange([this]() { invalidateOutputProperties(); });
    coordinateOffset_.onChange([this]() { invalidateOutputProperties(); });
    coordinateOffset_.setReadOnly(coordinateOffsetMode_.get() != CoordinateOffset::Custom);
    coordinateOffset_.readonlyDependsOn(
        coordinateOffsetMode_, [](auto& prop) { return prop.get() != CoordinateOffset::Custom; });
    if (camera_) {
        camera_->onChange([this]() { invalidateOutputProperties(); });
    }
}

PositionProperty* PositionProperty::clone() const { return new PositionProperty(*this); }

vec3 PositionProperty::get(CoordinateSpace space) const {
    return convert(position_.get() + getOffset(referenceSpace_), referenceSpace_, space);
}

void PositionProperty::set(const vec3& pos, CoordinateSpace space, ApplyOffset applyOffset) {
    NetworkLock lock(this);
    referenceSpace_.set(space);
    const vec3 offset{applyOffset == ApplyOffset::Yes ? getOffset(space) : vec3{0.0f}};
    position_.set(pos + offset);
}

void PositionProperty::updatePosition(const vec3& pos, CoordinateSpace sourceSpace) {
    position_.set(convert(pos, sourceSpace, referenceSpace_));
}

vec3 PositionProperty::getOffset(CoordinateSpace space) const {
    switch (coordinateOffsetMode_) {
        case CoordinateOffset::None:
            return convert(vec3{0.0f}, referenceSpace_, space);
        case CoordinateOffset::CameraLookAt:
            if (camera_) {
                return convertFromWorld(camera_->getLookTo(), space);
            } else {
                return convert(vec3{0.0f}, referenceSpace_, space);
            }
        case CoordinateOffset::Custom:
            return convert(coordinateOffset_, referenceSpace_, space);
        default:
            return convert(vec3{0.0f}, referenceSpace_, space);
    }
}

CoordinateSpace PositionProperty::getCoordinateSpace() const { return referenceSpace_; }

vec3 PositionProperty::getDirection(CoordinateSpace space) const {
    const vec3 v = get(space) - getOffset(space);
    return glm::normalize(v);
}

vec3 PositionProperty::convert(const vec3& pos, CoordinateSpace sourceSpace,
                               CoordinateSpace targetSpace) const {
    if (sourceSpace == targetSpace) {
        return pos;
    }
    return convertFromWorld(convertToWorld(pos, sourceSpace), targetSpace);
}

vec3 PositionProperty::convertFromWorld(const vec3& pos, CoordinateSpace targetSpace) const {
    if (!camera_) {
        return pos;
    }
    SpatialIdentity identity;
    SpatialCameraCoordinateTransformerImpl transformer{identity, camera_->get()};
    return transformer.transformPosition(pos, CoordinateSpace::World, targetSpace);
}

vec3 PositionProperty::convertToWorld(const vec3& pos, CoordinateSpace sourceSpace) const {
    if (!camera_) {
        return pos;
    }
    SpatialIdentity identity;
    SpatialCameraCoordinateTransformerImpl transformer{identity, camera_->get()};
    return transformer.transformPosition(pos, sourceSpace, CoordinateSpace::World);
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
    for (auto& p : util::ref<Property>(worldPos_, viewPos_, clipPos_, screenPos_)) {
        p.get().propertyModified();
    }
}

void PositionProperty::referenceSpaceChanged() {
    NetworkLock lock(this);
    if (previousReferenceSpace_.has_value()) {
        position_.set(convert(position_, *previousReferenceSpace_, referenceSpace_));
    } else {
        invalidateOutputProperties();
    }
    previousReferenceSpace_ = referenceSpace_;
}

}  // namespace inviwo
