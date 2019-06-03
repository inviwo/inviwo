/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_VECTORELEMENTSELECTORPROCESSOR_H
#define IVW_VECTORELEMENTSELECTORPROCESSOR_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <modules/base/basemoduledefine.h>
#include <modules/base/properties/sequencetimerproperty.h>

namespace inviwo {

namespace detail {

template <typename T>
void setFromMetaData(std::true_type, T *item, StringProperty &name, DoubleProperty &timestamp) {
    static_assert(std::is_convertible_v<T, MetaDataOwner>);

    auto metadataowner = static_cast<const MetaDataOwner *>(item);
    if (metadataowner->hasMetaData<StringMetaData>("name")) {
        name.set(metadataowner->getMetaData<StringMetaData>("name")->get());
        name.setVisible(true);
    } else {
        name.setVisible(false);
    }

    if (metadataowner->hasMetaData<DoubleMetaData>("timestamp") ||
        metadataowner->hasMetaData<FloatMetaData>("timestamp")) {
        if (auto metadata1 = metadataowner->getMetaData<DoubleMetaData>("timestamp")) {
            timestamp.set(metadata1->get());
            timestamp.setVisible(true);
        } else if (auto metadata2 = metadataowner->getMetaData<FloatMetaData>("timestamp")) {
            timestamp.set(static_cast<double>(metadata2->get()));
            timestamp.setVisible(true);
        }
    } else {
        timestamp.setVisible(false);
    }
}

template <typename T>
void setFromMetaData(std::false_type, T *, StringProperty &, DoubleProperty &) {}

}  // namespace detail

/**
 * Template for processors that want to select an element from an
 * input vector and set it as output.
 * @see VolumeSequenceElementSelectorProcessor for an example
 */
template <typename T, typename OutportType = DataOutport<T>>
class VectorElementSelectorProcessor : public Processor {
public:
    VectorElementSelectorProcessor();
    virtual ~VectorElementSelectorProcessor() = default;

    virtual const ProcessorInfo getProcessorInfo() const override = 0;

    void process() override;

protected:
    DataInport<std::vector<std::shared_ptr<T>>> inport_;
    OutportType outport_;
    SequenceTimerProperty timeStep_;

    StringProperty name_;
    DoubleProperty timestamp_;
};

template <typename T, typename OutportType>
VectorElementSelectorProcessor<T, OutportType>::VectorElementSelectorProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , timeStep_("timeStep", "Step")
    , name_("name", "Name")
    , timestamp_("timestamp", "Timestamp", 0, std::numeric_limits<double>::lowest(),
                 std::numeric_limits<double>::max(), std::numeric_limits<double>::epsilon(),
                 InvalidationLevel::Valid, PropertySemantics("Text")) {
    addPort(inport_);
    addPort(outport_);

    addProperty(timeStep_);
    addProperty(name_);
    addProperty(timestamp_);
    name_.setVisible(false);
    name_.setReadOnly(true);
    name_.setCurrentStateAsDefault();

    timestamp_.setVisible(false);
    timestamp_.setReadOnly(true);
    timestamp_.setCurrentStateAsDefault();

    // This needs to be added by the child class
    // timeStep_.index_.autoLinkToProperty<VectorElementSelectorProcessor<T>>("timeStep.selectedSequenceIndex");

    timeStep_.onChange([this]() {
        if (auto data = inport_.getData()) {

            size_t index =
                std::min(data->size() - 1, static_cast<size_t>(timeStep_.index_.get() - 1));
            auto selectedData = data->at(index).get();

            detail::setFromMetaData(typename std::is_convertible<T, MetaDataOwner>::type(), selectedData,
                                    name_, timestamp_);
        } else {
            name_.setVisible(false);
            timestamp_.setVisible(false);
        }
    });

    inport_.onChange([this]() {
        if (inport_.hasData()) {
            timeStep_.updateMax(inport_.getData()->size());
        }
    });
}

template <typename T, typename OutportType>
void VectorElementSelectorProcessor<T, OutportType>::process() {
    if (!inport_.isReady()) return;

    if (auto data = inport_.getData()) {
        if (data->size() == 0) {
            outport_.detachData();
            return;
        }
        size_t index = std::min(data->size() - 1, static_cast<size_t>(timeStep_.index_.get() - 1));

        outport_.setData((*data)[index]);
    }
}

}  // namespace inviwo

#endif  // IVW_VECTORELEMENTSELECTORPROCESSOR_H
