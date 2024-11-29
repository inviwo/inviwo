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

#include <modules/basegl/algorithm/dataminmaxgl.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/exception.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/glformats.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/texture3d.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/base/algorithm/dataminmax.h>

#include <string_view>
#include <array>

namespace inviwo::utilgl {

namespace {

std::string_view getSamplerPrefix(const GLFormat& glFormat) {
    if (glFormat.normalization != utilgl::Normalization::None) {
        return {};
    }
    const auto* format = DataFormatBase::get(GLFormats::get(glFormat));
    using namespace std::string_view_literals;
    switch (format->getNumericType()) {
        case NumericType::UnsignedInteger:
            return "u"sv;
        case NumericType::SignedInteger:
            return "i"sv;
        default:
            return {};
    }
}

std::string_view getImagePrefix(const GLFormat& glFormat) {
    using namespace std::string_view_literals;
    if (glFormat.layoutQualifier.ends_with("ui")) {
        return "u"sv;
    } else if (glFormat.layoutQualifier.ends_with("i")) {
        return "i"sv;
    } else {
        return {};
    }
}

[[nodiscard]] std::pair<dvec4, dvec4> downloadResultFromBuffer(const GLFormat& glFormat) {
    std::array minmaxGL{vec4{0}, vec4{0}};
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(glm::vec4), minmaxGL.data());

    if (glFormat.channels < 4) {
        // reset alpha channel since imageLoad in the shader returns a = 1.0 for non-rgba formats
        minmaxGL[0].a = 0.0;
        minmaxGL[1].a = 0.0;
    }

