/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_FORMATS_H
#define IVW_FORMATS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/defaultvalues.h>

#include <limits>
#include <string>
#include <array>
#include <memory>

/*! \brief Defines general useful formats and new data types
 * Non-virtual, meaning no dynamic_cast as string comparison is as fast/faster
 */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#endif

namespace inviwo {

// Do not set enums specifically, as NumberOfFormats is used to count the number of enums
enum class DataFormatId {
    NotSpecialized,
    Float16,
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
    Vec2Float16,
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
    Vec3Float16,
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
    Vec4Float16,
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

enum class NumericType { NotSpecialized, Float, UnsignedInteger, SignedInteger };

namespace util {

template <typename T>
constexpr NumericType getNumericType() {
    return (util::is_floating_point<T>::value
                ? NumericType::Float
                : (std::is_signed<T>::value ? NumericType::SignedInteger
                                            : NumericType::UnsignedInteger));
}

}  // namespace util

class IVW_CORE_API DataFormatException : public Exception {
public:
    DataFormatException(const std::string& message = "",
                        ExceptionContext context = ExceptionContext());
    virtual ~DataFormatException() = default;
};

class IVW_CORE_API DataFormatBase {
public:
    DataFormatBase(DataFormatId type, size_t components, size_t size, double max, double min,
                   double lowest, NumericType nt, const std::string& s);
    virtual ~DataFormatBase();

    static const DataFormatBase* get();
    static const DataFormatBase* get(DataFormatId id);
    static const DataFormatBase* get(const std::string& name);
    static const DataFormatBase* get(NumericType type, size_t components, size_t precision);

    // Runtime interface
    /**
     *	Returns the size of the format in bytes. For all components.
     */
    size_t getSize() const;
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
    const char* getString() const;
    DataFormatId getId() const;

    // Converter functions
    virtual double valueToDouble(void*) const;
    virtual dvec2 valueToVec2Double(void*) const;
    virtual dvec3 valueToVec3Double(void*) const;
    virtual dvec4 valueToVec4Double(void*) const;

    virtual double valueToNormalizedDouble(void*) const;
    virtual dvec2 valueToNormalizedVec2Double(void*) const;
    virtual dvec3 valueToNormalizedVec3Double(void*) const;
    virtual dvec4 valueToNormalizedVec4Double(void*) const;

    virtual void doubleToValue(double, void*) const;
    virtual void vec2DoubleToValue(dvec2, void*) const;
    virtual void vec3DoubleToValue(dvec3, void*) const;
    virtual void vec4DoubleToValue(dvec4, void*) const;

    // clang-format off
    // T Models a type with a type
    //    T::type = return type
    // and a function:
    //    template <class T>
    //    type dispatch(Args... args);
    template <typename T, typename... Args>
    [[deprecated("was declared deprecated. Use dispatch in formatdispatch.h")]] 
    auto dispatch(T& obj, Args&&... args) const -> typename T::type;
    //clang-format on

protected:
    static const DataFormatBase* getPointer(DataFormatId id);


    DataFormatId formatId_;
    size_t components_;
    size_t size_;
    NumericType numericType_;
    double max_;
    double min_;
    double lowest_;
    std::string formatStr_;
};

template <typename T>
class DataFormat : public DataFormatBase {
public:
    DataFormat();
    virtual ~DataFormat() = default;

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
    static constexpr size_t size();
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
    static std::string str();

    // Converter functions
    virtual double valueToDouble(void* val) const override;
    virtual dvec2 valueToVec2Double(void* val) const override;
    virtual dvec3 valueToVec3Double(void* val) const override;
    virtual dvec4 valueToVec4Double(void* val) const override;

    virtual double valueToNormalizedDouble(void* val) const override;
    virtual dvec2 valueToNormalizedVec2Double(void* val) const override;
    virtual dvec3 valueToNormalizedVec3Double(void* val) const override;
    virtual dvec4 valueToNormalizedVec4Double(void* val) const override;

