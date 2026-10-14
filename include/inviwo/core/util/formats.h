/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/utfutils.h>

#include <inviwo/core/algorithm/buildarray.h>

#include <glm/gtc/type_precision.hpp>

#include <limits>
#include <string>
#include <string_view>
#include <array>
#include <memory>
#include <span>
#include <optional>
#include <bit>
#include <tuple>

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
    Mat2Float32,
    Mat3Float32,
    Mat4Float32,
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
 * Determines the common numeric type of the given @p formats. Returns NumericType::Float if any of
 * the formats is a floating point format. Otherwise, NumericType::SignedInteger is returned if at
 * least one format is signed, and NumericType::UnsignedInteger if that is not the case.
 * @return NumericType based on @p formats. NumericType::NotSpecialized if the span is empty.
 */
IVW_CORE_API NumericType commonNumericType(std::span<const DataFormatBase*> formats);

/**
 * Determines the format precision of the given @p formats, that is the highest one.
 * @return maximum precision of all @p formats
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

/** @brief Defines general useful formats and new data types
 * Non-virtual, meaning no dynamic_cast as string comparison is as fast/faster
 */
class IVW_CORE_API DataFormatBase {
public:
    static constexpr const DataFormatBase* get();
    static constexpr const DataFormatBase* get(DataFormatId id);
    static constexpr const DataFormatBase* get(std::string_view name);
    static constexpr const DataFormatBase* get(NumericType type, size_t components,
                                               size_t precision);
    static constexpr const DataFormatBase* get(NumericType type, size_t components,
                                               size_t precision, size_t rank);

    static constexpr std::optional<DataFormatId> find(std::string_view name);
    static constexpr std::optional<DataFormatId> find(NumericType type, size_t components,
                                                      size_t precision);
    static constexpr std::optional<DataFormatId> find(NumericType type, size_t components,
                                                      size_t precision, size_t rank);

    // Runtime interface
    /**
     *	Returns the size of the format in bytes. For all components.
     */
    constexpr size_t getSizeInBytes() const;

    /**
     *	Returns the rank in the format, 0, 1 or 2.
     */
    constexpr size_t getRank() const;

    /**
     *	Returns the number of components in the format, 1 to 4.
     */
    constexpr size_t getComponents() const;

    /**
     *	Returns number of bits in each component in the format. can be 8, 16, 32 or 64.
     */
    constexpr size_t getPrecision() const;

    /**
     *	Returns numeric type of the format @see NumericType
     */
    constexpr NumericType getNumericType() const;

    /**
     *	Returns the name of the format
     */
    constexpr std::string_view getString() const;

    /**
     *	Returns DataFormatId of the format @see DataFormatId
     */
    constexpr DataFormatId getId() const;

    /**
     *	Returns the maximum value of the format
     */
    constexpr double getMax() const;
    /**
     *	Returns the minimum value of the format
     */
    constexpr double getMin() const;
    /**
     *	Returns the lowest value of the format
     */
    constexpr double getLowest() const;

    template <typename T>
    static constexpr DataFormatId typeToId() noexcept;

    constexpr DataFormatBase(DataFormatId type, size_t rank, size_t components, size_t size,
                             double max, double min, double lowest, NumericType nt,
                             std::string_view s);

protected:
    constexpr DataFormatBase();
    DataFormatBase(const DataFormatBase&) = default;
    DataFormatBase(DataFormatBase&&) = default;
    DataFormatBase& operator=(const DataFormatBase&) = default;
    DataFormatBase& operator=(DataFormatBase&&) = default;

    DataFormatId formatId_;
    size_t rank_;
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
    using primitive = util::value_type_t<T>;
    static constexpr size_t comp = util::extent_v<T>;
    static constexpr size_t typesize = sizeof(type);
    static constexpr size_t compsize = sizeof(primitive);
    static constexpr NumericType numtype = util::getNumericType<primitive>();

    // Static interface
    static constexpr DataFormatId id();
    static constexpr const DataFormatBase* get();

    /**
     *	Returns the size of the format in bytes. For all components.
     */
    static constexpr size_t sizeInBytes();

    /**
     *	Returns the rank in the format, 0, 1 or 2.
     */
    static constexpr size_t rank();

    /**
     *	Returns the number of components in the format, 1 to 4.
     */
    static constexpr size_t components();
    /**
     *	Returns number of bits in each component in the format. can be 8, 16, 32 or 64.
     */
    static constexpr size_t precision();

    /**
     *	Returns numeric type of the format @see NumericType
     */
    static constexpr NumericType numericType();

