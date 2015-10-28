/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <limits>
#include <string>

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

class IVW_CORE_API DataFormatBase {
public:
    DataFormatBase();
    DataFormatBase(DataFormatId type, size_t components, size_t size, double max, double min,
                   NumericType nt, std::string s);
    virtual ~DataFormatBase();

    static const DataFormatBase* get();
    static const DataFormatBase* get(DataFormatId id);
    static const DataFormatBase* get(std::string name);
    static const DataFormatBase* get(NumericType type, size_t components,
                                     size_t precision);

    static void cleanDataFormatBases();

    static size_t size() { return 0; }
    static size_t components() { return 0; }
    static NumericType numericType() {
        return NumericType::NotSpecialized;
    }
    static std::string str() { return "Error, type specialization not implemented"; }
    static DataFormatId id() { return DataFormatId::NotSpecialized; }

    size_t getSize() const;
    size_t getComponents() const;

    NumericType getNumericType() const;
    double getMax() const;
    double getMin() const;
    const char* getString() const;
    DataFormatId getId() const;

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
    static DataFormatBase* instance_[static_cast<size_t>(DataFormatId::NumberOfFormats)];
    static DataFormatBase* getNonConst(DataFormatId id) { return instance_[static_cast<size_t>(id)]; }

    DataFormatId formatId_;
    size_t components_;
    size_t size_;
    NumericType numericType_;
    double max_;
    double min_;

    #include <warn/push>
    #include <warn/ignore/dll-interface>
    std::string formatStr_;
    #include <warn/pop>
};

template <typename T>
class IVW_CORE_API DataFormat : public DataFormatBase {
public:
    typedef T type;
    typedef T primitive;
    static const size_t comp = 1;
    static const size_t typesize = sizeof(type);
    static const size_t compsize = sizeof(primitive);

    DataFormat()
        : DataFormatBase(id(), components(), size(), maxToDouble(), minToDouble(), numericType(),
                         str()) {}

    virtual ~DataFormat() {}

    static const DataFormat<T>* get() {
        DataFormatBase* d = DataFormatBase::getNonConst(id());
        if (!d) {
            d = new DataFormat<T>();
            instance_[static_cast<size_t>(id())] = d;
        }
        return static_cast<DataFormat<T>*>(d);
    }

    static size_t size() { return typesize; }
    static size_t components() { return comp; }
    static NumericType numericType() {
        return NumericType::NotSpecialized;
    }

    static T max() { return std::numeric_limits<T>::max(); }
    static T min() { return std::numeric_limits<T>::min(); }

    static double maxToDouble() { return static_cast<double>(max()); }
    static double minToDouble() { return static_cast<double>(min()); }

    static std::string str() { return DataFormatBase::str(); }
    static DataFormatId id() { return DataFormatBase::id(); }

    virtual double valueToDouble(void* val) const {
        return util::glm_convert<double>(*static_cast<type*>(val));
    }
    virtual dvec2 valueToVec2Double(void* val) const {
        return util::glm_convert<dvec2>(*static_cast<type*>(val));
    }
    virtual dvec3 valueToVec3Double(void* val) const {
        return util::glm_convert<dvec3>(*static_cast<type*>(val));
    }
    virtual dvec4 valueToVec4Double(void* val) const {
        return util::glm_convert<dvec4>(*static_cast<type*>(val));
    }

    virtual double valueToNormalizedDouble(void* val) const {
        return util::glm_convert_normalized<double>(*static_cast<type*>(val));
    }
    virtual dvec2 valueToNormalizedVec2Double(void* val) const {
        return util::glm_convert_normalized<dvec2>(*static_cast<type*>(val));
    }
    virtual dvec3 valueToNormalizedVec3Double(void* val) const {
        return util::glm_convert_normalized<dvec3>(*static_cast<type*>(val));
    }
    virtual dvec4 valueToNormalizedVec4Double(void* val) const {
        return util::glm_convert_normalized<dvec4>(*static_cast<type*>(val));
    }

