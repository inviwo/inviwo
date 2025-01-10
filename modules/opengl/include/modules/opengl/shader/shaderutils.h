/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/datastructures/light/lightingstate.h>  // for ShadingMode
#include <inviwo/core/datastructures/spatialdata.h>          // for SpatialEntity
#include <inviwo/core/properties/raycastingproperty.h>       // for RaycastingProperty, Raycasti...
#include <inviwo/core/util/detected.h>
#include <modules/opengl/inviwoopengl.h>          // for GLuint
#include <modules/opengl/shader/shader.h>         // IWYU pragma: keep
#include <modules/opengl/texture/textureutils.h>  // IWYU pragma: keep

#include <cstddef>      // for size_t
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for enable_if, is_base_of, is_enum
#include <utility>      // for pair
#include <vector>       // for vector

namespace inviwo {

class DataMapper;
class Camera;
class CameraProperty;
class DataFormatBase;
class IsoTFProperty;
class IsoValueProperty;
class Port;
class Property;
class SelectionColorProperty;
class ShaderResource;
class ShaderType;
class SimpleLightingProperty;
class SimpleRaycastingProperty;
class VolumeIndicatorProperty;
struct SelectionColorState;
template <size_t>
class BaseImageInport;
template <typename T>
class MinMaxProperty;
template <typename T>
class OptionProperty;
template <typename T>
class OrdinalProperty;
template <typename T>
class TemplateProperty;

using ImageInport = BaseImageInport<1>;

namespace utilgl {

/**
 * Set the shader uniforms of \p shader related to format scaling based on \p dataMapper and the
 * corresponding data format \p format  using \p name as a base name, that is `<name>.formatScaling`
 * etc. These uniforms are used in connection with sampler2d.glsl and sampler3d.glsl.
 */
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const DataMapper& dataMapper,
                                             const DataFormatBase* format, std::string_view name);

// TemplateProperty
template <typename T>
void setShaderUniforms(Shader& shader, const TemplateProperty<T>& property, std::string_view name) {
    shader.setUniform(name, property.get());
}
template <typename T>
void setShaderUniforms(Shader& shader, const TemplateProperty<T>& property) {
    setShaderUniforms(shader, property, property.getIdentifier());
}

// SimpleLightingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const SimpleLightingProperty& property);
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const LightingState& state);
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const ShadingMode& mode);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const SimpleLightingProperty& property,
                                             std::string_view name);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const LightingState& state,
                                             std::string_view name);

// CameraProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const CameraProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const CameraProperty& property,
                                             std::string_view name);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const Camera& camera,
                                             std::string_view name);

// RaycastingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const RaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const RaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const RaycastingProperty& property,
                                             std::string_view name);

IVW_MODULE_OPENGL_API void setShaderDefines(
    Shader& shader, const OptionProperty<RaycastingProperty::GradientComputation>& property,
    bool voxelClassification = false);

// SpatialEntity
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const SpatialEntity& object,
                                             std::string_view name);

// SimpleRaycastingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader,
                                            const SimpleRaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader,
                                             const SimpleRaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader,
                                             const SimpleRaycastingProperty& property,
                                             std::string_view name);

// IsoValueProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const IsoValueProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoValueProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoValueProperty& property,
                                             std::string_view name);

// IsoTFProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const IsoTFProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoTFProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoTFProperty& property,
                                             std::string_view name);

// SelectionColorProperty
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader,
                                             const SelectionColorProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const SelectionColorProperty& property,
                                             std::string_view name);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const SelectionColorState& state,
                                             std::string_view name);

// Background Image
IVW_MODULE_OPENGL_API void addShaderDefinesBGPort(Shader& shader, const ImageInport& port);

// VolumeIndicatorProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader,
                                            const VolumeIndicatorProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader,
                                             const VolumeIndicatorProperty& property,
                                             std::string_view name);

// Ordinal Property
template <typename T>
void setShaderUniforms(Shader& shader, const OrdinalProperty<T>& property, std::string_view name) {
    shader.setUniform(name, property.get());
}

template <typename T>
void setShaderUniforms(Shader& shader, const OrdinalProperty<T>& property) {
    setShaderUniforms(shader, property, property.getIdentifier());
}

namespace detail {
template <typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
T getOptionValue(const OptionProperty<T>& prop) {
    return prop.get();
}
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
auto getOptionValue(const OptionProperty<T>& prop) -> typename std::underlying_type<T>::type {
    return static_cast<typename std::underlying_type<T>::type>(prop.get());
}
}  // namespace detail

// Option Property
template <typename T>
void setShaderUniforms(Shader& shader, const OptionProperty<T>& property, std::string_view name) {
    shader.setUniform(name, detail::getOptionValue(property));
}

