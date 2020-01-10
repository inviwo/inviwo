/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_GLFORMATS_H
#define IVW_GLFORMATS_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/assertion.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglexception.h>

#include <array>

namespace inviwo {

class IVW_MODULE_OPENGL_API GLFormats {
public:
    static constexpr size_t size = static_cast<size_t>(DataFormatId::NumberOfFormats);

    enum class Normalization : char { None, Normalized, SignNormalized };

    struct GLFormat {
        constexpr GLFormat() noexcept = default;
        GLint format = 0;
        GLint internalFormat = 0;
        GLenum type = 0;
        GLuint channels = 0;
        GLuint typeSize = 0;
        Normalization normalization = Normalization::None;
        bool valid = false;

        friend constexpr bool operator==(const GLFormats::GLFormat& a,
                                         const GLFormats::GLFormat& b) {
            return a.format == b.format && a.internalFormat == b.internalFormat &&
                   a.type == b.type && a.channels == b.channels && a.typeSize == b.typeSize &&
                   a.normalization == b.normalization && a.valid == b.valid;
        }
        friend constexpr bool operator!=(const GLFormats::GLFormat& a,
                                         const GLFormats::GLFormat& b) {
            return !(a == b);
        }
    };

    static constexpr const GLFormat& get(DataFormatId id) {
        if (!formats_[static_cast<size_t>(id)].valid) {
            throw OpenGLException("Error no OpenGL format available for selected data format: " +
                                      std::string(DataFormatBase::get(id)->getString()),
                                  IVW_CONTEXT_CUSTOM("GLFormat"));
        }
        return formats_[static_cast<size_t>(id)];
    }

    static DataFormatId get(GLenum type, GLuint channels) {

        const auto it = std::find_if(formats_.begin(), formats_.end(), [&](const GLFormat& item) {
            return item.type == type && item.channels == channels;
        });

        if (it != formats_.end()) {
            return static_cast<DataFormatId>(it - formats_.begin());
        } else {
            return DataFormatId::NotSpecialized;
        }
    }

private:
    static constexpr std::array<GLFormat, size> formats_{{
        // clang-format off
        {},                                                                                               // NotSpecialized
        // 1 channels
        {GL_RED,          GL_R16F,         GL_HALF_FLOAT,     1, 2, Normalization::None,           true}, // Float16
        {GL_RED,          GL_R32F,         GL_FLOAT,          1, 4, Normalization::None,           true}, // Float32
        {},                                                                                               // Float64
        {GL_RED,          GL_R8_SNORM,     GL_BYTE,           1, 1, Normalization::SignNormalized, true}, // Int8
        {GL_RED,          GL_R16_SNORM,    GL_SHORT,          1, 2, Normalization::SignNormalized, true}, // Int16
        {GL_RED_INTEGER,  GL_R32I,         GL_INT,            1, 4, Normalization::None,           true}, // Int32
        {},                                                                                               // Int64
        {GL_RED,          GL_R8,           GL_UNSIGNED_BYTE,  1, 1, Normalization::Normalized,     true}, // UInt8
        {GL_RED,          GL_R16,          GL_UNSIGNED_SHORT, 1, 2, Normalization::Normalized,     true}, // UInt16
        {GL_RED_INTEGER,  GL_R32UI,        GL_UNSIGNED_INT,   1, 4, Normalization::None,           true}, // UInt32
        {},                                                                                               // UInt64
        // 2 channels
        {GL_RG,           GL_RG16F,        GL_HALF_FLOAT,     2, 2, Normalization::None,           true}, // Vec2Float16
        {GL_RG,           GL_RG32F,        GL_FLOAT,          2, 4, Normalization::None,           true}, // Vec2Float32
        {},                                                                                               // Vec2Float64
        {GL_RG,           GL_RG8_SNORM,    GL_BYTE,           2, 1, Normalization::SignNormalized, true}, // Vec2Int8
        {GL_RG,           GL_RG16_SNORM,   GL_SHORT,          2, 2, Normalization::SignNormalized, true}, // Vec2Int16
        {GL_RG_INTEGER,   GL_RG32I,        GL_INT,            2, 4, Normalization::None,           true}, // Vec2Int32
        {},                                                                                               // Vec2Int64
        {GL_RG,           GL_RG8,          GL_UNSIGNED_BYTE,  2, 1, Normalization::Normalized,     true}, // Vec2UInt8
        {GL_RG,           GL_RG16,         GL_UNSIGNED_SHORT, 2, 2, Normalization::Normalized,     true}, // Vec2UInt16
        {GL_RG_INTEGER,   GL_RG32UI,       GL_UNSIGNED_INT,   2, 4, Normalization::None,           true}, // Vec2UInt32
        {},                                                                                               // Vec2UInt64
        // 3 channels
        {GL_RGB,          GL_RGB16F,       GL_HALF_FLOAT,     3, 2, Normalization::None,           true}, // Vec3Float16
        {GL_RGB,          GL_RGB32F,       GL_FLOAT,          3, 4, Normalization::None,           true}, // Vec3Float32
        {},                                                                                               // Vec3Float64
        {GL_RGB,          GL_RGB8_SNORM,   GL_BYTE,           3, 1, Normalization::SignNormalized, true}, // Vec3Int8
        {GL_RGB,          GL_RGB16_SNORM,  GL_SHORT,          3, 2, Normalization::SignNormalized, true}, // Vec3Int16
        {GL_RGB_INTEGER,  GL_RGB32I,       GL_INT,            3, 4, Normalization::None,           true}, // Vec3Int32
        {},                                                                                               // Vec3Int64
        {GL_RGB,          GL_RGB8,         GL_UNSIGNED_BYTE,  3, 1, Normalization::Normalized,     true}, // Vec3UInt8
        {GL_RGB,          GL_RGB16,        GL_UNSIGNED_SHORT, 3, 2, Normalization::Normalized,     true}, // Vec3UInt16
        {GL_RGB_INTEGER,  GL_RGB32UI,      GL_UNSIGNED_INT,   3, 4, Normalization::None,           true}, // Vec3UInt32
        {},                                                                                               // Vec4UInt64
        // 4 channels
        {GL_RGBA,         GL_RGBA16F,      GL_HALF_FLOAT,     4, 2, Normalization::None,           true}, // Vec4Float16
        {GL_RGBA,         GL_RGBA32F,      GL_FLOAT,          4, 4, Normalization::None,           true}, // Vec4Float32
        {},                                                                                               // Vec4Float64
        {GL_RGBA,         GL_RGBA8_SNORM,  GL_BYTE,           4, 1, Normalization::SignNormalized, true}, // Vec4Int8
        {GL_RGBA,         GL_RGBA16_SNORM, GL_SHORT,          4, 2, Normalization::SignNormalized, true}, // Vec4Int16
        {GL_RGBA_INTEGER, GL_RGBA32I,      GL_INT,            4, 4, Normalization::None,           true}, // Vec4Int32
        {},                                                                                               // Vec4Int64
        {GL_RGBA,         GL_RGBA8,        GL_UNSIGNED_BYTE,  4, 1, Normalization::Normalized,     true}, // Vec4UInt8
        {GL_RGBA,         GL_RGBA16,       GL_UNSIGNED_SHORT, 4, 2, Normalization::Normalized,     true}, // Vec4UInt16
        {GL_RGBA_INTEGER, GL_RGBA32UI,     GL_UNSIGNED_INT,   4, 4, Normalization::None,           true}, // Vec4UInt32
        {}                                                                                                // Vec4UInt64
        // clang-format on
    }};
};

}  // namespace inviwo

#endif
