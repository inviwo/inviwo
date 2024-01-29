/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>

#include <string>

namespace inviwo {

class IVW_CORE_API MetaData : public Serializable {
public:
    virtual ~MetaData() = default;
    virtual const std::string& getClassIdentifier() const = 0;
    virtual MetaData* clone() const = 0;
    virtual void serialize(Serializer& s) const = 0;
    virtual void deserialize(Deserializer& d) = 0;
    virtual bool equal(const MetaData& rhs) const = 0;
    friend bool IVW_CORE_API operator==(const MetaData& lhs, const MetaData& rhs);
    friend bool IVW_CORE_API operator!=(const MetaData& lhs, const MetaData& rhs);

protected:
    MetaData() = default;
    MetaData(const MetaData& rhs) = default;
    MetaData(MetaData&& rhs) = default;
    MetaData& operator=(const MetaData& that) = default;
    MetaData& operator=(MetaData&& that) = default;
};

template <typename T>
class MetaDataType : public MetaData {
public:
    MetaDataType();
    MetaDataType(T value);
    MetaDataType(const MetaDataType& rhs) = default;
    MetaDataType(MetaDataType&& rhs) = default;
    MetaDataType& operator=(const MetaDataType& that) = default;
    MetaDataType& operator=(MetaDataType&& that) = default;
    virtual ~MetaDataType() = default;
    virtual const std::string& getClassIdentifier() const override;
    virtual MetaDataType<T>* clone() const override;
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;
    virtual void set(T value);
    virtual T get() const;
    virtual bool equal(const MetaData& rhs) const override;

    template <typename V>
    friend bool operator==(const MetaDataType<V>& lhs, const MetaDataType<V>& rhs);

protected:
    T value_;
};

template <typename T>
MetaDataType<T>::MetaDataType() : MetaData(), value_() {}

template <typename T>
MetaDataType<T>::MetaDataType(T value) : MetaData(), value_(value) {}

template <typename T>
bool MetaDataType<T>::equal(const MetaData& rhs) const {
    if (const auto* tmp = dynamic_cast<const MetaDataType<T>*>(&rhs)) {
        return tmp->value_ == value_;
    } else {
        return false;
    }
}

template <typename T>
bool operator==(const MetaDataType<T>& lhs, const MetaDataType<T>& rhs) {
    return lhs.value_ == rhs.value_;
}

template <typename T>
const std::string& MetaDataType<T>::getClassIdentifier() const {
    static const std::string identifier = "org.inviwo." + Defaultvalues<T>::getName() + "MetaData";
    return identifier;
}

template <typename T>
MetaDataType<T>* MetaDataType<T>::clone() const {
    return new MetaDataType<T>(*this);
}

template <typename T>
void MetaDataType<T>::set(T value) {
    value_ = value;
}

template <typename T>
T MetaDataType<T>::get() const {
    return value_;
}

template <typename T>
void MetaDataType<T>::serialize(Serializer& s) const {
    s.serialize("MetaData", value_);
    s.serialize(SerializeConstants::TypeAttribute, getClassIdentifier(),
                SerializationTarget::Attribute);
}

template <typename T>
void MetaDataType<T>::deserialize(Deserializer& d) {
    d.deserialize("MetaData", value_);
}

using BoolMetaData = MetaDataType<bool>;
using IntMetaData = MetaDataType<int>;
using FloatMetaData = MetaDataType<float>;
using DoubleMetaData = MetaDataType<double>;
using StringMetaData = MetaDataType<std::string>;
using SizeMetaData = MetaDataType<size_t>;

using FloatVec2MetaData = MetaDataType<vec2>;
using FloatVec3MetaData = MetaDataType<vec3>;
using FloatVec4MetaData = MetaDataType<vec4>;

using DoubleVec2MetaData = MetaDataType<dvec2>;
using DoubleVec3MetaData = MetaDataType<dvec3>;
using DoubleVec4MetaData = MetaDataType<dvec4>;

using IntVec2MetaData = MetaDataType<ivec2>;
using IntVec3MetaData = MetaDataType<ivec3>;
using IntVec4MetaData = MetaDataType<ivec4>;

using UIntVec2MetaData = MetaDataType<uvec2>;
using UIntVec3MetaData = MetaDataType<uvec3>;
using UIntVec4MetaData = MetaDataType<uvec4>;

using FloatMat2MetaData = MetaDataType<mat2>;
using FloatMat3MetaData = MetaDataType<mat3>;
using FloatMat4MetaData = MetaDataType<mat4>;

using DoubleMat2MetaData = MetaDataType<dmat2>;
using DoubleMat3MetaData = MetaDataType<dmat3>;
using DoubleMat4MetaData = MetaDataType<dmat4>;

using Size2MetaData = MetaDataType<size2_t>;
using Size3MetaData = MetaDataType<size3_t>;
using Size4MetaData = MetaDataType<size4_t>;

}  // namespace inviwo
