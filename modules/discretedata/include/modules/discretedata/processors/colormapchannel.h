/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.ColormapChannel, Colormap Channel}
 * ![](org.inviwo.ColormapChannel.png?classIdentifier=org.inviwo.ColormapChannel)
 * Map a scalar channel to color with a transfer function.
 */
class IVW_MODULE_DISCRETEDATA_API ColormapChannel : public Processor {
public:
    ColormapChannel();
    virtual ~ColormapChannel() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetInport dataIn_;
    DataSetOutport dataOut_;
    DataChannelProperty scalarChannel_;
    TransferFunctionProperty colormap_;
    StringProperty channelName_;
    BoolProperty automaticName_;
    BoolProperty symmetricRange_;
    DoubleVec2Property scalarRange_;
};

namespace detail {
struct ColorMappedChannelDispatcher {
    template <typename Result, typename T>
    Result operator()(std::shared_ptr<const Channel> ch, const TransferFunction& tf,
                      const std::string& name, double min, double max) {
        auto dataCh = std::dynamic_pointer_cast<const DataChannel<typename T::type, 1>>(ch);
        if (!dataCh) return nullptr;

        auto colorCh = std::make_shared<AnalyticChannel<double, 4, dvec4>>(
            [=](dvec4& vec, ind idx) {
                typename T::type scalar;
                dataCh->fill(scalar, idx, 1);

                vec = tf.sample((static_cast<double>(scalar) - min) / (max - min));
            },
            dataCh->size(), name, dataCh->getGridPrimitiveType());
        return colorCh;
    }
};
}  // namespace detail

}  // namespace discretedata
}  // namespace inviwo
