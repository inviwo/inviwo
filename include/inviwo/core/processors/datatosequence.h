/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

/**
 * @brief Create a DataSequence from a MultiInport
 */
template <typename T>
class DataToSequence : public Processor {
public:
    DataToSequence() : Processor(), inport_{"inport"}, outport_{"outport"} {
        addPorts(inport_, outport_);
    }
    virtual ~DataToSequence() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;

    void process() override {
        auto data = std::make_shared<DataSequence<T>>();
        for (auto item : inport_) {
            data->push_back(item);
        }
        outport_.setData(data);
    }

    static constexpr std::string_view identifierSuffix() { return ".to.sequence"; };

private:
    DataInport<T, 0, true> inport_;
    DataOutport<DataSequence<T>> outport_;
};

template <>
class DataToSequence<Image> : public Processor {
public:
    DataToSequence() : Processor(), inport_{"inport"}, outport_{"outport"} {
        addPorts(inport_, outport_);
    }
    virtual ~DataToSequence() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;

    void process() override {
        auto data = std::make_shared<DataSequence<Image>>();
        for (auto item : inport_) {
            data->push_back(item);
        }
        outport_.setData(data);
    }

    static constexpr std::string_view identifierSuffix() { return ".to.sequence"; };

private:
    ImageMultiInport inport_;
    DataOutport<DataSequence<Image>> outport_;
};

template <typename T>
struct ProcessorTraits<DataToSequence<T>> {
    static ProcessorInfo getProcessorInfo() {

        const auto name = fmt::format("{} To Sequence", DataTraits<T>::dataName());
        const auto cid = fmt::format("{}{}", DataTraits<T>::classIdentifier(),
                                     DataToSequence<T>::identifierSuffix());

        const auto doc =
            fmt::format("Create a sequence from a set of {0}", DataTraits<T>::dataName());

        return {
            cid,                // Class identifier
            name,               // Display name
            "Data Selector",    // Category
            CodeState::Stable,  // Code state
            Tags::CPU,          // Tags
            Document{doc},
            true  // Visible
        };
    }
};
template <typename T>
const ProcessorInfo& DataToSequence<T>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<DataToSequence<T>>::getProcessorInfo()};
    return info;
}

}  // namespace inviwo
