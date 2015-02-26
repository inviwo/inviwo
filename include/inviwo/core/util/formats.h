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

#pragma warning(disable : 4723)
#pragma warning(disable : 4756)
#pragma warning(disable : 4244) // min/max to double.

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <limits>
#include <string>

#include <half/half.hpp>

/*! \brief Defines general useful formats and new data types
 * Non-virtual, meaning no dynamic_cast as string comparison is as fast/faster
 */

#define BYTES_TO_BITS(bytes) (bytes*8)
#define BITS_TO_BYTES(bytes) (bytes/8)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#endif

namespace inviwo {

template <unsigned int N, typename T>
class Matrix {};

template <unsigned int N, typename T>
class Vector {};

template <typename T>
class Matrix<4, T> : public glm::detail::tmat4x4<T, glm::defaultp> {
public:
    Matrix<4, T>() : glm::detail::tmat4x4<T, glm::defaultp>() {};
    Matrix<4, T>(const Matrix<4, T>& m) : glm::detail::tmat4x4<T, glm::defaultp>(
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]) {};
    Matrix<4, T>(const glm::detail::tmat4x4<T, glm::defaultp>& m) : glm::detail::tmat4x4<T, glm::defaultp>(m) {};
    Matrix<4, T>(T m) : glm::detail::tmat4x4<T, glm::defaultp>(m) {};
    Matrix<4, T>(Vector<4,T>& m) : glm::detail::tmat4x4<T, glm::defaultp>() {*this = glm::diagonal4x4(m);}
    Matrix<4, T>(T x1, T y1, T z1, T w1,
                 T x2, T y2, T z2, T w2,
                 T x3, T y3, T z3, T w3,
                 T x4, T y4, T z4, T w4) :
        glm::detail::tmat4x4<T, glm::defaultp>(x1, y1, z1, w1,
                                               x2, y2, z2, w2,
                                               x3, y3, z3, w3,
                                               x4, y4, z4, w4) {};
    
    
    operator glm::detail::tmat4x4<T, glm::defaultp>&() { return *this; }
    operator const glm::detail::tmat4x4<T, glm::defaultp>&() const { return *this; }
    
    glm::detail::tmat4x4<T, glm::defaultp> getGLM() const {
        return *this;
    };
};
template <typename T>
class Matrix<3, T> : public glm::detail::tmat3x3<T, glm::defaultp> {
public:
    Matrix<3, T>() : glm::detail::tmat3x3<T, glm::defaultp>() {};
    Matrix<3, T>(const Matrix<3, T>& m) : glm::detail::tmat3x3<T, glm::defaultp>(
            m[0][0], m[0][1], m[0][2],
            m[1][0], m[1][1], m[1][2],
            m[2][0], m[2][1], m[2][2]) {};
    Matrix<3, T>(const glm::detail::tmat3x3<T, glm::defaultp>& m) : glm::detail::tmat3x3<T, glm::defaultp>(m) {};
    Matrix<3, T>(T m) : glm::detail::tmat3x3<T, glm::defaultp>(m) {};
    Matrix<3, T>(Vector<3,T>& m) : glm::detail::tmat3x3<T, glm::defaultp>() {*this = glm::diagonal3x3(m);}
    Matrix<3, T>(T x1, T y1, T z1,
                 T x2, T y2, T z2,
                 T x3, T y3, T z3) :
        glm::detail::tmat3x3<T, glm::defaultp>(x1, y1, z1,
                                               x2, y2, z2,
                                               x3, y3, z3) {};
    
    operator glm::detail::tmat3x3<T, glm::defaultp>&() { return *this; }
    operator const glm::detail::tmat3x3<T, glm::defaultp>&() const { return *this; }
    
    glm::detail::tmat3x3<T, glm::defaultp> getGLM() const {
        return *this;
    };
};
template <typename T>
class Matrix<2, T> : public glm::detail::tmat2x2<T, glm::defaultp> {
public:
    Matrix<2, T>() : glm::detail::tmat2x2<T, glm::defaultp>() {};
    Matrix<2, T>(const Matrix<2, T>& m) : glm::detail::tmat2x2<T, glm::defaultp>(
            m[0][0], m[0][1],
            m[1][0], m[1][1]) {};
    Matrix<2, T>(const glm::detail::tmat2x2<T, glm::defaultp>& m) : glm::detail::tmat2x2<T, glm::defaultp>(m) {};
    Matrix<2, T>(T m) : glm::detail::tmat2x2<T, glm::defaultp>(m) {};
    Matrix<2, T>(Vector<2,T>& m) : glm::detail::tmat2x2<T, glm::defaultp>() {*this = glm::diagonal2x2(m);}
    Matrix<2, T>(T x1, T y1,
                 T x2, T y2) :
        glm::detail::tmat2x2<T, glm::defaultp>(x1, y1,
                                               x2, y2) {};
    
     operator glm::detail::tmat2x2<T, glm::defaultp>&() { return *this; }
    operator const glm::detail::tmat2x2<T, glm::defaultp>&() const { return *this; }
    
    glm::detail::tmat2x2<T, glm::defaultp> getGLM() const {
        return *this;
    };
};

