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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/listproperty.h>

namespace inviwo {

namespace detail {

IVW_CORE_API void updateMetaData(ListProperty& metadata, const MetaDataOwner& data);
IVW_CORE_API void clearMetaData(ListProperty& metadata);
IVW_CORE_API std::vector<std::unique_ptr<Property>> createPrefabs();

}  // namespace detail

template <typename T, typename TInport = DataInport<T>, typename TOutport = DataOutport<T>>
    requires std::derived_from<T, MetaDataOwner>
class MetaDataProcessor : public Processor, public PropertyObserver, public PropertyOwnerObserver {
public:
    MetaDataProcessor();
    virtual ~MetaDataProcessor() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;

    void process() override;

    // override PropertyObserver
    virtual void onSetDisplayName(Property*, const std::string&) override;

    // override PropertyOwnerObserver
    virtual void onDidAddProperty(Property* property, size_t) override;

    static constexpr std::string_view identifierSuffix() { return ".metadata.processor"; };

private:
    TInport inport_;
    TOutport outport_;
    ListProperty metadata_;
};

template <typename T, typename TInport, typename TOutport>
    requires std::derived_from<T, MetaDataOwner>
MetaDataProcessor<T, TInport, TOutport>::MetaDataProcessor()
    : Processor()
    , inport_{"inport", "Data to extract meta data from"_help}
    , outport_{"outport", "Input data is passed to the outport"_help}
    , metadata_{"metadata", "MetaData", detail::createPrefabs()} {

    addPorts(inport_, outport_);
    addProperties(metadata_);

    const auto updateMeta = [this]() {
        if (auto data = inport_.getData()) {
            detail::updateMetaData(metadata_, *data);
        } else {
            detail::clearMetaData(metadata_);
        }
    };
    inport_.onChange(updateMeta);
    inport_.onConnect(updateMeta);
    inport_.onDisconnect(updateMeta);

    metadata_.PropertyOwnerObservable::addObserver(this);
}

template <typename T, typename TInport, typename TOutport>
    requires std::derived_from<T, MetaDataOwner>
void MetaDataProcessor<T, TInport, TOutport>::process() {
    outport_.setData(inport_.getData());
}
template <typename T, typename TInport, typename TOutport>
    requires std::derived_from<T, MetaDataOwner>
void MetaDataProcessor<T, TInport, TOutport>::onSetDisplayName(Property*, const std::string&) {
    if (auto data = inport_.getData()) {
        detail::updateMetaData(metadata_, *data);
    } else {
        detail::clearMetaData(metadata_);
    }
}

template <typename T, typename TInport, typename TOutport>
    requires std::derived_from<T, MetaDataOwner>
void MetaDataProcessor<T, TInport, TOutport>::onDidAddProperty(Property* property, size_t index) {
    property->addObserver(this);
    property->setIdentifier(fmt::format("meta{}", metadata_.size()));
}

template <typename T, typename TInport, typename TOutport>
struct ProcessorTraits<MetaDataProcessor<T, TInport, TOutport>> {
    static ProcessorInfo getProcessorInfo() {

        const auto name = fmt::format("{} MetaData", DataTraits<T>::dataName());
        const auto cid = fmt::format("{}{}", DataTraits<T>::classIdentifier(),
                                     MetaDataProcessor<T, TInport, TOutport>::identifierSuffix());
        const auto doc =
            fmt::format("Extract specific metadata items from the {}", DataTraits<T>::dataName());

        return {
            cid,                // Class identifier
            name,               // Display name
            "MetaData",         // Category
            CodeState::Stable,  // Code state
            Tags::CPU,          // Tags
            Document{doc},
            true  // Visible
        };
    }
};

template <typename T, typename TInport, typename TOutport>
    requires std::derived_from<T, MetaDataOwner>
const ProcessorInfo& MetaDataProcessor<T, TInport, TOutport>::getProcessorInfo() const {
    static const ProcessorInfo info{
        ProcessorTraits<MetaDataProcessor<T, TInport, TOutport>>::getProcessorInfo()};
    return info;
}

}  // namespace inviwo
