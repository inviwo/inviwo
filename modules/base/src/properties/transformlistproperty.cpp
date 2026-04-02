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
              return v;
          }(),
          0, ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove, invalidationLevel, semantics) {}

std::string_view TransformationList::getClassIdentifier() const { return classIdentifier; }
TransformationList* TransformationList::clone() const { return new TransformationList(*this); }

dmat4 TransformationList::getMatrix() const {
    dmat4 total{1.0};
    for (const auto& p : *this) {
        if (auto* prop = dynamic_cast<transform::TransformProperty*>(p)) {
            total = prop->getMatrix() * total;
        }
    }
    return total;
}

TransformListProperty::TransformListProperty(std::string_view identifier,
                                             std::string_view displayName, Document help,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, std::move(help), invalidationLevel,
                        std::move(semantics))
    , transforms_("internalTransforms", "Transformations")
    , result_(
          "result", "Result", dmat4(1.0),
          {util::filled<dmat4>(std::numeric_limits<double>::lowest()), ConstraintBehavior::Ignore},
          {util::filled<dmat4>(std::numeric_limits<double>::max()), ConstraintBehavior::Ignore},
          util::filled<dmat4>(0.001), InvalidationLevel::Valid, PropertySemantics::Text) {

    result_.setReadOnly(true);
    result_.setCurrentStateAsDefault();
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
    result_.setReadOnly(true);
    result_.setCurrentStateAsDefault();
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
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics) {}

std::string_view TranslateProperty::getClassIdentifier() const { return classIdentifier; }

TranslateProperty::TranslateProperty(std::string_view identifier, std::string_view displayName,
                                     const dvec3& value, const dvec3& minValue,
                                     const dvec3& maxValue, const dvec3& increment,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , translate("translate_", "Translate", value, {minValue, ConstraintBehavior::Ignore},
                {maxValue, ConstraintBehavior::Ignore}, increment) {
    translate.setSemantics(PropertySemantics::SpinBox);
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
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , mode("mode", "Angle Measure",
           {{"rad", "Radians", AngleMeasure::Radians}, {"deg", "Degrees", AngleMeasure::Degrees}})
    , axis("axis", "Axis", glm::normalize(axisValue), dvec3{-1.0}, dvec3{1.0})
    , angle("angle", "Angle", angleValue, {minAngle, ConstraintBehavior::Ignore},
            {maxAngle, ConstraintBehavior::Ignore}, increment) {
    mode.setSelectedValue(angleMeasure);
    axis.setSemantics(PropertySemantics::SpinBox);
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
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , scale("scale", "Scale", value, {minValue, ConstraintBehavior::Ignore},
            {maxValue, ConstraintBehavior::Ignore}, increment) {
    scale.setSemantics(PropertySemantics::SpinBox);
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
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , matrix("matrix", "Matrix", value, {minValue, ConstraintBehavior::Ignore},
             {maxValue, ConstraintBehavior::Ignore}, increment) {
    matrix.setSemantics(PropertySemantics::SpinBox);
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
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , port{"inport"}
    , matrix{"matrix",
             "Matrix",
             dmat4{1.0},
             {util::filled<dmat4>(-1.e6), ConstraintBehavior::Ignore},
             {util::filled<dmat4>(1.e6), ConstraintBehavior::Ignore},
             util::filled<dmat4>(0.001),
             InvalidationLevel::InvalidResources} {
    matrix.setReadOnly(true);

    addProperties(matrix);

    port.onChange([this]() {
        if (auto data = port.getData()) {
            matrix.set(*data);
        } else {
            matrix.set(dmat4{1.0});
        }
    });
}

PortTransformProperty::PortTransformProperty(const PortTransformProperty& rhs)
    : TransformProperty(rhs), port("inport"), matrix(rhs.matrix) {

    addProperties(matrix);

    port.onChange([this]() {
        if (auto data = port.getData()) {
            matrix.set(*data);
        } else {
            matrix.set(dmat4{1.0});
        }
    });
}

PortTransformProperty* PortTransformProperty::clone() const {
    return new PortTransformProperty(*this);
}

void PortTransformProperty::setOwner(PropertyOwner* owner) {
    TransformProperty::setOwner(owner);

    if (owner && owner->getProcessor()) {
        owner->getProcessor()->addPort(port);
    }
}

dmat4 PortTransformProperty::getMatrix() const {
    if (auto data = port.getData()) {
        return *data;
    } else {
        return dmat4{1.0};
    }
}

}  // namespace transform

}  // namespace inviwo
