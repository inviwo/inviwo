/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/channels/analyticchannel.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.ChannelOperations, Channel Operations}
 * ![](org.inviwo.ChannelOperations.png?classIdentifier=org.inviwo.ChannelOperations)
 * A number of miscallaneous operations to perform on channels.
 */
class IVW_MODULE_DISCRETEDATA_API ChannelOperations : public Processor {
public:
    ChannelOperations();
    virtual ~ChannelOperations() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataSetInport dataIn_;
    DataSetOutport dataOut_;
    ListProperty channelOperations_;
};

struct ChannelOpPropertyBase : public CompositeProperty {
    ChannelOpPropertyBase(
        const std::string& identifier, const std::string& displayName, DataSetInport& dataInport,
        DataChannelProperty::ChannelFilter filter = &DataChannelProperty::FilterPassAll);
    ChannelOpPropertyBase(const ChannelOpPropertyBase& prop);
    // ChannelOpPropertyBase& operator=(const ChannelOpPropertyBase& prop) = delete;
    virtual std::shared_ptr<Channel> applyOperation() = 0;

    DataChannelProperty baseChannel_;
    StringProperty channelName_;
};

template <typename ChannelOp>
struct ChannelOpProperty : public ChannelOpPropertyBase {
    ChannelOpProperty(
        const std::string& identifier, const std::string& displayName, DataSetInport& dataInport,
        DataChannelProperty::ChannelFilter filter = &DataChannelProperty::FilterPassAll);
    // ChannelOpProperty(const ChannelOpProperty<ChannelOp>& prop);
    virtual std::shared_ptr<Channel> applyOperation() override;
    virtual ChannelOpProperty<ChannelOp>* clone() const override {
        return new ChannelOpProperty<ChannelOp>(*this);
    }

    ChannelOp channelOperation_;
};

struct NormalizeChannelOperation {
    NormalizeChannelOperation(ChannelOpProperty<NormalizeChannelOperation>* property) {}

    template <typename T, ind N>
    std::shared_ptr<Channel> operator()(const DataChannel<T, N>* channel,
                                        ChannelOpProperty<NormalizeChannelOperation>* property) {

        glm::vec<N, T> min, max;
        channel->getMinMax(min, max);
        std::cout << fmt::format("+ MinMax: {} -> {}", min, max - min);

        return std::make_shared<AnalyticChannel<T, N>>(
            [channel, minVec = min, extVec = max - min](auto vec, ind idx) {
                channel->fill(vec, idx);
                if (idx % 10000 == 0)
                    std::cout << fmt::format("+   || ({}, {}) ||", vec[0], vec[1]);

                for (ind dim = 0; dim < N; ++dim) {
                    vec[dim] = (vec[dim] - minVec[dim]) / extVec[dim];
                }
                if (idx % 10000 == 0)
                    std::cout << fmt::format(" = ({}, {})", vec[0], vec[1]) << std::endl;

                // static size_t NUM_OUTPUTS = 0;
                // if (first < 50) {
                //     std::cout <<
                // }
                // first++;
            },
            channel->size(), property->channelName_.get(), channel->getGridPrimitiveType());
    }
};

struct MagnitudeOperation {
    MagnitudeOperation(ChannelOpProperty<MagnitudeOperation>* property) {}

    template <typename T, ind N>
    std::shared_ptr<Channel> operator()(const DataChannel<T, N>* channel,
                                        ChannelOpProperty<MagnitudeOperation>* property) {

        return std::make_shared<AnalyticChannel<double, 1, double>>(
            [channel](auto magnitude, ind idx) {
                std::array<T, N> vec;
                channel->fill(vec, idx);

                if (idx % 10000 == 0)
                    std::cout << fmt::format("-   || ({}, {}) || = ", vec[0], vec[1]);

                magnitude = 0;
                for (ind dim = 0; dim < N; ++dim) {
                    magnitude += vec[dim] * vec[dim];
                }
                magnitude = std::sqrt(magnitude);
                if (idx % 10000 == 0) std::cout << magnitude << std::endl;
            },
            channel->size(), property->channelName_.get(), channel->getGridPrimitiveType());
    }
};

template <typename ChannelOp>
ChannelOpProperty<ChannelOp>::ChannelOpProperty(const std::string& identifier,
                                                const std::string& displayName,
                                                DataSetInport& dataInport,
                                                DataChannelProperty::ChannelFilter filter)
    : ChannelOpPropertyBase(identifier, displayName, dataInport, filter), channelOperation_(this) {}

template <typename ChannelOp>
std::shared_ptr<Channel> ChannelOpProperty<ChannelOp>::applyOperation() {
    std::shared_ptr<const Channel> channel;
    if (!baseChannel_.hasSelectableChannels() || !(channel = baseChannel_.getCurrentChannel()))
        return nullptr;

    ChannelOp dispatcher(this);
    return channel->dispatch<std::shared_ptr<Channel>>(dispatcher, this);
}

}  // namespace discretedata
}  // namespace inviwo