    virtual void doubleToValue(double in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
    virtual void vec2DoubleToValue(dvec2 in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
    virtual void vec3DoubleToValue(dvec3 in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
    virtual void vec4DoubleToValue(dvec4 in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
};

template <typename T, template <typename, glm::precision> class G>
class DataFormat<G<T, glm::defaultp>> : public DataFormatBase {
public:
    typedef G<T, glm::defaultp> type;
    typedef T primitive;
    static const size_t comp = util::extent<type, 0>::value;
    static const size_t typesize = sizeof(type);
    static const size_t compsize = sizeof(primitive);

    DataFormat()
        : DataFormatBase(id(), components(), size(), maxToDouble(), minToDouble(), numericType(),
                         str()) {}

    virtual ~DataFormat() {}

    static const DataFormat<type>* get() {
        DataFormatBase* d = DataFormatBase::getNonConst(id());
        if (!d) {
            d = new DataFormat<type>();
            instance_[static_cast<size_t>(id())] = d;
        }
        return static_cast<DataFormat<type>*>(d);
    }

    static size_t size() { return typesize; }
    static size_t components() { return comp; }
    static NumericType numericType() { return DataFormat<T>::numericType(); }

    static type max() { return type(DataFormat<T>::max()); }
    static type min() { return type(DataFormat<T>::min()); }

    static double maxToDouble() { return static_cast<double>(DataFormat<T>::max()); }
    static double minToDouble() { return static_cast<double>(DataFormat<T>::min()); }

    static std::string str() { return "Vec" + toString(comp) + DataFormat<T>::str(); }
    static DataFormatId id() { return DataFormat<T>::id(); }

    virtual double valueToDouble(void* val) const {
        return util::glm_convert<double>(*static_cast<type*>(val));
    }
    virtual dvec2 valueToVec2Double(void* val) const {
        return util::glm_convert<dvec2>(*static_cast<type*>(val));
    }
    virtual dvec3 valueToVec3Double(void* val) const {
        return util::glm_convert<dvec3>(*static_cast<type*>(val));
    }
    virtual dvec4 valueToVec4Double(void* val) const {
        return util::glm_convert<dvec4>(*static_cast<type*>(val));
    }

    virtual double valueToNormalizedDouble(void* val) const {
        return util::glm_convert_normalized<double>(*static_cast<type*>(val));
    }
    virtual dvec2 valueToNormalizedVec2Double(void* val) const {
        return util::glm_convert_normalized<dvec2>(*static_cast<type*>(val));
    }
    virtual dvec3 valueToNormalizedVec3Double(void* val) const {
        return util::glm_convert_normalized<dvec3>(*static_cast<type*>(val));
    }
    virtual dvec4 valueToNormalizedVec4Double(void* val) const {
        return util::glm_convert_normalized<dvec4>(*static_cast<type*>(val));
    }

    virtual void doubleToValue(double in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
    virtual void vec2DoubleToValue(dvec2 in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
    virtual void vec3DoubleToValue(dvec3 in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
    virtual void vec4DoubleToValue(dvec4 in, void* out) const {
        *static_cast<type*>(out) = util::glm_convert<type>(in);
    }
};


/*---------------Single Value Formats------------------*/

// Floats
typedef DataFormat<half_float::half> DataFloat16;
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
typedef glm::detail::tvec2<half_float::half, glm::defaultp> f16vec2;
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
typedef glm::detail::tvec3<half_float::half, glm::defaultp> f16vec3;
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
typedef glm::detail::tvec4<half_float::half, glm::defaultp> f16vec4;
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
template<> inline size_t DataFloat16::size() { return DataFloat32::size(); }

// Type Function Specializations
template<> inline DataFormatId DataFloat16::id() { return DataFormatId::Float16; }
template<> inline DataFormatId DataFloat32::id() { return DataFormatId::Float32; }
template<> inline DataFormatId DataFloat64::id() { return DataFormatId::Float64; }

template<> inline DataFormatId DataInt8::id()  { return DataFormatId::Int8; }
template<> inline DataFormatId DataInt16::id() { return DataFormatId::Int16; }
template<> inline DataFormatId DataInt32::id() { return DataFormatId::Int32; }
template<> inline DataFormatId DataInt64::id() { return DataFormatId::Int64; }

template<> inline DataFormatId DataUInt8::id()  { return DataFormatId::UInt8; }
template<> inline DataFormatId DataUInt16::id() { return DataFormatId::UInt16; }
template<> inline DataFormatId DataUInt32::id() { return DataFormatId::UInt32; }
template<> inline DataFormatId DataUInt64::id() { return DataFormatId::UInt64; }

// Numeric type Specializations
template<> inline NumericType DataFloat16::numericType() { return NumericType::Float; }
template<> inline NumericType DataFloat32::numericType() { return NumericType::Float; }
template<> inline NumericType DataFloat64::numericType() { return NumericType::Float; }

template<> inline NumericType DataInt8::numericType()  { return NumericType::SignedInteger; }
template<> inline NumericType DataInt16::numericType() { return NumericType::SignedInteger; }
template<> inline NumericType DataInt32::numericType() { return NumericType::SignedInteger; }
template<> inline NumericType DataInt64::numericType() { return NumericType::SignedInteger; }

template<> inline NumericType DataUInt8::numericType()  { return NumericType::UnsignedInteger; }
template<> inline NumericType DataUInt16::numericType() { return NumericType::UnsignedInteger; }
template<> inline NumericType DataUInt32::numericType() { return NumericType::UnsignedInteger; }
template<> inline NumericType DataUInt64::numericType() { return NumericType::UnsignedInteger; }

// String Function Specializations
template<> inline std::string DataFloat16::str() { return "FLOAT16"; }
template<> inline std::string DataFloat32::str() { return "FLOAT32"; }
template<> inline std::string DataFloat64::str() { return "FLOAT64"; }

template<> inline std::string DataInt8::str() { return "INT8"; }
template<> inline std::string DataInt16::str() { return "INT16"; }
template<> inline std::string DataInt32::str() { return "INT32"; }
template<> inline std::string DataInt64::str() { return "INT64"; }

template<> inline std::string DataUInt8::str() { return "UINT8"; }
template<> inline std::string DataUInt16::str() { return "UINT16"; }
template<> inline std::string DataUInt32::str() { return "UINT32"; }
template<> inline std::string DataUInt64::str() { return "UINT64"; }


/*---------------Vec2 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec2Float16::size() { return DataVec2Float32::size(); }

// Type Function Specializations
template<> inline DataFormatId DataVec2Float16::id() { return DataFormatId::Vec2Float16; }
template<> inline DataFormatId DataVec2Float32::id() { return DataFormatId::Vec2Float32; }
template<> inline DataFormatId DataVec2Float64::id() { return DataFormatId::Vec2Float64; }

template<> inline DataFormatId DataVec2Int8::id() { return DataFormatId::Vec2Int8; }
template<> inline DataFormatId DataVec2Int16::id() { return DataFormatId::Vec2Int16; }
template<> inline DataFormatId DataVec2Int32::id() { return DataFormatId::Vec2Int32; }
template<> inline DataFormatId DataVec2Int64::id() { return DataFormatId::Vec2Int64; }

template<> inline DataFormatId DataVec2UInt8::id() { return DataFormatId::Vec2UInt8; }
template<> inline DataFormatId DataVec2UInt16::id() { return DataFormatId::Vec2UInt16; }
template<> inline DataFormatId DataVec2UInt32::id() { return DataFormatId::Vec2UInt32; }
template<> inline DataFormatId DataVec2UInt64::id() { return DataFormatId::Vec2UInt64; }


/*---------------Vec3 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec3Float16::size() { return DataVec3Float32::size(); }

// Type Function Specializations
template<> inline DataFormatId DataVec3Float16::id() { return DataFormatId::Vec3Float16; }
template<> inline DataFormatId DataVec3Float32::id() { return DataFormatId::Vec3Float32; }
template<> inline DataFormatId DataVec3Float64::id() { return DataFormatId::Vec3Float64; }

template<> inline DataFormatId DataVec3Int8::id() { return DataFormatId::Vec3Int8; }
template<> inline DataFormatId DataVec3Int16::id() { return DataFormatId::Vec3Int16; }
template<> inline DataFormatId DataVec3Int32::id() { return DataFormatId::Vec3Int32; }
template<> inline DataFormatId DataVec3Int64::id() { return DataFormatId::Vec3Int64; }

template<> inline DataFormatId DataVec3UInt8::id() { return DataFormatId::Vec3UInt8; }
template<> inline DataFormatId DataVec3UInt16::id() { return DataFormatId::Vec3UInt16; }
template<> inline DataFormatId DataVec3UInt32::id() { return DataFormatId::Vec3UInt32; }
template<> inline DataFormatId DataVec3UInt64::id() { return DataFormatId::Vec3UInt64; }



/*---------------Vec4 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec4Float16::size() { return DataVec4Float32::size(); }

// Type Function Specializations
template<> inline DataFormatId DataVec4Float16::id() { return DataFormatId::Vec4Float16; }
template<> inline DataFormatId DataVec4Float32::id() { return DataFormatId::Vec4Float32; }
template<> inline DataFormatId DataVec4Float64::id() { return DataFormatId::Vec4Float64; }

template<> inline DataFormatId DataVec4Int8::id() { return DataFormatId::Vec4Int8; }
template<> inline DataFormatId DataVec4Int16::id() { return DataFormatId::Vec4Int16; }
template<> inline DataFormatId DataVec4Int32::id() { return DataFormatId::Vec4Int32; }
template<> inline DataFormatId DataVec4Int64::id() { return DataFormatId::Vec4Int64; }

template<> inline DataFormatId DataVec4UInt8::id() { return DataFormatId::Vec4UInt8; }
template<> inline DataFormatId DataVec4UInt16::id() { return DataFormatId::Vec4UInt16; }
template<> inline DataFormatId DataVec4UInt32::id() { return DataFormatId::Vec4UInt32; }
template<> inline DataFormatId DataVec4UInt64::id() { return DataFormatId::Vec4UInt64; }


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
