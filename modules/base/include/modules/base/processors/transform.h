/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>  // for CoordinateSpace, Coordinat...
#include <inviwo/core/ports/datainport.h>                      // for DataInport
#include <inviwo/core/ports/dataoutport.h>                     // for DataOutport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>              // for Tags
#include <inviwo/core/processors/processortraits.h>            // for ProcessorTraits
#include <inviwo/core/properties/boolproperty.h>               // for BoolProperty
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty, OptionProp...
#include <modules/base/properties/transformlistproperty.h>     // for TransformListProperty

#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

template <typename T>
class Transform : public Processor {
public:
    Transform();
    virtual ~Transform() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;

protected:
    enum class Mode {
        WorldModelTransform,
        World_TransformModel,
        WorldTransform_Model,
        TransformWorldModel,
        WorldTransform,
        TransformModel
    };

    DataInport<T> inport_;
    DataOutport<T> outport_;

    OptionProperty<Mode> space_;
    TransformListProperty transforms_;
};

template <typename T>
const ProcessorInfo Transform<T>::getProcessorInfo() const {
    return ProcessorTraits<Transform<T>>::getProcessorInfo();
}

class Layer;

template <>
struct ProcessorTraits<Transform<Layer>> {
    static ProcessorInfo getProcessorInfo() {
        return {"org.inviwo.TransformLayer",                  // Class identifier
                "Transform Layer",                            // Display name
                "Coordinate Transforms",                      // Category
                CodeState::Stable,                            // Code state
                Tags::CPU | Tag{"Transform"} | Tag{"Layer"},  // Tags
                R"(Apply a transformation to a Layer.
    The transform can be applied in a number of different ways:
    * World * Model * Transform     Apply the transform first, this will update the Model Matrix
    * World * (Transform * Model)   Apply the transform between model and world, this will update the Model Matrix
    * (World * Transform) * Model   Apply the transform between model and world, this will update the World Matrix
    * Transform * World * Model     Apply the transform last, this will update the World Matrix
    * World * Transform             Replace the existing Model Matrix
    * Transform * Model             Replace the existing World Matrix)"_unindentHelp};
    }
};

class Mesh;

template <>
struct ProcessorTraits<Transform<Mesh>> {
    static ProcessorInfo getProcessorInfo() {
        return {"org.inviwo.TransformMesh",                  // Class identifier
                "Transform Mesh",                            // Display name
                "Coordinate Transforms",                     // Category
                CodeState::Stable,                           // Code state
                Tags::CPU | Tag{"Transform"} | Tag{"Mesh"},  // Tags
                R"(Apply a transformation to a Mesh.
    The transform can be applied in a number of different ways:
    * World * Model * Transform     Apply the transform first, this will update the Model Matrix
    * World * (Transform * Model)   Apply the transform between model and world, this will update the Model Matrix
    * (World * Transform) * Model   Apply the transform between model and world, this will update the World Matrix
    * Transform * World * Model     Apply the transform last, this will update the World Matrix
    * World * Transform             Replace the existing Model Matrix
    * Transform * Model             Replace the existing World Matrix)"_unindentHelp};
    }
};

class Volume;

template <>
struct ProcessorTraits<Transform<Volume>> {
    static ProcessorInfo getProcessorInfo() {
        return {"org.inviwo.TransformVolume",                  // Class identifier
                "Transform Volume",                            // Display name
                "Coordinate Transforms",                       // Category
                CodeState::Stable,                             // Code state
                Tags::CPU | Tag{"Transform"} | Tag{"Volume"},  // Tags
                R"(Apply a transformation to a Volume.
    The transform can be applied in a number of different ways:
    * World * Model * Transform     Apply the transform first, this will update the Model Matrix
    * World * (Transform * Model)   Apply the transform between model and world, this will update the Model Matrix
    * (World * Transform) * Model   Apply the transform between model and world, this will update the World Matrix
    * Transform * World * Model     Apply the transform last, this will update the World Matrix
    * World * Transform             Replace the existing Model Matrix
    * Transform * Model             Replace the existing World Matrix)"_unindentHelp};
    }
};

template <typename T>
Transform<T>::Transform()
    : Processor()
    , inport_("inport_")
    , outport_("outport_")
    , space_("space", "Space",
             {{"worldModelTransform", "World * (Model * Transform)", Mode::WorldModelTransform},
              {"world_TransformModel", "World * (Transform * Model)", Mode::World_TransformModel},
              {"worldTransform_Model", "(World * Transform) * Model", Mode::WorldTransform_Model},
              {"transformWorldModel", "(Transform * World) * Model", Mode::TransformWorldModel},
              {"worldTransform", "World * Transform", Mode::WorldTransform},
              {"transformModel", "Transform * Model", Mode::TransformModel}},
             3)
    , transforms_("transformations", "Transformation Stack") {

    addPort(inport_);
    addPort(outport_);

    addProperties(space_, transforms_);
}

template <typename T>
void Transform<T>::process() {
    std::shared_ptr<T> data(inport_.getData()->clone());

    switch (*space_) {
        case Mode::WorldModelTransform:
            data->setModelMatrix(data->getModelMatrix() * transforms_.getMatrix());
            break;

        case Mode::World_TransformModel:
            data->setModelMatrix(transforms_.getMatrix() * data->getModelMatrix());
            break;

        case Mode::WorldTransform_Model:
            data->setWorldMatrix(data->getWorldMatrix() * transforms_.getMatrix());
            break;

        case Mode::TransformWorldModel:
            data->setWorldMatrix(transforms_.getMatrix() * data->getWorldMatrix());
            break;
        case Mode::WorldTransform:
            data->setModelMatrix(transforms_.getMatrix());
            break;
        case Mode::TransformModel:
            data->setWorldMatrix(transforms_.getMatrix());
            break;
    }

    outport_.setData(data);
}

}  // namespace inviwo
