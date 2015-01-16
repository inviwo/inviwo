/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_METADATA_H
#define IVW_METADATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/formats.h>

namespace inviwo {

class IVW_CORE_API MetaData : public IvwSerializable {

public:
    MetaData();
    MetaData(const MetaData& rhs);
    MetaData& operator=(const MetaData& that);
    virtual ~MetaData();
    virtual std::string getClassIdentifier() const;
    virtual MetaData* clone() const;
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);
    virtual bool equal(const MetaData& rhs) const;
    friend bool IVW_CORE_API operator==(const MetaData& lhs, const MetaData& rhs);
};

bool IVW_CORE_API operator==(const MetaData& lhs, const MetaData& rhs);
bool IVW_CORE_API operator!=(const MetaData& lhs, const MetaData& rhs);


template <typename T, int N, int M>
class MetaDataPrimitiveType : public MetaData {};

template <typename T>
class MetaDataPrimitiveType<T, 0, 0> : public MetaData {
public:
    MetaDataPrimitiveType();
    MetaDataPrimitiveType(T value);
    MetaDataPrimitiveType(const MetaDataPrimitiveType& rhs);
    MetaDataPrimitiveType& operator=(const MetaDataPrimitiveType& that);
    virtual ~MetaDataPrimitiveType() {};
    virtual std::string getClassIdentifier() const;
    virtual MetaDataPrimitiveType<T, 0, 0>* clone() const;
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);
    virtual void set(T value);
    virtual T get() const;
    virtual bool equal(const MetaData& rhs) const;
    template <typename V>
    friend bool operator==(const MetaDataPrimitiveType<V, 0, 0>& lhs,
                           const MetaDataPrimitiveType<V, 0, 0>& rhs);
protected:
    T value_;
};

template <typename T>
MetaDataPrimitiveType<T, 0, 0>::MetaDataPrimitiveType() : MetaData(), value_() {}

template <typename T>
MetaDataPrimitiveType<T, 0, 0>::MetaDataPrimitiveType(T value) : MetaData(), value_(value) {}

template <typename T>
bool inviwo::MetaDataPrimitiveType<T, 0, 0>::equal(const MetaData& rhs) const {
    const MetaDataPrimitiveType<T, 0, 0>* tmp = dynamic_cast<const MetaDataPrimitiveType<T, 0, 0>*>(&rhs);
    if (tmp) {
        return tmp->value_ == value_;
    } else {
        return false;
    }
}

template <typename T>
bool operator==(const MetaDataPrimitiveType<T, 0, 0>& lhs, const MetaDataPrimitiveType<T, 0, 0>& rhs) {
    return lhs.value_ == rhs.value_;
}

template <typename T>
MetaDataPrimitiveType<T, 0, 0>::MetaDataPrimitiveType(const MetaDataPrimitiveType<T, 0, 0>& rhs)
    : MetaData(rhs), value_(rhs.value_) {}

template <typename T>
MetaDataPrimitiveType<T, 0, 0>& MetaDataPrimitiveType<T, 0, 0>::operator=(
    const MetaDataPrimitiveType<T, 0, 0>& that) {
    if (this != &that) {
        value_ = that.value_;
        MetaData::operator=(that);
    }
    return *this;
}

template <typename T>
std::string MetaDataPrimitiveType<T, 0, 0>::getClassIdentifier() const {
    std::ostringstream name;
    name << "org.inviwo." << Defaultvalues<T>::getName() << "MetaData";
    return name.str();
}

template <typename T>
MetaDataPrimitiveType<T, 0, 0>* MetaDataPrimitiveType<T, 0, 0>::clone() const {
    return new MetaDataPrimitiveType<T, 0, 0>(*this);
}

template <typename T>
void MetaDataPrimitiveType<T, 0, 0>::set(T value) {
    value_ = value;
}

template <typename T>
T MetaDataPrimitiveType<T, 0, 0>::get() const {
    return value_;
}