template <unsigned int N, typename T>
Matrix<N, T> MatrixInvert(const Matrix<N, T>& m) {
    return glm::inverse(m.getGLM());
}
template <typename T>
Matrix<4, T> MatrixInvert(const glm::detail::tmat4x4<T, glm::defaultp>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<3, T> MatrixInvert(const glm::detail::tmat3x3<T, glm::defaultp>& m) {
    return glm::inverse(m);
}
template <typename T>
Matrix<2, T> MatrixInvert(const glm::detail::tmat2x2<T, glm::defaultp>& m) {
    return glm::inverse(m);
}


template <typename T>
class Vector<4, T> : public glm::detail::tvec4<T, glm::defaultp> {
public:
    Vector<4, T>() : glm::detail::tvec4<T, glm::defaultp>() {};
    Vector<4, T>(const Vector<4, T>& v) : glm::detail::tvec4<T, glm::defaultp>(v.x, v.y, v.z, v.w) {};
    Vector<4, T>(const glm::detail::tvec4<T, glm::defaultp>& v) : glm::detail::tvec4<T, glm::defaultp>(v) {};
    Vector<4, T>(T v) : glm::detail::tvec4<T, glm::defaultp>(v) {};
    Vector<4, T>(T v1, T v2, T v3, T v4) : glm::detail::tvec2<T, glm::defaultp>(v1, v2, v3, v4) {};
    operator  glm::detail::tvec4<T, glm::defaultp>&() { return *this; }
    operator const  glm::detail::tvec4<T, glm::defaultp>&() const { return *this; }
    glm::detail::tvec4<T, glm::defaultp> getGLM() const { return *this; };
};
template <typename T>
class Vector<3, T> : public glm::detail::tvec3<T, glm::defaultp> {
public:
    Vector<3, T>() : glm::detail::tvec3<T, glm::defaultp>() {};
    Vector<3, T>(const Vector<3, T>& v) : glm::detail::tvec3<T, glm::defaultp>(v.x, v.y, v.z) {};
    Vector<3, T>(const glm::detail::tvec3<T, glm::defaultp>& v) : glm::detail::tvec3<T, glm::defaultp>(v) {};
    Vector<3, T>(T v) : glm::detail::tvec3<T, glm::defaultp>(v) {};
    Vector<3, T>(T v1, T v2, T v3) : glm::detail::tvec3<T, glm::defaultp>(v1, v2, v3) {};
    operator  glm::detail::tvec3<T, glm::defaultp>&() { return *this; }
    operator const  glm::detail::tvec3<T, glm::defaultp>&() const { return *this; }
    glm::detail::tvec3<T, glm::defaultp> getGLM() const { return *this; };
};
template <typename T>
class Vector<2, T> : public glm::detail::tvec2<T, glm::defaultp> {
public:
    Vector<2, T>() : glm::detail::tvec2<T, glm::defaultp>() {};
    Vector<2, T>(const Vector<2, T>& v) : glm::detail::tvec2<T, glm::defaultp>(v.x, v.y) {};
    Vector<2, T>(const glm::detail::tvec2<T, glm::defaultp>& v) : glm::detail::tvec2<T, glm::defaultp>(v) {};
    Vector<2, T>(T v) : glm::detail::tvec2<T, glm::defaultp>(v) {};
    Vector<2, T>(T v1, T v2) : glm::detail::tvec2<T, glm::defaultp>(v1, v2) {};
    operator  glm::detail::tvec2<T, glm::defaultp>&() { return *this; }
    operator const  glm::detail::tvec2<T, glm::defaultp>&() const { return *this; }
    glm::detail::tvec2<T, glm::defaultp> getGLM() const { return *this; };
};

namespace DataFormatEnums {

//Do not set enums specifically, as NUMBER_OF_FORMATS is used to count the number of enums
enum Id {
    NOT_SPECIALIZED,
    FLOAT16,
    FLOAT32,
    FLOAT64,
    INT8,
    INT12,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT12,
    UINT16,
    UINT32,
    UINT64,
    Vec2FLOAT16,
    Vec2FLOAT32,
    Vec2FLOAT64,
    Vec2INT8,
    Vec2INT12,
    Vec2INT16,
    Vec2INT32,
    Vec2INT64,
    Vec2UINT8,
    Vec2UINT12,
    Vec2UINT16,
    Vec2UINT32,
    Vec2UINT64,
    Vec3FLOAT16,
    Vec3FLOAT32,
    Vec3FLOAT64,
    Vec3INT8,
    Vec3INT12,
    Vec3INT16,
    Vec3INT32,
    Vec3INT64,
    Vec3UINT8,
    Vec3UINT12,
    Vec3UINT16,
    Vec3UINT32,
    Vec3UINT64,
    Vec4FLOAT16,
    Vec4FLOAT32,
    Vec4FLOAT64,
    Vec4INT8,
    Vec4INT12,
    Vec4INT16,
    Vec4INT32,
    Vec4INT64,
    Vec4UINT8,
    Vec4UINT12,
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
    DataFormatBase(DataFormatEnums::Id t, size_t bA, size_t bS, int c, double max, double min, DataFormatEnums::NumericType nt, std::string s);
    virtual ~DataFormatBase();

    static const DataFormatBase* get();
    static const DataFormatBase* get(DataFormatEnums::Id id);
    static const DataFormatBase* get(std::string name);
    static const DataFormatBase* get(DataFormatEnums::NumericType type, int components, int precision);

    static void cleanDataFormatBases();

    static size_t bitsAllocated() { return 0; }
    static size_t bitsStored() { return 0; }
    static int components() { return 0; }
    static DataFormatEnums::NumericType numericType() { return DataFormatEnums::NOT_SPECIALIZED_TYPE; }
    static std::string str() { return "Error, type specialization not implemented"; }
    static DataFormatEnums::Id id() { return DataFormatEnums::NOT_SPECIALIZED; }

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

    size_t getBitsAllocated() const;
    size_t getBitsStored() const;
    size_t getBytesAllocated() const;
    size_t getBytesStored() const;
    int getComponents() const;
    DataFormatEnums::NumericType getNumericType() const;
    double getMax() const;
    double getMin() const;
    const char* getString() const;
    DataFormatEnums::Id getId() const;


    // T Models a type with a function:
    //    template <class T>
    //    U dispatch(Args... args)
    // add a type
    //    T::type = return type
    template <typename T, typename... Args>
    auto dispatch(T& obj, Args&&... args) const -> typename T::type;


protected:
    static DataFormatBase* instance_[DataFormatEnums::NUMBER_OF_FORMATS];

    static DataFormatBase* getNonConst(DataFormatEnums::Id id) {
        return instance_[id];
    }

    DataFormatEnums::Id formatId_;
    size_t bitsAllocated_;
    size_t bitsStored_;
    int components_;
    DataFormatEnums::NumericType numericType_;
    double max_;
    double min_;
    std::string* formatStr_;

};

template<typename T, size_t B>
class IVW_CORE_API DataFormat : public DataFormatBase {
public:
    typedef T type;
    typedef T primitive;
    static const size_t bits = B;
    static const size_t comp = 1;
    static const size_t bitscomp = B;

    DataFormat() : DataFormatBase(id(),
                                      bitsAllocated(),
                                      bitsStored(),
                                      components(),
                                      maxToDouble(),
                                      minToDouble(),
                                      numericType(),
                                      str()) {
    }

    virtual ~DataFormat() {}

    static const DataFormat<T, B>* get() {
        DataFormatBase* d = DataFormatBase::getNonConst(id());

        if (!d) {
            d = new DataFormat<T, B>();
            instance_[id()] = d;
        }

        return static_cast<DataFormat<T, B>*>(d);
    }

    static size_t bitsAllocated() { return B; }
    static size_t bitsStored() { return B; }
    static int components() { return comp; }
    static DataFormatEnums::NumericType numericType() { return DataFormatEnums::NOT_SPECIALIZED_TYPE; }

    static T max() { return std::numeric_limits<T>::max(); }
    static T min() { return std::numeric_limits<T>::min(); }

    static double maxToDouble() { return static_cast<double>(max()); }
    static double minToDouble() { return static_cast<double>(min()); }

    static std::string str() { return DataFormatBase::str(); }
    static DataFormatEnums::Id id() { return DataFormatBase::id(); }

    inline double valueToDouble(void* val) const { return DataFormatBase::valueToDouble(val); }
    inline dvec2 valueToVec2Double(void* val) const { return DataFormatBase::valueToVec2Double(val); }
    inline dvec3 valueToVec3Double(void* val) const { return DataFormatBase::valueToVec3Double(val); }
    inline dvec4 valueToVec4Double(void* val) const { return DataFormatBase::valueToVec4Double(val); }

    inline double valueToNormalizedDouble(void* val) const { return DataFormatBase::valueToNormalizedDouble(val); }
    inline dvec2 valueToNormalizedVec2Double(void* val) const { return DataFormatBase::valueToNormalizedVec2Double(val); }
    inline dvec3 valueToNormalizedVec3Double(void* val) const { return DataFormatBase::valueToNormalizedVec3Double(val); }
    inline dvec4 valueToNormalizedVec4Double(void* val) const { return DataFormatBase::valueToNormalizedVec4Double(val); }

    inline void doubleToValue(double val, void* loc) const { DataFormatBase::doubleToValue(val, loc); }
    inline void vec2DoubleToValue(dvec2 val, void* loc) const { DataFormatBase::vec2DoubleToValue(val, loc); }
    inline void vec3DoubleToValue(dvec3 val, void* loc) const { DataFormatBase::vec3DoubleToValue(val, loc); }
    inline void vec4DoubleToValue(dvec4 val, void* loc) const { DataFormatBase::vec4DoubleToValue(val, loc); }
};

template <typename T, size_t B>
class DataFormat<glm::detail::tvec2<T, glm::defaultp>, B> : public DataFormatBase {
public:
    typedef glm::detail::tvec2<T, glm::defaultp> type;
    typedef T primitive;
    static const size_t bits = B;
    static const size_t comp = 2;
    static const size_t bitscomp = B/2;

    DataFormat() : DataFormatBase(id(),
        bitsAllocated(),
        bitsStored(),
        components(),
        maxToDouble(),
        minToDouble(),
        numericType(),
        str()) {
    }

    virtual ~DataFormat() {}

    static const DataFormat<glm::detail::tvec2<T, glm::defaultp>, B>* get() {
        DataFormatBase* d = DataFormatBase::getNonConst(id());

        if (!d) {
            d = new DataFormat<glm::detail::tvec2<T, glm::defaultp>, B>();
            instance_[id()] = d;
        }

        return static_cast<DataFormat<glm::detail::tvec2<T, glm::defaultp>, B>*>(d);
    }

    static size_t bitsAllocated() { return B; }
    static size_t bitsStored() { return B; }
    static int components() { return comp; }
    static DataFormatEnums::NumericType numericType() { return DataFormat<T, bitscomp>::numericType(); }

    static glm::detail::tvec2<T, glm::defaultp> max() { return glm::detail::tvec2<T, glm::defaultp>(DataFormat<T, bitscomp>::max()); }
    static glm::detail::tvec2<T, glm::defaultp> min() { return glm::detail::tvec2<T, glm::defaultp>(DataFormat<T, bitscomp>::min()); }

    static double maxToDouble() { return static_cast<double>(max()); }
    static double minToDouble() { return static_cast<double>(min()); }

    static std::string str() { return "Vec2"+DataFormat<T, bitscomp>::str(); }
    static DataFormatEnums::Id id() { return DataFormat<T, bitscomp>::id(); }

    inline double valueToDouble(void* val) const { return DataFormat<T, bitscomp>::valueToDouble(val); }
    inline dvec2 valueToVec2Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec2Double(val); }
    inline dvec3 valueToVec3Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec3Double(val); }
    inline dvec4 valueToVec4Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec4Double(val); }

    inline double valueToNormalizedDouble(void* val) const { return 0.0; }
    inline dvec2 valueToNormalizedVec2Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec2Double(val); }
    inline dvec3 valueToNormalizedVec3Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec3Double(val); }
    inline dvec4 valueToNormalizedVec4Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec4Double(val); }

    inline void doubleToValue(double val, void* loc) const { DataFormat<T, bitscomp>::doubleToValue(val, loc); }
    inline void vec2DoubleToValue(dvec2 val, void* loc) const { DataFormat<T, bitscomp>::vec2DoubleToValue(val, loc); }
    inline void vec3DoubleToValue(dvec3 val, void* loc) const { DataFormat<T, bitscomp>::vec3DoubleToValue(val, loc); }
    inline void vec4DoubleToValue(dvec4 val, void* loc) const { DataFormat<T, bitscomp>::vec4DoubleToValue(val, loc); }
};

