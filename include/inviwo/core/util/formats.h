/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/exception.h>

#include <limits>
#include <string>
#include <array>
#include <memory>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

/*! \brief Defines general useful formats and new data types
 * Non-virtual, meaning no dynamic_cast as string comparison is as fast/faster
 */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#endif

namespace inviwo {

//Do not set enums specifically, as NumberOfFormats is used to count the number of enums
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

enum class NumericType {
    NotSpecialized,
    Float,
    UnsignedInteger,
    SignedInteger
};

namespace util {

template <typename T>
constexpr NumericType getNumericType() {
    return (std::is_floating_point<T>::value
                ? NumericType::Float
                : (std::is_signed<T>::value ? NumericType::SignedInteger
                                            : NumericType::UnsignedInteger));
}

}  // namespace util

class IVW_CORE_API DataFormatException : public Exception {
public:
    DataFormatException(const std::string& message = "",
                        ExceptionContext context = ExceptionContext());
    virtual ~DataFormatException() throw() = default;
};

class IVW_CORE_API DataFormatBase {
public:
    DataFormatBase(DataFormatId type, size_t components, size_t size, double max, double min,
                   double lowest, NumericType nt, const std::string& s);
    virtual ~DataFormatBase() = default;

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

    // T Models a type with a type
    //    T::type = return type
    // and a function:
    //    template <class T>
    //    type dispatch(Args... args);
    template <typename T, typename... Args>
    auto dispatch(T& obj, Args&&... args) const -> typename T::type;

protected:
    static std::array<std::unique_ptr<DataFormatBase>,
                      static_cast<size_t>(DataFormatId::NumberOfFormats)> instance_;

    DataFormatId formatId_;
    size_t components_;
    size_t size_;
    NumericType numericType_;
    double max_;
    double min_;
    double lowest_;

#include <warn/push>
#include <warn/ignore/dll-interface>
    std::string formatStr_;
#include <warn/pop>
};

template <typename T>
class DataFormat : public DataFormatBase {
public:
    DataFormat();
    virtual ~DataFormat() = default;

    using type = T;
    using primitive = T;
    static const size_t comp = 1;
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

template <typename T, template <typename, glm::precision> class G>
class DataFormat<G<T, glm::defaultp>> : public DataFormatBase {
public:
    DataFormat();
    virtual ~DataFormat() = default;

    using type = G<T, glm::defaultp>;
    using primitive = T;
    static const size_t comp = util::extent<type, 0>::value;
    static const size_t typesize = sizeof(type);
    static const size_t compsize = sizeof(primitive);
    static const NumericType numtype = util::getNumericType<primitive>();  

