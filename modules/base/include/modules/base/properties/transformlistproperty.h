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

#pragma once

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <glm/ext/scalar_constants.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace inviwo {

class IVW_MODULE_BASE_API TransformationList : public ListProperty {
public:
    TransformationList(std::string_view identifier, std::string_view displayName,
                       InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                       PropertySemantics semantics = PropertySemantics::Default);
    TransformationList(const TransformationList& other) = default;
    ~TransformationList() = default;

    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.TransformationList"};

    virtual TransformationList* clone() const override;

    dmat4 getMatrix() const;
};

class IVW_MODULE_BASE_API TransformListProperty : public CompositeProperty {
public:
    TransformListProperty(std::string_view identifier, std::string_view displayName, Document help,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                          PropertySemantics semantics = PropertySemantics::Default);
    TransformListProperty(std::string_view identifier, std::string_view displayName,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                          PropertySemantics semantics = PropertySemantics::Default);
    TransformListProperty(const TransformListProperty& other);
    ~TransformListProperty() = default;

    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.TransformListProperty"};

    virtual TransformListProperty* clone() const override;

    const dmat4& getMatrix() const;

    TransformationList transforms_;
    DoubleMat4Property result_;
};

namespace transform {

class IVW_MODULE_BASE_API TransformProperty : public CompositeProperty {
public:
    TransformProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    virtual ~TransformProperty() = default;

    virtual dmat4 getMatrix() const = 0;
};

class IVW_MODULE_BASE_API TranslateProperty : public TransformProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.trafo.TranslateProperty"};

    TranslateProperty(std::string_view identifier, std::string_view displayName,
                      const dvec3& value = dvec3{0.0}, const dvec3& minValue = dvec3{-1.e6},
                      const dvec3& maxValue = dvec3{1.e6}, const dvec3& increment = dvec3{0.001},
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    TranslateProperty(const TranslateProperty& rhs);
    virtual TranslateProperty* clone() const override;
    virtual ~TranslateProperty() = default;

    virtual dmat4 getMatrix() const override;

    DoubleVec3Property translate;
};

class IVW_MODULE_BASE_API RotateProperty : public TransformProperty {
public:
    enum class AngleMeasure { Radians, Degrees };

    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.trafo.RotateProperty"};

    RotateProperty(std::string_view identifier, std::string_view displayName,
                   const dvec3& axis = dvec3{1.0, 0.0, 0.0}, const double angle = 0.0,
                   const double minAngle = -glm::pi<double>(),
                   const double maxAngle = glm::pi<double>(), const double increment = 0.001,
                   AngleMeasure angleMeasure = AngleMeasure::Radians,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                   PropertySemantics semantics = PropertySemantics::Default);
    RotateProperty(const RotateProperty& rhs);
    virtual RotateProperty* clone() const override;
    virtual ~RotateProperty() = default;

    virtual dmat4 getMatrix() const override;

    OptionProperty<AngleMeasure> mode;
    DoubleVec3Property axis;
    DoubleProperty angle;
};

class IVW_MODULE_BASE_API ScaleProperty : public TransformProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.trafo.ScaleProperty"};

    ScaleProperty(std::string_view identifier, std::string_view displayName,
                  const dvec3& value = dvec3{1.0}, const dvec3& minValue = dvec3{-1.e3},
                  const dvec3& maxValue = dvec3{1.e3}, const dvec3& increment = dvec3{0.001},
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);
    ScaleProperty(const ScaleProperty& rhs);
    virtual ScaleProperty* clone() const override;
    virtual ~ScaleProperty() = default;

    virtual dmat4 getMatrix() const override;

    DoubleVec3Property scale;
};

class IVW_MODULE_BASE_API CustomTransformProperty : public TransformProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.trafo.CustomTransformProperty"};

    CustomTransformProperty(
        std::string_view identifier, std::string_view displayName, const dmat4& value = dmat4{1.0},
        const dmat4& minValue = util::filled<dmat4>(-1.e6),
        const dmat4& maxValue = util::filled<dmat4>(1.e6),
        const dmat4& increment = util::filled<dmat4>(0.001),
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    CustomTransformProperty(const CustomTransformProperty& rhs);
    virtual CustomTransformProperty* clone() const override;
    virtual ~CustomTransformProperty() = default;

    virtual dmat4 getMatrix() const override;

    DoubleMat4Property matrix;
};

class IVW_MODULE_BASE_API PortTransformProperty : public TransformProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.trafo.PortTransformProperty"};

    PortTransformProperty(std::string_view identifier, std::string_view displayName,
                          InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                          PropertySemantics semantics = PropertySemantics::Default);
    PortTransformProperty(const PortTransformProperty& rhs);
    virtual PortTransformProperty* clone() const override;
    virtual ~PortTransformProperty() = default;

    virtual dmat4 getMatrix() const override;

    virtual void setOwner(PropertyOwner* owner) override;

    DataInport<dmat4> port;
    DoubleMat4Property matrix;
};

}  // namespace transform

}  // namespace inviwo
