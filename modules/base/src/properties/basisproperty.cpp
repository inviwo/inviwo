/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/base/properties/basisproperty.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>

namespace inviwo {

const std::string BasisProperty::classIdentifier = "org.inviwo.VolumeBasisProperty";
std::string BasisProperty::getClassIdentifier() const { return classIdentifier; }

BasisProperty::BasisProperty(std::string identifier, std::string displayName,
                             InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , mode_("mode", "Mode",
            {{"general", "General Basis", BasisPropertyMode::General},
             {"orthogonal", "Orthogonal Basis", BasisPropertyMode::Orthogonal}},
            0, InvalidationLevel::Valid)
    , reference_("reference", "Reference",
                 {{"Volume", "Volume", BasisPropertyReference::Volume},
                  {"Voxel", "Voxel", BasisPropertyReference::Voxel}},
                 0, InvalidationLevel::Valid)
    , overRideDefaults_("override", "Override", false)
    , updateForNewEntiry_("update", "Update On New Data", true, InvalidationLevel::Valid)
    , size_("size", "Size", vec3(1.0f), vec3(0.0), vec3(std::numeric_limits<float>::max()),
            vec3{0.001f}, InvalidationLevel::Valid)
    , a_("a", "A", vec3(1.0f, 0.0f, 0.0f), vec3(std::numeric_limits<float>::lowest()),
         vec3(std::numeric_limits<float>::max()), vec3{0.001f}, InvalidationLevel::Valid)
    , b_("b", "B", vec3(0.0f, 1.0f, 0.0f), vec3(std::numeric_limits<float>::lowest()),
         vec3(std::numeric_limits<float>::max()), vec3{0.001f}, InvalidationLevel::Valid)
    , c_("c", "C", vec3(0.0f, 0.0f, 1.0f), vec3(std::numeric_limits<float>::lowest()),
         vec3(std::numeric_limits<float>::max()), vec3{0.001f}, InvalidationLevel::Valid)
    , autoCenter_("autoCenter", "Center Automatically", true, InvalidationLevel::Valid)
    , offset_("offset", "Offset", vec3(-0.5f), vec3(std::numeric_limits<float>::lowest()),
              vec3(std::numeric_limits<float>::max()), vec3{0.001f}, InvalidationLevel::Valid)
    , resetOverride_("restore", "Revert Override", InvalidationLevel::Valid)
    , overrideModel_("overrideModel", mat4{1}) {

    util::for_each_in_tuple([&](auto& e) { addProperty(e); }, props());

    size_.setVisible(false);

    mode_.onChange([&]() { onModeChange(); });
    reference_.onChange([&]() { load(); });
    overRideDefaults_.onChange([this]() { onOverrideChange(); });
    autoCenter_.onChange([&]() { onAutoCenterChange(); });
    resetOverride_.onChange([&]() { onResetOverride(); });

    util::for_each_argument(
        [this](auto& elem) {
            elem.setReadOnly(true);
            elem.setSerializationMode(PropertySerializationMode::None);
            elem.setSemantics(PropertySemantics::SpinBox);
            elem.onChange([this]() { this->save(); });
        },
        size_, a_, b_, c_, offset_);
}

BasisProperty::BasisProperty(const BasisProperty& rhs)
    : CompositeProperty(rhs)
    , mode_(rhs.mode_)
    , reference_(rhs.reference_)
    , overRideDefaults_(rhs.overRideDefaults_)
    , updateForNewEntiry_(rhs.updateForNewEntiry_)
    , size_(rhs.size_)
    , a_(rhs.a_)
    , b_(rhs.b_)
    , c_(rhs.c_)
    , autoCenter_(rhs.autoCenter_)
    , offset_(rhs.offset_)
    , resetOverride_(rhs.resetOverride_)
    , model_(rhs.model_)
    , overrideModel_(rhs.overrideModel_) {

    util::for_each_in_tuple([&](auto& e) { addProperty(e); }, props());

    mode_.onChange([&]() { onModeChange(); });
    reference_.onChange([&]() { load(); });
    overRideDefaults_.onChange([this]() { onOverrideChange(); });
    autoCenter_.onChange([&]() { onAutoCenterChange(); });
    resetOverride_.onChange([&]() { onResetOverride(); });
    util::for_each_argument([this](auto& elem) { elem.onChange([this]() { this->save(); }); },
                            size_, a_, b_, c_, offset_);
}

BasisProperty* BasisProperty::clone() const { return new BasisProperty(*this); }

void BasisProperty::updateForNewEntity(const StructuredGridEntity<3>& volume, bool deserialize) {
    dimensions_ = static_cast<vec3>(volume.getDimensions());
    update(volume, deserialize);
}

void BasisProperty::updateForNewEntity(const SpatialEntity<3>& entity, bool deserialize) {
    dimensions_ = vec3(1.0f);
    update(entity, deserialize);
}

void BasisProperty::update(const SpatialEntity<3>& entity, bool deserialize) {
    // Set basis properties to the values from the new volume
    model_ = entity.getModelMatrix();

    const auto org = overrideModel_.value;
    overrideModel_.value = entity.getModelMatrix();
    overrideModel_.setAsDefault();
    load();
    util::for_each_argument([&](auto& elem) { elem.setCurrentStateAsDefault(); }, size_, a_, b_, c_,
                            offset_);

    if (deserialize || !updateForNewEntiry_) {
        overrideModel_.value = org;
        load();
    }
}

void BasisProperty::load() {
    mat3 basis{1};
    vec3 offset{0};

    if (overRideDefaults_) {
        basis = mat3{overrideModel_.value};
        offset = vec3{overrideModel_.value[3]};
    } else {
        basis = mat3{model_};
        offset = vec3{model_[3]};
    }

    if (reference_ == BasisPropertyReference::Voxel) {
        basis /= dimensions_;
        offset /= dimensions_;
    }

    util::KeepTrueWhileInScope block(&updating_);
    a_.set(basis[0]);
    b_.set(basis[1]);
    c_.set(basis[2]);
    offset_.set(offset);
    size_.set(vec3{glm::length(basis[0]), glm::length(basis[1]), glm::length(basis[2])});
}

void BasisProperty::save() {
    if (!overRideDefaults_ || updating_) return;

    mat3 basis{1};
    vec3 offset{0};
    if (mode_ == BasisPropertyMode::Orthogonal) {
        basis = glm::diagonal3x3(size_.get());
    } else {
        basis = mat3{a_.get(), b_.get(), c_.get()};
    }

    if (autoCenter_) {
        offset = -0.5f * (basis[0] + basis[1] + basis[2]);
        util::KeepTrueWhileInScope block(&updating_);
        offset_ = offset;
    } else {
        offset = offset_.get();
    }

    if (reference_ == BasisPropertyReference::Voxel) {
        basis *= dimensions_;
        offset *= dimensions_;
    }

    mat4 model(basis);
    model[3] = vec4(offset, 1.0f);
    if (overrideModel_ != model) {
        overrideModel_ = model;
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void BasisProperty::onOverrideChange() {
    util::for_each_argument([&](auto& elem) { elem.setReadOnly(!overRideDefaults_.get()); }, size_,
                            a_, b_, c_, offset_);
    offset_.setReadOnly(autoCenter_ || !overRideDefaults_);
    load();
}

void BasisProperty::onAutoCenterChange() {
    if (autoCenter_) {
        const auto basis = mat3{getBasisAndOffset()};
        offset_ = -0.5f * (basis[0] + basis[1] + basis[2]);
    }
    offset_.setReadOnly(autoCenter_ || !overRideDefaults_);
}

void BasisProperty::onModeChange() {
    if (mode_ == BasisPropertyMode::Orthogonal) {
        const auto basis = mat3{getBasisAndOffset()};
        if (glm::dot(basis[0], basis[1]) != 0.0f || glm::dot(basis[0], basis[2]) != 0.0f ||
            glm::dot(basis[0], basis[1]) != 0.0f) {

            mode_.set(BasisPropertyMode::General);
            return;
        }
    }

    load();
    if (mode_ == BasisPropertyMode::General) {
        util::show(a_, b_, c_);
        util::hide(size_);
    } else {
        util::hide(a_, b_, c_);
        util::show(size_);
    }
}

void BasisProperty::onResetOverride() {
    if (overrideModel_ != model_) {
        overrideModel_ = model_;
        load();
        invalidate(InvalidationLevel::InvalidOutput);
    }
}

mat4 BasisProperty::getBasisAndOffset() const {
    if (overRideDefaults_) {
        return overrideModel_.value;
    } else {
        return model_;
    }
}

void BasisProperty::serialize(Serializer& s) const {
    overrideModel_.serialize(s, PropertySerializationMode::All);
    CompositeProperty::serialize(s);
}

void BasisProperty::deserialize(Deserializer& d) {
    CompositeProperty::deserialize(d);
    const bool modified = overrideModel_.deserialize(d, PropertySerializationMode::All);
    load();
    if (modified) propertyModified();
}

BasisProperty& BasisProperty::setCurrentStateAsDefault() {
    CompositeProperty::setCurrentStateAsDefault();
    overrideModel_.setAsDefault();
    return *this;
}

BasisProperty& BasisProperty::resetToDefaultState() {
    CompositeProperty::resetToDefaultState();
    overrideModel_.reset();
    load();
    return *this;
}

void BasisProperty::updateEntity(SpatialEntity<3>& volume) {
    volume.setModelMatrix(getBasisAndOffset());
}

}  // namespace inviwo