template <typename T, size_t B>
class DataFormat<glm::detail::tvec3<T, glm::defaultp>, B> : public DataFormatBase {
public:
    typedef glm::detail::tvec3<T, glm::defaultp> type;
    typedef T primitive;
    static const size_t bits = B;
    static const size_t comp = 3;
    static const size_t bitscomp = B/3;

    DataFormat() : DataFormatBase(id(),
        bitsAllocated(),
        bitsStored(),
        components(),
        maxToDouble(),
        minToDouble(),
        numericType(),
        str()) {
    }

    virtual ~DataFormat() {}

    static const DataFormat<glm::detail::tvec3<T, glm::defaultp>, B>* get() {
        DataFormatBase* d = DataFormatBase::getNonConst(id());

        if (!d) {
            d = new DataFormat<glm::detail::tvec3<T, glm::defaultp>, B>();
            instance_[id()] = d;
        }

        return static_cast<DataFormat<glm::detail::tvec3<T, glm::defaultp>, B>*>(d);
    }

    static size_t bitsAllocated() { return B; }
    static size_t bitsStored() { return B; }
    static int components() { return comp; }
    static DataFormatEnums::NumericType numericType() { return DataFormat<T, bitscomp>::numericType(); }

    static glm::detail::tvec3<T, glm::defaultp> max() { return glm::detail::tvec3<T, glm::defaultp>(DataFormat<T, bitscomp>::max()); }
    static glm::detail::tvec3<T, glm::defaultp> min() { return glm::detail::tvec3<T, glm::defaultp>(DataFormat<T, bitscomp>::min()); }

    static double maxToDouble() { return static_cast<double>(max()); }
    static double minToDouble() { return static_cast<double>(min()); }

    static std::string str() { return "Vec3"+DataFormat<T, bitscomp>::str(); }
    static DataFormatEnums::Id id() { return DataFormat<T, bitscomp>::id(); }

    inline double valueToDouble(void* val) const { return DataFormat<T, bitscomp>::valueToDouble(val); }
    inline dvec2 valueToVec2Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec2Double(val); }
    inline dvec3 valueToVec3Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec3Double(val); }
    inline dvec4 valueToVec4Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec4Double(val); }

    inline double valueToNormalizedDouble(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedDouble(val); }
    inline dvec2 valueToNormalizedVec2Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec2Double(val); }
    inline dvec3 valueToNormalizedVec3Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec3Double(val); }
    inline dvec4 valueToNormalizedVec4Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec4Double(val); }

    inline void doubleToValue(double val, void* loc) const { DataFormat<T, bitscomp>::doubleToValue(val, loc); }
    inline void vec2DoubleToValue(dvec2 val, void* loc) const { DataFormat<T, bitscomp>::vec2DoubleToValue(val, loc); }
    inline void vec3DoubleToValue(dvec3 val, void* loc) const { DataFormat<T, bitscomp>::vec3DoubleToValue(val, loc); }
    inline void vec4DoubleToValue(dvec4 val, void* loc) const { DataFormat<T, bitscomp>::vec4DoubleToValue(val, loc); }
};

template <typename T, size_t B>
class DataFormat<glm::detail::tvec4<T, glm::defaultp>, B> : public DataFormatBase {
public:
    typedef glm::detail::tvec4<T, glm::defaultp> type;
    typedef T primitive;
    static const size_t bits = B;
    static const size_t comp = 4;
    static const size_t bitscomp = B/4;

    DataFormat() : DataFormatBase(id(),
        bitsAllocated(),
        bitsStored(),
        components(),
        maxToDouble(),
        minToDouble(),
        numericType(),
        str()) {
    }

    virtual ~DataFormat() {}

    static const DataFormat<glm::detail::tvec4<T, glm::defaultp>, B>* get() {
        DataFormatBase* d = DataFormatBase::getNonConst(id());

        if (!d) {
            d = new DataFormat<glm::detail::tvec4<T, glm::defaultp>, B>();
            instance_[id()] = d;
        }

        return static_cast<DataFormat<glm::detail::tvec4<T, glm::defaultp>, B>*>(d);
    }

    static size_t bitsAllocated() { return B; }
    static size_t bitsStored() { return B; }
    static int components() { return comp; }
    static DataFormatEnums::NumericType numericType() { return DataFormat<T, bitscomp>::numericType(); }

    static glm::detail::tvec4<T, glm::defaultp> max() { return glm::detail::tvec4<T, glm::defaultp>(DataFormat<T, bitscomp>::max()); }
    static glm::detail::tvec4<T, glm::defaultp> min() { return glm::detail::tvec4<T, glm::defaultp>(DataFormat<T, bitscomp>::min()); }

    static double maxToDouble() { return static_cast<double>(max()); }
    static double minToDouble() { return static_cast<double>(min()); }

    static std::string str() { return "Vec4"+DataFormat<T, bitscomp>::str(); }
    static DataFormatEnums::Id id() { return DataFormat<T, bitscomp>::id(); }

    inline double valueToDouble(void* val) const { return DataFormat<T, bitscomp>::valueToDouble(val); }
    inline dvec2 valueToVec2Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec2Double(val); }
    inline dvec3 valueToVec3Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec3Double(val); }
    inline dvec4 valueToVec4Double(void* val) const { return DataFormat<T, bitscomp>::valueToVec4Double(val); }

    inline double valueToNormalizedDouble(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedDouble(val); }
    inline dvec2 valueToNormalizedVec2Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec2Double(val); }
    inline dvec3 valueToNormalizedVec3Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec3Double(val); }
    inline dvec4 valueToNormalizedVec4Double(void* val) const { return DataFormat<T, bitscomp>::valueToNormalizedVec4Double(val); }

    inline void doubleToValue(double val, void* loc) const { DataFormat<T, bitscomp>::doubleToValue(val, loc); }
    inline void vec2DoubleToValue(dvec2 val, void* loc) const { DataFormat<T, bitscomp>::vec2DoubleToValue(val, loc); }
    inline void vec3DoubleToValue(dvec3 val, void* loc) const { DataFormat<T, bitscomp>::vec3DoubleToValue(val, loc); }
    inline void vec4DoubleToValue(dvec4 val, void* loc) const { DataFormat<T, bitscomp>::vec4DoubleToValue(val, loc); }
};

#define GenericDataBits(T) BYTES_TO_BITS(sizeof(T))

#define GenericDataFormat(T) DataFormat<T, GenericDataBits(T)>

/*---------------Single Value Formats------------------*/

// Floats
#ifdef __unix
typedef DataFormat<glm::f32, 16>  DataFLOAT16;
#else
typedef GenericDataFormat(half_float::half) DataFLOAT16;
#endif
typedef GenericDataFormat(glm::f32) DataFLOAT32;
typedef GenericDataFormat(glm::f64) DataFLOAT64;

// Integers
typedef GenericDataFormat(glm::i8)   DataINT8;
typedef DataFormat<glm::i16, 12>     DataINT12;
typedef GenericDataFormat(glm::i16)  DataINT16;
typedef GenericDataFormat(glm::i32)  DataINT32;
typedef GenericDataFormat(glm::i64)  DataINT64;

// Unsigned Integers
typedef GenericDataFormat(glm::u8)   DataUINT8;
typedef DataFormat<glm::u16, 12>     DataUINT12;
typedef GenericDataFormat(glm::u16)  DataUINT16;
typedef GenericDataFormat(glm::u32)  DataUINT32;
typedef GenericDataFormat(glm::u64)  DataUINT64;

/*---------------Vec2 Formats--------------------*/

// Floats
#ifdef __unix
typedef DataFormat<glm::f32vec2, 32>  DataVec2FLOAT16;
#else
typedef glm::detail::tvec2<half_float::half, glm::defaultp> f16vec2;
typedef GenericDataFormat(f16vec2) DataVec2FLOAT16;
#endif
typedef GenericDataFormat(glm::f32vec2) DataVec2FLOAT32;
typedef GenericDataFormat(glm::f64vec2) DataVec2FLOAT64;

// Integers
typedef GenericDataFormat(glm::i8vec2)  DataVec2INT8;
typedef DataFormat<glm::i16vec2, 24>    DataVec2INT12;
typedef GenericDataFormat(glm::i16vec2) DataVec2INT16;
typedef GenericDataFormat(glm::i32vec2) DataVec2INT32;
typedef GenericDataFormat(glm::i64vec2) DataVec2INT64;

// Unsigned Integers
typedef GenericDataFormat(glm::u8vec2)  DataVec2UINT8;
typedef DataFormat<glm::u16vec2, 24>    DataVec2UINT12;
typedef GenericDataFormat(glm::u16vec2) DataVec2UINT16;
typedef GenericDataFormat(glm::u32vec2) DataVec2UINT32;
typedef GenericDataFormat(glm::u64vec2) DataVec2UINT64;

