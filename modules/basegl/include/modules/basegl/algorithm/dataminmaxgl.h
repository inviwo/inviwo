/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/formats.h>
#include <modules/opengl/shader/shader.h>

#include <utility>
#include <string_view>
#include <unordered_map>

namespace inviwo {

class Volume;
class VolumeGL;
class Layer;
class LayerGL;
class BufferBase;
class BufferGL;
struct GLFormat;
class DataFormatBase;

namespace utilgl {

/**
 * \brief Utility class for computing min and max values using OpenGL
 *
 * This class uses compute shaders to compute min and max values of Buffers, Layers, and Volumes.
 * If compute shaders are not available, the corresponding CPU functions from
 * modules/base/algorithm/dataminmax.h are used instead.
 *
 * The GPU needs to support the following OpenGL extensions:
 *  + `GL_KHR_shader_subgroup`
 *  + for 8bit and 16bit data type support for SSBOs (int8_t, i8vec2, ...)
 *      - NVIDIA: `GL_NV_gpu_shader5`
 *      - AMD: `GL_EXT_shader_explicit_arithmetic_types`
 */
class IVW_MODULE_BASEGL_API DataMinMaxGL {
public:
    DataMinMaxGL();
    DataMinMaxGL(const DataMinMaxGL&) = delete;
    DataMinMaxGL(DataMinMaxGL&&) noexcept = default;
    ~DataMinMaxGL() = default;
    DataMinMaxGL& operator=(const DataMinMaxGL&) = delete;
    DataMinMaxGL& operator=(DataMinMaxGL&&) noexcept = default;

    /**
     * Check whether the GPU supports compute shaders and the necessary OpenGL extensions.
     *  + `GL_KHR_shader_subgroup`
     *  + for 8bit and 16bit data type support for SSBOs (int8_t, i8vec2, ...)
     *      - NVIDIA: `GL_NV_gpu_shader5`
     *      - AMD: `GL_EXT_shader_explicit_arithmetic_types`
     *
     * @return true if the GPU fullfills all requirements
     */
    static bool isSuppportedByGPU();

    std::pair<dvec4, dvec4> minMax(const Volume* volume);
    std::pair<dvec4, dvec4> minMax(const VolumeGL* volumeGL);

    std::pair<dvec4, dvec4> minMax(const Layer* layer);
    std::pair<dvec4, dvec4> minMax(const LayerGL* layerGL);

    std::pair<dvec4, dvec4> minMax(const BufferBase* buffer);
    std::pair<dvec4, dvec4> minMax(const BufferGL* bufferGL);

private:
    Shader& getNdShader(Shader& shader, const GLFormat& glFormat, int nd);
    Shader& getVolumeMinMaxShader(const GLFormat& glFormat);
    Shader& getLayerMinMaxShader(const GLFormat& glFormat);
    Shader& getBufferMinMaxShader(const DataFormatBase* format);
    Shader& getLinearReduceShader();

    bool gpuSupport_;

    Shader volumeMinMax_;
    Shader layerMinMax_;
    Shader linearMinMax_;
    Shader bufferMinMax_;

    std::unordered_map<GLuint, DataFormatId> shaderDataFormat_;

    const DataFormatBase* bufferFormat_ = nullptr;
};

}  // namespace utilgl

}  // namespace inviwo
