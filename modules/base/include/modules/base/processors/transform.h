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

#pragma once

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

namespace inviwo {

template <typename T>
class Transform : public Processor {
public:
    Transform();
    virtual ~Transform() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;

protected:
    DataInport<T> inport_;
    DataOutport<T> outport_;

    TemplateOptionProperty<CoordinateSpace> space_;
    BoolProperty replace_;
    ListProperty transforms_;
    FloatMat4Property result_;
};

template <typename T>
const ProcessorInfo Transform<T>::getProcessorInfo() const {
    return ProcessorTraits<Transform<T>>::getProcessorInfo();
}

/** \docpage{org.inviwo.TransformMesh, Transform Mesh}
 * ![](org.inviwo.TransformMesh.png?classIdentifier=org.inviwo.TransformMesh)
 * Apply a model or world transformation to a mesh.
 */
class Mesh;
template <>
struct ProcessorTraits<Transform<Mesh>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.TransformMesh",  // Class identifier
            "Transform Mesh",            // Display name
            "Coordinate Transforms",     // Category
            CodeState::Stable,           // Code state
            "Mesh, Transform"            // Tags
        };
    }
};

/** \docpage{org.inviwo.TransformVolume, Transform Volume}
 * ![](org.inviwo.TransformVolume.png?classIdentifier=org.inviwo.TransformVolume)
 * Apply a model or world transformation to a volume.
 */
class Volume;
template <>
struct ProcessorTraits<Transform<Volume>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.TransformVolume",  // Class identifier
            "Transform Volume",            // Display name
            "Coordinate Transforms",       // Category
            CodeState::Stable,             // Code state
            "Volume, Transform"            // Tags
        };
    }
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

template <typename T>
Transform<T>::Transform()
    : Processor()
    , inport_("inport_")
    , outport_("outport_")
    , space_(
          "space", "Space",
          {{"model", "Model", CoordinateSpace::Model}, {"world", "World", CoordinateSpace::World}},
          1)
    , replace_("replace", "Replace Input Transformation", false)
    , transforms_(
          "transforms", "Transformations",
          []() {
              std::vector<std::unique_ptr<Property>> v;
              v.emplace_back(
                  std::make_unique<transform::TranslateProperty>("translation", "Translation"));
              v.emplace_back(std::make_unique<transform::RotateProperty>("rotation", "Rotation"));
              v.emplace_back(std::make_unique<transform::ScaleProperty>("scaling", "Scaling"));
              v.emplace_back(
                  std::make_unique<transform::CustomTransformProperty>("custom", "Custom Matrix"));
              return v;
          }())
    , result_("result", "Result", mat4(1.0f),
              util::filled<mat4>(std::numeric_limits<float>::lowest()),
              util::filled<mat4>(std::numeric_limits<float>::max()), util::filled<mat4>(0.001f),
              InvalidationLevel::Valid) {

    addPort(inport_);
    addPort(outport_);

    result_.setSemantics(PropertySemantics::Text);
    result_.setReadOnly(true);

    addProperties(space_, replace_, transforms_, result_);

    transforms_.onChange([this]() {
        mat4 t{1.0f};
        for (auto p : transforms_) {
            if (auto prop = dynamic_cast<transform::TransformProperty*>(p)) {
                t = prop->getMatrix() * t;
            }
        }
        result_.set(t);
    });
}

template <typename T>
void Transform<T>::process() {
    std::shared_ptr<T> data(inport_.getData()->clone());

    switch (*space_) {
        case CoordinateSpace::Model:
            if (replace_) {
                data->setModelMatrix(*result_);
            } else {
                data->setModelMatrix(*result_ * data->getModelMatrix());
            }
            break;
        case CoordinateSpace::World:
        default:
            if (replace_) {
                data->setWorldMatrix(*result_);
            } else {
                data->setWorldMatrix(*result_ * data->getWorldMatrix());
            }
            break;
    }

    outport_.setData(data);
}

}  // namespace inviwo