/*---------------Vec3 Formats--------------------*/

// Floats
#ifdef __unix
typedef DataFormat<glm::f32vec3, 48>  DataVec3FLOAT16;
#else
typedef glm::detail::tvec3<half_float::half, glm::defaultp> f16vec3;
typedef GenericDataFormat(f16vec3) DataVec3FLOAT16;
#endif
typedef GenericDataFormat(glm::f32vec3) DataVec3FLOAT32;
typedef GenericDataFormat(glm::f64vec3) DataVec3FLOAT64;

// Integers
typedef GenericDataFormat(glm::i8vec3)  DataVec3INT8;
typedef DataFormat<glm::i16vec3, 36>    DataVec3INT12;
typedef GenericDataFormat(glm::i16vec3) DataVec3INT16;
typedef GenericDataFormat(glm::i32vec3) DataVec3INT32;
typedef GenericDataFormat(glm::i64vec3) DataVec3INT64;

// Unsigned Integers
typedef GenericDataFormat(glm::u8vec3)  DataVec3UINT8;
typedef DataFormat<glm::u16vec3, 36>    DataVec3UINT12;
typedef GenericDataFormat(glm::u16vec3) DataVec3UINT16;
typedef GenericDataFormat(glm::u32vec3) DataVec3UINT32;
typedef GenericDataFormat(glm::u64vec3) DataVec3UINT64;

/*---------------Vec4 Value Formats------------------*/

// Floats
#ifdef __unix
typedef DataFormat<glm::f32vec4, 64>  DataVec4FLOAT16;
#else
typedef glm::detail::tvec4<half_float::half, glm::defaultp> f16vec4;
typedef GenericDataFormat(f16vec4) DataVec4FLOAT16;
#endif
typedef GenericDataFormat(glm::f32vec4) DataVec4FLOAT32;
typedef GenericDataFormat(glm::f64vec4) DataVec4FLOAT64;

// Integers
typedef GenericDataFormat(glm::i8vec4)  DataVec4INT8;
typedef DataFormat<glm::i16vec4, 48>    DataVec4INT12;
typedef GenericDataFormat(glm::i16vec4) DataVec4INT16;
typedef GenericDataFormat(glm::i32vec4) DataVec4INT32;
typedef GenericDataFormat(glm::i64vec4) DataVec4INT64;

// Unsigned Integers
typedef GenericDataFormat(glm::u8vec4)  DataVec4UINT8;
typedef DataFormat<glm::u16vec4, 48>    DataVec4UINT12;
typedef GenericDataFormat(glm::u16vec4) DataVec4UINT16;
typedef GenericDataFormat(glm::u32vec4) DataVec4UINT32;
typedef GenericDataFormat(glm::u64vec4) DataVec4UINT64;

/*--------------- Conversions------------------*/

template<typename T>
inline glm::detail::tvec2<T, glm::defaultp> singleToVec2(T val) {
    return glm::detail::tvec2<T, glm::defaultp>(val);
}

template<typename T>
inline glm::detail::tvec3<T, glm::defaultp> singleToVec3(T val) {
    return glm::detail::tvec3<T, glm::defaultp>(val);
}

template<typename T>
inline glm::detail::tvec4<T, glm::defaultp> singleToVec4(T val) {
    return glm::detail::tvec4<T, glm::defaultp>(val);
}

template<typename T>
glm::detail::tvec3<T, glm::defaultp> vec2ToVec3(glm::detail::tvec2<T, glm::defaultp> val) {
    glm::detail::tvec3<T, glm::defaultp> result = glm::detail::tvec3<T, glm::defaultp>(static_cast<T>(0));
    result.x = val.x;
    result.y = val.y;
    return result;
}

template<typename T>
glm::detail::tvec4<T, glm::defaultp> vec2ToVec4(glm::detail::tvec2<T, glm::defaultp> val) {
    glm::detail::tvec4<T, glm::defaultp> result = glm::detail::tvec4<T, glm::defaultp>(static_cast<T>(0));
    result.x = val.x;
    result.y = val.y;
    return result;
}

template<typename T>
inline glm::detail::tvec4<T, glm::defaultp> vec3ToVec4(glm::detail::tvec3<T, glm::defaultp> val) {
    glm::detail::tvec4<T, glm::defaultp> result = glm::detail::tvec4<T, glm::defaultp>(static_cast<T>(0));
    result.x = val.x;
    result.y = val.y;
    result.z = val.z;
    return result;
}

//typename D = dest, typename S = src

template<typename D, typename S, size_t B>
inline D normalizeSigned(S val) {
    return (static_cast<D>(val) - static_cast<D>(DataFormat<S, B>::min())) /
        (static_cast<D>(DataFormat<S, B>::max()) - static_cast<D>(DataFormat<S, B>::min()));
}

template<typename D, typename S, size_t B>
inline D normalizeUnsigned(S val) {
    return static_cast<D>(val) / static_cast<D>(DataFormat<S, B>::max());
}

template<typename D, typename S, size_t B>
inline D normalizeSignedSingle(void* val) {
    S valT = *static_cast<S*>(val);
    return normalizeSigned<D, S, B>(valT);
}

template<typename D, typename S, size_t B>
inline D normalizeUnsignedSingle(void* val) {
    S valT = *static_cast<S*>(val);
    return normalizeUnsigned<D, S, B>(valT);
}

template<typename D, typename S, size_t B>
inline glm::detail::tvec2<D, glm::defaultp> normalizeSignedVec2(void* val) {
    glm::detail::tvec2<S, glm::defaultp> valT = *static_cast<glm::detail::tvec2<S, glm::defaultp>*>(val);
    glm::detail::tvec2<D, glm::defaultp> result;
    result.x = normalizeSigned<D, S, B>(valT.x);
    result.y = normalizeSigned<D, S, B>(valT.y);
    return result;
}

template<typename D, typename S, size_t B>
inline glm::detail::tvec2<D, glm::defaultp> normalizeUnsignedVec2(void* val) {
    glm::detail::tvec2<S, glm::defaultp> valT = *static_cast<glm::detail::tvec2<S, glm::defaultp>*>(val);
    glm::detail::tvec2<D, glm::defaultp> result;
    result.x = normalizeUnsigned<D, S, B>(valT.x);
    result.y = normalizeUnsigned<D, S, B>(valT.y);
    return result;
}

template<typename D, typename S, size_t B>
inline glm::detail::tvec3<D, glm::defaultp> normalizeSignedVec3(void* val) {
    glm::detail::tvec3<S, glm::defaultp> valT = *static_cast<glm::detail::tvec3<S, glm::defaultp>*>(val);
    glm::detail::tvec3<D, glm::defaultp> result;
    result.x = normalizeSigned<D, S, B>(valT.x);
    result.y = normalizeSigned<D, S, B>(valT.y);
    result.y = normalizeSigned<D, S, B>(valT.y);
    return result;
}

template<typename D, typename S, size_t B>
inline glm::detail::tvec3<D, glm::defaultp> normalizeUnsignedVec3(void* val) {
    glm::detail::tvec3<S, glm::defaultp> valT = *static_cast<glm::detail::tvec3<S, glm::defaultp>*>(val);
    glm::detail::tvec3<D, glm::defaultp> result;
    result.x = normalizeUnsigned<D, S, B>(valT.x);
    result.y = normalizeUnsigned<D, S, B>(valT.y);
    result.z = normalizeUnsigned<D, S, B>(valT.z);
    return result;
}

template<typename D, typename S, size_t B>
inline glm::detail::tvec4<D, glm::defaultp> normalizeSignedVec4(void* val) {
    glm::detail::tvec4<S, glm::defaultp> valT = *static_cast<glm::detail::tvec4<S, glm::defaultp>*>(val);
    glm::detail::tvec4<D, glm::defaultp> result;
    result.x = normalizeSigned<D, S, B>(valT.x);
    result.y = normalizeSigned<D, S, B>(valT.y);
    result.y = normalizeSigned<D, S, B>(valT.y);
    result.w = normalizeSigned<D, S, B>(valT.w);
    return result;
}

template<typename D, typename S, size_t B>
inline glm::detail::tvec4<D, glm::defaultp> normalizeUnsignedVec4(void* val) {
    glm::detail::tvec4<S, glm::defaultp> valT = *static_cast<glm::detail::tvec4<S, glm::defaultp>*>(val);
    glm::detail::tvec4<D, glm::defaultp> result;
    result.x = normalizeUnsigned<D, S, B>(valT.x);
    result.y = normalizeUnsigned<D, S, B>(valT.y);
    result.z = normalizeUnsigned<D, S, B>(valT.z);
    result.w = normalizeUnsigned<D, S, B>(valT.w);
    return result;
}

/*---------------Single Value Formats------------------*/