template <typename T>
void inviwo::MetaDataPrimitiveType<T, 0, 0>::serialize(IvwSerializer& s) const {
    s.serialize("MetaData", value_);
    s.serialize(IvwSerializeConstants::TYPE_ATTRIBUTE, getClassIdentifier(), true);
}

template <typename T>
void inviwo::MetaDataPrimitiveType<T, 0, 0>::deserialize(IvwDeserializer& d) {
    d.deserialize("MetaData", value_);
}


typedef MetaDataPrimitiveType<bool, 0, 0> BoolMetaData;
typedef MetaDataPrimitiveType<int, 0, 0> IntMetaData;
typedef MetaDataPrimitiveType<float, 0, 0> FloatMetaData;
typedef MetaDataPrimitiveType<double, 0, 0> DoubleMetaData;
typedef MetaDataPrimitiveType<std::string, 0, 0> StringMetaData;

typedef MetaDataPrimitiveType<vec2, 0, 0> FloatVec2MetaData;
typedef MetaDataPrimitiveType<vec3, 0, 0> FloatVec3MetaData;
typedef MetaDataPrimitiveType<vec4, 0, 0> FloatVec4MetaData;

typedef MetaDataPrimitiveType<dvec2, 0, 0> DoubleVec2MetaData;
typedef MetaDataPrimitiveType<dvec3, 0, 0> DoubleVec3MetaData;
typedef MetaDataPrimitiveType<dvec4, 0, 0> DoubleVec4MetaData;

typedef MetaDataPrimitiveType<ivec2, 0, 0> IntVec2MetaData;
typedef MetaDataPrimitiveType<ivec3, 0, 0> IntVec3MetaData;
typedef MetaDataPrimitiveType<ivec4, 0, 0> IntVec4MetaData;

typedef MetaDataPrimitiveType<uvec2, 0, 0> UIntVec2MetaData;
typedef MetaDataPrimitiveType<uvec3, 0, 0> UIntVec3MetaData;
typedef MetaDataPrimitiveType<uvec4, 0, 0> UIntVec4MetaData;

typedef MetaDataPrimitiveType<mat2, 0, 0> FloatMat2MetaData;
typedef MetaDataPrimitiveType<mat3, 0, 0> FloatMat3MetaData;
typedef MetaDataPrimitiveType<mat4, 0, 0> FloatMat4MetaData;

typedef MetaDataPrimitiveType<dmat2, 0, 0> DoubleMat2MetaData;
typedef MetaDataPrimitiveType<dmat3, 0, 0> DoubleMat4MetaData;
typedef MetaDataPrimitiveType<dmat4, 0, 0> DoubleMat3MetaData;

// Vector specialization
template <typename T, int N>
class MetaDataPrimitiveType<T, N, 0> : public MetaData {
public:
    MetaDataPrimitiveType();
    MetaDataPrimitiveType(T value);
    MetaDataPrimitiveType(const MetaDataPrimitiveType& rhs);
    MetaDataPrimitiveType& operator=(const MetaDataPrimitiveType& that);
    virtual ~MetaDataPrimitiveType() {};
    virtual std::string getClassIdentifier() const;
    virtual MetaDataPrimitiveType<T, N, 0>* clone() const;
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);
    virtual void set(Vector<N, T> value);
    virtual Vector<N, T> get() const;
    virtual bool equal(const MetaData& rhs) const;
    template <typename V>
    friend bool operator==(const MetaDataPrimitiveType<V, N, 0>& lhs,
                           const MetaDataPrimitiveType<V, 0, 0>& rhs);
protected:
    Vector<N, T> value_;
};

template <typename T, int N>
MetaDataPrimitiveType<T, N, 0>::MetaDataPrimitiveType() : MetaData(), value_(0) {}

template <typename T, int N>
MetaDataPrimitiveType<T, N, 0>::MetaDataPrimitiveType(T value) : MetaData(), value_(value) {}

