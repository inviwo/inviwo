/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_DATAREADERFACTORY_H
#define IVW_DATAREADERFACTORY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datareader.h>

namespace inviwo {

class IVW_CORE_API DataReaderFactory : public Factory<DataReader, const FileExtension&> {
public:
    using Map = std::unordered_map<FileExtension, DataReader*>;

    DataReaderFactory() = default;
    virtual ~DataReaderFactory() = default;

    bool registerObject(DataReader* reader);
    bool unRegisterObject(DataReader* reader);
    virtual std::unique_ptr<DataReader> create(const FileExtension& key) const override;
    virtual std::unique_ptr<DataReader> create(const std::string& key) const;
    virtual bool hasKey(const std::string& key) const;
    virtual bool hasKey(const FileExtension& key) const override;

    template <typename T>
    std::vector<FileExtension> getExtensionsForType() const;

    template <typename T>
    std::unique_ptr<DataReaderType<T>> getReaderForTypeAndExtension(const std::string& ext) const;
    template <typename T>
    std::unique_ptr<DataReaderType<T>> getReaderForTypeAndExtension(const FileExtension& ext) const;

    /**
     * First look for a reader using the FileExtension ext, and if no reader was found look for a
     * reader using the fallbackExt. This is often used with a file open dialog, where the dialog
     * will have a selectedFileExtension that will be used as ext, and the fallbackExt is taken from
     * the file to be opened. This way any selected reader will have priority over the file
     * extension.
     */
    template <typename T>
    std::unique_ptr<DataReaderType<T>> getReaderForTypeAndExtension(
        const FileExtension& ext, const std::string& fallbackExt) const;

    template <typename T>
    bool hasReaderForTypeAndExtension(const std::string& ext) const;
    template <typename T>
    bool hasReaderForTypeAndExtension(const FileExtension& ext) const;

protected:
    Map map_;
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
    const std::string& ext) const {

    auto lkey = toLower(ext);
    for (auto& elem : map_) {
        if (toLower(elem.first.extension_) == lkey) {
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
    const FileExtension& ext, const std::string& fallbackExt) const {
    if (auto reader = getReaderForTypeAndExtension<T>(ext)) {
        return reader;
    }
    return getReaderForTypeAndExtension<T>(fallbackExt);
}

template <typename T>
bool DataReaderFactory::hasReaderForTypeAndExtension(const std::string& ext) const {
    auto lkey = toLower(ext);
    for (auto& elem : map_) {
        if (toLower(elem.first.extension_) == lkey) {
            if (auto r = dynamic_cast<DataReaderType<T>*>(elem.second)) {
                return true;
            }
        }
    }
    return false;
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

#endif  // IVW_DATAREADERFACTORY_H
