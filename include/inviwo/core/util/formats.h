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

/*! \brief Defines general useful formats and new data types
 * Non-virtual, meaning no dynamic_cast as string comparison is as fast/faster
 */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#endif

namespace inviwo {

namespace DataFormatEnums {

//Do not set enums specifically, as NUMBER_OF_FORMATS is used to count the number of enums
enum Id {
    NOT_SPECIALIZED,
    FLOAT16,
    FLOAT32,
    FLOAT64,
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    Vec2FLOAT16,
    Vec2FLOAT32,
    Vec2FLOAT64,
    Vec2INT8,
    Vec2INT16,
    Vec2INT32,
    Vec2INT64,
    Vec2UINT8,
    Vec2UINT16,
    Vec2UINT32,
    Vec2UINT64,
    Vec3FLOAT16,
    Vec3FLOAT32,
    Vec3FLOAT64,
    Vec3INT8,
    Vec3INT16,
    Vec3INT32,
    Vec3INT64,
    Vec3UINT8,
    Vec3UINT16,
    Vec3UINT32,
    Vec3UINT64,
    Vec4FLOAT16,
    Vec4FLOAT32,
    Vec4FLOAT64,
    Vec4INT8,
    Vec4INT16,
    Vec4INT32,
    Vec4INT64,
    Vec4UINT8,
    Vec4UINT16,
    Vec4UINT32,
    Vec4UINT64,
    NUMBER_OF_FORMATS
};

enum NumericType {
    NOT_SPECIALIZED_TYPE,
    FLOAT_TYPE,
    UNSIGNED_INTEGER_TYPE,
    SIGNED_INTEGER_TYPE
};

}

class IVW_CORE_API DataFormatBase {
public:
    DataFormatBase();
    DataFormatBase(DataFormatEnums::Id type, size_t components, size_t size, double max, double min,
                   DataFormatEnums::NumericType nt, std::string s);
    virtual ~DataFormatBase();

    static const DataFormatBase* get();
    static const DataFormatBase* get(DataFormatEnums::Id id);
    static const DataFormatBase* get(std::string name);
    static const DataFormatBase* get(DataFormatEnums::NumericType type, size_t components,
                                     size_t precision);

    static void cleanDataFormatBases();

    static size_t size() { return 0; }
    static size_t components() { return 0; }
    static DataFormatEnums::NumericType numericType() {
        return DataFormatEnums::NOT_SPECIALIZED_TYPE;
    }
    static std::string str() { return "Error, type specialization not implemented"; }
    static DataFormatEnums::Id id() { return DataFormatEnums::NOT_SPECIALIZED; }

    size_t getSize() const;
    size_t getComponents() const;

    DataFormatEnums::NumericType getNumericType() const;
    double getMax() const;
    double getMin() const;
    const char* getString() const;
    DataFormatEnums::Id getId() const;

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
    static DataFormatBase* instance_[DataFormatEnums::NUMBER_OF_FORMATS];
    static DataFormatBase* getNonConst(DataFormatEnums::Id id) { return instance_[id]; }

    DataFormatEnums::Id formatId_;
    size_t components_;
    size_t size_;
    DataFormatEnums::NumericType numericType_;
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
            instance_[id()] = d;
        }
        return static_cast<DataFormat<T>*>(d);
    }

    static size_t size() { return typesize; }
    static size_t components() { return comp; }
    static DataFormatEnums::NumericType numericType() {
        return DataFormatEnums::NOT_SPECIALIZED_TYPE;
    }

    static T max() { return std::numeric_limits<T>::max(); }
    static T min() { return std::numeric_limits<T>::min(); }

    static double maxToDouble() { return static_cast<double>(max()); }
    static double minToDouble() { return static_cast<double>(min()); }

    static std::string str() { return DataFormatBase::str(); }
    static DataFormatEnums::Id id() { return DataFormatBase::id(); }

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
            instance_[id()] = d;
        }
        return static_cast<DataFormat<type>*>(d);
    }

    static size_t size() { return typesize; }
    static size_t components() { return comp; }
    static DataFormatEnums::NumericType numericType() { return DataFormat<T>::numericType(); }

    static type max() { return type(DataFormat<T>::max()); }
    static type min() { return type(DataFormat<T>::min()); }

    static double maxToDouble() { return static_cast<double>(DataFormat<T>::max()); }
    static double minToDouble() { return static_cast<double>(DataFormat<T>::min()); }

    static std::string str() { return "Vec" + toString(comp) + DataFormat<T>::str(); }
    static DataFormatEnums::Id id() { return DataFormat<T>::id(); }

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
typedef DataFormat<half_float::half> DataFLOAT16;
typedef DataFormat<glm::f32> DataFLOAT32;
typedef DataFormat<glm::f64> DataFLOAT64;