// Bit Specializations
template<> inline size_t DataFLOAT16::bitsAllocated() { return DataFLOAT32::bitsAllocated(); }
template<> inline size_t DataINT12::bitsAllocated() { return DataINT16::bitsAllocated(); }
template<> inline size_t DataUINT12::bitsAllocated() { return DataUINT16::bitsAllocated(); }

// Min/Max Specializations
//template<> inline DataFLOAT16::type DataFLOAT16::max() { return DataFLOAT16::type(65504.f); }
//template<> inline DataFLOAT16::type DataFLOAT16::min() { return DataFLOAT16::type(1.f/16384.f); }

template<> inline DataINT12::type DataINT12::max() { return static_cast<DataINT12::type>(2047); }
template<> inline DataINT12::type DataINT12::min() { return static_cast<DataINT12::type>(-2048); }

template<> inline DataUINT12::type DataUINT12::max() { return static_cast<DataUINT12::type>(4095); }
template<> inline DataUINT12::type DataUINT12::min() { return static_cast<DataUINT12::type>(0); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataFLOAT16::id() { return DataFormatEnums::FLOAT16; }
template<> inline DataFormatEnums::Id DataFLOAT32::id() { return DataFormatEnums::FLOAT32; }
template<> inline DataFormatEnums::Id DataFLOAT64::id() { return DataFormatEnums::FLOAT64; }

template<> inline DataFormatEnums::Id DataINT8::id() { return DataFormatEnums::INT8; }
template<> inline DataFormatEnums::Id DataINT12::id() { return DataFormatEnums::INT12; }
template<> inline DataFormatEnums::Id DataINT16::id() { return DataFormatEnums::INT16; }
template<> inline DataFormatEnums::Id DataINT32::id() { return DataFormatEnums::INT32; }
template<> inline DataFormatEnums::Id DataINT64::id() { return DataFormatEnums::INT64; }

template<> inline DataFormatEnums::Id DataUINT8::id() { return DataFormatEnums::UINT8; }
template<> inline DataFormatEnums::Id DataUINT12::id() { return DataFormatEnums::UINT12; }
template<> inline DataFormatEnums::Id DataUINT16::id() { return DataFormatEnums::UINT16; }
template<> inline DataFormatEnums::Id DataUINT32::id() { return DataFormatEnums::UINT32; }
template<> inline DataFormatEnums::Id DataUINT64::id() { return DataFormatEnums::UINT64; }

// String Function Specializations
template<> inline std::string DataFLOAT16::str() { return "FLOAT16"; }
template<> inline std::string DataFLOAT32::str() { return "FLOAT32"; }
template<> inline std::string DataFLOAT64::str() { return "FLOAT64"; }

template<> inline std::string DataINT8::str() { return "INT8"; }
template<> inline std::string DataINT12::str() { return "INT12"; }
template<> inline std::string DataINT16::str() { return "INT16"; }
template<> inline std::string DataINT32::str() { return "INT32"; }
template<> inline std::string DataINT64::str() { return "INT64"; }

template<> inline std::string DataUINT8::str() { return "UINT8"; }
template<> inline std::string DataUINT12::str() { return "UINT12"; }
template<> inline std::string DataUINT16::str() { return "UINT16"; }
template<> inline std::string DataUINT32::str() { return "UINT32"; }
template<> inline std::string DataUINT64::str() { return "UINT64"; }

// Type Conversion Specializations
#define DataConvertSingle(F) \
        DataFromDouble(F) \
        DataToDouble(F)

#define DataFromDouble(F) \
    template<> inline void F::doubleToValue(double val, void* loc) const { *static_cast<F::type*>(loc) = static_cast<F::type>(val); } \
    template<> inline void F::vec2DoubleToValue(dvec2 val, void* loc) const { *static_cast<F::type*>(loc) = static_cast<F::type>(val.x); } \
    template<> inline void F::vec3DoubleToValue(dvec3 val, void* loc) const { *static_cast<F::type*>(loc) = static_cast<F::type>(val.x); } \
    template<> inline void F::vec4DoubleToValue(dvec4 val, void* loc) const { *static_cast<F::type*>(loc) = static_cast<F::type>(val.x); }

#define DataToDouble(F) \
    template<> inline double F::valueToDouble(void* val) const { return static_cast<double>(*static_cast<F::type*>(val)); } \
    template<> inline dvec2 F::valueToVec2Double(void* val) const { return singleToVec2<double>(static_cast<double>(*static_cast<F::type*>(val))); } \
    template<> inline dvec3 F::valueToVec3Double(void* val) const { return singleToVec3<double>(static_cast<double>(*static_cast<F::type*>(val))); } \
    template<> inline dvec4 F::valueToVec4Double(void* val) const { return singleToVec4<double>(static_cast<double>(*static_cast<F::type*>(val))); }

#define DataUnchanged(F) \
    template<> inline DataFormatEnums::NumericType F::numericType() { return DataFormatEnums::FLOAT_TYPE; } \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return static_cast<double>(*static_cast<F::type*>(val)); } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return singleToVec2<double>(static_cast<double>(*static_cast<F::type*>(val))); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return singleToVec3<double>(static_cast<double>(*static_cast<F::type*>(val))); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return singleToVec4<double>(static_cast<double>(*static_cast<F::type*>(val))); } \
    DataConvertSingle(F)

#define DataNormalizedSignedSingle(F) \
    template<> inline DataFormatEnums::NumericType F::numericType() { return DataFormatEnums::SIGNED_INTEGER_TYPE; } \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeSignedSingle<double, F::type, F::bits>(val); } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return singleToVec2<double>(normalizeSignedSingle<double, F::type, F::bits>(val)); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return singleToVec3<double>(normalizeSignedSingle<double, F::type, F::bits>(val)); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return singleToVec4<double>(normalizeSignedSingle<double, F::type, F::bits>(val)); } \
    DataConvertSingle(F)

#define DataNormalizedUnsignedSingle(F) \
    template<> inline DataFormatEnums::NumericType F::numericType() { return DataFormatEnums::UNSIGNED_INTEGER_TYPE; } \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeUnsignedSingle<double, F::type, F::bits>(val); } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return singleToVec2<double>(normalizeUnsignedSingle<double, F::type, F::bits>(val)); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return singleToVec3<double>(normalizeUnsignedSingle<double, F::type, F::bits>(val)); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return singleToVec4<double>(normalizeUnsignedSingle<double, F::type, F::bits>(val)); } \
    DataConvertSingle(F)

DataUnchanged(DataFLOAT16)
DataUnchanged(DataFLOAT32)
DataUnchanged(DataFLOAT64)

DataNormalizedSignedSingle(DataINT8)
DataNormalizedSignedSingle(DataINT12)
DataNormalizedSignedSingle(DataINT16)
DataNormalizedSignedSingle(DataINT32)
DataNormalizedSignedSingle(DataINT64)

DataNormalizedUnsignedSingle(DataUINT8)
DataNormalizedUnsignedSingle(DataUINT12)
DataNormalizedUnsignedSingle(DataUINT16)
DataNormalizedUnsignedSingle(DataUINT32)
DataNormalizedUnsignedSingle(DataUINT64)

