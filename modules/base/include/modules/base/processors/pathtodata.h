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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/datastructures/path.h>
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/fileextensionutils.h>

#include <modules/base/processors/filecache.h>

namespace inviwo {

template <typename DataType, typename OutportType = DataOutport<DataType>>
class PathToData : public Processor {
public:
    PathToData(DataReaderFactory* rf = util::getDataReaderFactory());
    PathToData(const PathToData&) = delete;
    PathToData(PathToData&&) = delete;
    PathToData& operator=(const PathToData&) = delete;
    PathToData& operator=(PathToData&&) = delete;

    virtual ~PathToData() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;

    void process() override;

    static constexpr std::string_view identifierSuffix() { return ".from.path"; }

    PathInport inport_;
    OutportType outport_;

    OptionProperty<FileExtension> reader_;
    ButtonProperty reload_;
    FileProperty path_;
    DataReaderFactory* rf_;
    RAMCache<std::shared_ptr<const DataType>> ram_;
};

template <typename DataType, typename OutportType>
struct ProcessorTraits<PathToData<DataType, OutportType>> {
    static ProcessorInfo getProcessorInfo() {
        const auto name = fmt::format("Path To {}", DataTraits<DataType>::dataName());
        const auto cid = fmt::format("{}{}", DataTraits<DataType>::classIdentifier(),
                                     PathToData<DataType, OutportType>::identifierSuffix());
        const auto doc =
            fmt::format("Load a {} from a given path", DataTraits<DataType>::dataName());
        return {
            cid,                // Class identifier
            name,               // Display name
            "Data Input",       // Category
            CodeState::Stable,  // Code state
            Tags::CPU,          // Tags
            Document{doc},      // Help
            true                // Visible
        };
    }
};

template <typename DataType, typename OutportType>
const ProcessorInfo& PathToData<DataType, OutportType>::getProcessorInfo() const {
    static const ProcessorInfo info{
        ProcessorTraits<PathToData<DataType, OutportType>>::getProcessorInfo()};
    return info;
}

template <typename DataType, typename OutportType>
PathToData<DataType, OutportType>::PathToData(DataReaderFactory* rf)
    : Processor()
    , inport_{"inport"}
    , outport_{"outport"}
    , reader_{"reader", "Data Reader",
              [&]() {
                  auto opts =
                      util::optionsForFileExtensions(rf->getExtensionsForTypesView<DataType>());
                  opts.insert(opts.begin(), {"automatic", "Automatic", FileExtension{}});
                  return opts;
              }()}
    , reload_{"reload", "Reload data"}
    , path_{"path",
            "Path",
            "The current loaded file"_help,
            {},
            AcceptMode::Open,
            FileMode::ExistingFile,
            FileBase::defaultContentType,
            InvalidationLevel::Valid}
    , rf_{rf}
    , ram_{} {

    addPorts(inport_, outport_);
    addProperties(reader_, reload_, path_, ram_.capacity);

    path_.setReadOnly(true).setSerializationMode(PropertySerializationMode::None);
}

template <typename DataType, typename OutportType>
void PathToData<DataType, OutportType>::process() {
    const auto path = inport_.getData();
    path_.set(*path);

    const auto key = [&]() {
        if (reader_.getSelectedIndex() == 0) {
            return fmt::format("Automatic-{}", *path);
        } else {
            return fmt::format("{}-{}", reader_.getSelectedValue(), *path);
        }
    }();

    if (auto ramData = ram_.get(key)) {
        outport_.setData(*ramData);
    } else {
        const auto getReader = [&]() {
            if (reader_.getSelectedIndex() == 0) {
                return rf_->template getReaderForTypeAndExtension<DataType>(*path);
            } else {
                return rf_->template getReaderForTypeAndExtension<DataType>(
                    reader_.getSelectedValue(), *path);
            }
        };

        if (auto reader = getReader()) {
            try {
                auto data = reader->readData(*path);
                outport_.setData(data);
                ram_.add(key, data);
            } catch (const DataReaderException& e) {
                throw Exception{SourceContext{}, "Could not load: {}.\n{}", *path, e.getMessage()};
            }
        } else {
            throw Exception{SourceContext{}, "Could not find a data reader for file: {}", *path};
        }
    }
}

}  // namespace inviwo
