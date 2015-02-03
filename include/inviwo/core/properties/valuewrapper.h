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

#ifndef IVW_VALUEWRAPPER_H
#define IVW_VALUEWRAPPER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

enum IVW_CORE_API PropertySerializationMode { DEFAULT = 0, ALL };

template <typename T>
struct ValueWrapper {
    ValueWrapper(T val) : value(val), defaultValue(val), name("") {}
    ValueWrapper(std::string valname, T val) : value(val), defaultValue(val), name(valname) {}
    ValueWrapper(const ValueWrapper<T>& rhs)
        : value(rhs.value), defaultValue(rhs.defaultValue), name(rhs.name) {}
    ValueWrapper<T>& operator=(const ValueWrapper<T>& that) {
        if (this != &that) {
            value = that.value;
            defaultValue = that.defaultValue;
            name = that.name;
        }
        return *this;
    }
    ValueWrapper<T>& operator=(const T& val) {
        value = val;
        return *this;
    }
    operator T&() { return value; }
    operator const T&() const { return value; }

    bool isDefault() const { return value == defaultValue; }
    void reset() { value = defaultValue; }
    void setAsDefault() { defaultValue = value; }

    void serialize(IvwSerializer& s, PropertySerializationMode mode = DEFAULT) const {
        if (mode == ALL || !isDefault()) s.serialize(name, value);
    }

    void deserialize(IvwDeserializer& d) { d.deserialize(name, value); }

    T value;
    T defaultValue;
    std::string name;

    template <typename U>
    friend bool operator==(const ValueWrapper<U>& lhs, const ValueWrapper<U>& rhs);
    template <typename U>
    friend bool operator<(const ValueWrapper<U>& lhs, const ValueWrapper<U>& rhs);

    template <typename U>
    friend bool operator==(const ValueWrapper<U>& lhs, const U& rhs);
    template <typename U>
    friend bool operator<(const ValueWrapper<U>& lhs, const U& rhs);
};

template <typename T>
bool operator==(const ValueWrapper<T>& lhs, const ValueWrapper<T>& rhs) {
    return lhs.value == rhs.value;
}
template <typename T>
bool operator<(const ValueWrapper<T>& lhs, const ValueWrapper<T>& rhs) {
    return lhs.value < rhs.value;
}
template <typename T>
bool operator!=(const ValueWrapper<T>& lhs, const ValueWrapper<T>& rhs) {
    return !operator==(lhs, rhs);
}
template <typename T>
bool operator>(const ValueWrapper<T>& lhs, const ValueWrapper<T>& rhs) {
    return operator<(rhs, lhs);
}
template <typename T>
bool operator<=(const ValueWrapper<T>& lhs, const ValueWrapper<T>& rhs) {
    return !operator>(lhs, rhs);
}
template <typename T>
bool operator>=(const ValueWrapper<T>& lhs, const ValueWrapper<T>& rhs) {
    return !operator<(lhs, rhs);
}

template <typename T>
bool operator==(const ValueWrapper<T>& lhs, const T& rhs) {
    return lhs.value == rhs;
}
template <typename T>
bool operator<(const ValueWrapper<T>& lhs, const T& rhs) {
    return lhs.value < rhs;
}
template <typename T>
bool operator!=(const ValueWrapper<T>& lhs, const T& rhs) {
    return !operator==(lhs, rhs);
}
template <typename T>
bool operator>(const ValueWrapper<T>& lhs, const T& rhs) {
    return operator<(rhs, lhs);
}
template <typename T>
bool operator<=(const ValueWrapper<T>& lhs, const T& rhs) {
    return !operator>(lhs, rhs);
}
template <typename T>
bool operator>=(const ValueWrapper<T>& lhs, const T& rhs) {
    return !operator<(lhs, rhs);
}

template <typename T>
bool operator==(const T& lhs, const ValueWrapper<T>& rhs) {
    return rhs == lhs;
}
template <typename T>
bool operator<(const T& lhs, const ValueWrapper<T>& rhs) {
    return rhs >= lhs;
}
template <typename T>
bool operator!=(const T& lhs, const ValueWrapper<T>& rhs) {
    return !operator==(rhs, lhs);
}
template <typename T>
bool operator>(const T& lhs, const ValueWrapper<T>& rhs) {
    return operator<(rhs, lhs);
}
template <typename T>
bool operator<=(const T& lhs, const ValueWrapper<T>& rhs) {
    return !operator>(lhs, rhs);
}
template <typename T>
bool operator>=(const T& lhs, const ValueWrapper<T>& rhs) {
    return !operator<(lhs, rhs);
}

}  // namespace

#endif  // IVW_VALUEWRAPPER_H