    // undo GL format normalization
    if (glFormat.normalization == utilgl::Normalization::Normalized) {
        const DataMapper dataMapper{DataFormatBase::get(GLFormats::get(glFormat))};

        minmaxGL[0] = dataMapper.mapFromNormalizedToData(minmaxGL[0]);
        minmaxGL[1] = dataMapper.mapFromNormalizedToData(minmaxGL[1]);
    } else if (glFormat.normalization == utilgl::Normalization::SignNormalized) {
        using enum DataMapper::SignedNormalization;
        const DataMapper dataMapper{
            DataFormatBase::get(GLFormats::get(glFormat)),
            OpenGLCapabilities::isSignedIntNormalizationSymmetric() ? Symmetric : Asymmetric};

        minmaxGL[0] = dataMapper.mapFromSignNormalizedToData(minmaxGL[0]);
        minmaxGL[1] = dataMapper.mapFromSignNormalizedToData(minmaxGL[1]);
    }
    return {minmaxGL[0], minmaxGL[1]};
}

[[nodiscard]] std::pair<dvec4, dvec4> reduce2D(Shader& shader, Shader& linearReduce, uvec2 dims,
                                               const GLFormat& glFormat, const Texture& texture) {
    // group size needs to match local_size_x, _y, and _z in compute shader layout since each thread
    // only processes one texel (2D texture) or one z-column of voxels (3D texture).
    const uvec3 groupSize{32, 32, 1};
    const uvec3 numGroups{(uvec3{dims.x, dims.y, 1} + groupSize - uvec3{1}) / groupSize};

    const bool useImageLoadStore = (glFormat.channels != 3);

    if (useImageLoadStore) {
        glBindImageTexture(0, texture.getID(), 0, GL_TRUE, 0, GL_READ_ONLY,
                           glFormat.internalFormat);
    }

    // global buffer for min/max values for all work groups
    const size_t bufSize = 2 * glm::compMul(numGroups);
    const BufferObject buf(bufSize * sizeof(vec4), GLFormats::get(DataFormatId::Vec4Float32),
                           GL_DYNAMIC_READ, GL_SHADER_STORAGE_BUFFER);
    buf.bindBase(1);

    {
        const utilgl::Activate activateShader{&shader};

        if (!useImageLoadStore) {
            const TextureUnit texUnit;
            texUnit.activate();
            texture.bind();
            shader.setUniform("sourceImage", texUnit.getUnitNumber());
            TextureUnit::setZeroUnit();
        }

        glDispatchCompute(numGroups.x, numGroups.y, numGroups.z);
    }
    const std::uint32_t arrayLen = glm::compMul(numGroups);
    {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        const utilgl::Activate activateShader{&linearReduce};
        linearReduce.setUniform("arrayLength", arrayLen);
        glDispatchCompute(1, 1, 1);
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    return downloadResultFromBuffer(glFormat);
}

std::string_view glslDataType(DataFormatId id) {
    using namespace std::string_view_literals;

    // Special handling for formats with 3 channels since those formats are usually padded to the
    // corresponding vec4 types when used with SSBOs, including arrays. To avoid padding, we instead
    // use linear arrays for 3 channels. For example `float buf[]` instead of `vec3 buf[]`. Note
    // that this affects data access and indexing!
    //
    // See OpenGL 4.6 specification, Section 7.6.2.2 Standard Uniform Block Layout
    static constexpr auto size = static_cast<size_t>(DataFormatId::NumberOfFormats);
    static constexpr std::array<std::string_view, size> glslType{{
        // clang-format off
        {},           // NotSpecialized
        // 1 channels
        "float"sv,    // Float32
        {},           // Float64
        "int8_t"sv,   // Int8
        "int16_t"sv,  // Int16
        "int"sv,      // Int32
        {},           // Int64
        "uint8_t"sv,  // UInt8
        "uint16_t"sv, // UInt16
        "uint"sv,     // UInt32
        {},           // UInt64
        // 2 channels
        "vec2"sv,     // Vec2Float32
        {},           // Vec2Float64
        "i8vec2"sv,   // Vec2Int8
        "i16vec2"sv,  // Vec2Int16
        "int"sv,      // Vec2Int32
        {},           // Vec2Int64
        "u8vec2"sv,   // Vec2UInt8
        "u16vec2"sv,  // Vec2UInt16
        "uvec2"sv,    // Vec2UInt32
        {},           // Vec2UInt64
        // 3 channels (special case, use scalars to avoid padding)
        "float"sv,    // Vec3Float32
        {},           // Vec3Float64
        "int8_t"sv,   // Vec3Int8
        "int16_t"sv,  // Vec3Int16
        "int"sv,      // Vec3Int32
        {},           // Vec3Int64
        "uint8_t"sv,  // Vec3UInt8
        "uint16_t"sv, // Vec3UInt16
        "uint"sv,     // Vec3UInt32
        {},           // Vec4UInt64
        // 4 channels
        "vec4"sv,     // Vec4Float32
        {},           // Vec4Float64
        "i8vec4"sv,   // Vec4Int8
        "i16vec4"sv,  // Vec4Int16
        "ivec4"sv,    // Vec4Int32
        {},           // Vec4Int64
        "u8vec4"sv,   // Vec4UInt8
        "u16vec4"sv,  // Vec4UInt16
        "uvec4"sv,    // Vec4UInt32
        {},           // Vec4UInt64
        // clang-format on
    }};

    if (glslType[static_cast<size_t>(id)].empty()) {
        throw OpenGLException(IVW_CONTEXT_CUSTOM("GLFormat"),
                              "Error no GLSL data type found for selected data format: {}",
                              DataFormatBase::get(id)->getString());
    }
    return glslType[static_cast<size_t>(id)];
}

std::string_view glslDataConversion(size_t numComponents) {
    // Conversion from any data type with numComponents channels to vec4.
    // Special case for 3 component types.
    // See glslDataType()
    using namespace std::string_view_literals;
    static constexpr std::array<std::string_view, 4> conversion{{
        "vec4(value[index], 0, 0, 0)"sv,
        "vec4(value[index], 0, 0)"sv,
        "vec4(value[3 * index], value[3 * index + 1], value[3 * index + 2], 0)"sv,
        "value[index]"sv,
    }};
    return conversion[std::min(numComponents - 1, conversion.size())];
}

}  // namespace

DataMinMaxGL::DataMinMaxGL()
    : gpuSupport_{isSuppportedByGPU()}
    , volumeMinMax_{{{ShaderType::Compute, "compute/volumeminmax.comp"}}, Shader::Build::No}
    , layerMinMax_{{{ShaderType::Compute, "compute/layerminmax.comp"}}, Shader::Build::No}
    , linearMinMax_{{{ShaderType::Compute, "compute/linearminmax.comp"}}, Shader::Build::No}
    , bufferMinMax_{{{ShaderType::Compute, "compute/bufferminmax.comp"}}, Shader::Build::No} {}

std::pair<dvec4, dvec4> DataMinMaxGL::minMax(const Volume& volume) {
    if (!gpuSupport_) {
        return util::volumeMinMax(&volume);
    }
    return minMax(*volume.getRepresentation<VolumeGL>());
}

std::pair<dvec4, dvec4> DataMinMaxGL::minMax(const VolumeGL& volumeGL) {
    if (!gpuSupport_) {
        throw Exception(IVW_CONTEXT,
                        "Cannot compute min/max values of GL representation. Missing GPU support.");
    }

    const auto& glFormat = GLFormats::get(volumeGL.getDataFormatId());

    Shader& volumeMinMax = getVolumeMinMaxShader(glFormat);
    Shader& linearReduce = getLinearReduceShader();

    const uvec3 dims{volumeGL.getDimensions()};
    // ignore z dimension since the compute shader iterates over all voxels in z direction
    return reduce2D(volumeMinMax, linearReduce, uvec2{dims}, glFormat, *volumeGL.getTexture());
}

std::pair<dvec4, dvec4> DataMinMaxGL::minMax(const Layer& layer) {
    if (!gpuSupport_) {
        return util::layerMinMax(&layer);
    }
    return minMax(*layer.getRepresentation<LayerGL>());
}

std::pair<dvec4, dvec4> DataMinMaxGL::minMax(const LayerGL& layerGL) {
    if (!gpuSupport_) {
        throw Exception(IVW_CONTEXT,
                        "Cannot compute min/max values of GL representation. Missing GPU support.");
    }

    const auto& glFormat = GLFormats::get(layerGL.getDataFormatId());

    Shader& layerMinMax = getLayerMinMaxShader(glFormat);
    Shader& linearReduce = getLinearReduceShader();

    return reduce2D(layerMinMax, linearReduce, layerGL.getDimensions(), glFormat,
                    *layerGL.getTexture());
}

std::pair<dvec4, dvec4> DataMinMaxGL::minMax(const BufferBase& buffer) {
    if (!gpuSupport_) {
        return util::bufferMinMax(&buffer);
    }
    return minMax(*buffer.getRepresentation<BufferGL>());
}

std::pair<dvec4, dvec4> DataMinMaxGL::minMax(const BufferGL& bufferGL) {
    if (!gpuSupport_) {
        throw Exception(IVW_CONTEXT,
                        "Cannot compute min/max values of GL representation. Missing GPU support.");
    }

    Shader& bufferMinMax = getBufferMinMaxShader(bufferGL.getDataFormat());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferGL.getId());
    const auto bufferSize = static_cast<std::uint32_t>(bufferGL.getSize());

