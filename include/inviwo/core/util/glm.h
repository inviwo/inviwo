/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_GLM_H
#define IVW_GLM_H

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_SWIZZLE
#define GLM_SWIZZLE
#endif
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/common.hpp>
#include <glm/detail/precision.hpp>
#include <glm/gtx/io.hpp>

#include <type_traits>

namespace inviwo {

typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::dvec2 dvec2;
typedef glm::dvec3 dvec3;
typedef glm::dvec4 dvec4;
typedef glm::bvec2 bvec2;
typedef glm::bvec3 bvec3;
typedef glm::bvec4 bvec4;
typedef glm::uvec2 uvec2;
typedef glm::uvec3 uvec3;
typedef glm::uvec4 uvec4;
typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::dmat2 dmat2;
typedef glm::dmat3 dmat3;
typedef glm::dmat4 dmat4;
typedef glm::quat quat;



namespace util {

template<class U, class T, class BinaryOperation>
U accumulate(T x, U init, BinaryOperation op) {
    init = op(init, x);
    return init;
}

template <class U, glm::precision P, template <typename, glm::precision> class vecType,
          class BinaryOperation>
typename std::enable_if<
            std::is_same< typename std::decay<vecType<U, P>>::type ,
                          glm::detail::tvec2<U, P>>::value ||
            std::is_same< typename std::decay<vecType<U, P>>::type ,
                          glm::detail::tvec3<U, P>>::value ||
            std::is_same< typename std::decay<vecType<U, P>>::type ,
                          glm::detail::tvec4<U, P>>::value, U >::type
    accumulate(vecType<U, P> const& x, U init, BinaryOperation op) {
    for (int i = 0; i < x.length(); ++i) init = op(init, x[i]);

    return init;
}

template <class U, glm::precision P, template <typename, glm::precision> class vecType,
          class BinaryOperation>
typename std::enable_if<
            std::is_same< typename std::decay<vecType<U, P>>::type ,
                          glm::detail::tmat2x2<U, P>>::value ||
            std::is_same< typename std::decay<vecType<U, P>>::type ,
                          glm::detail::tmat3x3<U, P>>::value ||
            std::is_same< typename std::decay<vecType<U, P>>::type ,
                          glm::detail::tmat4x4<U, P>>::value, U >::type
    accumulate(vecType<U, P> const& x, U init, BinaryOperation op) {
    for (int i = 0; i < x.length(); ++i)
        for (int j = 0; j< x[i].length(); ++j)
            init = op(init, x[i][j]);

    return init;
}


} // namespace util


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




} // namespace

namespace glm {


#define VECTORIZE2_MAT(func)                                                        \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat2x2<T, P> func(detail::tmat2x2<T, P> const& x) { \
        return detail::tmat2x2<T, P>(                                               \
            func(x[0][0]), func(x[1][0]),                                           \
            func(x[0][1]), func(x[1][1])                                            \
            );                                                                      \
    }

#define VECTORIZE3_MAT(func)                                                          \
    template <typename T, precision P>                                                \
    GLM_FUNC_QUALIFIER detail::tmat3x3<T, P> func(detail::tmat3x3<T, P> const& x) {   \
        return detail::tmat3x3<T, P>(                                                 \
            func(x[0][0]), func(x[1][0]), func(x[2][0]),                              \
            func(x[0][1]), func(x[1][1]), func(x[2][1]),                              \
            func(x[0][2]), func(x[1][2]), func(x[2][2])                               \
            );                                                                        \
    }

#define VECTORIZE4_MAT(func)                                                        \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat4x4<T, P> func(detail::tmat4x4<T, P> const& x) { \
        return detail::tmat4x4<T, P>(                                               \
            func(x[0][0]), func(x[1][0]), func(x[2][0]), func(x[3][0]),             \
            func(x[0][1]), func(x[1][1]), func(x[2][1]), func(x[3][1]),             \
            func(x[0][2]), func(x[1][2]), func(x[2][2]), func(x[3][2]),             \
            func(x[0][3]), func(x[1][3]), func(x[2][3]), func(x[3][3])              \
            );                                                                      \
    }


#define VECTORIZE_MAT(func) \
    VECTORIZE2_MAT(func)    \
    VECTORIZE3_MAT(func)    \
    VECTORIZE4_MAT(func)


#define VECTORIZE2_MAT_SCA(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat2x2<T, P> func(detail::tmat2x2<T, P> const& x,   \
                                                  T const& y) {                     \
        return detail::tmat2x2<T, P>(                                               \
            func(x[0][0], y), func(x[1][0], y),                                     \
            func(x[0][1], y), func(x[1][1], y)                                      \
            );                                                                      \
    }

