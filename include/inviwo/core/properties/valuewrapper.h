/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>

#include <concepts>

namespace inviwo {

enum class PropertySerializationMode { Default = 0, All, None };

template <typename T>
struct ValueWrapper {

    template <typename... U>
    explicit ValueWrapper(std::string_view name, U&&... vals)
        requires std::constructible_from<T, U&&...>
        : value(std::forward<U>(vals)...), defaultValue(value), name(name) {}

    ValueWrapper(const ValueWrapper<T>& rhs) = default;
    ValueWrapper(ValueWrapper<T>&& rhs) = default;
    ValueWrapper<T>& operator=(const ValueWrapper<T>& that) = default;
    ValueWrapper<T>& operator=(ValueWrapper<T>&& that) = default;
    ~ValueWrapper() = default;

    ValueWrapper<T>& operator=(const T& val) {
        value = val;
        return *this;
    }

    operator const T&() const { return value; }

    const T& operator*() const { return value; }
    const T* operator->() const { return &value; }

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
                if (d.hasElement(name)) {
                    auto old = value;
                    d.deserialize(name, value);
                    return old != value;
                } else {
                    // Need to call reset here since we might not deserialize if default. I.e. the
                    // lack of a serialized element means we should set the state to the default.
                    return reset();
                }
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

    /**
     * Update the value of this to that of src.
     * @returns if value was modified
     */
    template <typename U>
    bool update(const U& src)
        requires std::assignable_from<T&, U> &&
                 (!std::is_same_v<T, U>) && std::equality_comparable_with<const U&, const T&>
    {
        if (value != src) {
            value = src;
            return true;
        } else {
            return false;
        }
    }

    auto operator<=>(const ValueWrapper<T>& rhs) const { return value <=> rhs.value; }
    bool operator==(const ValueWrapper<T>& rhs) const { return value == rhs.value; }
    auto operator<=>(const T& rhs) const { return value <=> rhs; }
    bool operator==(const T& rhs) const { return value == rhs; }

    T value;
    T defaultValue;
    std::string name;
};

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <typename T>
struct fmt::formatter<inviwo::ValueWrapper<T>> : fmt::formatter<T> {
    template <typename FormatContext>
    auto format(const inviwo::ValueWrapper<T>& val, FormatContext& ctx) const {
        return fmt::formatter<T>::format(val.value, ctx);
    }
};
#endif