template <typename T, int N>
bool inviwo::MetaDataPrimitiveType<T, N, 0>::equal(const MetaData& rhs) const {
    const MetaDataPrimitiveType<T, N, 0>* tmp = dynamic_cast<const MetaDataPrimitiveType<T, N, 0>*>(&rhs);
    if (tmp) {
        return tmp->value_ == value_;
    } else {
        return false;
    }
}

template <typename T, int N>
bool operator==(const MetaDataPrimitiveType<T, N, 0>& lhs, const MetaDataPrimitiveType<T, N, 0>& rhs) {
    return lhs.value_ == rhs.value_;
}

template <typename T, int N>
MetaDataPrimitiveType<T, N, 0>::MetaDataPrimitiveType(const MetaDataPrimitiveType<T, N, 0>& rhs)
    : MetaData(rhs), value_(rhs.value_) {}

template <typename T, int N>
MetaDataPrimitiveType<T, N, 0>& MetaDataPrimitiveType<T, N, 0>::operator=(
    const MetaDataPrimitiveType<T, N, 0>& that) {
    if (this != &that) {
        value_ = that.value_;
        MetaData::operator=(that);
    }

    return *this;
}

template <typename T, int N>
std::string MetaDataPrimitiveType<T, N, 0>::getClassIdentifier() const {
    std::ostringstream name;
    name << "org.inviwo.VectorMetaData<" << N << ", " << Defaultvalues<T>::getName() << ">";
    return name.str();
}

template <typename T, int N>
MetaDataPrimitiveType<T, N, 0>* MetaDataPrimitiveType<T, N, 0>::clone() const {
    return new MetaDataPrimitiveType<T, N, 0>(*this);
}

template <typename T, int N>
void MetaDataPrimitiveType<T, N, 0>::set(Vector<N, T> value) {
    value_ = value;
}

template <typename T, int N>
Vector<N, T> MetaDataPrimitiveType<T, N, 0>::get() const {
    return value_;
}

template <typename T, int N>
void inviwo::MetaDataPrimitiveType<T, N, 0>::serialize(IvwSerializer& s) const {
    s.serialize(IvwSerializeConstants::TYPE_ATTRIBUTE, getClassIdentifier(), true);
    s.serialize("MetaData", value_);
}

template <typename T, int N>
void inviwo::MetaDataPrimitiveType<T, N, 0>::deserialize(IvwDeserializer& d) {
    d.deserialize("MetaData", value_);
}

template <int N, typename T>
class VectorMetaData : public MetaDataPrimitiveType<T, N, 0> {
public:
    VectorMetaData() : MetaDataPrimitiveType<T, N, 0>() {}
    VectorMetaData(T value) : MetaDataPrimitiveType<T, N, 0>(value) {}
    VectorMetaData(const VectorMetaData& rhs) : MetaDataPrimitiveType<T, N, 0>(rhs) {}
    VectorMetaData& operator=(const VectorMetaData& that) {
        if (this != &that) {
            MetaDataPrimitiveType<T, N, 0>::operator=(that);
        }
        return *this;
    }
    virtual ~VectorMetaData(){};
    virtual VectorMetaData<N, T>* clone() const { return new VectorMetaData<N, T>(*this); }
};


// Matrix specialization
template <typename T, int N>
class MetaDataPrimitiveType<T, N, N> : public MetaData {
public:
    MetaDataPrimitiveType();
    MetaDataPrimitiveType(T value);
    MetaDataPrimitiveType(const MetaDataPrimitiveType& rhs);
    MetaDataPrimitiveType& operator=(const MetaDataPrimitiveType& that);
    virtual ~MetaDataPrimitiveType() {};
    virtual std::string getClassIdentifier() const;
    virtual MetaDataPrimitiveType<T, N, N>* clone() const;
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);
    virtual void set(Matrix<N, T> value);
    virtual Matrix<N, T> get() const;
    virtual bool equal(const MetaData& rhs) const;
    template <typename V>
    friend bool operator==(const MetaDataPrimitiveType<V, N, N>& lhs,
                           const MetaDataPrimitiveType<V, N, N>& rhs);
