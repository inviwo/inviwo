/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <modules/base/properties/transformlistproperty.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>

#include <limits>
#include <memory>

#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <utility>

namespace inviwo {

TransformationList::TransformationList(std::string_view identifier, std::string_view displayName,
                                       InvalidationLevel invalidationLevel,
                                       PropertySemantics semantics)
    : ListProperty(
          identifier, displayName,
          []() {
              std::vector<std::unique_ptr<Property>> v;
              v.emplace_back(
                  std::make_unique<transform::TranslateProperty>("translation", "Translation"));
              v.emplace_back(std::make_unique<transform::RotateProperty>("rotation", "Rotation"));
              v.emplace_back(std::make_unique<transform::ScaleProperty>("scaling", "Scaling"));
              v.emplace_back(
                  std::make_unique<transform::CustomTransformProperty>("custom", "Custom Matrix"));
              v.emplace_back(
                  std::make_unique<transform::PortTransformProperty>("port", "Inport Matrix"));
              v.emplace_back(std::make_unique<transform::SpatialEntityTransformProperty>(
                  "spatial", "Spatial Entity"));
              return v;
          }(),
          0, ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove, invalidationLevel,
          std::move(semantics)) {}

std::string_view TransformationList::getClassIdentifier() const { return classIdentifier; }
TransformationList* TransformationList::clone() const { return new TransformationList(*this); }

dmat4 TransformationList::getMatrix() const {
    dmat4 total{1.0};
    for (const auto& p : *this) {
        if (const auto* prop = dynamic_cast<transform::TransformProperty*>(p)) {
            total = prop->getMatrix() * total;
        }
    }
    return total;
}

TransformListProperty::TransformListProperty(std::string_view identifier,
                                             std::string_view displayName, Document help,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, std::move(help), invalidationLevel,
                        std::move(semantics)}
    , transforms_{"internalTransforms", "Transformations"}
    , result_{"result", "Result",
              util::ordinalSymmetricVector(dmat4{1.0}, util::filled<dmat4>(100.0))
                  .setInc(util::filled<dmat4>(0.001))
                  .set(InvalidationLevel::Valid)
                  .set(ReadOnly::Yes)} {

    addProperties(transforms_, result_);

    transforms_.onChange([this]() { result_.set(transforms_.getMatrix()); });
}
TransformListProperty::TransformListProperty(std::string_view identifier,
                                             std::string_view displayName,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : TransformListProperty(identifier, displayName,
                            "List of transformations being applied on each other."_help,
                            invalidationLevel, std::move(semantics)) {}

TransformListProperty::TransformListProperty(const TransformListProperty& other)
    : CompositeProperty(other), transforms_(other.transforms_), result_(other.result_) {
    addProperties(transforms_, result_);

    transforms_.onChange([this]() { result_.set(transforms_.getMatrix()); });
}

std::string_view TransformListProperty::getClassIdentifier() const { return classIdentifier; }

TransformListProperty* TransformListProperty::clone() const {
    return new TransformListProperty(*this);
}

const dmat4& TransformListProperty::getMatrix() const { return result_.get(); }

namespace transform {

TransformProperty::TransformProperty(std::string_view identifier, std::string_view displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, std::move(semantics)) {}

std::string_view TranslateProperty::getClassIdentifier() const { return classIdentifier; }

TranslateProperty::TranslateProperty(std::string_view identifier, std::string_view displayName,
                                     const dvec3& value, const dvec3& minValue,
                                     const dvec3& maxValue, const dvec3& increment,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, std::move(semantics))
    , translate("translate_", "Translate", value, {minValue, ConstraintBehavior::Ignore},
                {maxValue, ConstraintBehavior::Ignore}, increment) {
    addProperty(translate);
}

TranslateProperty::TranslateProperty(const TranslateProperty& rhs)
    : TransformProperty(rhs), translate(rhs.translate) {
    addProperty(translate);
}

TranslateProperty* TranslateProperty::clone() const { return new TranslateProperty(*this); }

dmat4 TranslateProperty::getMatrix() const { return glm::translate(*translate); }

std::string_view RotateProperty::getClassIdentifier() const { return classIdentifier; }

RotateProperty::RotateProperty(std::string_view identifier, std::string_view displayName,
                               const dvec3& axisValue, const double angleValue,
                               const double minAngle, const double maxAngle, const double increment,
                               AngleMeasure angleMeasure, InvalidationLevel invalidationLevel,
                               PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, std::move(semantics))
    , mode("mode", "Angle Measure",
           OptionPropertyState<AngleMeasure>{.options = {{"rad", "Radians", AngleMeasure::Radians},
                                                         {"deg", "Degrees", AngleMeasure::Degrees}}}
               .setSelectedValue(angleMeasure))
    , axis("axis", "Axis", glm::normalize(axisValue), dvec3{-1.0}, dvec3{1.0})
    , angle("angle", "Angle", angleValue, {minAngle, ConstraintBehavior::Ignore},
            {maxAngle, ConstraintBehavior::Ignore}, increment) {
    addProperties(mode, axis, angle);

    mode.onChange([this]() {
        if (mode == AngleMeasure::Radians) {
            angle.set(glm::radians(*angle), glm::radians(angle.getMinValue()),
                      glm::radians(angle.getMaxValue()), angle.getIncrement());
        } else {
            angle.set(glm::degrees(*angle), glm::degrees(angle.getMinValue()),
                      glm::degrees(angle.getMaxValue()), angle.getIncrement());
        }
    });
}

RotateProperty::RotateProperty(const RotateProperty& rhs)
    : TransformProperty(rhs), mode(rhs.mode), axis(rhs.axis), angle(rhs.angle) {
    addProperties(mode, axis, angle);

    mode.onChange([this]() {
        if (mode == AngleMeasure::Radians) {
            angle.set(glm::radians(*angle), glm::radians(angle.getMinValue()),
                      glm::radians(angle.getMaxValue()), angle.getIncrement());
        } else {
            angle.set(glm::degrees(*angle), glm::degrees(angle.getMinValue()),
                      glm::degrees(angle.getMaxValue()), angle.getIncrement());
        }
    });
}

RotateProperty* RotateProperty::clone() const { return new RotateProperty(*this); }

dmat4 RotateProperty::getMatrix() const {
    const double angleRad = (mode == AngleMeasure::Radians ? angle : glm::radians(*angle));
    return glm::rotate(angleRad, *axis);
}

std::string_view ScaleProperty::getClassIdentifier() const { return classIdentifier; }

ScaleProperty::ScaleProperty(std::string_view identifier, std::string_view displayName,
                             const dvec3& value, const dvec3& minValue, const dvec3& maxValue,
                             const dvec3& increment, InvalidationLevel invalidationLevel,
                             PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, std::move(semantics))
    , scale("scale", "Scale", value, {minValue, ConstraintBehavior::Ignore},
            {maxValue, ConstraintBehavior::Ignore}, increment) {
    addProperty(scale);
}

ScaleProperty::ScaleProperty(const ScaleProperty& rhs) : TransformProperty(rhs), scale(rhs.scale) {
    addProperty(scale);
}

ScaleProperty* ScaleProperty::clone() const { return new ScaleProperty(*this); }

dmat4 ScaleProperty::getMatrix() const { return glm::scale(*scale); }

std::string_view CustomTransformProperty::getClassIdentifier() const { return classIdentifier; }

CustomTransformProperty::CustomTransformProperty(std::string_view identifier,
                                                 std::string_view displayName, const dmat4& value,
                                                 const dmat4& minValue, const dmat4& maxValue,
                                                 const dmat4& increment,
                                                 InvalidationLevel invalidationLevel,
                                                 PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, std::move(semantics))
    , matrix("matrix", "Matrix", value, {minValue, ConstraintBehavior::Ignore},
             {maxValue, ConstraintBehavior::Ignore}, increment) {
    addProperty(matrix);
}

CustomTransformProperty::CustomTransformProperty(std::string_view identifier,
                                                 std::string_view displayName,
                                                 const OrdinalPropertyState<dmat4>& state)
    : TransformProperty{identifier, displayName, state.invalidationLevel, state.semantics}
    , matrix{"matrix", "Matrix", state} {
    addProperty(matrix);
}

CustomTransformProperty::CustomTransformProperty(const CustomTransformProperty& rhs)
    : TransformProperty(rhs), matrix(rhs.matrix) {
    addProperty(matrix);
}

CustomTransformProperty* CustomTransformProperty::clone() const {
    return new CustomTransformProperty(*this);
}

dmat4 CustomTransformProperty::getMatrix() const { return matrix; }

std::string_view PortTransformProperty::getClassIdentifier() const { return classIdentifier; }

PortTransformProperty::PortTransformProperty(std::string_view identifier,
                                             std::string_view displayName,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, std::move(semantics))
    , port{"inport"}
    , matrix{"matrix", "Matrix",
             util::ordinalSymmetricVector(dmat4{1.0}, util::filled<dmat4>(100.0))
                 .setInc(util::filled<dmat4>(0.001))
                 .set(InvalidationLevel::InvalidResources)
                 .set(ReadOnly::Yes)} {
    addProperties(matrix);

    port.onChange([this]() { matrix.set(getMatrix()); });
}

PortTransformProperty::PortTransformProperty(const PortTransformProperty& rhs)
    : TransformProperty(rhs), port("inport"), matrix(rhs.matrix) {

    addProperties(matrix);

    port.onChange([this]() { matrix.set(getMatrix()); });
}

PortTransformProperty* PortTransformProperty::clone() const {
    return new PortTransformProperty(*this);
}

void PortTransformProperty::setOwner(PropertyOwner* owner) {
    if (auto* prevOwner = getOwner(); prevOwner && prevOwner->getProcessor()) {
        prevOwner->getProcessor()->removePort(&port);
    }

    TransformProperty::setOwner(owner);

    if (owner && owner->getProcessor()) {
        auto* processor = owner->getProcessor();
        port.setIdentifier(util::findUniqueIdentifier(
            port.getIdentifier(),
            [&](std::string_view id) { return processor->getInport(id) == nullptr; }, ""));

        owner->getProcessor()->addPort(port);
    }
}

dmat4 PortTransformProperty::getMatrix() const {
    if (const auto data = port.getData()) {
        return *data;
    } else {
        return dmat4{1.0};
    }
}
std::string_view SpatialEntityTransformProperty::getClassIdentifier() const {
    return classIdentifier;
}

SpatialEntityTransformProperty::SpatialEntityTransformProperty(std::string_view identifier,
                                                               std::string_view displayName,
                                                               InvalidationLevel invalidationLevel,
                                                               PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, std::move(semantics))
    , port{"inport"}
    , space{"space",
            "Space",
            {{"world", "World", 0}, {"model", "Model", 1}, {"modelWorld", "World * Model", 2}}}
    , matrix{"matrix", "Matrix",
             util::ordinalSymmetricVector(dmat4{1.0}, util::filled<dmat4>(100.0))
                 .setInc(util::filled<dmat4>(0.001))
                 .set(InvalidationLevel::InvalidResources)
                 .set(ReadOnly::Yes)} {
    addProperties(space, matrix);

    port.onChange([this]() { matrix.set(getMatrix()); });
}

SpatialEntityTransformProperty::SpatialEntityTransformProperty(
    const SpatialEntityTransformProperty& rhs)
    : TransformProperty{rhs}, port{"inport"}, space{rhs.space}, matrix{rhs.matrix} {

    addProperties(space, matrix);

    port.onChange([this]() { matrix.set(getMatrix()); });
}

SpatialEntityTransformProperty* SpatialEntityTransformProperty::clone() const {
    return new SpatialEntityTransformProperty(*this);
}

void SpatialEntityTransformProperty::setOwner(PropertyOwner* owner) {
    if (auto* prevOwner = getOwner(); prevOwner && prevOwner->getProcessor()) {
        prevOwner->getProcessor()->removePort(&port);
    }

    TransformProperty::setOwner(owner);

    if (owner && owner->getProcessor()) {
        auto* processor = owner->getProcessor();
        port.setIdentifier(util::findUniqueIdentifier(
            port.getIdentifier(),
            [&](std::string_view id) { return processor->getInport(id) == nullptr; }, ""));

        owner->getProcessor()->addPort(port);
    }
}

dmat4 SpatialEntityTransformProperty::getMatrix() const {
    if (const auto data = port.getData()) {
        switch (space) {
            case 0:  // world
                return data->getWorldMatrix();
            case 1:  // model
                return data->getModelMatrix();
            case 2:  // world * model
                return data->getWorldMatrix() * data->getModelMatrix();
            default: return dmat4{1.0}; break;
        }
    } else {
        return dmat4{1.0};
    }
}

}  // namespace transform

}  // namespace inviwo