/*---------------Vec2 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec2FLOAT16::bitsAllocated() { return DataVec2FLOAT32::bitsAllocated(); }
template<> inline size_t DataVec2INT12::bitsAllocated() { return DataVec2INT16::bitsAllocated(); }
template<> inline size_t DataVec2UINT12::bitsAllocated() { return DataVec2UINT16::bitsAllocated(); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataVec2FLOAT16::id() { return DataFormatEnums::Vec2FLOAT16; }
template<> inline DataFormatEnums::Id DataVec2FLOAT32::id() { return DataFormatEnums::Vec2FLOAT32; }
template<> inline DataFormatEnums::Id DataVec2FLOAT64::id() { return DataFormatEnums::Vec2FLOAT64; }

template<> inline DataFormatEnums::Id DataVec2INT8::id() { return DataFormatEnums::Vec2INT8; }
template<> inline DataFormatEnums::Id DataVec2INT12::id() { return DataFormatEnums::Vec2INT12; }
template<> inline DataFormatEnums::Id DataVec2INT16::id() { return DataFormatEnums::Vec2INT16; }
template<> inline DataFormatEnums::Id DataVec2INT32::id() { return DataFormatEnums::Vec2INT32; }
template<> inline DataFormatEnums::Id DataVec2INT64::id() { return DataFormatEnums::Vec2INT64; }

template<> inline DataFormatEnums::Id DataVec2UINT8::id() { return DataFormatEnums::Vec2UINT8; }
template<> inline DataFormatEnums::Id DataVec2UINT12::id() { return DataFormatEnums::Vec2UINT12; }
template<> inline DataFormatEnums::Id DataVec2UINT16::id() { return DataFormatEnums::Vec2UINT16; }
template<> inline DataFormatEnums::Id DataVec2UINT32::id() { return DataFormatEnums::Vec2UINT32; }
template<> inline DataFormatEnums::Id DataVec2UINT64::id() { return DataFormatEnums::Vec2UINT64; }

//Vec to Single
#define DataVecToSingle(F, G) \
    template<> inline double F::maxToDouble() { return max().x; } \
    template<> inline double F::minToDouble() { return min().x; }

// Type Conversion Specializations
#define DataConvertVec2(F, G) \
    DataVecToSingle(F, G) \
    DataFromVec2(F, G) \
    DataToVec2(F, G)

#define DataFromVec2(F, G) \
    template<> inline double F::valueToDouble(void* val) const { return static_cast<double>(static_cast<F::type*>(val)->x); } \
    template<> inline dvec2 F::valueToVec2Double(void* val) const { return dvec2(*static_cast<F::type*>(val)); } \
    template<> inline dvec3 F::valueToVec3Double(void* val) const { return vec2ToVec3<double>(dvec2(*static_cast<F::type*>(val))); } \
    template<> inline dvec4 F::valueToVec4Double(void* val) const { return vec2ToVec4<double>(dvec2(*static_cast<F::type*>(val))); }

#define DataToVec2(F, G) \
    template<> inline void F::doubleToValue(double val, void* loc) const { *static_cast<F::type*>(loc) = singleToVec2<G::type>(static_cast<G::type>(val)); } \
    template<> inline void F::vec2DoubleToValue(dvec2 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y)); } \
    template<> inline void F::vec3DoubleToValue(dvec3 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y)); } \
    template<> inline void F::vec4DoubleToValue(dvec4 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y)); }

#define DataUnchangedVec2(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return static_cast<double>(static_cast<F::type*>(val)->x); } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return dvec2(*static_cast<F::type*>(val)); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return vec2ToVec3<double>(dvec2(*static_cast<F::type*>(val))); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return vec2ToVec4<double>(dvec2(*static_cast<F::type*>(val))); } \
    DataConvertVec2(F, G)

#define DataNormalizedSignedVec2(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeSignedVec2<double, G::type, G::bits>(val).x; } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return normalizeSignedVec2<double, G::type, G::bits>(val); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return vec2ToVec3<double>(normalizeSignedVec2<double, G::type, G::bits>(val)); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return vec2ToVec4<double>(normalizeSignedVec2<double, G::type, G::bits>(val)); } \
    DataConvertVec2(F, G)

#define DataNormalizedUnsignedVec2(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeUnsignedVec2<double, G::type, G::bits>(val).x; } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return normalizeUnsignedVec2<double, G::type, G::bits>(val); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return vec2ToVec3<double>(normalizeUnsignedVec2<double, G::type, G::bits>(val)); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return vec2ToVec4<double>(normalizeUnsignedVec2<double, G::type, G::bits>(val)); } \
    DataConvertVec2(F, G)

DataUnchangedVec2(DataVec2FLOAT16, DataFLOAT16)
DataUnchangedVec2(DataVec2FLOAT32, DataFLOAT32)
DataUnchangedVec2(DataVec2FLOAT64, DataFLOAT64)

DataNormalizedSignedVec2(DataVec2INT8, DataINT8)
DataNormalizedSignedVec2(DataVec2INT12, DataINT12)
DataNormalizedSignedVec2(DataVec2INT16, DataINT16)
DataNormalizedSignedVec2(DataVec2INT32, DataINT32)
DataNormalizedSignedVec2(DataVec2INT64, DataINT64)

DataNormalizedUnsignedVec2(DataVec2UINT8, DataUINT8)
DataNormalizedUnsignedVec2(DataVec2UINT12, DataUINT12)
DataNormalizedUnsignedVec2(DataVec2UINT16, DataUINT16)
DataNormalizedUnsignedVec2(DataVec2UINT32, DataUINT32)
DataNormalizedUnsignedVec2(DataVec2UINT64, DataUINT64)

/*---------------Vec3 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec3FLOAT16::bitsAllocated() { return DataVec3FLOAT32::bitsAllocated(); }
template<> inline size_t DataVec3INT12::bitsAllocated() { return DataVec3INT16::bitsAllocated(); }
template<> inline size_t DataVec3UINT12::bitsAllocated() { return DataVec3UINT16::bitsAllocated(); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataVec3FLOAT16::id() { return DataFormatEnums::Vec3FLOAT16; }
template<> inline DataFormatEnums::Id DataVec3FLOAT32::id() { return DataFormatEnums::Vec3FLOAT32; }
template<> inline DataFormatEnums::Id DataVec3FLOAT64::id() { return DataFormatEnums::Vec3FLOAT64; }

template<> inline DataFormatEnums::Id DataVec3INT8::id() { return DataFormatEnums::Vec3INT8; }
template<> inline DataFormatEnums::Id DataVec3INT12::id() { return DataFormatEnums::Vec3INT12; }
template<> inline DataFormatEnums::Id DataVec3INT16::id() { return DataFormatEnums::Vec3INT16; }
template<> inline DataFormatEnums::Id DataVec3INT32::id() { return DataFormatEnums::Vec3INT32; }
template<> inline DataFormatEnums::Id DataVec3INT64::id() { return DataFormatEnums::Vec3INT64; }

template<> inline DataFormatEnums::Id DataVec3UINT8::id() { return DataFormatEnums::Vec3UINT8; }
template<> inline DataFormatEnums::Id DataVec3UINT12::id() { return DataFormatEnums::Vec3UINT12; }
template<> inline DataFormatEnums::Id DataVec3UINT16::id() { return DataFormatEnums::Vec3UINT16; }
template<> inline DataFormatEnums::Id DataVec3UINT32::id() { return DataFormatEnums::Vec3UINT32; }
template<> inline DataFormatEnums::Id DataVec3UINT64::id() { return DataFormatEnums::Vec3UINT64; }

// Type Conversion Specializations
#define DataConvertVec3(F, G) \
    DataVecToSingle(F, G) \
    DataFromVec3(F, G) \
    DataToVec3(F, G)

#define DataFromVec3(F, G) \
    template<> inline double F::valueToDouble(void* val) const { return static_cast<double>(static_cast<F::type*>(val)->x); } \
    template<> inline dvec2 F::valueToVec2Double(void* val) const { return dvec2(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y); } \
    template<> inline dvec3 F::valueToVec3Double(void* val) const { return dvec3(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y, static_cast<F::type*>(val)->z); } \
    template<> inline dvec4 F::valueToVec4Double(void* val) const { return vec3ToVec4<double>(dvec3(*static_cast<F::type*>(val))); }

#define DataToVec3(F, G) \
    template<> inline void F::doubleToValue(double val, void* loc) const { *static_cast<F::type*>(loc) = singleToVec3<G::type>(static_cast<G::type>(val)); } \
    template<> inline void F::vec2DoubleToValue(dvec2 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y), static_cast<G::type>(0)); } \
    template<> inline void F::vec3DoubleToValue(dvec3 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y), static_cast<G::type>(val.z)); } \
    template<> inline void F::vec4DoubleToValue(dvec4 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y), static_cast<G::type>(val.z)); }

#define DataUnchangedVec3(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return static_cast<double>(static_cast<F::type*>(val)->x); } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return dvec2(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return dvec3(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y, static_cast<F::type*>(val)->z); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return vec3ToVec4<double>(dvec3(*static_cast<F::type*>(val))); } \
    DataConvertVec3(F, G)

#define DataNormalizedSignedVec3(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeSignedVec3<double, G::type, G::bits>(val).x; } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return normalizeSignedVec3<double, G::type, G::bits>(val).xy(); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return normalizeSignedVec3<double, G::type, G::bits>(val); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return vec3ToVec4<double>(normalizeSignedVec3<double, G::type, G::bits>(val)); } \
    DataConvertVec3(F, G)

#define DataNormalizedUnsignedVec3(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeUnsignedVec3<double, G::type, G::bits>(val).x; } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return normalizeUnsignedVec3<double, G::type, G::bits>(val).xy(); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return normalizeUnsignedVec3<double, G::type, G::bits>(val); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return vec3ToVec4<double>(normalizeUnsignedVec3<double, G::type, G::bits>(val)); } \
    DataConvertVec3(F, G)

DataUnchangedVec3(DataVec3FLOAT16, DataFLOAT16)
DataUnchangedVec3(DataVec3FLOAT32, DataFLOAT32)
DataUnchangedVec3(DataVec3FLOAT64, DataFLOAT64)

DataNormalizedSignedVec3(DataVec3INT8, DataINT8)
DataNormalizedSignedVec3(DataVec3INT12, DataINT12)
DataNormalizedSignedVec3(DataVec3INT16, DataINT16)
DataNormalizedSignedVec3(DataVec3INT32, DataINT32)
DataNormalizedSignedVec3(DataVec3INT64, DataINT64)

DataNormalizedUnsignedVec3(DataVec3UINT8, DataUINT8)
DataNormalizedUnsignedVec3(DataVec3UINT12, DataUINT12)
DataNormalizedUnsignedVec3(DataVec3UINT16, DataUINT16)
DataNormalizedUnsignedVec3(DataVec3UINT32, DataUINT32)
DataNormalizedUnsignedVec3(DataVec3UINT64, DataUINT64)

/*---------------Vec4 Formats--------------------*/