protected:
    Matrix<N, T> value_;
};

template <typename T, int N>
MetaDataPrimitiveType<T, N, N>::MetaDataPrimitiveType() : MetaData(), value_(0) {}

template <typename T, int N>
MetaDataPrimitiveType<T, N, N>::MetaDataPrimitiveType(T value) : MetaData(), value_(value) {}

template <typename T, int N>
bool inviwo::MetaDataPrimitiveType<T, N, N>::equal(const MetaData& rhs) const {
    const MetaDataPrimitiveType<T, N, N>* tmp = dynamic_cast<const MetaDataPrimitiveType<T, N, N>*>(&rhs);
    if (tmp) {
        return tmp->value_ == value_;
    } else {
        return false;
    }
}

template <typename T, int N>
bool operator==(const MetaDataPrimitiveType<T, N, N>& lhs, const MetaDataPrimitiveType<T, N, N>& rhs) {
    return lhs.value_ == rhs.value_;
}

template <typename T, int N>
MetaDataPrimitiveType<T, N, N>::MetaDataPrimitiveType(const MetaDataPrimitiveType<T, N, N>& rhs)
    : MetaData(rhs), value_(rhs.value_) {}

template <typename T, int N>
MetaDataPrimitiveType<T, N, N>& MetaDataPrimitiveType<T, N, N>::operator=(
    const MetaDataPrimitiveType<T, N, N>& that) {
    if (this != &that) {
        value_ = that.value_;
        MetaData::operator=(that);
    }

    return *this;
}

template <typename T, int N>
std::string MetaDataPrimitiveType<T, N, N>::getClassIdentifier() const {
    std::ostringstream name;
    name << "org.inviwo.MatrixMetaData<" << N << ", " << Defaultvalues<T>::getName() << ">";
    return name.str();
}

template <typename T, int N>
MetaDataPrimitiveType<T, N, N>* MetaDataPrimitiveType<T, N, N>::clone() const {
    return new MetaDataPrimitiveType<T, N, N>(*this);
}

template <typename T, int N>
void MetaDataPrimitiveType<T, N, N>::set(Matrix<N, T> value) {
    value_ = value;
}

template <typename T, int N>
Matrix<N, T> MetaDataPrimitiveType<T, N, N>::get() const {
    return value_;
}

template <typename T, int N>
void inviwo::MetaDataPrimitiveType<T, N, N>::serialize(IvwSerializer& s) const {
    s.serialize(IvwSerializeConstants::TYPE_ATTRIBUTE, getClassIdentifier(), true);
    s.serialize("MetaData", value_);
}

template <typename T, int N>
void inviwo::MetaDataPrimitiveType<T, N, N>::deserialize(IvwDeserializer& d) {
    d.deserialize("MetaData", value_);
}

template <int N, typename T>
class MatrixMetaData : public MetaDataPrimitiveType<T, N, N> {
public:
    MatrixMetaData() : MetaDataPrimitiveType<T, N, N>() {}
    MatrixMetaData(T value) : MetaDataPrimitiveType<T, N, N>(value) {}
    MatrixMetaData(const MatrixMetaData& rhs) : MetaDataPrimitiveType<T, N, N>(rhs) {}
    MatrixMetaData& operator=(const MatrixMetaData& that) {
        if (this != &that) {
            MetaDataPrimitiveType<T, N, N>::operator=(that);
        }
        return *this;
    }
    virtual ~MatrixMetaData(){};
    virtual MatrixMetaData<N, T>* clone() const { return new MatrixMetaData<N, T>(*this); }
};

} // namespace

#endif // IVW_PROCESSOR_H
