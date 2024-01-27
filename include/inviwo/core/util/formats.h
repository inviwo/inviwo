/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmconvert.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/staticstring.h>

#include <glm/gtc/type_precision.hpp>

#include <limits>
#include <string>
#include <array>
#include <memory>
#include <span>
#include <fmt/format.h>

namespace inviwo {

class DataFormatBase;

// Do not set enums specifically, as NumberOfFormats is used to count the number of enums
enum class DataFormatId : char {
    NotSpecialized = 0,
    Float32,
    Float64,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Vec2Float32,
    Vec2Float64,
    Vec2Int8,
    Vec2Int16,
    Vec2Int32,
    Vec2Int64,
    Vec2UInt8,
    Vec2UInt16,
    Vec2UInt32,
    Vec2UInt64,
    Vec3Float32,
    Vec3Float64,
    Vec3Int8,
    Vec3Int16,
    Vec3Int32,
    Vec3Int64,
    Vec3UInt8,
    Vec3UInt16,
    Vec3UInt32,
    Vec3UInt64,
    Vec4Float32,
    Vec4Float64,
    Vec4Int8,
    Vec4Int16,
    Vec4Int32,
    Vec4Int64,
    Vec4UInt8,
    Vec4UInt16,
    Vec4UInt32,
    Vec4UInt64,
    NumberOfFormats,
};

enum class NumericType : char { NotSpecialized, Float, UnsignedInteger, SignedInteger };

namespace util {

template <typename T>
constexpr NumericType getNumericType() {
    return (util::is_floating_point<T>::value
                ? NumericType::Float
                : (std::is_signed<T>::value ? NumericType::SignedInteger
                                            : NumericType::UnsignedInteger));
}

/**
 * Determines the common numeric type of the given \p formats. Returns NumericType::Float if any of
 * the formats is a floating point format. Otherwise, NumericType::SignedInteger is returned if at
 * least one format is signed, and NumericType::UnsignedInteger if that is not the case.
 * @return NumericType based on \p formats. NumericType::NotSpecialized if the span is empty.
 */
IVW_CORE_API NumericType commonNumericType(std::span<const DataFormatBase*> formats);

/**
 * Determines the format precision of the given \p formats, that is the highest one.
 * @return maximum precision of all \p formats
 */
IVW_CORE_API size_t commonFormatPrecision(std::span<const DataFormatBase*> formats);

namespace detail {
template <template <typename> typename Wrapper, typename List>
struct wrap;

template <template <typename> typename Wrapper, template <typename...> typename List,
          typename... Ts>
struct wrap<Wrapper, List<Ts...>> {
    using type = List<Wrapper<Ts>...>;
};
}  // namespace detail

template <template <typename> typename Wrapper, typename List>
using wrap = typename detail::wrap<Wrapper, List>::type;

}  // namespace util

class IVW_CORE_API DataFormatException : public Exception {
public:
    using Exception::Exception;
};

/** \brief Defines general useful formats and new data types
 * Non-virtual, meaning no dynamic_cast as string comparison is as fast/faster
 */
class IVW_CORE_API DataFormatBase {
public:
    static const DataFormatBase* get();
    static const DataFormatBase* get(DataFormatId id);
    static const DataFormatBase* get(std::string_view name);
    static const DataFormatBase* get(NumericType type, size_t components, size_t precision);

    // Runtime interface
    /**
     *	Returns the size of the format in bytes. For all components.
     */
    size_t getSizeInBytes() const;
    [[deprecated("use getSizeInBytes")]] size_t getSize() const;
    /**
     *	Returns the number of components in the format, 1 to 4.
     */
    size_t getComponents() const;
    /**
     *	Returns number of bits in each component in the format. can be 8, 16, 32 or 64.
     */
    size_t getPrecision() const;

    NumericType getNumericType() const;
    double getMax() const;
    double getMin() const;
    double getLowest() const;
    std::string_view getString() const;
    DataFormatId getId() const;

    template <typename T>
    static constexpr DataFormatId typeToId() noexcept;

protected:
    DataFormatBase(DataFormatId type, size_t components, size_t size, double max, double min,
                   double lowest, NumericType nt, std::string_view s);
    DataFormatBase();
    DataFormatBase(const DataFormatBase&) = delete;
    DataFormatBase(DataFormatBase&&) = delete;
    DataFormatBase& operator=(const DataFormatBase&) = delete;
    DataFormatBase& operator=(DataFormatBase&&) = delete;

