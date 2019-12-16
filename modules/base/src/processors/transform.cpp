/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/processors/transform.h>

namespace inviwo {

namespace transform {

TransformProperty::TransformProperty(const std::string& identifier, const std::string& displayName,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics) {}

const std::string TranslateProperty::classIdentifier = "org.inviwo.trafo.TranslateProperty";
std::string TranslateProperty::getClassIdentifier() const { return classIdentifier; }

TranslateProperty::TranslateProperty(const std::string& identifier, const std::string& displayName,
                                     const vec3& value, const vec3& minValue, const vec3& maxValue,
                                     const vec3& increment, InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , translate("translate_", "Translate", value, minValue, maxValue, increment) {
    translate.setSemantics(PropertySemantics::SpinBox);
    addProperty(translate);
}

TranslateProperty::TranslateProperty(const TranslateProperty& rhs)
    : TransformProperty(rhs), translate(rhs.translate) {
    addProperty(translate);
}

TranslateProperty* TranslateProperty::clone() const { return new TranslateProperty(*this); }

mat4 TranslateProperty::getMatrix() const { return glm::translate(*translate); }

const std::string RotateProperty::classIdentifier = "org.inviwo.trafo.RotateProperty";
std::string RotateProperty::getClassIdentifier() const { return classIdentifier; }

RotateProperty::RotateProperty(const std::string& identifier, const std::string& displayName,
                               const vec3& axisValue, const float angleValue, const float minAngle,
                               const float maxAngle, const float increment,
                               AngleMeasure angleMeasure, InvalidationLevel invalidationLevel,
                               PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , mode("mode", "Angle Measure",
           {{"rad", "Radians", AngleMeasure::Radians}, {"deg", "Degrees", AngleMeasure::Degrees}})
    , axis("axis", "Axis", glm::normalize(axisValue), vec3{-1.0f}, vec3{1.0f})
    , angle("angle", "Angle", angleValue, minAngle, maxAngle, increment) {
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

mat4 RotateProperty::getMatrix() const {
    const float angleRad = (mode == AngleMeasure::Radians ? angle : glm::radians(*angle));
    return glm::rotate(angleRad, *axis);
}

const std::string ScaleProperty::classIdentifier = "org.inviwo.trafo.ScaleProperty";
std::string ScaleProperty::getClassIdentifier() const { return classIdentifier; }

ScaleProperty::ScaleProperty(const std::string& identifier, const std::string& displayName,
                             const vec3& value, const vec3& minValue, const vec3& maxValue,
                             const vec3& increment, InvalidationLevel invalidationLevel,
                             PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , scale("scale", "Scale", value, minValue, maxValue, increment) {
    scale.setSemantics(PropertySemantics::SpinBox);
    addProperty(scale);
}

ScaleProperty::ScaleProperty(const ScaleProperty& rhs) : TransformProperty(rhs), scale(rhs.scale) {
    addProperty(scale);
}

ScaleProperty* ScaleProperty::clone() const { return new ScaleProperty(*this); }

mat4 ScaleProperty::getMatrix() const { return glm::scale(*scale); }

const std::string CustomTransformProperty::classIdentifier =
    "org.inviwo.trafo.CustomTransformProperty";
std::string CustomTransformProperty::getClassIdentifier() const { return classIdentifier; }

CustomTransformProperty::CustomTransformProperty(const std::string& identifier,
                                                 const std::string& displayName, const mat4& value,
                                                 const mat4& minValue, const mat4& maxValue,
                                                 const mat4& increment,
                                                 InvalidationLevel invalidationLevel,
                                                 PropertySemantics semantics)
    : TransformProperty(identifier, displayName, invalidationLevel, semantics)
    , matrix("matrix", "Matrix", value, minValue, maxValue, increment) {
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

mat4 CustomTransformProperty::getMatrix() const { return matrix; }

}  // namespace transform

}  // namespace inviwo