// Bit Specializations
template<> inline size_t DataVec4FLOAT16::bitsAllocated() { return DataVec4FLOAT32::bitsAllocated(); }
template<> inline size_t DataVec4INT12::bitsAllocated() { return DataVec4INT16::bitsAllocated(); }
template<> inline size_t DataVec4UINT12::bitsAllocated() { return DataVec4UINT16::bitsAllocated(); }

// Type Function Specializations
template<> inline DataFormatEnums::Id DataVec4FLOAT16::id() { return DataFormatEnums::Vec4FLOAT16; }
template<> inline DataFormatEnums::Id DataVec4FLOAT32::id() { return DataFormatEnums::Vec4FLOAT32; }
template<> inline DataFormatEnums::Id DataVec4FLOAT64::id() { return DataFormatEnums::Vec4FLOAT64; }

template<> inline DataFormatEnums::Id DataVec4INT8::id() { return DataFormatEnums::Vec4INT8; }
template<> inline DataFormatEnums::Id DataVec4INT12::id() { return DataFormatEnums::Vec4INT12; }
template<> inline DataFormatEnums::Id DataVec4INT16::id() { return DataFormatEnums::Vec4INT16; }
template<> inline DataFormatEnums::Id DataVec4INT32::id() { return DataFormatEnums::Vec4INT32; }
template<> inline DataFormatEnums::Id DataVec4INT64::id() { return DataFormatEnums::Vec4INT64; }

template<> inline DataFormatEnums::Id DataVec4UINT8::id() { return DataFormatEnums::Vec4UINT8; }
template<> inline DataFormatEnums::Id DataVec4UINT12::id() { return DataFormatEnums::Vec4UINT12; }
template<> inline DataFormatEnums::Id DataVec4UINT16::id() { return DataFormatEnums::Vec4UINT16; }
template<> inline DataFormatEnums::Id DataVec4UINT32::id() { return DataFormatEnums::Vec4UINT32; }
template<> inline DataFormatEnums::Id DataVec4UINT64::id() { return DataFormatEnums::Vec4UINT64; }

// Type Conversion Specializations
#define DataConvertVec4(F, G) \
    DataVecToSingle(F, G) \
    DataFromVec4(F, G) \
    DataToVec4(F, G)

#define DataFromVec4(F, G) \
    template<> inline double F::valueToDouble(void* val) const { return static_cast<double>((*static_cast<F::type*>(val)).x); } \
    template<> inline dvec2 F::valueToVec2Double(void* val) const { return dvec2(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y); } \
    template<> inline dvec3 F::valueToVec3Double(void* val) const { return dvec3(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y, static_cast<F::type*>(val)->z); } \
    template<> inline dvec4 F::valueToVec4Double(void* val) const { return dvec4(*static_cast<F::type*>(val)); }

#define DataToVec4(F, G) \
    template<> inline void F::doubleToValue(double val, void* loc) const { *static_cast<F::type*>(loc) = singleToVec4<G::type>(static_cast<G::type>(val)); } \
    template<> inline void F::vec2DoubleToValue(dvec2 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y), static_cast<G::type>(0), static_cast<G::type>(1)); } \
    template<> inline void F::vec3DoubleToValue(dvec3 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y), static_cast<G::type>(val.z), static_cast<G::type>(1)); } \
    template<> inline void F::vec4DoubleToValue(dvec4 val, void* loc) const { *static_cast<F::type*>(loc) = F::type(static_cast<G::type>(val.x), static_cast<G::type>(val.y), static_cast<G::type>(val.z), static_cast<G::type>(val.w)); }

#define DataUnchangedVec4(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return static_cast<double>((*static_cast<F::type*>(val)).x); } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return dvec2(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return dvec3(static_cast<F::type*>(val)->x, static_cast<F::type*>(val)->y, static_cast<F::type*>(val)->z); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return dvec4(*static_cast<F::type*>(val)); } \
    DataConvertVec4(F, G)

#define DataNormalizedSignedVec4(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeSignedVec4<double, G::type, G::bits>(val).x; } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return normalizeSignedVec4<double, G::type, G::bits>(val).xy(); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return normalizeSignedVec4<double, G::type, G::bits>(val).xyz(); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return normalizeSignedVec4<double, G::type, G::bits>(val); } \
    DataConvertVec4(F, G)

#define DataNormalizedUnsignedVec4(F, G) \
    template<> inline double F::valueToNormalizedDouble(void* val) const { return normalizeUnsignedVec4<double, G::type, G::bits>(val).x; } \
    template<> inline dvec2 F::valueToNormalizedVec2Double(void* val) const { return normalizeUnsignedVec4<double, G::type, G::bits>(val).xy(); } \
    template<> inline dvec3 F::valueToNormalizedVec3Double(void* val) const { return normalizeUnsignedVec4<double, G::type, G::bits>(val).xyz(); } \
    template<> inline dvec4 F::valueToNormalizedVec4Double(void* val) const { return normalizeUnsignedVec4<double, G::type, G::bits>(val); } \
    DataConvertVec4(F, G)

DataUnchangedVec4(DataVec4FLOAT16, DataFLOAT16)
DataUnchangedVec4(DataVec4FLOAT32, DataFLOAT32)
DataUnchangedVec4(DataVec4FLOAT64, DataFLOAT64)

DataNormalizedSignedVec4(DataVec4INT8, DataINT8)
DataNormalizedSignedVec4(DataVec4INT12, DataINT12)
DataNormalizedSignedVec4(DataVec4INT16, DataINT16)
DataNormalizedSignedVec4(DataVec4INT32, DataINT32)
DataNormalizedSignedVec4(DataVec4INT64, DataINT64)

DataNormalizedUnsignedVec4(DataVec4UINT8, DataUINT8)
DataNormalizedUnsignedVec4(DataVec4UINT12, DataUINT12)
DataNormalizedUnsignedVec4(DataVec4UINT16, DataUINT16)
DataNormalizedUnsignedVec4(DataVec4UINT32, DataUINT32)
DataNormalizedUnsignedVec4(DataVec4UINT64, DataUINT64)


template<typename T>
struct Defaultvalues {};

#define DEFAULTVALUES(type, dim, name, val, min, max, inc) \
    template<> \