// Integers
typedef DataFormat<glm::i8>   DataINT8;
typedef DataFormat<glm::i16>  DataINT16;
typedef DataFormat<glm::i32>  DataINT32;
typedef DataFormat<glm::i64>  DataINT64;

// Unsigned Integers
typedef DataFormat<glm::u8>   DataUINT8;
typedef DataFormat<glm::u16>  DataUINT16;
typedef DataFormat<glm::u32>  DataUINT32;
typedef DataFormat<glm::u64>  DataUINT64;

/*---------------Vec2 Formats--------------------*/

// Floats
typedef glm::detail::tvec2<half_float::half, glm::defaultp> f16vec2;
typedef DataFormat<f16vec2> DataVec2FLOAT16;
typedef DataFormat<glm::f32vec2> DataVec2FLOAT32;
typedef DataFormat<glm::f64vec2> DataVec2FLOAT64;

// Integers
typedef DataFormat<glm::i8vec2>  DataVec2INT8;
typedef DataFormat<glm::i16vec2> DataVec2INT16;
typedef DataFormat<glm::i32vec2> DataVec2INT32;
typedef DataFormat<glm::i64vec2> DataVec2INT64;

// Unsigned Integers
typedef DataFormat<glm::u8vec2>  DataVec2UINT8;
typedef DataFormat<glm::u16vec2> DataVec2UINT16;
typedef DataFormat<glm::u32vec2> DataVec2UINT32;
typedef DataFormat<glm::u64vec2> DataVec2UINT64;

/*---------------Vec3 Formats--------------------*/

// Floats
typedef glm::detail::tvec3<half_float::half, glm::defaultp> f16vec3;
typedef DataFormat<f16vec3> DataVec3FLOAT16;
typedef DataFormat<glm::f32vec3> DataVec3FLOAT32;
typedef DataFormat<glm::f64vec3> DataVec3FLOAT64;

// Integers
typedef DataFormat<glm::i8vec3>  DataVec3INT8;
typedef DataFormat<glm::i16vec3> DataVec3INT16;
typedef DataFormat<glm::i32vec3> DataVec3INT32;
typedef DataFormat<glm::i64vec3> DataVec3INT64;

// Unsigned Integers
typedef DataFormat<glm::u8vec3>  DataVec3UINT8;
typedef DataFormat<glm::u16vec3> DataVec3UINT16;
typedef DataFormat<glm::u32vec3> DataVec3UINT32;
typedef DataFormat<glm::u64vec3> DataVec3UINT64;

/*---------------Vec4 Value Formats------------------*/

// Floats
typedef glm::detail::tvec4<half_float::half, glm::defaultp> f16vec4;
typedef DataFormat<f16vec4> DataVec4FLOAT16;
typedef DataFormat<glm::f32vec4> DataVec4FLOAT32;
typedef DataFormat<glm::f64vec4> DataVec4FLOAT64;

// Integers
typedef DataFormat<glm::i8vec4>  DataVec4INT8;
typedef DataFormat<glm::i16vec4> DataVec4INT16;
typedef DataFormat<glm::i32vec4> DataVec4INT32;
typedef DataFormat<glm::i64vec4> DataVec4INT64;

// Unsigned Integers
typedef DataFormat<glm::u8vec4>  DataVec4UINT8;
typedef DataFormat<glm::u16vec4> DataVec4UINT16;
typedef DataFormat<glm::u32vec4> DataVec4UINT32;
typedef DataFormat<glm::u64vec4> DataVec4UINT64;


