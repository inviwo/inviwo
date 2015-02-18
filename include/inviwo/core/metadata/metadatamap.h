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

#ifndef IVW_METADATA_MAP_H
#define IVW_METADATA_MAP_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadatafactory.h>

namespace inviwo {

class IVW_CORE_API MetaDataMap : public IvwSerializable {
public:
    MetaDataMap();
    MetaDataMap(const MetaDataMap&);
    virtual ~MetaDataMap();
    virtual MetaDataMap* clone() const;
    void add(const std::string &key, MetaData* metaData);
    void remove(const std::string &key);
    void removeAll();
    void rename(const std::string &newKey, const std::string& oldKey);
    std::vector<std::string> getKeys() const;
    MetaData* get(const std::string &key);
    const MetaData* get(const std::string &key) const;

    bool empty() const { return metaData_.empty(); }
    
    MetaDataMap& operator=(const MetaDataMap& map);

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    typedef std::map<std::string, MetaData*>::const_iterator cIterator;
    typedef std::map<std::string, MetaData*>::iterator iterator;

    friend bool IVW_CORE_API operator==(const MetaDataMap& lhs, const MetaDataMap& rhs);

private:
    std::map<std::string, MetaData*> metaData_;
};

bool IVW_CORE_API operator==(const MetaDataMap& lhs, const MetaDataMap& rhs);
bool IVW_CORE_API operator!=(const MetaDataMap& lhs, const MetaDataMap& rhs);

}  // namespace

#endif  // IVW_METADATA_MAP_H
