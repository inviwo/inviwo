/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>

namespace inviwo {

enum class PropertySerializationMode { Default = 0, All, None };

template <typename T>
struct ValueWrapper {
    ValueWrapper(std::string valname, T val) : value(val), defaultValue(val), name(valname) {}
    ValueWrapper(const ValueWrapper<T>& rhs) = default;
    ValueWrapper<T>& operator=(const ValueWrapper<T>& that) = default;

    ValueWrapper<T>& operator=(const T& val) {
        value = val;
        return *this;
    }
    operator T&() { return value; }
    operator const T&() const { return value; }

    bool isDefault() const { return value == defaultValue; }

    // return if the value changes while resetting
    bool reset() { return update(defaultValue); }
    void setAsDefault() { defaultValue = value; }

    void serialize(Serializer& s,
                   PropertySerializationMode mode = PropertySerializationMode::Default) const {
        switch (mode) {
            case PropertySerializationMode::Default:
                if (!isDefault()) s.serialize(name, value);
                break;
            case PropertySerializationMode::All:
                s.serialize(name, value);
                break;
            case PropertySerializationMode::None:
                break;
        }
    }

    // return if the value changes while deserializing
    bool deserialize(Deserializer& d,
                     PropertySerializationMode mode = PropertySerializationMode::Default) {
        switch (mode) {
            case PropertySerializationMode::Default: {
                auto old = value;
                reset();  // Need to call reset here since we might not deserialize if default.
                d.deserialize(name, value);
                return old != value;
            }
            case PropertySerializationMode::All: {
                auto old = value;
                d.deserialize(name, value);
                return old != value;
            }
            case PropertySerializationMode::None:
                return false;
            default:
                return false;
        }
    }

    /**
     * Update the value of this to that of src.
     * @returns if value was modified
     */
    bool update(const ValueWrapper<T>& src) {
        if (value != src.value) {
            value = src.value;
            return true;
        } else {
            return false;
        }
    }
    /**
     * Update the value of this to that of src.
     * @returns if value was modified
     */
    bool update(const T& src) {
        if (value != src) {
            value = src;
            return true;
        } else {
            return false;
        }
    }

    T value;
    T defaultValue;
    std::string name;
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

}  // namespace inviwo

#endif  // IVW_VALUEWRAPPER_H
