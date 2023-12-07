/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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
    DataInport<T> inport_;
    DataOutport<T> outport_;

    OptionProperty<CoordinateSpace> space_;
    BoolProperty replace_;
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
                "Apply a model or world transformation to a layer."_help};
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
                "Apply a model or world transformation to a mesh."_help};
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
                "Apply a model or world transformation to a volume."_help};
    }
};

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
    , transforms_("transformations", "Transformation Stack") {

    addPort(inport_);
    addPort(outport_);

    addProperties(space_, replace_, transforms_);
}

template <typename T>
void Transform<T>::process() {
    std::shared_ptr<T> data(inport_.getData()->clone());

    switch (*space_) {
        case CoordinateSpace::Model:
            if (replace_) {
                data->setModelMatrix(transforms_.getMatrix());
            } else {
                data->setModelMatrix(transforms_.getMatrix() * data->getModelMatrix());
            }
            break;
        case CoordinateSpace::World:
        default:
            if (replace_) {
                data->setWorldMatrix(transforms_.getMatrix());
            } else {
                data->setWorldMatrix(transforms_.getMatrix() * data->getWorldMatrix());
            }
            break;
    }

    outport_.setData(data);
}

}  // namespace inviwo