    static constexpr T max();
    static constexpr T min();
    static constexpr T lowest();
    static constexpr double maxToDouble();
    static constexpr double minToDouble();
    static constexpr double lowestToDouble();
    static constexpr std::string_view str();
    static constexpr auto staticStr();

private:
    static constexpr DataFormatBase base();
    static constexpr auto string = staticStr();
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
               glm::u32vec4, glm::u64vec4, glm::f32mat2, glm::f32mat3, glm::f32mat4>;

using DefaultDataFormats = util::wrap<DataFormat, DefaultDataTypes>;

template <typename T>
DataFormat<T>::DataFormat()
    : DataFormatBase(id(), rank(), components(), sizeInBytes(), maxToDouble(), minToDouble(),
                     lowestToDouble(), numericType(), str()) {}

template <typename T>
constexpr DataFormatBase DataFormat<T>::base() {
    return DataFormatBase(id(), rank(), components(), sizeInBytes(), maxToDouble(), minToDouble(),
                          lowestToDouble(), numericType(), str());
}

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
constexpr const DataFormatBase* DataFormat<T>::get() {
    return DataFormatBase::get(id());
}

template <typename T>
constexpr size_t DataFormat<T>::sizeInBytes() {
    return typesize;
}

template <typename T>
constexpr size_t DataFormat<T>::rank() {
    return util::rank_v<T>;
}

template <typename T>
constexpr size_t DataFormat<T>::components() {
    return comp;
}

template <typename T>
constexpr size_t DataFormat<T>::precision() {
    switch (rank()) {
        case 0:  return sizeInBytes() * 8;
        case 1:  return sizeInBytes() / components() * 8;
        case 2:  return sizeInBytes() / components() / components() * 8;
        default: throw DataFormatException(SourceContext{}, "Invalid format rank {}", rank());
    }
}

template <typename T>
constexpr NumericType DataFormat<T>::numericType() {
    return numtype;
}

template <typename T>
constexpr auto DataFormat<T>::staticStr() {
    constexpr std::tuple prefix{StaticString{""}, StaticString{"Vec"}, StaticString{"Mat"}};
    constexpr std::tuple comps{StaticString{""}, StaticString{"2"}, StaticString{"3"},
                               StaticString{"4"}};
    constexpr std::tuple type{StaticString{"FLOAT"}, StaticString{"UINT"}, StaticString{"INT"}};
    constexpr std::tuple prec{StaticString{"8"}, StaticString{"16"}, StaticString{"32"},
                              StaticString{"64"}};

    constexpr auto r = rank();
    constexpr auto c = components() - 1;
    constexpr auto t = static_cast<size_t>(numericType()) - 1;
    constexpr auto p = static_cast<size_t>(64 - std::countl_zero(precision()) - 4);

    static_assert(std::has_single_bit(precision()));
    static_assert(r < std::tuple_size_v<decltype(prefix)>);
    static_assert(c < std::tuple_size_v<decltype(comps)>);
    static_assert(t < std::tuple_size_v<decltype(type)>);
    static_assert(p < std::tuple_size_v<decltype(prec)>);

    return std::get<r>(prefix) + std::get<c>(comps) + std::get<t>(type) + std::get<p>(prec);
}

template <typename T>
constexpr std::string_view DataFormat<T>::str() {
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

constexpr std::string_view format_as(DataFormatId id) {
    return DataFormatBase::get(id)->getString();
}

constexpr std::string_view format_as(const DataFormatBase& format) { return format.getString(); }

template <typename T>
constexpr std::string_view format_as(const DataFormat<T>&) {
    return DataFormat<T>::str();
}

constexpr DataFormatBase::DataFormatBase(DataFormatId t, size_t r, size_t c, size_t size,
                                         double max, double min, double lowest, NumericType nt,
                                         std::string_view s)
    : formatId_(t)
    , rank_(r)
    , components_(c)
    , sizeInBytes_(size)
    , numericType_(nt)
    , max_(max)
    , min_(min)
    , lowest_(lowest)
    , formatStr_(s) {

    static_assert(DataFormatBase::typeToId<glm::f32>() == DataFormatId::Float32);
    static_assert(DataFormatBase::typeToId<glm::u64vec4>() == DataFormatId::Vec4UInt64);
    static_assert(DataFormatBase::typeToId<glm::mat4>() == DataFormatId::Mat4Float32);
    static_assert(DataFormatBase::typeToId<glm::dmat4>() == DataFormatId::NotSpecialized);
}

constexpr DataFormatBase::DataFormatBase()
    : DataFormatBase(DataFormatId::NotSpecialized, 0, 0, 0, 0.0, 0.0, 0.0,
                     NumericType::NotSpecialized, "NotSpecialized") {}

constexpr size_t DataFormatBase::getSizeInBytes() const { return sizeInBytes_; }

constexpr NumericType DataFormatBase::getNumericType() const { return numericType_; }

constexpr size_t DataFormatBase::getRank() const { return rank_; }

constexpr size_t DataFormatBase::getComponents() const { return components_; }

constexpr size_t DataFormatBase::getPrecision() const {
    switch (getRank()) {
        case 0:  return getSizeInBytes() * 8;
        case 1:  return getSizeInBytes() / getComponents() * 8;
        case 2:  return getSizeInBytes() / getComponents() / getComponents() * 8;
        default: throw DataFormatException(SourceContext{}, "Invalid format rank {}", getRank());
    }
}

constexpr double DataFormatBase::getMax() const { return max_; }

constexpr double DataFormatBase::getMin() const { return min_; }

constexpr double DataFormatBase::getLowest() const { return lowest_; }

constexpr std::string_view DataFormatBase::getString() const { return formatStr_; }

constexpr DataFormatId DataFormatBase::getId() const { return formatId_; }

constexpr const DataFormatBase* DataFormatBase::get() { return get(DataFormatId::NotSpecialized); }

constexpr const DataFormatBase* DataFormatBase::get(DataFormatId id) {
    static constexpr auto N = std::tuple_size_v<DefaultDataTypes>;
    static_assert(N + 1 == static_cast<size_t>(DataFormatId::NumberOfFormats));
    static constexpr auto formats = util::build_array<N + 1>([&](auto idx) {
        static constexpr size_t index = decltype(idx)::value;
        if constexpr (index == 0) {
            return DataFormatBase{};
        } else {
            using T = std::tuple_element_t<index - 1uz, DefaultDataTypes>;
            return DataFormat<T>::base();
        }
    });

    if (static_cast<size_t>(id) >= formats.size()) {
        throw DataFormatException(SourceContext{}, "Invalid format id {}", static_cast<int>(id));
    }
    return &formats[static_cast<size_t>(id)];
}

constexpr std::optional<DataFormatId> DataFormatBase::find(std::string_view name) {
    static constexpr auto table = []() {
        static constexpr auto extraNames =
            std::to_array<std::pair<std::string_view, DataFormatId>>({
                {"uchar", DataFormatId::UInt8},
                {"char", DataFormatId::Int8},
                {"ushort", DataFormatId::UInt16},
                {"short", DataFormatId::Int16},
                {"uint", DataFormatId::UInt32},
                {"int", DataFormatId::Int32},
                {"float", DataFormatId::Float32},
                {"double", DataFormatId::Float64},
                {"notspecialized", DataFormatId::NotSpecialized},
            });
        static constexpr auto nFormats = std::tuple_size_v<DefaultDataFormats>;
        static constexpr auto nExtra = extraNames.size();

        std::array<std::pair<std::string_view, DataFormatId>, nFormats + nExtra> res{};
        size_t i = 0;
        util::for_each_type<DefaultDataFormats>{}(
            [&]<typename F>() { res[i++] = {F::str(), F::id()}; });
        for (const auto& extra : extraNames) {
            res[i++] = extra;
        }
        return res;
    }();

    for (const auto& [n, id] : table) {
        if (CaseInsensitiveEqual{}(n, name)) return id;
    }
    return std::nullopt;
}

constexpr std::optional<DataFormatId> DataFormatBase::find(NumericType type, size_t components,
                                                           size_t precision) {
    return find(type, components, precision, 1);
}

constexpr std::optional<DataFormatId> DataFormatBase::find(NumericType type, size_t components,
                                                           size_t precision, size_t rank) {
    static constexpr auto table = []() {
        std::array<std::array<std::array<std::array<std::optional<DataFormatId>, 4>, 4>, 4>, 2>
            res{};
        util::for_each_type<DefaultDataFormats>{}([&]<typename F>() {
            constexpr auto r = F::rank() == 2 ? 1 : 0;
            constexpr auto n = static_cast<size_t>(F::numericType());
            constexpr auto c = F::components() - 1;
            constexpr auto p = 64 - std::countl_zero(F::precision()) - 4;
            res[r][n][c][p] = F::id();
        });
        return res;
    }();

    const auto r = rank == 2 ? size_t{1} : size_t{0};
    const auto n = static_cast<size_t>(type);
    const auto c = components - 1;
    const auto p = static_cast<size_t>(64 - std::countl_zero(precision) - 4);

    if (std::has_single_bit(precision) && r < table.size() && n < table[0].size() &&
        c < table[0][0].size() && p < table[0][0][0].size()) {
        return table[r][n][c][p];
    } else {
        return std::nullopt;
    }
}

constexpr const DataFormatBase* DataFormatBase::get(std::string_view name) {
    return find(name)
        .transform([](DataFormatId id) { return get(id); })
        .value_or<const DataFormatBase*>(nullptr);
}
constexpr const DataFormatBase* DataFormatBase::get(NumericType type, size_t components,
                                                    size_t precision) {
    return find(type, components, precision)
        .transform([](DataFormatId id) { return get(id); })
        .value_or<const DataFormatBase*>(nullptr);
}

constexpr const DataFormatBase* DataFormatBase::get(NumericType type, size_t components,
                                                    size_t precision, size_t rank) {
    return find(type, components, precision, rank)
        .transform([](DataFormatId id) { return get(id); })
        .value_or<const DataFormatBase*>(nullptr);
}

}  // namespace inviwo