    // global buffer for min/max values
    const size_t resultSize = 2;
    const BufferObject buf(resultSize * sizeof(vec4), GLFormats::get(DataFormatId::Vec4Float32),
                           GL_DYNAMIC_READ, GL_SHADER_STORAGE_BUFFER);
    buf.bindBase(1);

    {
        const utilgl::Activate activateShade{&bufferMinMax};
        bufferMinMax.setUniform("arrayLength", bufferSize);
        glDispatchCompute(1, 1, 1);
    }

    // download result
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    std::array minmaxGL{vec4{0}, vec4{0}};
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(glm::vec4), minmaxGL.data());

    if (bufferGL.getDataFormat()->getComponents() < 4) {
        // reset alpha channel since imageLoad in the shader returns a = 1.0 for non-rgba formats
        minmaxGL[0].a = 0.0;
        minmaxGL[1].a = 0.0;
    }
    return {minmaxGL[0], minmaxGL[1]};
}

Shader& DataMinMaxGL::getNdShader(Shader& shader, const GLFormat& glFormat, int nd) {
    using enum ShaderObject::ExtensionBehavior;

    auto* computeShader = shader.getComputeShaderObject();
    if (!computeShader->hasShaderExtension("GL_KHR_shader_subgroup_basic")) {
        computeShader->addShaderExtension("GL_KHR_shader_subgroup_basic", Require);
        computeShader->addShaderExtension("GL_KHR_shader_subgroup_arithmetic", Require);
        shader.invalidate();
    }

    const auto formatId = GLFormats::get(glFormat);
    if (auto it = shaderDataFormat_.find(computeShader->getID());
        it == shaderDataFormat_.end() || it->second != formatId) {

        const bool useImageLoadStore = (glFormat.channels != 3);

        StrBuffer buf;
        if (useImageLoadStore) {
            // use image and imageLoad()
            computeShader->addShaderDefine("IMAGE_FORMAT", glFormat.layoutQualifier);
            computeShader->removeShaderDefine("USE_IMAGE_SAMPLER");
            buf.append("{}image{}D", getImagePrefix(glFormat), nd);
        } else {
            // use regular texture sampler since imageLoad() does not support RGB formats
            computeShader->addShaderDefine("USE_IMAGE_SAMPLER");
            buf.append("{}sampler{}D", getSamplerPrefix(glFormat), nd);
        }
        computeShader->addShaderDefine("IMAGE_SOURCE", buf);

        shaderDataFormat_[computeShader->getID()] = formatId;
        shader.invalidate();
    }

    if (!shader.isReady()) {
        shader.build();
    }
    return shader;
}

