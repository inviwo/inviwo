/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/basemoduledefine.h>
#include <modules/base/properties/basisproperty.h>

namespace inviwo {

template <typename T>
class BasisTransform : public Processor {
public:
    BasisTransform();
    virtual ~BasisTransform() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;

    virtual void deserialize(Deserializer& d) override;

protected:
    virtual void process() override;

private:
    std::shared_ptr<T> data_;
    DataInport<T> inport_;
    DataOutport<T> outport_;

    BasisProperty basis_;
    bool deserialized_ = false;
};

template <typename T>
const ProcessorInfo& BasisTransform<T>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<BasisTransform<T>>::getProcessorInfo()};
    return info;
}

class Mesh;
template <>
struct ProcessorTraits<BasisTransform<Mesh>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.BasisTransformGeometry",  // Class identifier
            "Basis Transform Mesh",               // Display name
            "Coordinate Transforms",              // Category
            CodeState::Experimental,              // Code state
            Tags::CPU,                            // Tags
            "Transforms the basis of a mesh."_help,
        };
    }
};

class Volume;
template <>
struct ProcessorTraits<BasisTransform<Volume>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.BasisTransformVolume",  // Class identifier
            "Basis Transform Volume",           // Display name
            "Coordinate Transforms",            // Category
            CodeState::Experimental,            // Code state
            Tags::CPU,                          // Tags
            "Transforms the world of a volume."_help,
        };
    }
};

template <typename T>
BasisTransform<T>::BasisTransform()
    : Processor(), inport_("inport"), outport_("outport"), basis_("basis", "Basis") {
    addPort(inport_);
    addPort(outport_);
    addProperty(basis_);
}

template <typename T>
void BasisTransform<T>::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

template <typename T>
void BasisTransform<T>::process() {
    if (inport_.hasData()) {
        if (inport_.isChanged()) {
            data_.reset(inport_.getData()->clone());
            basis_.updateForNewEntity(*data_, deserialized_);
            deserialized_ = false;
            outport_.setData(data_);
        }
        basis_.updateEntity(*data_);
    }
}

}  // namespace inviwo
