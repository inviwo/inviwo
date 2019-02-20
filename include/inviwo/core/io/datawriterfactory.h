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

#ifndef IVW_DATAWRITERFACTORY_H
#define IVW_DATAWRITERFACTORY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

template <typename T>
class DataWriterType;

class IVW_CORE_API DataWriterFactory : public Factory<DataWriter, const FileExtension&> {
public:
    using Map = std::unordered_map<FileExtension, DataWriter*>;
    DataWriterFactory() = default;
    virtual ~DataWriterFactory() = default;

    bool registerObject(DataWriter* reader);
    bool unRegisterObject(DataWriter* reader);

    virtual std::unique_ptr<DataWriter> create(const std::string& key) const;
    virtual std::unique_ptr<DataWriter> create(const FileExtension& key) const override;

    virtual bool hasKey(const std::string& key) const;
    virtual bool hasKey(const FileExtension& key) const override;

    template <typename T>
    std::vector<FileExtension> getExtensionsForType();

    template <typename T>
    std::unique_ptr<DataWriterType<T>> getWriterForTypeAndExtension(const std::string& ext);

    template <typename T>
    std::unique_ptr<DataWriterType<T>> getWriterForTypeAndExtension(const FileExtension& ext);

    /**
     * First look for a writer using the FileExtension ext, and if no writer was found look for a
     * writer using the fallbackExt. This is often used with a file open dialog, where the dialog
     * will have a selectedFileExtension that will be used as ext, and the fallbackExt is taken from
     * the file to be written. This way any selected writer will have priority over the file
     * extension.
     */
    template <typename T>
    std::unique_ptr<DataWriterType<T>> getWriterForTypeAndExtension(const FileExtension& ext,
                                                                    const std::string& fallbackExt);

protected:
    Map map_;
};

template <typename T>
std::vector<FileExtension> DataWriterFactory::getExtensionsForType() {
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
    const std::string& ext) {
    auto lkey = toLower(ext);
    for (auto& elem : map_) {
        if (toLower(elem.first.extension_) == lkey) {
            if (auto r = dynamic_cast<DataWriterType<T>*>(elem.second)) {
                return std::unique_ptr<DataWriterType<T>>(r->clone());
            }
        }
    }

    return std::unique_ptr<DataWriterType<T>>();
}

template <typename T>
std::unique_ptr<DataWriterType<T>> DataWriterFactory::getWriterForTypeAndExtension(
    const FileExtension& ext) {
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
    const FileExtension& ext, const std::string& fallbackExt) {
    if (auto writer = this->getWriterForTypeAndExtension<T>(ext)) {
        return writer;
    }
    return this->getWriterForTypeAndExtension<T>(fallbackExt);
}

}  // namespace inviwo

#endif  // IVW_DATAWRITERFACTORY_H
