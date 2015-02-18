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