    DataFormatId formatId_;
    size_t components_;
    size_t sizeInBytes_;
    NumericType numericType_;
    double max_;
    double min_;
    double lowest_;
    std::string_view formatStr_;
};

template <typename T>
class DataFormat final : public DataFormatBase {
public:
    using type = T;
    using primitive = typename util::value_type<T>::type;
    static const size_t comp = util::extent<T>::value;
    static const size_t typesize = sizeof(type);
    static const size_t compsize = sizeof(primitive);
    static const NumericType numtype = util::getNumericType<primitive>();

    // Static interface
    static constexpr DataFormatId id();
    static const DataFormat<T>* get();

    /**
     *	Returns the size of the format in bytes. For all components.
     */
    static constexpr size_t sizeInBytes();
    /**
     *	Returns the number of components in the format, 1 to 4.
     */
    static constexpr size_t components();
    /**
     *	Returns number of bits in each component in the format. can be 8, 16, 32 or 64.
     */
    static constexpr size_t precision();
    static constexpr NumericType numericType();

    static constexpr T max();
    static constexpr T min();
    static constexpr T lowest();
    static constexpr double maxToDouble();
    static constexpr double minToDouble();
    static constexpr double lowestToDouble();
    static std::string_view str();
    static constexpr auto staticStr();

private:
    friend DataFormatBase;
    DataFormat();
};

using DefaultDataTypes =
    std::tuple<glm::f32, glm::f64, glm::i8, glm::i16, glm::i32, glm::i64, glm::u8, glm::u16,
               glm::u32, glm::u64, glm::f32vec2, glm::f64vec2, glm::i8vec2, glm::i16vec2,
               glm::i32vec2, glm::i64vec2, glm::u8vec2, glm::u16vec2, glm::u32vec2, glm::u64vec2,
               glm::f32vec3, glm::f64vec3, glm::i8vec3, glm::i16vec3, glm::i32vec3, glm::i64vec3,
               glm::u8vec3, glm::u16vec3, glm::u32vec3, glm::u64vec3, glm::f32vec4, glm::f64vec4,
               glm::i8vec4, glm::i16vec4, glm::i32vec4, glm::i64vec4, glm::u8vec4, glm::u16vec4,
               glm::u32vec4, glm::u64vec4>;

using DefaultDataFormats = util::wrap<DataFormat, DefaultDataTypes>;

template <typename T>
DataFormat<T>::DataFormat()
    : DataFormatBase(id(), components(), sizeInBytes(), maxToDouble(), minToDouble(),
                     lowestToDouble(), numericType(), str()) {}

template <typename T>
constexpr DataFormatId DataFormat<T>::id() {
    return typeToId<T>();
}
template <typename T>
constexpr DataFormatId DataFormatBase::typeToId() noexcept {
    constexpr size_t index = []<std::size_t... Is>(
                                 std::integer_sequence<std::size_t, Is...>) noexcept {
        return ((std::is_same_v<std::tuple_element_t<Is, DefaultDataTypes>, T> ? Is + 1 : 0) + ...);
    }(std::make_integer_sequence<std::size_t, std::tuple_size_v<DefaultDataTypes>>{});

    if constexpr (index == 0) {
        return DataFormatId::NotSpecialized;
    } else {
        return DataFormatId{index};
    }
}

template <typename T>
const DataFormat<T>* DataFormat<T>::get() {
    return static_cast<const DataFormat<T>*>(DataFormatBase::get(id()));
}

template <typename T>
constexpr size_t DataFormat<T>::sizeInBytes() {
    return typesize;
}

template <typename T>
constexpr size_t DataFormat<T>::components() {
    return comp;
}

template <typename T>
constexpr size_t DataFormat<T>::precision() {
    return sizeInBytes() / components() * 8;
}

template <typename T>
constexpr NumericType DataFormat<T>::numericType() {
    return numtype;
}

template <typename T>
constexpr auto DataFormat<T>::staticStr() {
    constexpr auto prefix = []() {
        if constexpr (components() == 1) {
            return StaticString{""};
        } else if constexpr (components() == 2) {
            return StaticString{"Vec2"};
        } else if constexpr (components() == 3) {
            return StaticString{"Vec3"};
        } else if constexpr (components() == 4) {
            return StaticString{"Vec4"};
        } else {
            throw DataFormatException("Invalid format", IVW_CONTEXT_CUSTOM("DataFormat"));
        }
    };

    constexpr auto type = []() {
        if constexpr (numericType() == NumericType::Float) {
            return StaticString{"FLOAT"};
        } else if constexpr (numericType() == NumericType::SignedInteger) {
            return StaticString{"INT"};
        } else if constexpr (numericType() == NumericType::UnsignedInteger) {
            return StaticString{"UINT"};
        } else {
            throw DataFormatException("Invalid format", IVW_CONTEXT_CUSTOM("DataFormat"));
        }
    };

    constexpr auto prec = []() {
        if constexpr (precision() == 8) {
            return StaticString{"8"};
        } else if constexpr (precision() == 16) {
            return StaticString{"16"};
        } else if constexpr (precision() == 32) {
            return StaticString{"32"};
        } else if constexpr (precision() == 64) {
            return StaticString{"64"};
        } else {
            throw DataFormatException("Invalid format", IVW_CONTEXT_CUSTOM("DataFormat"));
        }
    };

    return prefix() + type() + prec();
}

template <typename T>
std::string_view DataFormat<T>::str() {
    static constexpr auto string = staticStr();
    return string.view();
}

template <typename T>
constexpr T DataFormat<T>::lowest() {
    return T{std::numeric_limits<primitive>::lowest()};
}

template <typename T>
constexpr T DataFormat<T>::min() {
    return T{std::numeric_limits<primitive>::min()};
}

template <typename T>
constexpr T DataFormat<T>::max() {
    return T{std::numeric_limits<primitive>::max()};
}

template <typename T>
constexpr double DataFormat<T>::lowestToDouble() {
    return static_cast<double>(std::numeric_limits<primitive>::lowest());
}

template <typename T>
constexpr double DataFormat<T>::minToDouble() {
    return static_cast<double>(std::numeric_limits<primitive>::min());
}

template <typename T>
constexpr double DataFormat<T>::maxToDouble() {
    return static_cast<double>(std::numeric_limits<primitive>::max());
}

// Scalars
using DataFloat32 = DataFormat<glm::f32>;
using DataFloat64 = DataFormat<glm::f64>;

using DataInt8 = DataFormat<glm::i8>;
using DataInt16 = DataFormat<glm::i16>;
using DataInt32 = DataFormat<glm::i32>;
using DataInt64 = DataFormat<glm::i64>;

using DataUInt8 = DataFormat<glm::u8>;
using DataUInt16 = DataFormat<glm::u16>;
using DataUInt32 = DataFormat<glm::u32>;
using DataUInt64 = DataFormat<glm::u64>;

// Vec2
using DataVec2Float32 = DataFormat<glm::f32vec2>;
using DataVec2Float64 = DataFormat<glm::f64vec2>;

using DataVec2Int8 = DataFormat<glm::i8vec2>;
using DataVec2Int16 = DataFormat<glm::i16vec2>;
using DataVec2Int32 = DataFormat<glm::i32vec2>;
using DataVec2Int64 = DataFormat<glm::i64vec2>;

using DataVec2UInt8 = DataFormat<glm::u8vec2>;
using DataVec2UInt16 = DataFormat<glm::u16vec2>;
using DataVec2UInt32 = DataFormat<glm::u32vec2>;
using DataVec2UInt64 = DataFormat<glm::u64vec2>;

// Vec3
using DataVec3Float32 = DataFormat<glm::f32vec3>;
using DataVec3Float64 = DataFormat<glm::f64vec3>;

using DataVec3Int8 = DataFormat<glm::i8vec3>;
using DataVec3Int16 = DataFormat<glm::i16vec3>;
using DataVec3Int32 = DataFormat<glm::i32vec3>;
using DataVec3Int64 = DataFormat<glm::i64vec3>;

using DataVec3UInt8 = DataFormat<glm::u8vec3>;
using DataVec3UInt16 = DataFormat<glm::u16vec3>;
using DataVec3UInt32 = DataFormat<glm::u32vec3>;
using DataVec3UInt64 = DataFormat<glm::u64vec3>;

//  Vec4
using DataVec4Float32 = DataFormat<glm::f32vec4>;
using DataVec4Float64 = DataFormat<glm::f64vec4>;

using DataVec4Int8 = DataFormat<glm::i8vec4>;
using DataVec4Int16 = DataFormat<glm::i16vec4>;
using DataVec4Int32 = DataFormat<glm::i32vec4>;
using DataVec4Int64 = DataFormat<glm::i64vec4>;

using DataVec4UInt8 = DataFormat<glm::u8vec4>;
using DataVec4UInt16 = DataFormat<glm::u16vec4>;
using DataVec4UInt32 = DataFormat<glm::u32vec4>;
using DataVec4UInt64 = DataFormat<glm::u64vec4>;

inline std::string_view format_as(DataFormatId id) { return DataFormatBase::get(id)->getString(); }

inline std::string_view format_as(const DataFormatBase& format) { return format.getString(); }

template <typename T>
constexpr std::string_view format_as(const DataFormat<T>&) {
    return DataFormat<T>::str();
}

}  // namespace inviwo