#define VECTORIZE3_MAT_SCA(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat3x3<T, P> func(detail::tmat3x3<T, P> const& x,   \
                                                  T const& y) {                     \
        return detail::tmat3x3<T, P>(                                               \
            func(x[0][0], y), func(x[1][0], y), func(x[2][0], y),                   \
            func(x[0][1], y), func(x[1][1], y), func(x[2][1], y),                   \
            func(x[0][2], y), func(x[1][2], y), func(x[2][2], y)                    \
            );                                                                      \
    }

#define VECTORIZE4_MAT_SCA(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat4x4<T, P> func(detail::tmat4x4<T, P> const& x,   \
                                                  T const& y) {                     \
        return detail::tmat4x4<T, P>(                                               \
            func(x[0][0], y), func(x[1][0], y), func(x[2][0], y), func(x[3][0], y), \
            func(x[0][1], y), func(x[1][1], y), func(x[2][1], y), func(x[3][1], y), \
            func(x[0][2], y), func(x[1][2], y), func(x[2][2], y), func(x[3][2], y), \
            func(x[0][3], y), func(x[1][3], y), func(x[2][3], y), func(x[3][3], y)  \
            );                                                                      \
    }

#define VECTORIZE_MAT_SCA(func) \
    VECTORIZE2_MAT_SCA(func)    \
    VECTORIZE3_MAT_SCA(func)    \
    VECTORIZE4_MAT_SCA(func)


#define VECTORIZE2_MAT_MAT(func)                                                    \
    template <typename T, precision P>                                              \
    GLM_FUNC_QUALIFIER detail::tmat2x2<T, P> func(detail::tmat2x2<T, P> const& x,   \
                                                  detail::tmat2x2<T, P> const& y) { \
        return detail::tmat2x2<T, P>(                                               \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]),                         \
            func(x[0][1], y[0][1]), func(x[1][1], y[1][1])                          \
            );                                                                      \
    }

#define VECTORIZE3_MAT_MAT(func)                                                      \
    template <typename T, precision P>                                                \
    GLM_FUNC_QUALIFIER detail::tmat3x3<T, P> func(detail::tmat3x3<T, P> const& x,     \
                                                  detail::tmat3x3<T, P> const& y) {   \
        return detail::tmat3x3<T, P>(                                                 \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]), func(x[2][0], y[2][0]),   \
            func(x[0][1], y[0][1]), func(x[1][1], y[1][1]), func(x[2][1], y[2][1]),   \
            func(x[0][2], y[0][2]), func(x[1][2], y[1][2]), func(x[2][2], y[2][2])    \
            );                                                                        \
    }

#define VECTORIZE4_MAT_MAT(func)                                                                            \
    template <typename T, precision P>                                                                      \
    GLM_FUNC_QUALIFIER detail::tmat4x4<T, P> func(detail::tmat4x4<T, P> const& x,                           \
                                                  detail::tmat4x4<T, P> const& y) {                         \
        return detail::tmat4x4<T, P>(                                                                       \
            func(x[0][0], y[0][0]), func(x[1][0], y[1][0]), func(x[2][0], y[2][0]), func(x[3][0], y[3][0]), \
            func(x[0][1], y[0][1]), func(x[1][1], y[1][1]), func(x[2][1], y[2][1]), func(x[3][1], y[3][1]), \
            func(x[0][2], y[0][2]), func(x[1][2], y[1][2]), func(x[2][2], y[2][2]), func(x[3][2], y[3][2]), \
            func(x[0][3], y[0][3]), func(x[1][3], y[1][3]), func(x[2][3], y[2][3]), func(x[3][3], y[3][3])  \
            );                                                                                              \
    }

#define VECTORIZE_MAT_MAT(func) \
    VECTORIZE2_MAT_MAT(func)    \
    VECTORIZE3_MAT_MAT(func)    \
    VECTORIZE4_MAT_MAT(func)


VECTORIZE_MAT(abs)
VECTORIZE_MAT(sign)
VECTORIZE_MAT(floor)
VECTORIZE_MAT(trunc)
VECTORIZE_MAT(round)
VECTORIZE_MAT(roundEven)
VECTORIZE_MAT(ceil)
VECTORIZE_MAT(fract)

VECTORIZE_MAT_SCA(mod)
VECTORIZE_MAT_SCA(min)
VECTORIZE_MAT_SCA(max)

VECTORIZE_MAT_MAT(min)
VECTORIZE_MAT_MAT(max)
VECTORIZE_MAT_MAT(mod)

} // namespace

#endif // IVW_GLM_H