Shader& DataMinMaxGL::getVolumeMinMaxShader(const GLFormat& glFormat) {
    return getNdShader(volumeMinMax_, glFormat, 3);
}

Shader& DataMinMaxGL::getLayerMinMaxShader(const GLFormat& glFormat) {
    return getNdShader(layerMinMax_, glFormat, 2);
}

bool DataMinMaxGL::isSuppportedByGPU() {
    return OpenGLCapabilities::isComputeShadersSupported() &&
           (OpenGLCapabilities::isExtensionSupported("GL_NV_gpu_shader5") ||
            OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_explicit_arithmetic_types")) &&
           OpenGLCapabilities::isExtensionSupported("GL_KHR_shader_subgroup");
}

Shader& DataMinMaxGL::getBufferMinMaxShader(const DataFormatBase* format) {
    using enum ShaderObject::ExtensionBehavior;

    auto* computeShader = bufferMinMax_.getComputeShaderObject();
    if (!computeShader->hasShaderExtension("GL_KHR_shader_subgroup_basic")) {
        computeShader->addShaderExtension("GL_KHR_shader_subgroup_basic", Require);
        computeShader->addShaderExtension("GL_KHR_shader_subgroup_arithmetic", Require);

        // enable SSBO support for 8bit and 16bit data types (int8_t, uint16_t, i8vec2, ...)
        // NVIDIA:
        if (OpenGLCapabilities::isExtensionSupported("GL_NV_gpu_shader5")) {
            computeShader->addShaderExtension("GL_NV_gpu_shader5", Require);
        }
        // AMD:
        if (OpenGLCapabilities::isExtensionSupported("GL_EXT_shader_explicit_arithmetic_types")) {
            computeShader->addShaderExtension("GL_EXT_shader_explicit_arithmetic_types", Require);
        }
        bufferMinMax_.invalidate();
    }

    if (!bufferMinMax_.isReady() || bufferFormat_ != format) {
        bufferFormat_ = format;
        computeShader->addShaderDefine("BUFFER_DATATYPE", glslDataType(format->getId()));
        computeShader->addShaderDefine("GetValue(value, index)",
                                       glslDataConversion(format->getComponents()));
        bufferMinMax_.build();
    }
    return bufferMinMax_;
}

Shader& DataMinMaxGL::getLinearReduceShader() {
    if (!linearMinMax_.isReady()) {
        using enum ShaderObject::ExtensionBehavior;

        auto* computeShader = linearMinMax_.getComputeShaderObject();
        computeShader->addShaderExtension("GL_KHR_shader_subgroup_basic", Require);
        computeShader->addShaderExtension("GL_KHR_shader_subgroup_arithmetic", Require);
        linearMinMax_.build();
    }
    return linearMinMax_;
}

}  // namespace inviwo::utilgl
