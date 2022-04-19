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
#include <modules/discretedata/channels/formatconversionchannel.h>

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

    virtual void deserialize(Deserializer&) override;
    virtual void serialize(Serializer& s) const override;

private:
    DataSetInport dataIn_;
    DataSetOutport dataOut_;
    ListProperty channelOperations_;
};

struct ChannelOpPropertyBase : public CompositeProperty {
    ChannelOpPropertyBase(
        const std::string& identifier, const std::string& displayName, DataSetInport* dataInport,
        const std::string& defaultName,
        DataChannelProperty::ChannelFilter filter = &DataChannelProperty::FilterPassAll);
    ChannelOpPropertyBase(const ChannelOpPropertyBase& prop);
    virtual std::shared_ptr<Channel> applyOperation() = 0;
    void updateDataSet(DataSetInport* dataInport) { baseChannel_.setDatasetInput(dataInport); }

    DataChannelProperty baseChannel_;
    StringProperty channelName_;
};

template <typename ChannelOp>
struct ChannelOpProperty : public ChannelOpPropertyBase {

    ChannelOpProperty(
        const std::string& identifier, const std::string& displayName,
        DataSetInport* dataInport = nullptr, const std::string& defaultName = "NONE",
        DataChannelProperty::ChannelFilter filter = &DataChannelProperty::FilterPassAll);
    ChannelOpProperty(const ChannelOpProperty<ChannelOp>&);
    ChannelOpProperty& operator=(const ChannelOpProperty<ChannelOp>& prop);
    virtual std::shared_ptr<Channel> applyOperation() override;
    virtual ChannelOpProperty<ChannelOp>* clone() const override {
        return new ChannelOpProperty<ChannelOp>(*this);
    }

    virtual std::string getClassIdentifier() const override;

    ChannelOp channelOperation_;
};

struct NormalizeChannelOperation {
    NormalizeChannelOperation(ChannelOpProperty<NormalizeChannelOperation>* property) {}

    template <typename T, ind N>
    struct NormalizedFunctor {
        std::shared_ptr<const DataChannel<T, N>> channel_;
        glm::vec<N, T> min_, ext_;

        NormalizedFunctor(std::shared_ptr<const DataChannel<T, N>> channel, glm::vec<N, T> min,
                          glm::vec<N, T> ext)
            : channel_(channel), min_(min), ext_(ext) {}

        void operator()(std::array<T, N>& vec, ind idx) {
            channel_->fill(vec, idx);
            if (!channel_->isValid(vec[0])) {
                return;
            }

            for (ind dim = 0; dim < N; ++dim) {
                if (ext_[dim] != 0) vec[dim] = (vec[dim] - min_[dim]) / ext_[dim];
            }
        }
    };

    template <typename T, ind N>
    std::shared_ptr<Channel> operator()(std::shared_ptr<const DataChannel<T, N>> channel,
                                        ChannelOpProperty<NormalizeChannelOperation>* property) {
        glm::vec<N, T> min, max;
        channel->getMinMax(min, max);
        std::cout << fmt::format("+ MinExtent: {} -> {}", min, max - min);

        auto normalizedChannel = std::make_shared<AnalyticChannel<T, N>>(
            NormalizedFunctor<T, N>(channel, min, max - min), channel->size(),
            property->channelName_.get(), channel->getGridPrimitiveType());
        normalizedChannel->setInvalidValue(channel->getInvalidValue());
        return normalizedChannel;
    }

    static std::string getIdentifier() { return "normalized"; }
};

struct MagnitudeOperation {
    MagnitudeOperation(ChannelOpProperty<MagnitudeOperation>* property) {}

    template <typename T, ind N>
    struct MagnitudeFunctor {
        std::shared_ptr<const DataChannel<T, N>> channel_;

        MagnitudeFunctor(std::shared_ptr<const DataChannel<T, N>> channel) : channel_(channel) {}

        void operator()(double& magnitude, ind idx) {
            std::array<T, N> vec;
            channel_->fill(vec, idx);

            magnitude = 0;
            if (!channel_->isValid(vec[0])) return;

            for (ind dim = 0; dim < N; ++dim) {
                magnitude += vec[dim] * vec[dim];
            }
            magnitude = std::sqrt(magnitude);
        }
    };

    template <typename T, ind N>
    std::shared_ptr<Channel> operator()(std::shared_ptr<const DataChannel<T, N>> channel,
                                        ChannelOpProperty<MagnitudeOperation>* property) {

        return std::make_shared<AnalyticChannel<double, 1, double>>(
            MagnitudeFunctor<T, N>(channel), channel->size(), property->channelName_.get(),
            channel->getGridPrimitiveType());
    }

    static std::string getIdentifier() { return "magnitude"; }
};

struct NormalizedMagnitudeOperation {
    NormalizedMagnitudeOperation(ChannelOpProperty<NormalizedMagnitudeOperation>* property) {}

