/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/fileextension.h>

#include <vector>
#include <string>
#include <memory>

namespace inviwo {

template <typename T>
class DataWriterType;

class IVW_CORE_API DataWriterFactory : public Factory<DataWriter, const FileExtension&>,
                                       public Factory<DataWriter, std::string_view>,
                                       public FactoryObservable<DataWriter> {
public:
    DataWriterFactory() = default;
    virtual ~DataWriterFactory() = default;

    bool registerObject(DataWriter* writer);
    bool unRegisterObject(DataWriter* writer);

    virtual std::unique_ptr<DataWriter> create(std::string_view key) const override;
    virtual std::unique_ptr<DataWriter> create(const FileExtension& key) const override;

    virtual bool hasKey(std::string_view key) const override;
    virtual bool hasKey(const FileExtension& key) const override;
    
    auto getKeyView() const {
        constexpr auto getFirst = [](auto&& pair) -> decltype(auto) { return pair.first; };
        return util::as_range(util::makeTransformIterator(getFirst, map_.begin()),
                              util::makeTransformIterator(getFirst, map_.end()));
    }

    template <typename T>
    std::vector<FileExtension> getExtensionsForType() const;
    /**
     * \brief Return a writer matching the file extension of DataWriterType of type T.
     * Does case insensitive comparison between the last part of filePathOrExtension and each
     * registered extension. Does not check for "." before the extension, so it is valid to pass for
     * example "png".
     * @param filePathOrExtension Path to file, or simply the extension.
     * @return First available DataWriterType<T> if found, nullptr otherwise.
     */
    template <typename T>
    std::unique_ptr<DataWriterType<T>> getWriterForTypeAndExtension(
        const std::filesystem::path& filePathOrExtension) const;

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
        const FileExtension& ext, const std::filesystem::path& fallbackFilePathOrExtension) const;

    template <typename T>
    bool hasWriterForTypeAndExtension(const std::filesystem::path& filePathOrExtension) const;
    template <typename T>
    bool hasWriterForTypeAndExtension(const FileExtension& ext) const;

    template <typename T>
    bool writeDataForTypeAndExtension(const T* data, const std::filesystem::path& filePath,
                                      std::optional<FileExtension> ext = std::nullopt) const {
        if (auto writer = ext ? getWriterForTypeAndExtension<T>(*ext, filePath)
                              : getWriterForTypeAndExtension<T>(filePath)) {
            writer->writeData(data, filePath);
            return true;
        } else {
            return false;
        }
    }

protected:
    std::map<FileExtension, DataWriter*> map_;
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
    const std::filesystem::path& path) const {
    std::vector<std::pair<size_t, DataWriterType<T>*>> candidates;
    for (auto& [ext, writer] : map_) {
        if (util::iCaseEndsWith(path.string(), ext.extension_)) {
            if (auto r = dynamic_cast<DataWriterType<T>*>(writer)) {
                candidates.emplace_back(ext.extension_.size(), r);
            }
        }
    }

    // Select the match with the longest extension, for example select tar.gz over gz
    if (auto it = std::max_element(candidates.begin(), candidates.end(),
                                   [](const auto& a, const auto& b) { return a.first < b.first; });
        it != candidates.end()) {
        return std::unique_ptr<DataWriterType<T>>{it->second->clone()};
    } else {
        return {};
    }
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
    const FileExtension& ext, const std::filesystem::path& fallbackFilePathOrExtension) const {
    if (auto writer = this->getWriterForTypeAndExtension<T>(ext)) {
        return writer;
    }
    return this->getWriterForTypeAndExtension<T>(fallbackFilePathOrExtension);
}

template <typename T>
bool DataWriterFactory::hasWriterForTypeAndExtension(const std::filesystem::path& path) const {
    return getWriterForTypeAndExtension<T>(path) != nullptr;
}

template <typename T>
bool DataWriterFactory::hasWriterForTypeAndExtension(const FileExtension& ext) const {
    return getWriterForTypeAndExtension<T>(ext) != nullptr;
}

}  // namespace inviwo
