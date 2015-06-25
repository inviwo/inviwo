/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <modules/opengl/glwrap/shader.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/camera.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/volumeindicatorproperty.h>
#include <inviwo/core/ports/port.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/volumeutils.h>

namespace inviwo {

namespace utilgl {

// SimpleLightingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader* shader, const SimpleLightingProperty& property);
IVW_MODULE_OPENGL_API void addShaderDefines(Shader* shader, const ShadingMode::Modes& mode);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader* shader, const SimpleLightingProperty& property,
                                             std::string name);

// CameraProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader* shader, const CameraProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader* shader, const CameraProperty& property,
                                             std::string name);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader* shader, const Camera& property, std::string name);

// SpatialEntity
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader* shader, const SpatialEntity<3>& object,
                                             const std::string& name);

// SimpleRaycastingProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader* shader,
                                            const SimpleRaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader* shader,
                                             const SimpleRaycastingProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader* shader,
                                             const SimpleRaycastingProperty& property,
                                             std::string name);


// VolumeIndicatorProperty
IVW_MODULE_OPENGL_API void addShaderDefines(Shader* shader,
                                            const VolumeIndicatorProperty& property);
IVW_MODULE_OPENGL_API void setShaderUniforms(Shader* shader,
                                             const VolumeIndicatorProperty& property,
                                             std::string name);

// Ordinal Property
template <typename T>
void setShaderUniforms(Shader* shader, const OrdinalProperty<T>& property, std::string name) {
    shader->setUniform(name, property.get());
}

// Option Property
template <typename T>
void setShaderUniforms(Shader* shader, const TemplateOptionProperty<T>& property, std::string name) {
    shader->setUniform(name, property.get());
}

// Template magic...
template <typename T, typename std::enable_if<std::is_base_of<Property, T>::value, int>::type = 0>
void setUniforms(Shader* shader, const T& property) {
    setShaderUniforms(shader, property, property.getIdentifier());
}
template <typename T, typename std::enable_if<std::is_base_of<Port, T>::value, int>::type = 0>
void setUniforms(Shader* shader, const T& port) {
    setShaderUniforms(shader, port, port.getIdentifier() + "Parameters");
}
template <typename T, typename... Ts>
void setUniforms(Shader* shader, const T& elem, const Ts&... elements) {
    setUniforms(shader, elem);
    setUniforms(shader, elements...);
}


}  // namspace utilgl

}  // namespace

#endif  // IVW_SHADERUTILS_H
