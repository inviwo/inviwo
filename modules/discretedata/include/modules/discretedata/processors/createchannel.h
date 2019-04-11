/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/channels/channeldispatching.h>
#include <modules/discretedata/properties/datachannelproperty.h>

namespace inviwo {
namespace discretedata {

/** \class CreateConstantChannel
    \brief Create a channel with constant values
*/
class IVW_MODULE_DISCRETEDATA_API CreateConstantChannel : public Processor {

    // Construction / Deconstruction
public:
    CreateConstantChannel();
    virtual ~CreateConstantChannel() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

    // Ports
public:
    DataSetInport dataInport;
    DataSetOutport dataOutport;

    // Properties
public:
    /// Name for the new channel
    StringProperty name_;

    /// Format of data
    OptionPropertyInt format_;

    /// Where to create
    GridPrimitiveProperty primitive_;

    /// Number of components
    IntProperty numComponents_;

    // Value to be set, converted to format_
    DoubleProperty value_;

protected:
    struct CreateChannelDispatcher {
        template <typename Result, typename T, ind N, typename... Args>
        Result operator()(double value, const std::string& name, GridPrimitive primitive,
                          int numElements) {
            Channel* channel =
                new AnalyticChannel<typename T::type, N, std::array<typename T::type, N>>(
                    [value](std::array<typename T::type, N>& vec, ind) {
                        for (ind n = 0; n < N; ++n) vec[n] = static_cast<typename T::type>(value);
                    },
                    numElements, name, primitive);
            return channel;
        }
    };
};

}  // namespace discretedata
}  // namespace inviwo
