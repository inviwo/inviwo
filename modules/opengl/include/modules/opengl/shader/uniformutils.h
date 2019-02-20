/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_UNIFORMUTILS_H
#define IVW_UNIFORMUTILS_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>

#include <array>
#include <vector>
#include <typeinfo>

namespace inviwo {

namespace utilgl {

template <typename T>
struct dependent_false : std::false_type {};

template <typename T>
struct UniformSetter {
    static void set(...) {
        static_assert(dependent_false<T>::value,
                      "Unsupported type T. Missing specialization for setUniform (UniformSetter)");
    }
};

// scalar value specializations
template <>
struct UniformSetter<GLfloat> {
    static void set(GLint loc, GLsizei count, const GLfloat* ptr) { glUniform1fv(loc, count, ptr); }
    static void set(GLint loc, const GLfloat& value) { glUniform1f(loc, value); }
};

template <>
struct UniformSetter<GLint> {
    static void set(GLint loc, GLsizei count, const GLint* ptr) { glUniform1iv(loc, count, ptr); }
    static void set(GLint loc, const GLint& value) { glUniform1i(loc, value); }
};

template <>
struct UniformSetter<GLuint> {
    static void set(GLint loc, GLsizei count, const GLuint* ptr) { glUniform1uiv(loc, count, ptr); }
    static void set(GLint loc, const GLuint& value) { glUniform1ui(loc, value); }
};

// signed int vector specializations
template <>
struct UniformSetter<ivec2> {
    static void set(GLint loc, GLsizei count, const ivec2* ptr) {
        glUniform2iv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const ivec2& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<ivec3> {
    static void set(GLint loc, GLsizei count, const ivec3* ptr) {
        glUniform3iv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const ivec3& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<ivec4> {
    static void set(GLint loc, GLsizei count, const ivec4* ptr) {
        glUniform4iv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const ivec4& value) { set(loc, 1, &value); }
};

// unsigned int vector specializations
template <>
struct UniformSetter<uvec2> {
    static void set(GLint loc, GLsizei count, const uvec2* ptr) {
        glUniform2uiv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const uvec2& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<uvec3> {
    static void set(GLint loc, GLsizei count, const uvec3* ptr) {
        glUniform3uiv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const uvec3& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<uvec4> {
    static void set(GLint loc, GLsizei count, const uvec4* ptr) {
        glUniform4uiv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const uvec4& value) { set(loc, 1, &value); }
};

// float vector specializations
template <>
struct UniformSetter<vec2> {
    static void set(GLint loc, GLsizei count, const vec2* ptr) {
        glUniform2fv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const vec2& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<vec3> {
    static void set(GLint loc, GLsizei count, const vec3* ptr) {
        glUniform3fv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const vec3& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<vec4> {
    static void set(GLint loc, GLsizei count, const vec4* ptr) {
        glUniform4fv(loc, count, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const vec4& value) { set(loc, 1, &value); }
};

// matrix specializations
template <>
struct UniformSetter<mat2> {
    static void set(GLint loc, GLsizei count, const mat2* ptr) {
        glUniformMatrix2fv(loc, count, GL_FALSE, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const mat2& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<mat3> {
    static void set(GLint loc, GLsizei count, const mat3* ptr) {
        glUniformMatrix3fv(loc, count, GL_FALSE, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const mat3& value) { set(loc, 1, &value); }
};

template <>
struct UniformSetter<mat4> {
    static void set(GLint loc, GLsizei count, const mat4* ptr) {
        glUniformMatrix4fv(loc, count, GL_FALSE, glm::value_ptr(*ptr));
    }
    static void set(GLint loc, const mat4& value) { set(loc, 1, &value); }
};

// specializations for std::vector and std::array for multiple values
template <typename T>
struct UniformSetter<std::vector<T>> {
    static void set(GLint loc, const std::vector<T>& data) {
        UniformSetter<T>::set(loc, static_cast<GLsizei>(data.size()), data.data());
    }
};

template <typename T, std::size_t N>
struct UniformSetter<std::array<T, N>> {
    static void set(GLint loc, const std::array<T, N>& data) {
        UniformSetter<T>::set(loc, N, data.data());
    }
};

// specializations for template properties
template <typename T>
struct UniformSetter<OrdinalProperty<T>> {
    static void set(GLint loc, const OrdinalProperty<T>& property) {
        UniformSetter<T>::set(loc, property.get());
    }
};

// bool specializations
template <>
struct UniformSetter<bool> {
    static void set(GLint loc, GLsizei count, const bool* ptr) {
        std::vector<int> values;
        values.resize(count);
        for (GLsizei i = 0; i < count; ++i) {
            values[i] = static_cast<int>(ptr[i]);
        };
        glUniform1iv(loc, count, values.data());
    }
    static void set(GLint loc, const bool& value) { glUniform1i(loc, static_cast<int>(value)); }
};
template <>
struct UniformSetter<bvec2> {
    static void set(GLint loc, const bvec2& value) {
        ivec2 v(value);
        glUniform2iv(loc, 1, glm::value_ptr(v));
    }
};
template <>
struct UniformSetter<bvec3> {
    static void set(GLint loc, const bvec3& value) {
        ivec3 v(value);
        glUniform3iv(loc, 1, glm::value_ptr(v));
    }
};
template <>
struct UniformSetter<bvec4> {
    static void set(GLint loc, const bvec4& value) {
        ivec4 v(value);
        glUniform4iv(loc, 1, glm::value_ptr(v));
    }
};

template <std::size_t N>
struct UniformSetter<std::array<bool, N>> {
    static void set(GLint loc, const std::array<bool, N>& data) {
        std::array<int, N> values;
        for (std::size_t i = 0; i < N; ++i) {
            values[i] = static_cast<int>(data[i]);
        };
        glUniform1iv(loc, N, values.data());
    }
};

template <>
struct UniformSetter<BoolProperty> {
    static void set(GLint loc, const BoolProperty& property) {
        UniformSetter<bool>::set(loc, property.get());
    }
};

// specialization for TextureUnit
template <>
struct UniformSetter<TextureUnit> {
    static void set(GLint loc, const TextureUnit& texUnit) {
        glUniform1i(loc, texUnit.getUnitNumber());
    }
};

}  // namespace utilgl

}  // namespace inviwo

#endif  // IVW_UNIFORMUTILS_H
