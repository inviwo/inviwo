/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>          // for Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/datatraits.h>             // for DataTraits
#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for BufferTarget, BufferTarget...
#include <inviwo/core/ports/bufferport.h>                      // for BufferOutport
#include <inviwo/core/ports/datainport.h>                      // for DataInport
#include <inviwo/core/processors/processor.h>                  // for Processor
#include <inviwo/core/processors/processorinfo.h>              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>             // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortraits.h>            // for ProcessorTraits
#include <inviwo/core/properties/optionproperty.h>             // for OptionProperty, OptionProp...

#include <memory>                                              // for make_shared
#include <string>                                              // for string
#include <string_view>                                         // for string_view
#include <vector>                                              // for vector

namespace inviwo {

template <typename T>
class VectorToBuffer : public Processor {
public:
    VectorToBuffer()
        : Processor()
        , inport_{"inport"}
        , outport_{"outport"}
        , bufferUsage_{"bufferUsage",
                       "Buffer Usage",
                       {BufferUsage::Static, BufferUsage::Dynamic},
                       0}
        , bufferTarget_{"bufferTarget", "Buffer Target",
                        []() -> std::vector<BufferTarget> {
                            if constexpr (std::is_integral_v<T>) {
                                return {BufferTarget::Index, BufferTarget::Data};
                            } else {
                                return {BufferTarget::Data};
                            }
                        }(),
                        0} {

        addPorts(inport_, outport_);
        addProperties(bufferUsage_, bufferTarget_);
    }  // namespace inviwo
    virtual ~VectorToBuffer() override = default;

    virtual void process() override {
        if (bufferTarget_.getSelectedValue() == BufferTarget::Data) {
            auto repr = std::make_shared<BufferRAMPrecision<T, BufferTarget::Data>>(
                *inport_.getData(), bufferUsage_.getSelectedValue());
            auto buffer = std::make_shared<Buffer<T, BufferTarget::Data>>(repr);
            outport_.setData(buffer);
        } else {
            auto repr = std::make_shared<BufferRAMPrecision<T, BufferTarget::Index>>(
                *inport_.getData(), bufferUsage_.getSelectedValue());
            auto buffer = std::make_shared<Buffer<T, BufferTarget::Index>>(repr);
            outport_.setData(buffer);
        }
    }

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<std::vector<T>> inport_;
    BufferOutport outport_;

    OptionProperty<BufferUsage> bufferUsage_;
    OptionProperty<BufferTarget> bufferTarget_;
};

template <typename T>
const ProcessorInfo VectorToBuffer<T>::getProcessorInfo() const {
    return ProcessorTraits<VectorToBuffer<T>>::getProcessorInfo();
}

template <typename T>
struct ProcessorTraits<VectorToBuffer<T>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            fmt::format("org.inviwo.{}.ToBuffer",
                        DataTraits<std::vector<T>>::dataName()),  // Class identifier
            fmt::format("{} To Buffer",
                        DataTraits<std::vector<T>>::dataName()),  // Display name
            "Data Creation",                                      // Category
            CodeState::Stable,                                    // Code state
            "Buffer, Mesh"                                        // Tags
        };
    }
};

}  // namespace inviwo
