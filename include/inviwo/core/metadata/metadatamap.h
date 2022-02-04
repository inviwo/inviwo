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
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadatafactory.h>

namespace inviwo {

class IVW_CORE_API MetaDataMap : public Serializable {
public:
    MetaDataMap() = default;
    MetaDataMap(const MetaDataMap&);
    MetaDataMap& operator=(const MetaDataMap& map);
    virtual ~MetaDataMap() = default;

    MetaData* add(std::string_view key, MetaData* metaData);
    template <typename T>  // T Should derive from MetaData
    T* add(std::string_view key, std::unique_ptr<T> metaData);

    void remove(std::string_view key);
    void removeAll();

    void rename(std::string_view newKey, const std::string& oldKey);

    std::vector<std::string> getKeys() const;
    MetaData* get(std::string_view key);
    const MetaData* get(std::string_view key) const;

    bool empty() const;

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    friend bool IVW_CORE_API operator==(const MetaDataMap& lhs, const MetaDataMap& rhs);
    friend bool IVW_CORE_API operator!=(const MetaDataMap& lhs, const MetaDataMap& rhs);

private:
    std::map<std::string, std::unique_ptr<MetaData>, std::less<>> metaData_;
};

template <typename T>
T* MetaDataMap::add(std::string_view key, std::unique_ptr<T> metaData) {
    auto ptr = metaData.get();
    metaData_[std::string{key}] = std::move(metaData);
    return ptr;
}

}  // namespace inviwo
