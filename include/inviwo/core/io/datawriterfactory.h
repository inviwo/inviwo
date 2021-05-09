/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/fileextension.h>

#include <vector>
#include <string>
#include <memory>

namespace inviwo {

template <typename T>
class DataWriterType;

class IVW_CORE_API DataWriterFactory : public Factory<DataWriter, const FileExtension&> {
public:
    using Map = std::map<FileExtension, DataWriter*,
                         std::function<bool(const FileExtension&, const FileExtension&)>>;
    DataWriterFactory() = default;
    virtual ~DataWriterFactory() = default;

    bool registerObject(DataWriter* reader);
    bool unRegisterObject(DataWriter* reader);

    virtual std::unique_ptr<DataWriter> create(const std::string& key) const;
    virtual std::unique_ptr<DataWriter> create(const FileExtension& key) const override;

    virtual bool hasKey(const std::string& key) const;
    virtual bool hasKey(const FileExtension& key) const override;

    template <typename T>
    std::vector<FileExtension> getExtensionsForType() const;
    /**
     * \brief Return a writer matching the file extension of DataWriterType of type T.
     * Does case insensive comparison between the last part of filePathOrExtension and each
     * registered extension.
     * @param filePathOrExtension Path to file, or simply the extension.
     * @return First available DataWriterType<T> if found, nullptr otherwise.
     */
    template <typename T>
    std::unique_ptr<DataWriterType<T>> getWriterForTypeAndExtension(
        std::string_view filePathOrExtension) const;

    template <typename T>
    std::unique_ptr<DataWriterType<T>> getWriterForTypeAndExtension(const FileExtension& ext) const;

    /**
     * First look for a writer using the FileExtension ext, and if no writer was found look for a
     * writer using the fallbackFilePathOrExtension. This is often used with a file open dialog,
     * where the dialog will have a selectedFileExtension that will be used as ext, and the
     * fallbackFilePathOrExtension is taken from the file to be written. This way any selected
     * writer will have priority over the file extension.
     */
    template <typename T>
    std::unique_ptr<DataWriterType<T>> getWriterForTypeAndExtension(
        const FileExtension& ext, std::string_view fallbackFilePathOrExtension) const;

protected:
    // Sort extensions by length to first compare long extensions. Avoids selecting extension xyz in
    // case xyzw exist, see getReaderForTypeAndExtension
    Map map_ = Map([](const auto& a, const auto& b) -> bool {
        if (a.extension_.size() != b.extension_.size()) {
            return a.extension_.size() > b.extension_.size();
        } else {
            // Order does not matter
            return a < b;
        }
    });
};

template <typename T>
std::vector<FileExtension> DataWriterFactory::getExtensionsForType() const {
    std::vector<FileExtension> ext;

    for (auto& writer : map_) {
        if (auto r = dynamic_cast<DataWriterType<T>*>(writer.second)) {
            ext.push_back(writer.first);
        }
    }
    return ext;
}

template <typename T>
std::unique_ptr<DataWriterType<T>> DataWriterFactory::getWriterForTypeAndExtension(
    std::string_view path) const {
    for (auto& elem : map_) {
        if (path.size() >= elem.first.extension_.size() &&
            // Compare last part of path with the extension
            iCaseCmp(path.substr(path.size() - elem.first.extension_.size()),
                     elem.first.extension_)) {
            if (auto r = dynamic_cast<DataWriterType<T>*>(elem.second)) {
                return std::unique_ptr<DataWriterType<T>>(r->clone());
            }
        }
    }
    return std::unique_ptr<DataWriterType<T>>();
}

template <typename T>
std::unique_ptr<DataWriterType<T>> DataWriterFactory::getWriterForTypeAndExtension(
    const FileExtension& ext) const {
    return util::map_find_or_null(map_, ext, [](DataWriter* o) {
        if (auto r = dynamic_cast<DataWriterType<T>*>(o)) {
            return std::unique_ptr<DataWriterType<T>>(r->clone());
        } else {
            return std::unique_ptr<DataWriterType<T>>();
        }
    });
}

template <typename T>
std::unique_ptr<DataWriterType<T>> DataWriterFactory::getWriterForTypeAndExtension(
    const FileExtension& ext, std::string_view fallbackFilePathOrExtension) const {
    if (auto writer = this->getWriterForTypeAndExtension<T>(ext)) {
        return writer;
    }
    return this->getWriterForTypeAndExtension<T>(fallbackFilePathOrExtension);
}

}  // namespace inviwo
