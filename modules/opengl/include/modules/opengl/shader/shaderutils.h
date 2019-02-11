/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_SHADERUTILS_H
#define IVW_SHADERUTILS_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/shader/shader.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/stipplingproperty.h>
#include <inviwo/core/properties/raycastingproperty.h>
#include <inviwo/core/ports/port.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

class Camera;
class CameraProperty;
class SimpleRaycastingProperty;
class IsoValueProperty;
class IsoTFProperty;
class VolumeIndicatorProperty;

template <size_t>
class BaseImageInport;
using ImageInport = BaseImageInport<1>;

namespace utilgl {

// TemplateProperty
template <typename T>
void setShaderUniforms(Shader& shader, const TemplateProperty<T>& property, std::string name) {
    shader.setUniform(name, property.get());
}
template <typename T>
void setShaderUniforms(Shader& shader, const TemplateProperty<T>& property) {
    setShaderUniforms(shader, property, property.getIdentifier());
}

// SimpleLightingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const SimpleLightingProperty& property);
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const ShadingMode::Modes& mode);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const SimpleLightingProperty& property,
                                             std::string name);

// CameraProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const CameraProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const CameraProperty& property,
                                             std::string name);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const Camera& property,
                                             std::string name);

// RaycastingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const RaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const RaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const RaycastingProperty& property,
                                             std::string name);

IVW_MODULE_OPENGL_API void setShaderDefines(
    Shader& shader, const TemplateOptionProperty<RaycastingProperty::GradientComputation>& property,
    bool voxelClassification = false);

// SpatialEntity
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const SpatialEntity<3>& object,
                                             const std::string& name);

// SimpleRaycastingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader,
                                            const SimpleRaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader,
                                             const SimpleRaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader,
                                             const SimpleRaycastingProperty& property,
                                             std::string name);

// IsoValueProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const IsoValueProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoValueProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoValueProperty& property,
                                             std::string name);

// IsoTFProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const IsoTFProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoTFProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const IsoTFProperty& property,
                                             std::string name);

// Background Image
IVW_MODULE_OPENGL_API void addShaderDefinesBGPort(Shader& shader, const ImageInport& port);

// VolumeIndicatorProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader,
                                            const VolumeIndicatorProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader,
                                             const VolumeIndicatorProperty& property,
                                             std::string name);

// StipplingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const StipplingProperty& property);
IVW_MODULE_OPENGL_API void addShaderDefines(Shader& shader, const StipplingProperty::Mode& mode);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader& shader, const StipplingProperty& property,
                                             std::string name);

// Ordinal Property
template <typename T>
void setShaderUniforms(Shader& shader, const OrdinalProperty<T>& property, std::string name) {
    shader.setUniform(name, property.get());
}

namespace detail {
template <typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
T getOptionValue(const TemplateOptionProperty<T>& prop) {
    return prop.get();
}
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
auto getOptionValue(const TemplateOptionProperty<T>& prop) ->
    typename std::underlying_type<T>::type {
    return static_cast<typename std::underlying_type<T>::type>(prop.get());
}
}  // namespace detail

// Option Property
template <typename T>
void setShaderUniforms(Shader& shader, const TemplateOptionProperty<T>& property,
                       std::string name) {
    shader.setUniform(name, detail::getOptionValue(property));
}

// MinMax Property
template <typename T>
void setShaderUniforms(Shader& shader, const MinMaxProperty<T>& property, std::string name) {
    shader.setUniform(name, property.get());
}

// Template magic...
template <typename T, typename std::enable_if<std::is_base_of<Property, T>::value, int>::type = 0>
void addDefines(Shader& shader, const T& property) {
    addShaderDefines(shader, property);
}
template <typename T, typename... Ts>
void addDefines(Shader& shader, const T& elem, const Ts&... elements) {
    addDefines(shader, elem);
    addDefines(shader, elements...);
}

template <typename T, typename std::enable_if<std::is_base_of<Property, T>::value, int>::type = 0>
void setUniforms(Shader& shader, const T& property) {
    setShaderUniforms(shader, property, property.getIdentifier());
}
template <typename T, typename std::enable_if<std::is_base_of<Port, T>::value, int>::type = 0>
void setUniforms(Shader& shader, const T& port) {
    setShaderUniforms(shader, port, port.getIdentifier() + "Parameters");
}
template <typename T, typename... Ts>
void setUniforms(Shader& shader, const T& elem, const Ts&... elements) {
    setUniforms(shader, elem);
    setUniforms(shader, elements...);
}

IVW_MODULE_OPENGL_API int getLogLineNumber(const std::string& compileLogLine);

IVW_MODULE_OPENGL_API std::string getShaderInfoLog(GLuint id);

IVW_MODULE_OPENGL_API std::string getProgramInfoLog(GLuint id);

IVW_MODULE_OPENGL_API std::shared_ptr<const ShaderResource> findShaderResource(
    const std::string& fileName);

IVW_MODULE_OPENGL_API std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>
toShaderResources(const std::vector<std::pair<ShaderType, std::string>>& items);

IVW_MODULE_OPENGL_API std::string getGLSLTypeName(const DataFormatBase* format);

}  // namespace utilgl

}  // namespace inviwo

#endif  // IVW_SHADERUTILS_H