// MinMax Property
template <typename T>
void setShaderUniforms(Shader& shader, const MinMaxProperty<T>& property, std::string_view name) {
    shader.setUniform(name, property.get());
}

template <typename T, typename std::enable_if<std::is_base_of<Property, T>::value, int>::type = 0>
void setShaderUniforms(Shader& shader, const T& property) {
    setShaderUniforms(shader, property, property.getIdentifier());
}
template <typename T, typename std::enable_if<std::is_base_of<Port, T>::value, int>::type = 0>
void setShaderUniforms(Shader& shader, const T& port) {
    setShaderUniforms(shader, port, port.getIdentifier() + "Parameters");
}

namespace detail {
template <typename T>
using addDefinesMemberType = decltype(std::declval<const T&>().addDefines(std::declval<Shader&>()));

template <class T>
constexpr auto HasAddDefinesMember = util::is_detected_exact_v<void, addDefinesMemberType, T>;

template <typename T>
using addShaderDefinesFunctionType =
    decltype(addShaderDefines(std::declval<Shader&>(), std::declval<const T&>()));

template <class T>
constexpr auto HasAddShaderDefinesFunction =
    util::is_detected_exact_v<void, addShaderDefinesFunctionType, T>;

template <typename T>
using setUniformsMemberType =
    decltype(std::declval<const T&>().setUniforms(std::declval<Shader&>()));

template <class T>
constexpr auto HasSetUniformsMember = util::is_detected_exact_v<void, setUniformsMemberType, T>;

template <typename T>
using setShaderUniformsFunctionType =
    decltype(setShaderUniforms(std::declval<Shader&>(), std::declval<const T&>()));

template <class T>
constexpr auto HasSetShaderUniformsFunction =
    util::is_detected_exact_v<void, setShaderUniformsFunctionType, T>;

template <typename T>
using setShaderUniformsNamedFunctionType = decltype(setShaderUniforms(
    std::declval<Shader&>(), std::declval<const T&>(), std::declval<std::string_view>()));

template <class T>
constexpr auto HasSetShaderUniformsNamedFunction =
    util::is_detected_exact_v<void, setShaderUniformsNamedFunctionType, T>;

template <typename T>
void addDefinesImpl(Shader& shader, const T& element) {
    if constexpr (detail::HasAddDefinesMember<T>) {
        element.addDefines(shader);
    } else if constexpr (detail::HasAddShaderDefinesFunction<T>) {
        addShaderDefines(shader, element);
    } else {
        static_assert(util::alwaysFalse<T>(),
                      "Did not find an overload of either: "
                      "void addShaderDefines(Shader& shader, const T& element); "
                      "or void T::addDefines(Shader& shader) const;");
    }
}

template <typename T>
void setUniformsImpl(Shader& shader, const T& element) {
    if constexpr (detail::HasSetUniformsMember<T>) {
        element.setUniforms(shader);
    } else if constexpr (detail::HasSetShaderUniformsNamedFunction<T> &&
                         std::is_base_of_v<Property, T>) {
        setShaderUniforms(shader, element, element.getIdentifier());
    } else if constexpr (detail::HasSetShaderUniformsNamedFunction<T> &&
                         std::is_base_of_v<Port, T>) {
        StrBuffer buff;
        setShaderUniforms(shader, element, buff.replace("{}Parameters", element.getIdentifier()));
    } else if constexpr (detail::HasSetShaderUniformsFunction<T>) {
        setShaderUniforms(shader, element);
    } else {
        static_assert(util::alwaysFalse<T>(),
                      "Did not find an overload of either: "
                      "void setShaderUniforms(Shader& shader, const T& element); "
                      "or void T::setUniforms(Shader& shader) const;");
    }
}

}  // namespace detail

template <typename... Ts>
void addDefines(Shader& shader, const Ts&... elements) {
    (detail::addDefinesImpl(shader, elements), ...);
}

template <typename... Ts>
void setUniforms(Shader& shader, const Ts&... elements) {
    (detail::setUniformsImpl(shader, elements), ...);
}

IVW_MODULE_OPENGL_API int getLogLineNumber(std::string_view compileLogLine);

IVW_MODULE_OPENGL_API std::string getShaderInfoLog(GLuint id);

IVW_MODULE_OPENGL_API std::string getProgramInfoLog(GLuint id);

IVW_MODULE_OPENGL_API std::shared_ptr<const ShaderResource> findShaderResource(
    std::string_view fileName);

IVW_MODULE_OPENGL_API std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>
toShaderResources(const std::vector<std::pair<ShaderType, std::string>>& items);

IVW_MODULE_OPENGL_API std::string getGLSLTypeName(const DataFormatBase* format);

}  // namespace utilgl

}  // namespace inviwo
