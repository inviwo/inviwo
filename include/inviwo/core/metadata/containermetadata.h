/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_CONTAINERMETADATA_H
#define IVW_CONTAINERMETADATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/metadata/metadata.h>

namespace inviwo {

template <typename T>
class StdVectorMetaData : public MetaData {
public:
    StdVectorMetaData() = default;
    virtual ~StdVectorMetaData() = default;

    virtual std::string getClassIdentifier() const override {
        std::ostringstream name;
        name << "org.inviwo." << Defaultvalues<T>::getName() << "StdVectorMetaData";
        return name.str();
    }

    virtual MetaData* clone() const override { return new StdVectorMetaData<T>(*this); }

    virtual void serialize(Serializer& s) const override { s.serialize("Vector", vector_, "Item"); }

    virtual void deserialize(Deserializer& d) override { d.deserialize("Vector", vector_, "Item"); }

    virtual bool equal(const MetaData& rhs) const override {
        if (auto sv = dynamic_cast<const StdVectorMetaData<T>*>(&rhs)) {
            return vector_ == sv->vector_;
        } else {
            return false;
        }
    }

    std::vector<T>& getVector() { return vector_; }
    const std::vector<T>& getVector() const { return vector_; }

private:
    std::vector<T> vector_;
};

template <typename K, typename T>
class StdUnorderedMapMetaData : public MetaData {
public:
    StdUnorderedMapMetaData() = default;
    virtual ~StdUnorderedMapMetaData() = default;

    virtual std::string getClassIdentifier() const override {
        std::ostringstream name;
        name << "org.inviwo." << Defaultvalues<K>::getName() << Defaultvalues<T>::getName()
             << "StdUnorderedMapMetaData";
        return name.str();
    }

    virtual MetaData* clone() const override { return new StdUnorderedMapMetaData<K, T>(*this); }

    virtual void serialize(Serializer& s) const override {
        s.serialize(SerializeConstants::TypeAttribute, getClassIdentifier(),
                    SerializationTarget::Attribute);
        s.serialize("Map", map_, "Item");
    }

    virtual void deserialize(Deserializer& d) override { d.deserialize("Map", map_, "Item"); }

    virtual bool equal(const MetaData& rhs) const override {
        if (auto sv = dynamic_cast<const StdUnorderedMapMetaData<K, T>*>(&rhs)) {
            return map_ == sv->map_;
        } else {
            return false;
        }
    }

    std::map<K, T>& getMap() { return map_; }
    const std::map<K, T>& getMap() const { return map_; }

private:
    std::map<K, T> map_;
};

} // namespace

#endif // IVW_CONTAINERMETADATA_H

