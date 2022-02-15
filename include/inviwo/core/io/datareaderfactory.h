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
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datareader.h>

#include <memory>
#include <unordered_map>
#include <string>

namespace inviwo {

class IVW_CORE_API DataReaderFactory : public Factory<DataReader, const FileExtension&> {
public:
    DataReaderFactory() = default;
    virtual ~DataReaderFactory() = default;

    bool registerObject(DataReader* reader);
    bool unRegisterObject(DataReader* reader);
    virtual std::unique_ptr<DataReader> create(const FileExtension& key) const override;
    virtual std::unique_ptr<DataReader> create(std::string_view key) const;
    virtual bool hasKey(std::string_view key) const;
    virtual bool hasKey(const FileExtension& key) const override;

    template <typename T>
    std::vector<FileExtension> getExtensionsForType() const;

    /**
     * \brief Return a reader matching the file extension of DataReader of type T.
     * Does case insensitive comparison between the last part of filePathOrExtension and each
     * registered extension. Does not check for "." before the extension, so it is valid to pass for
     * example "png".
     * @param filePathOrExtension Path to file, or simply the extension.
     * @return First available DataReaderType<T> if found, nullptr otherwise.
     */
    template <typename T>
    std::unique_ptr<DataReaderType<T>> getReaderForTypeAndExtension(
        std::string_view filePathOrExtension) const;
    template <typename T>
    std::unique_ptr<DataReaderType<T>> getReaderForTypeAndExtension(const FileExtension& ext) const;

    /**
     * First look for a reader using the FileExtension ext, and if no reader was found look for a
     * reader using the fallbackFilePathOrExtension. This is often used with a file open dialog,
     * where the dialog will have a selectedFileExtension that will be used as ext, and the
     * fallbackFilePathOrExtension is taken from the file to be opened. This way any selected reader
     * will have priority over the file extension.
     */
    template <typename T>
    std::unique_ptr<DataReaderType<T>> getReaderForTypeAndExtension(
        const FileExtension& ext, std::string_view fallbackFilePathOrExtension) const;

    template <typename T>
    bool hasReaderForTypeAndExtension(std::string_view filePathOrExtension) const;
    template <typename T>
    bool hasReaderForTypeAndExtension(const FileExtension& ext) const;

protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Sort extensions by length to first compare long extensions. Avoids selecting extension xyz in
    // case xyzw exist, see getReaderForTypeAndExtension
    static constexpr auto comp = [](const FileExtension& a, const FileExtension& b) {
        if (a.extension_.size() != b.extension_.size()) {
            return a.extension_.size() > b.extension_.size();
        } else {
            // Order does not matter
            return a < b;
        }
    };
    std::map<FileExtension, DataReader*, decltype(comp)> map_{comp};
#endif
};

template <typename T>
std::vector<FileExtension> DataReaderFactory::getExtensionsForType() const {
    std::vector<FileExtension> ext;

    for (auto reader : map_) {
        if (auto r = dynamic_cast<DataReaderType<T>*>(reader.second)) {
            ext.push_back(reader.first);
        }
    }
    return ext;
}

template <typename T>
std::unique_ptr<DataReaderType<T>> DataReaderFactory::getReaderForTypeAndExtension(
    std::string_view path) const {
    for (auto& elem : map_) {
        if (util::iCaseEndsWith(path, elem.first.extension_)) {
            if (auto r = dynamic_cast<DataReaderType<T>*>(elem.second)) {
                return std::unique_ptr<DataReaderType<T>>(r->clone());
            }
        }
    }
    return std::unique_ptr<DataReaderType<T>>{};
}

template <typename T>
std::unique_ptr<DataReaderType<T>> DataReaderFactory::getReaderForTypeAndExtension(
    const FileExtension& ext) const {
    return util::map_find_or_null(map_, ext, [](DataReader* o) {
        if (auto r = dynamic_cast<DataReaderType<T>*>(o)) {
            return std::unique_ptr<DataReaderType<T>>(r->clone());
        } else {
            return std::unique_ptr<DataReaderType<T>>{};
        }
    });
}

template <typename T>
std::unique_ptr<DataReaderType<T>> DataReaderFactory::getReaderForTypeAndExtension(
    const FileExtension& ext, std::string_view fallbackFilePathOrExtension) const {
    if (auto reader = getReaderForTypeAndExtension<T>(ext)) {
        return reader;
    }
    return getReaderForTypeAndExtension<T>(fallbackFilePathOrExtension);
}

template <typename T>
bool DataReaderFactory::hasReaderForTypeAndExtension(std::string_view path) const {
    return getReaderForTypeAndExtension<T>(path) != nullptr;
}

template <typename T>
bool DataReaderFactory::hasReaderForTypeAndExtension(const FileExtension& ext) const {
    return util::map_find_or_null(map_, ext, [](DataReader* o) {
        if (auto r = dynamic_cast<DataReaderType<T>*>(o)) {
            return true;
        } else {
            return false;
        }
    });
}

}  // namespace inviwo
