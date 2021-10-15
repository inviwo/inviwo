/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.TransformListProperty, Transform List Property}
 * ![](org.inviwo.TransformListProperty.png?classIdentifier=org.inviwo.TransformListProperty)
 * List of transformations being applied on each other.
 */
class IVW_MODULE_BASE_API TransformListProperty : public CompositeProperty {
public:
    TransformListProperty(const std::string& identifier, const std::string& displayName,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                          PropertySemantics semantics = PropertySemantics::Default);
    TransformListProperty(const TransformListProperty& other);
    ~TransformListProperty() = default;

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    virtual TransformListProperty* clone() const override;

    const mat4& getMatrix() const;

    ListProperty transforms_;
    FloatMat4Property result_;
};

namespace transform {

class IVW_MODULE_BASE_API TransformProperty : public CompositeProperty {
public:
    TransformProperty(const std::string& identifier, const std::string& displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    virtual ~TransformProperty() = default;

    virtual mat4 getMatrix() const = 0;
};

class IVW_MODULE_BASE_API TranslateProperty : public TransformProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    TranslateProperty(const std::string& identifier, const std::string& displayName,
                      const vec3& value = vec3{0.0f}, const vec3& minValue = vec3{-1.e6f},
                      const vec3& maxValue = vec3{1.e6f},
                      const vec3& increment = Defaultvalues<vec3>::getInc(),
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    TranslateProperty(const TranslateProperty& rhs);
    virtual TranslateProperty* clone() const override;
    virtual ~TranslateProperty() = default;

    virtual mat4 getMatrix() const override;

    FloatVec3Property translate;
};

class IVW_MODULE_BASE_API RotateProperty : public TransformProperty {
public:
    enum class AngleMeasure { Radians, Degrees };

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    RotateProperty(const std::string& identifier, const std::string& displayName,
                   const vec3& axis = vec3{1.0f, 0.0f, 0.0f}, const float angle = 0.0f,
                   const float minAngle = -glm::pi<float>(),
                   const float maxAngle = glm::pi<float>(),
                   const float increment = Defaultvalues<float>::getInc(),
                   AngleMeasure angleMeasure = AngleMeasure::Radians,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);
    RotateProperty(const RotateProperty& rhs);
    virtual RotateProperty* clone() const override;
    virtual ~RotateProperty() = default;

    virtual mat4 getMatrix() const override;

    TemplateOptionProperty<AngleMeasure> mode;
    FloatVec3Property axis;
    FloatProperty angle;
};

class IVW_MODULE_BASE_API ScaleProperty : public TransformProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ScaleProperty(const std::string& identifier, const std::string& displayName,
                  const vec3& value = vec3{1.0f}, const vec3& minValue = vec3{-1.e3f},
                  const vec3& maxValue = vec3{1.e3f},
                  const vec3& increment = Defaultvalues<vec3>::getInc(),
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);
    ScaleProperty(const ScaleProperty& rhs);
    virtual ScaleProperty* clone() const override;
    virtual ~ScaleProperty() = default;

    virtual mat4 getMatrix() const override;

    FloatVec3Property scale;
};

class IVW_MODULE_BASE_API CustomTransformProperty : public TransformProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    CustomTransformProperty(
        const std::string& identifier, const std::string& displayName,
        const mat4& value = mat4{1.0f}, const mat4& minValue = util::filled<mat4>(-1.e6f),
        const mat4& maxValue = util::filled<mat4>(1.e6f),
        const mat4& increment = Defaultvalues<mat4>::getInc(),
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    CustomTransformProperty(const CustomTransformProperty& rhs);
    virtual CustomTransformProperty* clone() const override;
    virtual ~CustomTransformProperty() = default;

    virtual mat4 getMatrix() const override;

    FloatMat4Property matrix;
};

}  // namespace transform

}  // namespace inviwo