/*---------------Single Value Formats------------------*/

// Bit Specializations
template<> inline size_t DataFLOAT16::size() { return DataFLOAT32::size(); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataFLOAT16::id() { return DataFormatEnums::FLOAT16; }
template<> inline DataFormatEnums::Id DataFLOAT32::id() { return DataFormatEnums::FLOAT32; }
template<> inline DataFormatEnums::Id DataFLOAT64::id() { return DataFormatEnums::FLOAT64; }

template<> inline DataFormatEnums::Id DataINT8::id()  { return DataFormatEnums::INT8; }
template<> inline DataFormatEnums::Id DataINT16::id() { return DataFormatEnums::INT16; }
template<> inline DataFormatEnums::Id DataINT32::id() { return DataFormatEnums::INT32; }
template<> inline DataFormatEnums::Id DataINT64::id() { return DataFormatEnums::INT64; }

template<> inline DataFormatEnums::Id DataUINT8::id()  { return DataFormatEnums::UINT8; }
template<> inline DataFormatEnums::Id DataUINT16::id() { return DataFormatEnums::UINT16; }
template<> inline DataFormatEnums::Id DataUINT32::id() { return DataFormatEnums::UINT32; }
template<> inline DataFormatEnums::Id DataUINT64::id() { return DataFormatEnums::UINT64; }

// Numeric type Specializations
template<> inline DataFormatEnums::NumericType DataFLOAT16::numericType() { return DataFormatEnums::FLOAT_TYPE; }
template<> inline DataFormatEnums::NumericType DataFLOAT32::numericType() { return DataFormatEnums::FLOAT_TYPE; }
template<> inline DataFormatEnums::NumericType DataFLOAT64::numericType() { return DataFormatEnums::FLOAT_TYPE; }

template<> inline DataFormatEnums::NumericType DataINT8::numericType()  { return DataFormatEnums::SIGNED_INTEGER_TYPE; }
template<> inline DataFormatEnums::NumericType DataINT16::numericType() { return DataFormatEnums::SIGNED_INTEGER_TYPE; }
template<> inline DataFormatEnums::NumericType DataINT32::numericType() { return DataFormatEnums::SIGNED_INTEGER_TYPE; }
template<> inline DataFormatEnums::NumericType DataINT64::numericType() { return DataFormatEnums::SIGNED_INTEGER_TYPE; }

template<> inline DataFormatEnums::NumericType DataUINT8::numericType()  { return DataFormatEnums::UNSIGNED_INTEGER_TYPE; }
template<> inline DataFormatEnums::NumericType DataUINT16::numericType() { return DataFormatEnums::UNSIGNED_INTEGER_TYPE; }
template<> inline DataFormatEnums::NumericType DataUINT32::numericType() { return DataFormatEnums::UNSIGNED_INTEGER_TYPE; }
template<> inline DataFormatEnums::NumericType DataUINT64::numericType() { return DataFormatEnums::UNSIGNED_INTEGER_TYPE; }

// String Function Specializations
template<> inline std::string DataFLOAT16::str() { return "FLOAT16"; }
template<> inline std::string DataFLOAT32::str() { return "FLOAT32"; }
template<> inline std::string DataFLOAT64::str() { return "FLOAT64"; }

template<> inline std::string DataINT8::str() { return "INT8"; }
template<> inline std::string DataINT16::str() { return "INT16"; }
template<> inline std::string DataINT32::str() { return "INT32"; }
template<> inline std::string DataINT64::str() { return "INT64"; }

template<> inline std::string DataUINT8::str() { return "UINT8"; }
template<> inline std::string DataUINT16::str() { return "UINT16"; }
template<> inline std::string DataUINT32::str() { return "UINT32"; }
template<> inline std::string DataUINT64::str() { return "UINT64"; }


/*---------------Vec2 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec2FLOAT16::size() { return DataVec2FLOAT32::size(); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataVec2FLOAT16::id() { return DataFormatEnums::Vec2FLOAT16; }
template<> inline DataFormatEnums::Id DataVec2FLOAT32::id() { return DataFormatEnums::Vec2FLOAT32; }
template<> inline DataFormatEnums::Id DataVec2FLOAT64::id() { return DataFormatEnums::Vec2FLOAT64; }

template<> inline DataFormatEnums::Id DataVec2INT8::id() { return DataFormatEnums::Vec2INT8; }
template<> inline DataFormatEnums::Id DataVec2INT16::id() { return DataFormatEnums::Vec2INT16; }
template<> inline DataFormatEnums::Id DataVec2INT32::id() { return DataFormatEnums::Vec2INT32; }
template<> inline DataFormatEnums::Id DataVec2INT64::id() { return DataFormatEnums::Vec2INT64; }

template<> inline DataFormatEnums::Id DataVec2UINT8::id() { return DataFormatEnums::Vec2UINT8; }
template<> inline DataFormatEnums::Id DataVec2UINT16::id() { return DataFormatEnums::Vec2UINT16; }
template<> inline DataFormatEnums::Id DataVec2UINT32::id() { return DataFormatEnums::Vec2UINT32; }
template<> inline DataFormatEnums::Id DataVec2UINT64::id() { return DataFormatEnums::Vec2UINT64; }


/*---------------Vec3 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec3FLOAT16::size() { return DataVec3FLOAT32::size(); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataVec3FLOAT16::id() { return DataFormatEnums::Vec3FLOAT16; }
template<> inline DataFormatEnums::Id DataVec3FLOAT32::id() { return DataFormatEnums::Vec3FLOAT32; }
template<> inline DataFormatEnums::Id DataVec3FLOAT64::id() { return DataFormatEnums::Vec3FLOAT64; }

template<> inline DataFormatEnums::Id DataVec3INT8::id() { return DataFormatEnums::Vec3INT8; }
template<> inline DataFormatEnums::Id DataVec3INT16::id() { return DataFormatEnums::Vec3INT16; }
template<> inline DataFormatEnums::Id DataVec3INT32::id() { return DataFormatEnums::Vec3INT32; }
template<> inline DataFormatEnums::Id DataVec3INT64::id() { return DataFormatEnums::Vec3INT64; }

template<> inline DataFormatEnums::Id DataVec3UINT8::id() { return DataFormatEnums::Vec3UINT8; }
template<> inline DataFormatEnums::Id DataVec3UINT16::id() { return DataFormatEnums::Vec3UINT16; }
template<> inline DataFormatEnums::Id DataVec3UINT32::id() { return DataFormatEnums::Vec3UINT32; }
template<> inline DataFormatEnums::Id DataVec3UINT64::id() { return DataFormatEnums::Vec3UINT64; }



/*---------------Vec4 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec4FLOAT16::size() { return DataVec4FLOAT32::size(); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataVec4FLOAT16::id() { return DataFormatEnums::Vec4FLOAT16; }
template<> inline DataFormatEnums::Id DataVec4FLOAT32::id() { return DataFormatEnums::Vec4FLOAT32; }
template<> inline DataFormatEnums::Id DataVec4FLOAT64::id() { return DataFormatEnums::Vec4FLOAT64; }

template<> inline DataFormatEnums::Id DataVec4INT8::id() { return DataFormatEnums::Vec4INT8; }
template<> inline DataFormatEnums::Id DataVec4INT16::id() { return DataFormatEnums::Vec4INT16; }
template<> inline DataFormatEnums::Id DataVec4INT32::id() { return DataFormatEnums::Vec4INT32; }
template<> inline DataFormatEnums::Id DataVec4INT64::id() { return DataFormatEnums::Vec4INT64; }

template<> inline DataFormatEnums::Id DataVec4UINT8::id() { return DataFormatEnums::Vec4UINT8; }
template<> inline DataFormatEnums::Id DataVec4UINT16::id() { return DataFormatEnums::Vec4UINT16; }
template<> inline DataFormatEnums::Id DataVec4UINT32::id() { return DataFormatEnums::Vec4UINT32; }
template<> inline DataFormatEnums::Id DataVec4UINT64::id() { return DataFormatEnums::Vec4UINT64; }


template <typename T, typename... Args>
auto DataFormatBase::dispatch(T& obj, Args&&... args) const -> typename T::type {
    switch (formatId_) {
        case DataFormatEnums::FLOAT16:
            return obj.template dispatch<DataFLOAT16>(std::forward<Args>(args)...);
        case DataFormatEnums::FLOAT32:
            return obj.template dispatch<DataFLOAT32>(std::forward<Args>(args)...);
        case DataFormatEnums::FLOAT64:
            return obj.template dispatch<DataFLOAT64>(std::forward<Args>(args)...);
        case DataFormatEnums::INT8:
            return obj.template dispatch<DataINT8>(std::forward<Args>(args)...);
        case DataFormatEnums::INT16:
            return obj.template dispatch<DataINT16>(std::forward<Args>(args)...);
        case DataFormatEnums::INT32:
            return obj.template dispatch<DataINT32>(std::forward<Args>(args)...);
        case DataFormatEnums::INT64:
            return obj.template dispatch<DataINT64>(std::forward<Args>(args)...);
        case DataFormatEnums::UINT8:
            return obj.template dispatch<DataUINT8>(std::forward<Args>(args)...);
        case DataFormatEnums::UINT16:
            return obj.template dispatch<DataUINT16>(std::forward<Args>(args)...);
        case DataFormatEnums::UINT32:
            return obj.template dispatch<DataUINT32>(std::forward<Args>(args)...);
        case DataFormatEnums::UINT64:
            return obj.template dispatch<DataUINT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2FLOAT16:
            return obj.template dispatch<DataVec2FLOAT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2FLOAT32:
            return obj.template dispatch<DataVec2FLOAT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2FLOAT64:
            return obj.template dispatch<DataVec2FLOAT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2INT8:
            return obj.template dispatch<DataVec2INT8>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2INT16:
            return obj.template dispatch<DataVec2INT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2INT32:
            return obj.template dispatch<DataVec2INT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2INT64:
            return obj.template dispatch<DataVec2INT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2UINT8:
            return obj.template dispatch<DataVec2UINT8>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2UINT16:
            return obj.template dispatch<DataVec2UINT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2UINT32:
            return obj.template dispatch<DataVec2UINT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec2UINT64:
            return obj.template dispatch<DataVec2UINT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3FLOAT16:
            return obj.template dispatch<DataVec3FLOAT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3FLOAT32:
            return obj.template dispatch<DataVec3FLOAT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3FLOAT64:
            return obj.template dispatch<DataVec3FLOAT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3INT8:
            return obj.template dispatch<DataVec3INT8>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3INT16:
            return obj.template dispatch<DataVec3INT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3INT32:
            return obj.template dispatch<DataVec3INT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3INT64:
            return obj.template dispatch<DataVec3INT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3UINT8:
            return obj.template dispatch<DataVec3UINT8>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3UINT16:
            return obj.template dispatch<DataVec3UINT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3UINT32:
            return obj.template dispatch<DataVec3UINT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec3UINT64:
            return obj.template dispatch<DataVec3UINT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4FLOAT16:
            return obj.template dispatch<DataVec4FLOAT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4FLOAT32:
            return obj.template dispatch<DataVec4FLOAT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4FLOAT64:
            return obj.template dispatch<DataVec4FLOAT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4INT8:
            return obj.template dispatch<DataVec4INT8>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4INT16:
            return obj.template dispatch<DataVec4INT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4INT32:
            return obj.template dispatch<DataVec4INT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4INT64:
            return obj.template dispatch<DataVec4INT64>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4UINT8:
            return obj.template dispatch<DataVec4UINT8>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4UINT16:
            return obj.template dispatch<DataVec4UINT16>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4UINT32:
            return obj.template dispatch<DataVec4UINT32>(std::forward<Args>(args)...);
        case DataFormatEnums::Vec4UINT64:
            return obj.template dispatch<DataVec4UINT64>(std::forward<Args>(args)...);
        case DataFormatEnums::NOT_SPECIALIZED:
        case DataFormatEnums::NUMBER_OF_FORMATS:
        default:
            return nullptr;
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

DEFAULTVALUES(float, uvec2(1, 1), "Float", 0.0f, 0.0f, 1.0f, 0.01f)
DEFAULTVALUES(double, uvec2(1, 1), "Double", 0.0, 0.0, 1.0, 0.01)
DEFAULTVALUES(int, uvec2(1, 1), "Int", 0, -100, 100, 1)
DEFAULTVALUES(unsigned int, uvec2(1, 1), "UInt", 0, 0, 100, 1)
DEFAULTVALUES(glm::i64, uvec2(1, 1), "Int64", 0, 0, 1024, 1)

DEFAULTVALUES(vec2, uvec2(2, 1), "FloatVec2", vec2(0.f), vec2(0.f), vec2(1.f), vec2(0.01f))
DEFAULTVALUES(vec3, uvec2(3, 1), "FloatVec3", vec3(0.f), vec3(0.f), vec3(1.f), vec3(0.01f))
DEFAULTVALUES(vec4, uvec2(4, 1), "FloatVec4", vec4(0.f), vec4(0.f), vec4(1.f), vec4(0.01f))

DEFAULTVALUES(dvec2, uvec2(2, 1), "DoubleVec2", dvec2(0.), dvec2(0.), dvec2(1.), dvec2(0.01))
DEFAULTVALUES(dvec3, uvec2(3, 1), "DoubleVec3", dvec3(0.), dvec3(0.), dvec3(1.), dvec3(0.01))
DEFAULTVALUES(dvec4, uvec2(4, 1), "DoubleVec4", dvec4(0.), dvec4(0.), dvec4(1.), dvec4(0.01))

DEFAULTVALUES(ivec2, uvec2(2, 1), "IntVec2", ivec2(0), ivec2(0), ivec2(10), ivec2(1))
DEFAULTVALUES(ivec3, uvec2(3, 1), "IntVec3", ivec3(0), ivec3(0), ivec3(10), ivec3(1))
DEFAULTVALUES(ivec4, uvec2(4, 1), "IntVec4", ivec4(0), ivec4(0), ivec4(10), ivec4(1))

DEFAULTVALUES(uvec2, uvec2(2, 1), "UIntVec2", uvec2(0), uvec2(0), uvec2(10), uvec2(1))
DEFAULTVALUES(uvec3, uvec2(3, 1), "UIntVec3", uvec3(0), uvec3(0), uvec3(10), uvec3(1))
DEFAULTVALUES(uvec4, uvec2(4, 1), "UIntVec4", uvec4(0), uvec4(0), uvec4(10), uvec4(1))

DEFAULTVALUES(mat2, uvec2(2, 2), "FloatMat2", mat2(0.f), mat2(0.f), mat2(0.f)+1.0f, mat2(0.f)+0.01f)
DEFAULTVALUES(mat3, uvec2(3, 3), "FloatMat3", mat3(0.f), mat3(0.f), mat3(0.f)+1.0f, mat3(0.f)+0.01f)
DEFAULTVALUES(mat4, uvec2(4, 4), "FloatMat4", mat4(0.f), mat4(0.f), mat4(0.f)+1.0f, mat4(0.f)+0.01f)

DEFAULTVALUES(dmat2, uvec2(2, 2), "DoubleMat2", dmat2(0.), dmat2(0.), dmat2(0.)+1.0, dmat2(0.)+0.01)
DEFAULTVALUES(dmat3, uvec2(3, 3), "DoubleMat3", dmat3(0.), dmat3(0.), dmat3(0.)+1.0, dmat3(0.)+0.01)
DEFAULTVALUES(dmat4, uvec2(4, 4), "DoubleMat4", dmat4(0.), dmat4(0.), dmat4(0.)+1.0, dmat4(0.)+0.01)

DEFAULTVALUES(std::string, uvec2(1, 1), "String", "", "", "", "")
DEFAULTVALUES(bool, uvec2(1, 1), "Bool", false, false, true, true)

#undef DEFAULTVALUES


#define CallFunctionWithTemplateArgsForType(fun, id) \
    switch (id) {\
    case DataFormatEnums::FLOAT16: fun<DataFLOAT16::type>(); break; \
    case DataFormatEnums::FLOAT32: fun<DataFLOAT32::type>(); break; \
    case DataFormatEnums::FLOAT64: fun<DataFLOAT64::type>(); break; \
    case DataFormatEnums::INT8: fun<DataINT8::type>(); break; \
    case DataFormatEnums::INT16: fun<DataINT16::type>(); break; \
    case DataFormatEnums::INT32: fun<DataINT32::type>(); break; \
    case DataFormatEnums::INT64: fun<DataINT64::type>(); break; \
    case DataFormatEnums::UINT8: fun<DataUINT8::type>(); break; \
    case DataFormatEnums::UINT16: fun<DataUINT16::type>(); break; \
    case DataFormatEnums::UINT32: fun<DataUINT32::type>(); break; \
    case DataFormatEnums::UINT64: fun<DataUINT64::type>(); break; \
    case DataFormatEnums::Vec2FLOAT16: fun<DataVec2FLOAT16::type>(); break; \
    case DataFormatEnums::Vec2FLOAT32: fun<DataVec2FLOAT32::type>(); break; \
    case DataFormatEnums::Vec2FLOAT64: fun<DataVec2FLOAT64::type>(); break; \
    case DataFormatEnums::Vec2INT8: fun<DataVec2INT8::type>(); break; \
    case DataFormatEnums::Vec2INT16: fun<DataVec2INT16::type>(); break; \
    case DataFormatEnums::Vec2INT32: fun<DataVec2INT32::type>(); break; \
    case DataFormatEnums::Vec2INT64: fun<DataVec2INT64::type>(); break; \
    case DataFormatEnums::Vec2UINT8: fun<DataVec2UINT8::type>(); break; \
    case DataFormatEnums::Vec2UINT16: fun<DataVec2UINT16::type>(); break; \
    case DataFormatEnums::Vec2UINT32: fun<DataVec2UINT32::type>(); break; \
    case DataFormatEnums::Vec2UINT64: fun<DataVec2UINT64::type>(); break; \
    case DataFormatEnums::Vec3FLOAT16: fun<DataVec3FLOAT16::type>(); break; \
    case DataFormatEnums::Vec3FLOAT32: fun<DataVec3FLOAT32::type>(); break; \
    case DataFormatEnums::Vec3FLOAT64: fun<DataVec3FLOAT64::type>(); break; \
    case DataFormatEnums::Vec3INT8: fun<DataVec3INT8::type>(); break; \
    case DataFormatEnums::Vec3INT16: fun<DataVec3INT16::type>(); break; \
    case DataFormatEnums::Vec3INT32: fun<DataVec3INT32::type>(); break; \
    case DataFormatEnums::Vec3INT64: fun<DataVec3INT64::type>(); break; \
    case DataFormatEnums::Vec3UINT8: fun<DataVec3UINT8::type>(); break; \
    case DataFormatEnums::Vec3UINT16: fun<DataVec3UINT16::type>(); break; \
    case DataFormatEnums::Vec3UINT32: fun<DataVec3UINT32::type>(); break; \
    case DataFormatEnums::Vec3UINT64: fun<DataVec3UINT64::type>(); break; \
    case DataFormatEnums::Vec4FLOAT16: fun<DataVec4FLOAT16::type>(); break; \
    case DataFormatEnums::Vec4FLOAT32: fun<DataVec4FLOAT32::type>(); break; \
    case DataFormatEnums::Vec4FLOAT64: fun<DataVec4FLOAT64::type>(); break; \
    case DataFormatEnums::Vec4INT8: fun<DataVec4INT8::type>(); break; \
    case DataFormatEnums::Vec4INT16: fun<DataVec4INT16::type>(); break; \
    case DataFormatEnums::Vec4INT32: fun<DataVec4INT32::type>(); break; \
    case DataFormatEnums::Vec4INT64: fun<DataVec4INT64::type>(); break; \
    case DataFormatEnums::Vec4UINT8: fun<DataVec4UINT8::type>(); break; \
    case DataFormatEnums::Vec4UINT16: fun<DataVec4UINT16::type>(); break; \
    case DataFormatEnums::Vec4UINT32: fun<DataVec4UINT32::type>(); break; \
    case DataFormatEnums::Vec4UINT64: fun<DataVec4UINT64::type>(); break; \
    default: break; \
}


}

#endif