    template <typename T, ind N>
    std::shared_ptr<Channel> operator()(std::shared_ptr<const DataChannel<T, N>> channel,
                                        ChannelOpProperty<NormalizedMagnitudeOperation>* property) {

        auto magnitudeChannel = std::make_shared<AnalyticChannel<double, 1, double>>(
            MagnitudeOperation::MagnitudeFunctor<T, N>(channel), channel->size(),
            property->channelName_.get(), channel->getGridPrimitiveType());

        glm::vec<1, double> min, max;
        magnitudeChannel->getMinMax(min, max);
        auto normalizedChannel = std::make_shared<AnalyticChannel<double, 1>>(
            NormalizeChannelOperation::NormalizedFunctor<double, 1>(magnitudeChannel, min,
                                                                    max - min),
            channel->size(), property->channelName_.get(), channel->getGridPrimitiveType());
        normalizedChannel->setInvalidValue(channel->getInvalidValue());

        return normalizedChannel;
    }

    static std::string getIdentifier() { return "normmag"; }
};

struct AppendOperation {
    AppendOperation(ChannelOpProperty<AppendOperation>* property)
        : appendValue_("appendValue", "Value", 1, 0, 10, 0.1) {
        property->addProperty(appendValue_);
    }

    template <typename T, ind N>
    std::shared_ptr<Channel> operator()(std::shared_ptr<const DataChannel<T, N>> channel,
                                        ChannelOpProperty<AppendOperation>* property) {

        return std::make_shared<AnalyticChannel<T, N + 1>>(
            [appendValue = appendValue_.get(), channel](auto& magnitude, ind idx) {
                std::array<T, N + 1> vec;
                channel->fillRaw(reinterpret_cast<T*>(&vec), idx);
                vec[N] = appendValue;
            },
            channel->size(), property->channelName_.get(), channel->getGridPrimitiveType());
    }

    static std::string getIdentifier() { return "append"; }

    DoubleProperty appendValue_;
};

struct DataFormatOperation {
    DataFormatOperation(ChannelOpProperty<DataFormatOperation>* property)
        : format_("format", "Data Format") {
        property->addProperty(format_);
        for (int format = static_cast<int>(DataFormatId::Float16);
             format <= static_cast<int>(DataFormatId::UInt64); ++format) {
            std::string name =
                std::string(DataFormatBase::get(static_cast<DataFormatId>(format))->getString());
            format_.addOption(name, name, format);
        }
    }

    template <typename T, ind N>
    std::shared_ptr<Channel> operator()(std::shared_ptr<const DataChannel<T, N>> channel,
                                        ChannelOpProperty<DataFormatOperation>* property) {

        return createFormatConversionChannel(channel, static_cast<DataFormatId>(format_.get()),
                                             property->channelName_.get());
    }

    static std::string getIdentifier() { return "format"; }

    OptionPropertyInt format_;
};

template <typename ChannelOp>
ChannelOpProperty<ChannelOp>::ChannelOpProperty(const std::string& identifier,
                                                const std::string& displayName,
                                                DataSetInport* dataInport,
                                                const std::string& defaultName,
                                                DataChannelProperty::ChannelFilter filter)
    : ChannelOpPropertyBase(identifier, displayName, dataInport, defaultName, filter)
    , channelOperation_(this) {}

template <typename ChannelOp>
ChannelOpProperty<ChannelOp>& ChannelOpProperty<ChannelOp>::operator=(
    const ChannelOpProperty<ChannelOp>& prop) {
    if (prop.channelName_.get().compare("NONE") != 0) channelName_.set(prop.channelName_.get());
    if (prop.baseChannel_.datasetInput_)
        baseChannel_.datasetInput_ = prop.baseChannel_.datasetInput_;
    setIdentifier(prop.getIdentifier());
    setDisplayName(prop.getDisplayName());
}

template <typename ChannelOp>
ChannelOpProperty<ChannelOp>::ChannelOpProperty(const ChannelOpProperty<ChannelOp>& prop)
    : ChannelOpPropertyBase(prop.getIdentifier(), prop.getDisplayName(),
                            prop.baseChannel_.getDatasetInput(),
                            prop.baseChannel_.channelName_.get(), prop.baseChannel_.channelFilter_)
    , channelOperation_(this) {}

template <typename ChannelOp>
std::shared_ptr<Channel> ChannelOpProperty<ChannelOp>::applyOperation() {
    std::shared_ptr<const Channel> channel;
    if (!baseChannel_.hasSelectableChannels() || !(channel = baseChannel_.getCurrentChannel()))
        return nullptr;

    ChannelOp dispatcher(this);
    return Channel::dispatchSharedPointer<std::shared_ptr<Channel>>(channel, dispatcher, this);
}

}  // namespace discretedata

template <typename ChannelOp>
struct PropertyTraits<discretedata::ChannelOpProperty<ChannelOp>> {
    static std::string classIdentifier() {
        return "inviwo.discretedata.channeloperation" + ChannelOp::getIdentifier();
    }
};

template <typename ChannelOp>
std::string discretedata::ChannelOpProperty<ChannelOp>::getClassIdentifier() const {
    return PropertyTraits<ChannelOpProperty<ChannelOp>>::classIdentifier();
}

}  // namespace inviwo
