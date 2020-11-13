/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/util/detected.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/stringconversion.h>

#include <span>
#include <optional>
#include <tuple>
#include <utility>

namespace inviwo {

struct IVW_MODULE_OPENGL_API TextureNumberCounter {
    GLint number = 0;
    constexpr GLint next() { return number++; }
};
struct IVW_MODULE_OPENGL_API TextureNumber {
    constexpr explicit TextureNumber(TextureNumberCounter& counter) : number{counter.next()} {}
    constexpr explicit TextureNumber(GLint num) : number{num} {}
    GLint number;
    constexpr GLint unit() const { return GL_TEXTURE0 + number; }
};

template <size_t N>
struct TextureNumbers {
    TextureNumbers(TextureNumberCounter& counter)
        : nums{util::make_array<N>([&](size_t) { return TextureNumber(counter); })} {}
    constexpr const TextureNumber& operator[](size_t i) { return nums[i]; }

    constexpr const TextureNumber& color() { return nums[0]; }
    constexpr const TextureNumber& depth() { return nums[1]; }
    constexpr const TextureNumber& picking() { return nums[2]; }

private:
    std::array<TextureNumber, N> nums;
};

namespace uniform {

// clang-format off
inline void set(GLint loc, int val)                { glUniform1i(loc, val); }
inline void set(GLint loc, unsigned int val)       { glUniform1ui(loc, val); }
inline void set(GLint loc, float val)              { glUniform1f(loc, val); }
inline void set(GLint loc, double val)             { glUniform1d(loc, val); }

inline void set(GLint loc, const glm::ivec2& val)  { glUniform2iv(loc,  1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::ivec3& val)  { glUniform3iv(loc,  1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::ivec4& val)  { glUniform4iv(loc,  1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::uvec2& val)  { glUniform2uiv(loc, 1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::uvec3& val)  { glUniform3uiv(loc, 1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::uvec4& val)  { glUniform4uiv(loc, 1, glm::value_ptr(val)); }

inline void set(GLint loc, const glm::vec2& val)   { glUniform2fv(loc, 1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::vec3& val)   { glUniform3fv(loc, 1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::vec4& val)   { glUniform4fv(loc, 1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::dvec2& val)  { glUniform2dv(loc, 1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::dvec3& val)  { glUniform3dv(loc, 1, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::dvec4& val)  { glUniform4dv(loc, 1, glm::value_ptr(val)); }

inline void set(GLint loc, const glm::mat2& val)   { glUniformMatrix2fv(loc, 1, GL_FALSE, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::mat3& val)   { glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::mat4& val)   { glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::dmat2& val)  { glUniformMatrix2dv(loc, 1, GL_FALSE, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::dmat3& val)  { glUniformMatrix3dv(loc, 1, GL_FALSE, glm::value_ptr(val)); }
inline void set(GLint loc, const glm::dmat4& val)  { glUniformMatrix4dv(loc, 1, GL_FALSE, glm::value_ptr(val)); }

inline void set(GLint loc, std::span<const int> val)          { glUniform1iv(loc,  static_cast<GLsizei>(val.size()), val.data()); }
inline void set(GLint loc, std::span<const unsigned int> val) { glUniform1uiv(loc, static_cast<GLsizei>(val.size()), val.data()); }
inline void set(GLint loc, std::span<const float> val)        { glUniform1fv(loc,  static_cast<GLsizei>(val.size()), val.data()); }
inline void set(GLint loc, std::span<const double> val)       { glUniform1dv(loc,  static_cast<GLsizei>(val.size()), val.data()); }

inline void set(GLint loc, std::span<const glm::ivec2> val)   { glUniform2iv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::ivec3> val)   { glUniform3iv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::ivec4> val)   { glUniform4iv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::uvec2> val)   { glUniform2uiv(loc, static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::uvec3> val)   { glUniform3uiv(loc, static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::uvec4> val)   { glUniform4uiv(loc, static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }

inline void set(GLint loc, std::span<const glm::vec2> val)    { glUniform2fv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::vec3> val)    { glUniform3fv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::vec4> val)    { glUniform4fv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::dvec2> val)   { glUniform2dv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::dvec3> val)   { glUniform3dv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::dvec4> val)   { glUniform4dv(loc,  static_cast<GLsizei>(val.size()), glm::value_ptr(val[0])); }

inline void set(GLint loc, std::span<const glm::mat2> val)    { glUniformMatrix2fv(loc,  static_cast<GLsizei>(val.size()), GL_FALSE, glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::mat3> val)    { glUniformMatrix3fv(loc,  static_cast<GLsizei>(val.size()), GL_FALSE, glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::mat4> val)    { glUniformMatrix4fv(loc,  static_cast<GLsizei>(val.size()), GL_FALSE, glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::dmat2> val)   { glUniformMatrix2dv(loc,  static_cast<GLsizei>(val.size()), GL_FALSE, glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::dmat3> val)   { glUniformMatrix3dv(loc,  static_cast<GLsizei>(val.size()), GL_FALSE, glm::value_ptr(val[0])); }
inline void set(GLint loc, std::span<const glm::dmat4> val)   { glUniformMatrix4dv(loc,  static_cast<GLsizei>(val.size()), GL_FALSE, glm::value_ptr(val[0])); }

inline void set(GLint loc, const TextureNumber& val) { glUniform1i(loc, val.number); }
// clang-format on

namespace detail {

template <typename T>
constexpr bool alwaysFalse() {
    return false;
}

template <typename T>
using StructFormat = decltype(T::structFormat);

template <typename T>
using UniformTuple = decltype(T::uniforms);

template <typename T>
using UniformGetter = decltype(T::getter);

template <typename T>
using UniformSetter = decltype(::inviwo::uniform::set(std::declval<GLint>(), std::declval<T>()));

template <class T>
constexpr auto HasStructFormat = ::inviwo::util::is_detected<StructFormat, T>::value;

template <class T>
constexpr auto HasUniformTuple = ::inviwo::util::is_detected<UniformTuple, T>::value;

template <class T>
constexpr auto HasUniformGetter = ::inviwo::util::is_detected<UniformGetter, T>::value;

template <class T>
constexpr auto HasUniformSetter = ::inviwo::util::is_detected<UniformSetter, T>::value;

}  // namespace detail

static_assert(detail::HasUniformSetter<bool>, "");

template <typename T>
struct UniformTraits {};

template <typename T>
class UniformManager {
public:
    void bind(Shader& shader, std::string_view structName) {
        bindings.clear();

        [[maybe_unused]] std::string name{};
        if constexpr (detail::HasStructFormat<UniformTraits<T>>) {
            name = fmt::format(UniformTraits<T>::structFormat, structName);
            structName = name;
        }

        if constexpr (detail::HasUniformTuple<UniformTraits<T>>) {
            std::apply(
                [&](auto&&... items) {
                    StrBuffer buff;
                    (bind(shader, buff.replace("{}.{}", structName, std::get<0>(items)).c_str(),
                          std::forward<decltype(std::get<1>(items))>(std::get<1>(items))),
                     ...);
                },
                UniformTraits<T>::uniforms);
        } else if constexpr (detail::HasUniformGetter<UniformTraits<T>>) {
            StrBuffer buff;
            bind(shader, buff.replace("{}", structName).c_str(), UniformTraits<T>::getter);
        } else if constexpr (detail::HasUniformSetter<T>) {
            StrBuffer buff;
            auto cstr = buff.replace("{}", structName).c_str();
            if (GLint location = glGetUniformLocation(shader.getID(), cstr); location != -1) {
                bindings.push_back([location](const T& item) { uniform::set(location, item); });
            }
        } else {
            static_assert(detail::alwaysFalse<T>(),
                          "Missing specialization of UniformTraits for T. "
                          "Did you forget to include the header with the specialization?");
        }
    }

    void setUniforms(const T& item) const {
        for (auto& binding : bindings) binding(item);
    }

private:
    template <typename Getter>
    void bind(Shader& shader, const char* name, Getter&& getter) {
        static_assert(std::is_invocable_v<Getter, const T&>);

        if (GLint location = glGetUniformLocation(shader.getID(), name); location != -1) {
            bindings.push_back([location, get = std::forward<Getter>(getter)](const T& item) {
                uniform::set(location, std::invoke(get, item));
            });
        }
    }
    std::vector<std::function<void(const T&)>> bindings;
};

template <typename Prop>
struct UniformWrapper {
    template <typename... Args>
    UniformWrapper(Args&&... args) : property{std::forward<Args>(args)...}, manager{} {}

    void bind(Shader& shader, std::optional<std::string_view> name = std::nullopt) {
        manager.bind(shader, name ? *name : property.getIdentifier());
    }
    void setUniforms() const { manager.setUniforms(property); }

    Prop property;
    UniformManager<Prop> manager;
};

namespace detail {
template <typename Func, typename Tuple, size_t... Is>
auto tupleMapImpl(Func&& func, Tuple&& tuple, std::index_sequence<Is...>) {
    return std::tuple{std::invoke(func, std::get<Is>(tuple))...};
}
}  // namespace detail

template <typename Func, typename Tuple>
auto tupleMap(Func&& func, Tuple&& tuple) {
    return detail::tupleMapImpl(
        std::forward<Func>(func), std::forward<Tuple>(tuple),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

template <typename F1, typename F2, typename... FN>
constexpr auto composition(F1&& f1, F2&& f2, FN&&... fn) noexcept {
    if constexpr (sizeof...(fn) == 0) {
        return [f1 = std::forward<F1>(f1), f2 = std::forward<F2>(f2)](const auto& arg) {
            return std::invoke(f1, std::invoke(f2, arg));
        };
    } else {
        return composition(std::forward<FN>(fn)...,
                           composition(std::forward<F1>(f1), std::forward<F2>(f2)));
    }
}

template <typename Arg, typename Res, typename Func>
auto asIdentity(Func&& func) {
    return [f = std::forward<Func>(func)](const Arg& arg) {
        return static_cast<Res>(std::invoke(f, arg));
    };
}

template <typename Arg, typename Res, typename Func>
auto asReciprocal(Func&& func) {
    return [f = std::forward<Func>(func)](const Arg& arg) {
        return Res{1} / static_cast<Res>(std::invoke(f, arg));
    };
}

template <typename Arg, typename Res, typename Func>
auto asOneMinus(Func&& func) {
    return [f = std::forward<Func>(func)](const Arg& arg) {
        return Res{1} - static_cast<Res>(std::invoke(f, arg));
    };
}

}  // namespace uniform

}  // namespace inviwo