    virtual void doubleToValue(double in, void* out) const override;
    virtual void vec2DoubleToValue(dvec2 in, void* out) const override;
    virtual void vec3DoubleToValue(dvec3 in, void* out) const override;
    virtual void vec4DoubleToValue(dvec4 in, void* out) const override;
};

template <typename T>
DataFormat<T>::DataFormat()
    : DataFormatBase(id(), components(), size(), maxToDouble(), minToDouble(), lowestToDouble(),
                     numericType(), str()) {}

template <typename T>
constexpr DataFormatId DataFormat<T>::id() {
    return DataFormatId::NotSpecialized;
}

template <typename T>
const DataFormat<T>* DataFormat<T>::get() {
    return static_cast<const DataFormat<T>*>(getPointer(id()));
}

template <typename T>
constexpr size_t DataFormat<T>::size() {
    return typesize;
}

template <typename T>
constexpr size_t DataFormat<T>::components() {
    return comp;
}

template <typename T>
constexpr size_t DataFormat<T>::precision() {
    return size() / components() * 8;
}

template <typename T>
constexpr NumericType DataFormat<T>::numericType() {
    return numtype;
}

template <typename T>
std::string DataFormat<T>::str() {
    const std::string prefix = comp > 1 ? "Vec" + std::to_string(comp) : "";
    switch (numtype) {
        case NumericType::Float:
            return prefix + "FLOAT" + std::to_string(precision());
        case NumericType::SignedInteger:
            return prefix + "INT" + std::to_string(precision());
        case NumericType::UnsignedInteger:
            return prefix + "UINT" + std::to_string(precision());
        case NumericType::NotSpecialized:
        default:
            throw DataFormatException("Invalid format", IVW_CONTEXT_CUSTOM("DataFormat"));
    }
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

template <typename T>
double DataFormat<T>::valueToNormalizedDouble(void* val) const {
    return util::glm_convert_normalized<double>(*static_cast<type*>(val));
}
template <typename T>
dvec2 DataFormat<T>::valueToNormalizedVec2Double(void* val) const {
    return util::glm_convert_normalized<dvec2>(*static_cast<type*>(val));
}
template <typename T>
dvec3 DataFormat<T>::valueToNormalizedVec3Double(void* val) const {
    return util::glm_convert_normalized<dvec3>(*static_cast<type*>(val));
}
template <typename T>
dvec4 DataFormat<T>::valueToNormalizedVec4Double(void* val) const {
    return util::glm_convert_normalized<dvec4>(*static_cast<type*>(val));
}

template <typename T>
double DataFormat<T>::valueToDouble(void* val) const {
    return util::glm_convert<double>(*static_cast<type*>(val));
}
template <typename T>
dvec2 DataFormat<T>::valueToVec2Double(void* val) const {
    return util::glm_convert<dvec2>(*static_cast<type*>(val));
}
template <typename T>
dvec3 DataFormat<T>::valueToVec3Double(void* val) const {
    return util::glm_convert<dvec3>(*static_cast<type*>(val));
}
template <typename T>
dvec4 DataFormat<T>::valueToVec4Double(void* val) const {
    return util::glm_convert<dvec4>(*static_cast<type*>(val));
}

template <typename T>
void DataFormat<T>::doubleToValue(double in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}
template <typename T>
void DataFormat<T>::vec2DoubleToValue(dvec2 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}
template <typename T>
void DataFormat<T>::vec3DoubleToValue(dvec3 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}
template <typename T>
void DataFormat<T>::vec4DoubleToValue(dvec4 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}

/*---------------Single Value Formats------------------*/
// Floats
using f16 = half_float::half;
using DataFloat16 = DataFormat<f16>;
using DataFloat32 = DataFormat<glm::f32>;
using DataFloat64 = DataFormat<glm::f64>;

// Integers
using DataInt8 = DataFormat<glm::i8>;
using DataInt16 = DataFormat<glm::i16>;
using DataInt32 = DataFormat<glm::i32>;
using DataInt64 = DataFormat<glm::i64>;

// Unsigned Integers
using DataUInt8 = DataFormat<glm::u8>;
using DataUInt16 = DataFormat<glm::u16>;
using DataUInt32 = DataFormat<glm::u32>;
using DataUInt64 = DataFormat<glm::u64>;

/*---------------Vec2 Formats--------------------*/
// Floats
using f16vec2 = glm::tvec2<half_float::half, glm::defaultp>;
using DataVec2Float16 = DataFormat<f16vec2>;
using DataVec2Float32 = DataFormat<glm::f32vec2>;
using DataVec2Float64 = DataFormat<glm::f64vec2>;

// Integers
using DataVec2Int8 = DataFormat<glm::i8vec2>;
using DataVec2Int16 = DataFormat<glm::i16vec2>;
using DataVec2Int32 = DataFormat<glm::i32vec2>;
using DataVec2Int64 = DataFormat<glm::i64vec2>;

// Unsigned Integers
using DataVec2UInt8 = DataFormat<glm::u8vec2>;
using DataVec2UInt16 = DataFormat<glm::u16vec2>;
using DataVec2UInt32 = DataFormat<glm::u32vec2>;
using DataVec2UInt64 = DataFormat<glm::u64vec2>;

/*---------------Vec3 Formats--------------------*/
// Floats
using f16vec3 = glm::tvec3<half_float::half, glm::defaultp>;
using DataVec3Float16 = DataFormat<f16vec3>;
using DataVec3Float32 = DataFormat<glm::f32vec3>;
using DataVec3Float64 = DataFormat<glm::f64vec3>;

// Integers
using DataVec3Int8 = DataFormat<glm::i8vec3>;
using DataVec3Int16 = DataFormat<glm::i16vec3>;
using DataVec3Int32 = DataFormat<glm::i32vec3>;
using DataVec3Int64 = DataFormat<glm::i64vec3>;

// Unsigned Integers
using DataVec3UInt8 = DataFormat<glm::u8vec3>;
using DataVec3UInt16 = DataFormat<glm::u16vec3>;
using DataVec3UInt32 = DataFormat<glm::u32vec3>;
using DataVec3UInt64 = DataFormat<glm::u64vec3>;

/*---------------Vec4 Value Formats------------------*/

// Floats
using f16vec4 = glm::tvec4<half_float::half, glm::defaultp>;
using DataVec4Float16 = DataFormat<f16vec4>;
using DataVec4Float32 = DataFormat<glm::f32vec4>;
using DataVec4Float64 = DataFormat<glm::f64vec4>;

// Integers
using DataVec4Int8 = DataFormat<glm::i8vec4>;
using DataVec4Int16 = DataFormat<glm::i16vec4>;
using DataVec4Int32 = DataFormat<glm::i32vec4>;
using DataVec4Int64 = DataFormat<glm::i64vec4>;

// Unsigned Integers
using DataVec4UInt8 = DataFormat<glm::u8vec4>;
using DataVec4UInt16 = DataFormat<glm::u16vec4>;
using DataVec4UInt32 = DataFormat<glm::u32vec4>;
using DataVec4UInt64 = DataFormat<glm::u64vec4>;

/*---------------Single Value Formats------------------*/
// clang-format off
// Type Function Specializations
template<> constexpr DataFormatId DataFloat16::id() { return DataFormatId::Float16; }
template<> constexpr DataFormatId DataFloat32::id() { return DataFormatId::Float32; }
template<> constexpr DataFormatId DataFloat64::id() { return DataFormatId::Float64; }

template<> constexpr DataFormatId DataInt8::id()  { return DataFormatId::Int8; }
template<> constexpr DataFormatId DataInt16::id() { return DataFormatId::Int16; }
template<> constexpr DataFormatId DataInt32::id() { return DataFormatId::Int32; }
template<> constexpr DataFormatId DataInt64::id() { return DataFormatId::Int64; }

template<> constexpr DataFormatId DataUInt8::id()  { return DataFormatId::UInt8; }
template<> constexpr DataFormatId DataUInt16::id() { return DataFormatId::UInt16; }
template<> constexpr DataFormatId DataUInt32::id() { return DataFormatId::UInt32; }
template<> constexpr DataFormatId DataUInt64::id() { return DataFormatId::UInt64; }


/*---------------Vec2 Formats--------------------*/

// Type Function Specializations
template<> constexpr DataFormatId DataVec2Float16::id() { return DataFormatId::Vec2Float16; }
template<> constexpr DataFormatId DataVec2Float32::id() { return DataFormatId::Vec2Float32; }
template<> constexpr DataFormatId DataVec2Float64::id() { return DataFormatId::Vec2Float64; }

template<> constexpr DataFormatId DataVec2Int8::id() { return DataFormatId::Vec2Int8; }
template<> constexpr DataFormatId DataVec2Int16::id() { return DataFormatId::Vec2Int16; }
template<> constexpr DataFormatId DataVec2Int32::id() { return DataFormatId::Vec2Int32; }
template<> constexpr DataFormatId DataVec2Int64::id() { return DataFormatId::Vec2Int64; }

template<> constexpr DataFormatId DataVec2UInt8::id() { return DataFormatId::Vec2UInt8; }
template<> constexpr DataFormatId DataVec2UInt16::id() { return DataFormatId::Vec2UInt16; }
template<> constexpr DataFormatId DataVec2UInt32::id() { return DataFormatId::Vec2UInt32; }
template<> constexpr DataFormatId DataVec2UInt64::id() { return DataFormatId::Vec2UInt64; }


/*---------------Vec3 Formats--------------------*/

// Type Function Specializations
template<> constexpr DataFormatId DataVec3Float16::id() { return DataFormatId::Vec3Float16; }
template<> constexpr DataFormatId DataVec3Float32::id() { return DataFormatId::Vec3Float32; }
template<> constexpr DataFormatId DataVec3Float64::id() { return DataFormatId::Vec3Float64; }

template<> constexpr DataFormatId DataVec3Int8::id() { return DataFormatId::Vec3Int8; }
template<> constexpr DataFormatId DataVec3Int16::id() { return DataFormatId::Vec3Int16; }
template<> constexpr DataFormatId DataVec3Int32::id() { return DataFormatId::Vec3Int32; }
template<> constexpr DataFormatId DataVec3Int64::id() { return DataFormatId::Vec3Int64; }

template<> constexpr DataFormatId DataVec3UInt8::id() { return DataFormatId::Vec3UInt8; }
template<> constexpr DataFormatId DataVec3UInt16::id() { return DataFormatId::Vec3UInt16; }
template<> constexpr DataFormatId DataVec3UInt32::id() { return DataFormatId::Vec3UInt32; }
template<> constexpr DataFormatId DataVec3UInt64::id() { return DataFormatId::Vec3UInt64; }



/*---------------Vec4 Formats--------------------*/

// Type Function Specializations
template<> constexpr DataFormatId DataVec4Float16::id() { return DataFormatId::Vec4Float16; }
template<> constexpr DataFormatId DataVec4Float32::id() { return DataFormatId::Vec4Float32; }
template<> constexpr DataFormatId DataVec4Float64::id() { return DataFormatId::Vec4Float64; }

template<> constexpr DataFormatId DataVec4Int8::id() { return DataFormatId::Vec4Int8; }
template<> constexpr DataFormatId DataVec4Int16::id() { return DataFormatId::Vec4Int16; }
template<> constexpr DataFormatId DataVec4Int32::id() { return DataFormatId::Vec4Int32; }
template<> constexpr DataFormatId DataVec4Int64::id() { return DataFormatId::Vec4Int64; }

template<> constexpr DataFormatId DataVec4UInt8::id() { return DataFormatId::Vec4UInt8; }
template<> constexpr DataFormatId DataVec4UInt16::id() { return DataFormatId::Vec4UInt16; }
template<> constexpr DataFormatId DataVec4UInt32::id() { return DataFormatId::Vec4UInt32; }
template<> constexpr DataFormatId DataVec4UInt64::id() { return DataFormatId::Vec4UInt64; }

// clang-format on

using DefaultDataFormats = std::tuple<
    DataFloat16, DataFloat32, DataFloat64, DataInt8, DataInt16, DataInt32, DataInt64, DataUInt8,
    DataUInt16, DataUInt32, DataUInt64, DataVec2Float16, DataVec2Float32, DataVec2Float64,
    DataVec2Int8, DataVec2Int16, DataVec2Int32, DataVec2Int64, DataVec2UInt8, DataVec2UInt16,
    DataVec2UInt32, DataVec2UInt64, DataVec3Float16, DataVec3Float32, DataVec3Float64, DataVec3Int8,
    DataVec3Int16, DataVec3Int32, DataVec3Int64, DataVec3UInt8, DataVec3UInt16, DataVec3UInt32,
    DataVec3UInt64, DataVec4Float16, DataVec4Float32, DataVec4Float64, DataVec4Int8, DataVec4Int16,
    DataVec4Int32, DataVec4Int64, DataVec4UInt8, DataVec4UInt16, DataVec4UInt32, DataVec4UInt64>;

namespace util {

namespace detail {

template <typename T, typename Formats>
struct HasDataFormatImpl;

template <typename T, typename Head, typename... Rest>
struct HasDataFormatImpl<T, std::tuple<Head, Rest...>> : HasDataFormatImpl<T, std::tuple<Rest...>> {
};

template <typename Head, typename... Rest>
struct HasDataFormatImpl<typename Head::type, std::tuple<Head, Rest...>> : std::true_type {};

template <typename T>
struct HasDataFormatImpl<T, std::tuple<>> : std::false_type {};

}  // namespace detail

template <typename T>
struct HasDataFormat : detail::HasDataFormatImpl<T, DefaultDataFormats> {};
}  // namespace util

template <typename T, typename... Args>
auto DataFormatBase::dispatch(T& obj, Args&&... args) const -> typename T::type {
    using R = typename T::type;
    switch (formatId_) {
        case DataFormatId::Float16:
            return obj.template dispatch<DataFloat16>(std::forward<Args>(args)...);
        case DataFormatId::Float32:
            return obj.template dispatch<DataFloat32>(std::forward<Args>(args)...);
        case DataFormatId::Float64:
            return obj.template dispatch<DataFloat64>(std::forward<Args>(args)...);
        case DataFormatId::Int8:
            return obj.template dispatch<DataInt8>(std::forward<Args>(args)...);
        case DataFormatId::Int16:
            return obj.template dispatch<DataInt16>(std::forward<Args>(args)...);
        case DataFormatId::Int32:
            return obj.template dispatch<DataInt32>(std::forward<Args>(args)...);
        case DataFormatId::Int64:
            return obj.template dispatch<DataInt64>(std::forward<Args>(args)...);
        case DataFormatId::UInt8:
            return obj.template dispatch<DataUInt8>(std::forward<Args>(args)...);
        case DataFormatId::UInt16:
            return obj.template dispatch<DataUInt16>(std::forward<Args>(args)...);
        case DataFormatId::UInt32:
            return obj.template dispatch<DataUInt32>(std::forward<Args>(args)...);
        case DataFormatId::UInt64:
            return obj.template dispatch<DataUInt64>(std::forward<Args>(args)...);
        case DataFormatId::Vec2Float16:
            return obj.template dispatch<DataVec2Float16>(std::forward<Args>(args)...);
        case DataFormatId::Vec2Float32:
            return obj.template dispatch<DataVec2Float32>(std::forward<Args>(args)...);
        case DataFormatId::Vec2Float64:
            return obj.template dispatch<DataVec2Float64>(std::forward<Args>(args)...);
        case DataFormatId::Vec2Int8:
            return obj.template dispatch<DataVec2Int8>(std::forward<Args>(args)...);
        case DataFormatId::Vec2Int16:
            return obj.template dispatch<DataVec2Int16>(std::forward<Args>(args)...);
        case DataFormatId::Vec2Int32:
            return obj.template dispatch<DataVec2Int32>(std::forward<Args>(args)...);
        case DataFormatId::Vec2Int64:
            return obj.template dispatch<DataVec2Int64>(std::forward<Args>(args)...);
        case DataFormatId::Vec2UInt8:
            return obj.template dispatch<DataVec2UInt8>(std::forward<Args>(args)...);
        case DataFormatId::Vec2UInt16:
            return obj.template dispatch<DataVec2UInt16>(std::forward<Args>(args)...);
        case DataFormatId::Vec2UInt32:
            return obj.template dispatch<DataVec2UInt32>(std::forward<Args>(args)...);
        case DataFormatId::Vec2UInt64:
            return obj.template dispatch<DataVec2UInt64>(std::forward<Args>(args)...);
        case DataFormatId::Vec3Float16:
            return obj.template dispatch<DataVec3Float16>(std::forward<Args>(args)...);
        case DataFormatId::Vec3Float32:
            return obj.template dispatch<DataVec3Float32>(std::forward<Args>(args)...);
        case DataFormatId::Vec3Float64:
            return obj.template dispatch<DataVec3Float64>(std::forward<Args>(args)...);
        case DataFormatId::Vec3Int8:
            return obj.template dispatch<DataVec3Int8>(std::forward<Args>(args)...);
        case DataFormatId::Vec3Int16:
            return obj.template dispatch<DataVec3Int16>(std::forward<Args>(args)...);
        case DataFormatId::Vec3Int32:
            return obj.template dispatch<DataVec3Int32>(std::forward<Args>(args)...);
        case DataFormatId::Vec3Int64:
            return obj.template dispatch<DataVec3Int64>(std::forward<Args>(args)...);
        case DataFormatId::Vec3UInt8:
            return obj.template dispatch<DataVec3UInt8>(std::forward<Args>(args)...);
        case DataFormatId::Vec3UInt16:
            return obj.template dispatch<DataVec3UInt16>(std::forward<Args>(args)...);
        case DataFormatId::Vec3UInt32:
            return obj.template dispatch<DataVec3UInt32>(std::forward<Args>(args)...);
        case DataFormatId::Vec3UInt64:
            return obj.template dispatch<DataVec3UInt64>(std::forward<Args>(args)...);
        case DataFormatId::Vec4Float16:
            return obj.template dispatch<DataVec4Float16>(std::forward<Args>(args)...);
        case DataFormatId::Vec4Float32:
            return obj.template dispatch<DataVec4Float32>(std::forward<Args>(args)...);
        case DataFormatId::Vec4Float64:
            return obj.template dispatch<DataVec4Float64>(std::forward<Args>(args)...);
        case DataFormatId::Vec4Int8:
            return obj.template dispatch<DataVec4Int8>(std::forward<Args>(args)...);
        case DataFormatId::Vec4Int16:
            return obj.template dispatch<DataVec4Int16>(std::forward<Args>(args)...);
        case DataFormatId::Vec4Int32:
            return obj.template dispatch<DataVec4Int32>(std::forward<Args>(args)...);
        case DataFormatId::Vec4Int64:
            return obj.template dispatch<DataVec4Int64>(std::forward<Args>(args)...);
        case DataFormatId::Vec4UInt8:
            return obj.template dispatch<DataVec4UInt8>(std::forward<Args>(args)...);
        case DataFormatId::Vec4UInt16:
            return obj.template dispatch<DataVec4UInt16>(std::forward<Args>(args)...);
        case DataFormatId::Vec4UInt32:
            return obj.template dispatch<DataVec4UInt32>(std::forward<Args>(args)...);
        case DataFormatId::Vec4UInt64:
            return obj.template dispatch<DataVec4UInt64>(std::forward<Args>(args)...);
        case DataFormatId::NotSpecialized:
        case DataFormatId::NumberOfFormats:
        default:
            return R();
    }
}

}  // namespace inviwo

#endif
