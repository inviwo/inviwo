/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/stringconversion.h>

#include <inviwo/core/io/datareader.h>

namespace inviwo {

class IVW_CORE_API DataReaderFactory : public Factory<DataReader>,
                                       public Singleton<DataReaderFactory> {
public:
    using Map = std::unordered_map<std::string, DataReader*>;

    DataReaderFactory() = default;
    virtual ~DataReaderFactory() = default;

    bool registerObject(DataReader* reader);
    virtual std::unique_ptr<DataReader> create(const std::string& key) const override;
    virtual bool hasKey(const std::string& key) const override;

    template <typename T>
    std::vector<FileExtension> getExtensionsForType();

    template <typename T>
    std::unique_ptr<DataReaderType<T>> getReaderForTypeAndExtension(const std::string& ext);

protected:
    Map map_;
};

template <typename T>
std::vector<FileExtension> DataReaderFactory::getExtensionsForType() {
    std::vector<FileExtension> ext;

    for (auto reader : map_) {
        if (auto r = dynamic_cast<DataReaderType<T>*>(reader.second)) {
            auto rext = r->getExtensions();
            ext.insert(ext.end(), rext.begin(), rext.end());
        }
    }
    return ext;
}

template <typename T>
std::unique_ptr<DataReaderType<T>> DataReaderFactory::getReaderForTypeAndExtension(
    const std::string& ext) {
    auto it = map_.find(toLower(ext));

    if (it != map_.end()) {
        if (auto r = dynamic_cast<DataReaderType<T>*>(it->second)) {
            return std::unique_ptr<DataReaderType<T>>(r->clone());
        }
    }

    return std::unique_ptr<DataReaderType<T>>();
}

}  // namespace

#endif  // IVW_DATAREADERFACTORY_H