    // Static interface
    static constexpr DataFormatId id();
    static const DataFormat<type>* get();
    
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
    static constexpr type max();
    static constexpr type min();
    static constexpr type lowest();
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

// Template implementations for DataFormat<T>

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
    auto& d = instance_[static_cast<size_t>(id())];
    if (!d) d = std::make_unique<DataFormat<T>>();
    return static_cast<DataFormat<T>*>(d.get());
}

template <typename T>
constexpr size_t DataFormat<T>::components() {
    return comp;
}

template <typename T>
constexpr size_t DataFormat<T>::size() {
    return typesize;
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
    switch (numtype) {
        case NumericType::Float: return "FLOAT" + toString(precision());
        case NumericType::SignedInteger: return "INT" + toString(precision());
        case NumericType::UnsignedInteger: return "UINT" + toString(precision());
        case NumericType::NotSpecialized:
        default:
            throw DataFormatException("Invalid format", IvwContextCustom("DataForrmat"));
    }
}

template <typename T>
constexpr double DataFormat<T>::lowestToDouble() {
    return static_cast<double>(lowest());
}

template <typename T>
constexpr double DataFormat<T>::minToDouble() {
    return static_cast<double>(min());
}

template <typename T>
constexpr double DataFormat<T>::maxToDouble() {
    return static_cast<double>(max());
}

template <typename T>
constexpr T DataFormat<T>::lowest() {
    return std::numeric_limits<T>::lowest();
}

template <typename T>
constexpr T DataFormat<T>::min() {
    return std::numeric_limits<T>::min();
}

template <typename T>
constexpr T DataFormat<T>::max() {
    return std::numeric_limits<T>::max();
}

template <typename T>
void DataFormat<T>::vec4DoubleToValue(dvec4 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}

template <typename T>
void DataFormat<T>::vec3DoubleToValue(dvec3 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}

template <typename T>
void DataFormat<T>::vec2DoubleToValue(dvec2 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}

template <typename T>
void DataFormat<T>::doubleToValue(double in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}

template <typename T>
dvec4 DataFormat<T>::valueToNormalizedVec4Double(void* val) const {
    return util::glm_convert_normalized<dvec4>(*static_cast<type*>(val));
}

template <typename T>
dvec3 DataFormat<T>::valueToNormalizedVec3Double(void* val) const {
    return util::glm_convert_normalized<dvec3>(*static_cast<type*>(val));
}

template <typename T>
dvec2 DataFormat<T>::valueToNormalizedVec2Double(void* val) const {
    return util::glm_convert_normalized<dvec2>(*static_cast<type*>(val));
}

template <typename T>
double DataFormat<T>::valueToNormalizedDouble(void* val) const {
    return util::glm_convert_normalized<double>(*static_cast<type*>(val));
}

template <typename T>
dvec4 DataFormat<T>::valueToVec4Double(void* val) const {
    return util::glm_convert<dvec4>(*static_cast<type*>(val));
}

template <typename T>
dvec3 DataFormat<T>::valueToVec3Double(void* val) const {
    return util::glm_convert<dvec3>(*static_cast<type*>(val));
}

template <typename T>
dvec2 DataFormat<T>::valueToVec2Double(void* val) const {
    return util::glm_convert<dvec2>(*static_cast<type*>(val));
}

template <typename T>
double DataFormat<T>::valueToDouble(void* val) const {
    return util::glm_convert<double>(*static_cast<type*>(val));
}

// Template implementations for DataFormat<G<T, glm::defaultp>>

template <typename T, template <typename, glm::precision> class G>
DataFormat<G<T, glm::defaultp>>::DataFormat()
    : DataFormatBase(id(), components(), size(), maxToDouble(), minToDouble(), lowestToDouble(),
                     numericType(), str()) {}


template <typename T, template <typename, glm::precision> class G>
constexpr DataFormatId DataFormat<G<T, glm::defaultp>>::id() {
    return DataFormatId::NotSpecialized;
}

template <typename T, template <typename, glm::precision> class G>
auto DataFormat<G<T, glm::defaultp>>::get() -> const DataFormat<type>* {
    auto& d = instance_[static_cast<size_t>(id())];
    if (!d) d = std::make_unique<DataFormat<type>>();
    return static_cast<DataFormat<type>*>(d.get());
}

template <typename T, template <typename, glm::precision> class G>
constexpr size_t DataFormat<G<T, glm::defaultp>>::size() {
    return typesize;
}
template <typename T, template <typename, glm::precision> class G>
constexpr size_t DataFormat<G<T, glm::defaultp>>::components() {
    return comp;
}
template <typename T, template <typename, glm::precision> class G>
constexpr size_t DataFormat<G<T, glm::defaultp>>::precision() {
    return size() / components() * 8;
}
template <typename T, template <typename, glm::precision> class G>
constexpr NumericType DataFormat<G<T, glm::defaultp>>::numericType() {
    return DataFormat<T>::numericType();
}

template <typename T, template <typename, glm::precision> class G>
std::string DataFormat<G<T, glm::defaultp>>::str() {
    return "Vec" + toString(comp) + DataFormat<T>::str();
}

template <typename T, template <typename, glm::precision> class G>
constexpr auto DataFormat<G<T, glm::defaultp>>::max() -> type {
    return type(DataFormat<T>::max());
}
template <typename T, template <typename, glm::precision> class G>
constexpr auto DataFormat<G<T, glm::defaultp>>::min() -> type {
    return type(DataFormat<T>::min());
}
template <typename T, template <typename, glm::precision> class G>
constexpr auto DataFormat<G<T, glm::defaultp>>::lowest() -> type{
    return type(DataFormat<T>::lowest());
}

template <typename T, template <typename, glm::precision> class G>
constexpr double DataFormat<G<T, glm::defaultp>>::maxToDouble() {
    return static_cast<double>(DataFormat<T>::max());
}
template <typename T, template <typename, glm::precision> class G>
constexpr double DataFormat<G<T, glm::defaultp>>::minToDouble() {
    return static_cast<double>(DataFormat<T>::min());
}
template <typename T, template <typename, glm::precision> class G>
constexpr double DataFormat<G<T, glm::defaultp>>::lowestToDouble() {
    return static_cast<double>(DataFormat<T>::lowest());
}

template <typename T, template <typename, glm::precision> class G>
double DataFormat<G<T, glm::defaultp>>::valueToDouble(void* val) const {
    return util::glm_convert<double>(*static_cast<type*>(val));
}
template <typename T, template <typename, glm::precision> class G>
dvec2 DataFormat<G<T, glm::defaultp>>::valueToVec2Double(void* val) const {
    return util::glm_convert<dvec2>(*static_cast<type*>(val));
}
template <typename T, template <typename, glm::precision> class G>
dvec3 DataFormat<G<T, glm::defaultp>>::valueToVec3Double(void* val) const {
    return util::glm_convert<dvec3>(*static_cast<type*>(val));
}
template <typename T, template <typename, glm::precision> class G>
dvec4 DataFormat<G<T, glm::defaultp>>::valueToVec4Double(void* val) const {
    return util::glm_convert<dvec4>(*static_cast<type*>(val));
}

template <typename T, template <typename, glm::precision> class G>
double DataFormat<G<T, glm::defaultp>>::valueToNormalizedDouble(void* val) const {
    return util::glm_convert_normalized<double>(*static_cast<type*>(val));
}
template <typename T, template <typename, glm::precision> class G>
dvec2 DataFormat<G<T, glm::defaultp>>::valueToNormalizedVec2Double(void* val) const {
    return util::glm_convert_normalized<dvec2>(*static_cast<type*>(val));
}
template <typename T, template <typename, glm::precision> class G>
dvec3 DataFormat<G<T, glm::defaultp>>::valueToNormalizedVec3Double(void* val) const {
    return util::glm_convert_normalized<dvec3>(*static_cast<type*>(val));
}
template <typename T, template <typename, glm::precision> class G>
dvec4 DataFormat<G<T, glm::defaultp>>::valueToNormalizedVec4Double(void* val) const {
    return util::glm_convert_normalized<dvec4>(*static_cast<type*>(val));
}

template <typename T, template <typename, glm::precision> class G>
void DataFormat<G<T, glm::defaultp>>::doubleToValue(double in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}
template <typename T, template <typename, glm::precision> class G>
void DataFormat<G<T, glm::defaultp>>::vec2DoubleToValue(dvec2 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}
template <typename T, template <typename, glm::precision> class G>
void DataFormat<G<T, glm::defaultp>>::vec3DoubleToValue(dvec3 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}
template <typename T, template <typename, glm::precision> class G>
void DataFormat<G<T, glm::defaultp>>::vec4DoubleToValue(dvec4 in, void* out) const {
    *static_cast<type*>(out) = util::glm_convert<type>(in);
}



/*---------------Single Value Formats------------------*/

// Floats
typedef half_float::half f16;
typedef DataFormat<f16> DataFloat16;
typedef DataFormat<glm::f32> DataFloat32;
typedef DataFormat<glm::f64> DataFloat64;

// Integers
typedef DataFormat<glm::i8>   DataInt8;
typedef DataFormat<glm::i16>  DataInt16;
typedef DataFormat<glm::i32>  DataInt32;
typedef DataFormat<glm::i64>  DataInt64;

// Unsigned Integers
typedef DataFormat<glm::u8>   DataUInt8;
typedef DataFormat<glm::u16>  DataUInt16;
typedef DataFormat<glm::u32>  DataUInt32;
typedef DataFormat<glm::u64>  DataUInt64;

/*---------------Vec2 Formats--------------------*/

// Floats
typedef glm::tvec2<half_float::half, glm::defaultp> f16vec2;
typedef DataFormat<f16vec2> DataVec2Float16;
typedef DataFormat<glm::f32vec2> DataVec2Float32;
typedef DataFormat<glm::f64vec2> DataVec2Float64;

// Integers
typedef DataFormat<glm::i8vec2>  DataVec2Int8;
typedef DataFormat<glm::i16vec2> DataVec2Int16;
typedef DataFormat<glm::i32vec2> DataVec2Int32;
typedef DataFormat<glm::i64vec2> DataVec2Int64;

// Unsigned Integers
typedef DataFormat<glm::u8vec2>  DataVec2UInt8;
typedef DataFormat<glm::u16vec2> DataVec2UInt16;
typedef DataFormat<glm::u32vec2> DataVec2UInt32;
typedef DataFormat<glm::u64vec2> DataVec2UInt64;

/*---------------Vec3 Formats--------------------*/

// Floats
typedef glm::tvec3<half_float::half, glm::defaultp> f16vec3;
typedef DataFormat<f16vec3> DataVec3Float16;
typedef DataFormat<glm::f32vec3> DataVec3Float32;
typedef DataFormat<glm::f64vec3> DataVec3Float64;

// Integers
typedef DataFormat<glm::i8vec3>  DataVec3Int8;
typedef DataFormat<glm::i16vec3> DataVec3Int16;
typedef DataFormat<glm::i32vec3> DataVec3Int32;
typedef DataFormat<glm::i64vec3> DataVec3Int64;

// Unsigned Integers
typedef DataFormat<glm::u8vec3>  DataVec3UInt8;
typedef DataFormat<glm::u16vec3> DataVec3UInt16;
typedef DataFormat<glm::u32vec3> DataVec3UInt32;
typedef DataFormat<glm::u64vec3> DataVec3UInt64;

/*---------------Vec4 Value Formats------------------*/

// Floats
typedef glm::tvec4<half_float::half, glm::defaultp> f16vec4;
typedef DataFormat<f16vec4> DataVec4Float16;
typedef DataFormat<glm::f32vec4> DataVec4Float32;
typedef DataFormat<glm::f64vec4> DataVec4Float64;

// Integers
typedef DataFormat<glm::i8vec4>  DataVec4Int8;
typedef DataFormat<glm::i16vec4> DataVec4Int16;
typedef DataFormat<glm::i32vec4> DataVec4Int32;
typedef DataFormat<glm::i64vec4> DataVec4Int64;

// Unsigned Integers
typedef DataFormat<glm::u8vec4>  DataVec4UInt8;
typedef DataFormat<glm::u16vec4> DataVec4UInt16;
typedef DataFormat<glm::u32vec4> DataVec4UInt32;
typedef DataFormat<glm::u64vec4> DataVec4UInt64;


/*---------------Single Value Formats------------------*/

// Bit Specializations
template<> constexpr  size_t DataFloat16::size() { return DataFloat32::size(); }

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

// Bit Specializations
template<> constexpr size_t DataVec2Float16::size() { return DataVec2Float32::size(); }

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

// Bit Specializations
template<> constexpr size_t DataVec3Float16::size() { return DataVec3Float32::size(); }

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

// Bit Specializations
template<> constexpr size_t DataVec4Float16::size() { return DataVec4Float32::size(); }

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


template <typename T>
struct Defaultvalues {};

#define DEFAULTVALUES(type, dim, name, val, min, max, inc) \
    template <>                                            \
    struct Defaultvalues<type> {                           \
    public:                                                \
        static type getVal() { return val; }               \
        static type getMin() { return min; }               \
        static type getMax() { return max; }               \
        static type getInc() { return inc; }               \
        static uvec2 getDim() { return dim; }              \
        static std::string getName() { return name; }      \
    };

DEFAULTVALUES(int, uvec2(1, 1), "Int", 0, -100, 100, 1)
DEFAULTVALUES(glm::i64, uvec2(1, 1), "Int64", 0, 0, 1024, 1)
DEFAULTVALUES(ivec2, uvec2(2, 1), "IntVec2", ivec2(0), ivec2(0), ivec2(10), ivec2(1))
DEFAULTVALUES(ivec3, uvec2(3, 1), "IntVec3", ivec3(0), ivec3(0), ivec3(10), ivec3(1))
DEFAULTVALUES(ivec4, uvec2(4, 1), "IntVec4", ivec4(0), ivec4(0), ivec4(10), ivec4(1))

DEFAULTVALUES(unsigned int, uvec2(1, 1), "UInt", 0, 0, 100, 1)
DEFAULTVALUES(uvec2, uvec2(2, 1), "UIntVec2", uvec2(0), uvec2(0), uvec2(10), uvec2(1))
DEFAULTVALUES(uvec3, uvec2(3, 1), "UIntVec3", uvec3(0), uvec3(0), uvec3(10), uvec3(1))
DEFAULTVALUES(uvec4, uvec2(4, 1), "UIntVec4", uvec4(0), uvec4(0), uvec4(10), uvec4(1))

#if !defined(ENVIRONMENT32)
DEFAULTVALUES(size_t, uvec2(1, 1), "Size_t", 0, 0, 100, 1)
DEFAULTVALUES(size2_t, uvec2(2, 1), "IntSize2", size2_t(0), size2_t(0), size2_t(10), size2_t(1))
DEFAULTVALUES(size3_t, uvec2(3, 1), "IntSize3", size3_t(0), size3_t(0), size3_t(10), size3_t(1))
DEFAULTVALUES(size4_t, uvec2(4, 1), "IntSize4", size4_t(0), size4_t(0), size4_t(10), size4_t(1))
#endif

DEFAULTVALUES(float, uvec2(1, 1), "Float", 0.0f, 0.0f, 1.0f, 0.01f)
DEFAULTVALUES(vec2, uvec2(2, 1), "FloatVec2", vec2(0.f), vec2(0.f), vec2(1.f), vec2(0.01f))
DEFAULTVALUES(vec3, uvec2(3, 1), "FloatVec3", vec3(0.f), vec3(0.f), vec3(1.f), vec3(0.01f))
DEFAULTVALUES(vec4, uvec2(4, 1), "FloatVec4", vec4(0.f), vec4(0.f), vec4(1.f), vec4(0.01f))
DEFAULTVALUES(mat2, uvec2(2, 2), "FloatMat2", mat2(0.f), mat2(0.f), mat2(0.f) + 1.0f, mat2(0.f) + 0.01f)
DEFAULTVALUES(mat3, uvec2(3, 3), "FloatMat3", mat3(0.f), mat3(0.f), mat3(0.f) + 1.0f, mat3(0.f) + 0.01f)
DEFAULTVALUES(mat4, uvec2(4, 4), "FloatMat4", mat4(0.f), mat4(0.f), mat4(0.f) + 1.0f, mat4(0.f) + 0.01f)

DEFAULTVALUES(double, uvec2(1, 1), "Double", 0.0, 0.0, 1.0, 0.01)
DEFAULTVALUES(dvec2, uvec2(2, 1), "DoubleVec2", dvec2(0.), dvec2(0.), dvec2(1.), dvec2(0.01))
DEFAULTVALUES(dvec3, uvec2(3, 1), "DoubleVec3", dvec3(0.), dvec3(0.), dvec3(1.), dvec3(0.01))
DEFAULTVALUES(dvec4, uvec2(4, 1), "DoubleVec4", dvec4(0.), dvec4(0.), dvec4(1.), dvec4(0.01))
DEFAULTVALUES(dmat2, uvec2(2, 2), "DoubleMat2", dmat2(0.), dmat2(0.), dmat2(0.) + 1.0, dmat2(0.) + 0.01)
DEFAULTVALUES(dmat3, uvec2(3, 3), "DoubleMat3", dmat3(0.), dmat3(0.), dmat3(0.) + 1.0, dmat3(0.) + 0.01)
DEFAULTVALUES(dmat4, uvec2(4, 4), "DoubleMat4", dmat4(0.), dmat4(0.), dmat4(0.) + 1.0, dmat4(0.) + 0.01)

DEFAULTVALUES(std::string, uvec2(1, 1), "String", "", "", "", "")
DEFAULTVALUES(bool, uvec2(1, 1), "Bool", false, false, true, true)

#undef DEFAULTVALUES

}

#endif