struct Defaultvalues<type> { \
public: \
    static type getVal() { return val; } \
    static type getMin() { return min; } \
    static type getMax() { return max; } \
    static type getInc() { return inc; } \
    static uvec2 getDim() { return dim; } \
    static std::string getName() { return name; } \
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


// BT is the vector base type, ie float, int,.. T is the GLM vector or float, int..
template <typename BT, typename T>
class glmwrapper {};

template <typename T>
class glmwrapper<T, glm::detail::tmat4x4<T, glm::defaultp> > {
public:
    static T getval(glm::detail::tmat4x4<T, glm::defaultp> mat, size_t const ind) {
        return mat[static_cast<int>(ind)/4][static_cast<int>(ind) % 4];
    }
    static glm::detail::tmat4x4<T, glm::defaultp> setval(glm::detail::tmat4x4<T, glm::defaultp> mat,
                                                          size_t const ind, T val) {
        mat[static_cast<int>(ind) / 4][static_cast<int>(ind) % 4] = val;
        return mat;
    }
};
template <typename T>
class glmwrapper<T, glm::detail::tmat3x3<T, glm::defaultp> > {
public:
    static T getval(glm::detail::tmat3x3<T, glm::defaultp> mat, size_t const ind) {
        return mat[static_cast<int>(ind) / 3][static_cast<int>(ind) % 3];
    }
    static glm::detail::tmat3x3<T, glm::defaultp> setval(glm::detail::tmat3x3<T, glm::defaultp> mat,
                                                          size_t const ind, T val) {
        mat[static_cast<int>(ind) / 3][static_cast<int>(ind) % 3] = val;
        return mat;
    }
};
template <typename T>
class glmwrapper<T, glm::detail::tmat2x2<T, glm::defaultp> > {
public:
    static T getval(glm::detail::tmat2x2<T, glm::defaultp> mat, size_t const ind) {
        return mat[static_cast<int>(ind) / 2][static_cast<int>(ind) % 2];
    }
    static glm::detail::tmat2x2<T, glm::defaultp> setval(glm::detail::tmat2x2<T, glm::defaultp> mat,
                                                          size_t const ind, T val) {
        mat[static_cast<int>(ind) / 2][static_cast<int>(ind) % 2] = val;
        return mat;
    }
};

template <typename T>
class glmwrapper<T, glm::detail::tvec4<T, glm::defaultp> > {
public:
    static T getval(glm::detail::tvec4<T, glm::defaultp> vec, size_t const ind) {
        return vec[static_cast<int>(ind)];
    }
    static glm::detail::tvec4<T, glm::defaultp> setval(glm::detail::tvec4<T, glm::defaultp> vec,
                                                       size_t const ind, T val) {
        vec[static_cast<int>(ind)] = val;
        return vec;
    }
};
template <typename T>
class glmwrapper<T, glm::detail::tvec3<T, glm::defaultp> > {
public:
    static T getval(glm::detail::tvec3<T, glm::defaultp> vec, size_t const ind) {
        return vec[static_cast<int>(ind)];
    }
    static glm::detail::tvec3<T, glm::defaultp> setval(glm::detail::tvec3<T, glm::defaultp> vec,
                                                       size_t const ind, T val) {
        vec[static_cast<int>(ind)] = val;
        return vec;
    }
};
template <typename T>
class glmwrapper<T, glm::detail::tvec2<T, glm::defaultp> > {
public:
    static T getval(glm::detail::tvec2<T, glm::defaultp> vec, size_t const ind) {
        return vec[static_cast<int>(ind)];
    }
    static glm::detail::tvec2<T, glm::defaultp> setval(glm::detail::tvec2<T, glm::defaultp> vec,
                                                       size_t const ind, T val) {
        vec[static_cast<int>(ind)] = val;
        return vec;
    }
};
template <typename T>
class glmwrapper<T, T> {
public:
    static T getval(T val, size_t const ind) {
        return val;
    }
    static T setval(T org, size_t const ind, T val) {
        return val;
    }
};

template <typename T, typename... Args>
auto DataFormatBase::dispatch(T& obj, Args&&... args) const -> typename T::type {
    switch (formatId_) {
#define DataFormatIdMacro(i) \
    case DataFormatEnums::i: \
        return obj.template dispatch<Data##i>(std::forward<Args>(args)...);
#include <inviwo/core/util/formatsdefinefunc.h>
#undef DataFormatIdMacro
        default:
            return nullptr;
    }
}

#define CallFunctionWithTemplateArgsForType(fun, id) \
    switch (id) {\
    case DataFormatEnums::FLOAT16: fun<DataFLOAT16::type, DataFLOAT16::bits>(); break; \
    case DataFormatEnums::FLOAT32: fun<DataFLOAT32::type, DataFLOAT32::bits>(); break; \
    case DataFormatEnums::FLOAT64: fun<DataFLOAT64::type, DataFLOAT64::bits>(); break; \
    case DataFormatEnums::INT8: fun<DataINT8::type, DataINT8::bits>(); break; \
    case DataFormatEnums::INT12: fun<DataINT12::type, DataINT12::bits>(); break; \
    case DataFormatEnums::INT16: fun<DataINT16::type, DataINT16::bits>(); break; \
    case DataFormatEnums::INT32: fun<DataINT32::type, DataINT32::bits>(); break; \
    case DataFormatEnums::INT64: fun<DataINT64::type, DataINT64::bits>(); break; \
    case DataFormatEnums::UINT8: fun<DataUINT8::type, DataUINT8::bits>(); break; \
    case DataFormatEnums::UINT12: fun<DataUINT12::type, DataUINT12::bits>(); break; \
    case DataFormatEnums::UINT16: fun<DataUINT16::type, DataUINT16::bits>(); break; \
    case DataFormatEnums::UINT32: fun<DataUINT32::type, DataUINT32::bits>(); break; \
    case DataFormatEnums::UINT64: fun<DataUINT64::type, DataUINT64::bits>(); break; \
    case DataFormatEnums::Vec2FLOAT16: fun<DataVec2FLOAT16::type, DataVec2FLOAT16::bits>(); break; \
    case DataFormatEnums::Vec2FLOAT32: fun<DataVec2FLOAT32::type, DataVec2FLOAT32::bits>(); break; \
    case DataFormatEnums::Vec2FLOAT64: fun<DataVec2FLOAT64::type, DataVec2FLOAT64::bits>(); break; \
    case DataFormatEnums::Vec2INT8: fun<DataVec2INT8::type, DataVec2INT8::bits>(); break; \
    case DataFormatEnums::Vec2INT12: fun<DataVec2INT12::type, DataVec2INT12::bits>(); break; \
    case DataFormatEnums::Vec2INT16: fun<DataVec2INT16::type, DataVec2INT16::bits>(); break; \
    case DataFormatEnums::Vec2INT32: fun<DataVec2INT32::type, DataVec2INT32::bits>(); break; \
    case DataFormatEnums::Vec2INT64: fun<DataVec2INT64::type, DataVec2INT64::bits>(); break; \
    case DataFormatEnums::Vec2UINT8: fun<DataVec2UINT8::type, DataVec2UINT8::bits>(); break; \
    case DataFormatEnums::Vec2UINT12: fun<DataVec2UINT12::type, DataVec2UINT12::bits>(); break; \
    case DataFormatEnums::Vec2UINT16: fun<DataVec2UINT16::type, DataVec2UINT16::bits>(); break; \
    case DataFormatEnums::Vec2UINT32: fun<DataVec2UINT32::type, DataVec2UINT32::bits>(); break; \
    case DataFormatEnums::Vec2UINT64: fun<DataVec2UINT64::type, DataVec2UINT64::bits>(); break; \
    case DataFormatEnums::Vec3FLOAT16: fun<DataVec3FLOAT16::type, DataVec3FLOAT16::bits>(); break; \
    case DataFormatEnums::Vec3FLOAT32: fun<DataVec3FLOAT32::type, DataVec3FLOAT32::bits>(); break; \
    case DataFormatEnums::Vec3FLOAT64: fun<DataVec3FLOAT64::type, DataVec3FLOAT64::bits>(); break; \
    case DataFormatEnums::Vec3INT8: fun<DataVec3INT8::type, DataVec3INT8::bits>(); break; \
    case DataFormatEnums::Vec3INT12: fun<DataVec3INT12::type, DataVec3INT12::bits>(); break; \
    case DataFormatEnums::Vec3INT16: fun<DataVec3INT16::type, DataVec3INT16::bits>(); break; \
    case DataFormatEnums::Vec3INT32: fun<DataVec3INT32::type, DataVec3INT32::bits>(); break; \
    case DataFormatEnums::Vec3INT64: fun<DataVec3INT64::type, DataVec3INT64::bits>(); break; \
    case DataFormatEnums::Vec3UINT8: fun<DataVec3UINT8::type, DataVec3UINT8::bits>(); break; \
    case DataFormatEnums::Vec3UINT12: fun<DataVec3UINT12::type, DataVec3UINT12::bits>(); break; \
    case DataFormatEnums::Vec3UINT16: fun<DataVec3UINT16::type, DataVec3UINT16::bits>(); break; \
    case DataFormatEnums::Vec3UINT32: fun<DataVec3UINT32::type, DataVec3UINT32::bits>(); break; \
    case DataFormatEnums::Vec3UINT64: fun<DataVec3UINT64::type, DataVec3UINT64::bits>(); break; \
    case DataFormatEnums::Vec4FLOAT16: fun<DataVec4FLOAT16::type, DataVec4FLOAT16::bits>(); break; \
    case DataFormatEnums::Vec4FLOAT32: fun<DataVec4FLOAT32::type, DataVec4FLOAT32::bits>(); break; \
    case DataFormatEnums::Vec4FLOAT64: fun<DataVec4FLOAT64::type, DataVec4FLOAT64::bits>(); break; \
    case DataFormatEnums::Vec4INT8: fun<DataVec4INT8::type, DataVec4INT8::bits>(); break; \
    case DataFormatEnums::Vec4INT12: fun<DataVec4INT12::type, DataVec4INT12::bits>(); break; \
    case DataFormatEnums::Vec4INT16: fun<DataVec4INT16::type, DataVec4INT16::bits>(); break; \
    case DataFormatEnums::Vec4INT32: fun<DataVec4INT32::type, DataVec4INT32::bits>(); break; \
    case DataFormatEnums::Vec4INT64: fun<DataVec4INT64::type, DataVec4INT64::bits>(); break; \
    case DataFormatEnums::Vec4UINT8: fun<DataVec4UINT8::type, DataVec4UINT8::bits>(); break; \
    case DataFormatEnums::Vec4UINT12: fun<DataVec4UINT12::type, DataVec4UINT12::bits>(); break; \
    case DataFormatEnums::Vec4UINT16: fun<DataVec4UINT16::type, DataVec4UINT16::bits>(); break; \
    case DataFormatEnums::Vec4UINT32: fun<DataVec4UINT32::type, DataVec4UINT32::bits>(); break; \
    case DataFormatEnums::Vec4UINT64: fun<DataVec4UINT64::type, DataVec4UINT64::bits>(); break; \
    default: break; \
}


}

#endif
