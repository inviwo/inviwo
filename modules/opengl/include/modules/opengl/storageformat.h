/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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
#include <modules/opengl/glformats.h>

#include <inviwo/core/util/glm.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace inviwo {

class DataFormatBase;
class MetaDataOwner;
class Volume;
class Layer;
class Texture;

namespace utilgl {

/**
 * The requested storage format of the OpenGL representation of a Volume or Layer.
 *
 * By default, the GL representation uses the same format as the RAM representation (lossless).
 * A lower precision storage format can be requested to save GPU memory or to make data available
 * on the GPU that has no matching GL format, like Float64 textures. The GL representation is then
 * marked as lossy (see DataRepresentation::isLossy) and will never be used as the source when
 * updating other representations. The data format reported by the GL representation is still the
 * format of the source data, the texture stores the data using the reduced format.
 *
 * Supported mappings, where the number of components is preserved:
 *   Float64 -> Float32, Float64 -> Float16, Float32 -> Float16, UInt16 -> UNorm8, Int16 -> SNorm8
 *
 * The normalized integer mappings keep the normalized value, i.e. no data range is taken into
 * account. Unsupported mappings will fall back to Default and log a warning.
 */
enum class StorageFormat : std::uint8_t { Default, Float32, Float16, UNorm8, SNorm8 };

IVW_MODULE_OPENGL_API std::string_view enumToStr(StorageFormat format);

/**
 * The meta data key, a StringMetaData, used to store the requested StorageFormat
 */
constexpr std::string_view storageFormatMetaDataKey = "org.inviwo.opengl.storageFormat";

/**
 * Get the requested storage format of @p owner, returns StorageFormat::Default if none is set.
 */
IVW_MODULE_OPENGL_API StorageFormat getStorageFormat(const MetaDataOwner* owner);

/**
 * Request a storage format for the GL representation. Any existing GL representations will be
 * removed and recreated with the new storage format on demand. If the GL representation is the
 * only up to date representation, it is first downloaded to a RAM representation.
 * @note Setting the meta data directly will only affect GL representations created afterwards.
 */
IVW_MODULE_OPENGL_API void setStorageFormat(Volume& volume, StorageFormat format);
IVW_MODULE_OPENGL_API void setStorageFormat(Layer& layer, StorageFormat format);

/**
 * Describes how data of a specific format is stored in a texture
 */
struct IVW_MODULE_OPENGL_API TextureStorage {
    /// The format of the data transferred to and from the texture (client side)
    const DataFormatBase* format;
    /// GL format, type, and internal format of the texture
    GLFormat glFormat;
    /// True if the internal format can not represent @p format without loss of precision
    bool lossy;
};

/**
 * Get the texture storage for data of @p format and requested @p storage. Falls back to
 * StorageFormat::Default for unsupported mappings.
 * @throws ConverterException if there is no GL format for @p format
 */
IVW_MODULE_OPENGL_API TextureStorage textureStorage(const DataFormatBase* format,
                                                    StorageFormat storage);

/**
 * Get the GL format of @p texture. Format and type match the client data format of the texture
 * (Texture::getDataFormat), while the internal format and layout qualifier match the storage of
 * the texture, if it is one of the lossy storage formats.
 */
IVW_MODULE_OPENGL_API GLFormat getGLFormat(const Texture& texture);

/**
 * Check if @p texture stores data of @p format in a lossy way
 */
IVW_MODULE_OPENGL_API bool isLossy(const DataFormatBase* format, const Texture& texture);

/**
 * Convert @p count elements between Float64 and Float32 formats with the same number of
 * components. OpenGL can not transfer double data to or from textures (GL_DOUBLE is not a valid
 * pixel transfer type), so Float64 data has to be converted on the CPU. All other storage format
 * conversions are done by OpenGL during upload and download.
 * @throws ConverterException for any other combination of formats
 */
IVW_MODULE_OPENGL_API void convertFloatPrecision(const void* src, const DataFormatBase* srcFormat,
                                                 void* dst, const DataFormatBase* dstFormat,
                                                 size_t count);

/**
 * Upload the data of the RAM representation @p src to @p texture. OpenGL converts the data to the
 * internal format of the texture, except for Float64 data which is converted to the Float32 client
 * format of the texture on the CPU, see convertFloatPrecision.
 */
template <typename RAM, typename Tex>
void uploadToTexture(const RAM& src, Tex& texture, bool initialize) {
    const auto* textureFormat = texture.getDataFormat();
    const void* data = src.getData();
    std::vector<std::byte> converted;
    if (textureFormat != src.getDataFormat()) {
        const auto count = glm::compMul(src.getDimensions());
        converted.resize(count * textureFormat->getSizeInBytes());
        convertFloatPrecision(data, src.getDataFormat(), converted.data(), textureFormat, count);
        data = converted.data();
    }
    if (initialize) {
        texture.initialize(data);
    } else {
        texture.upload(data);
    }
}

/**
 * Download the data of @p texture to the RAM representation @p dst. OpenGL converts the data from
 * the internal format of the texture, except for Float64 data which is converted from the Float32
 * client format of the texture on the CPU, see convertFloatPrecision.
 */
template <typename Tex, typename RAM>
void downloadFromTexture(const Tex& texture, RAM& dst) {
    const auto* textureFormat = texture.getDataFormat();
    if (textureFormat != dst.getDataFormat()) {
        const auto count = glm::compMul(dst.getDimensions());
        std::vector<std::byte> data(count * textureFormat->getSizeInBytes());
        texture.download(data.data());
        convertFloatPrecision(data.data(), textureFormat, dst.getData(), dst.getDataFormat(),
                              count);
    } else {
        texture.download(dst.getData());
    }
}

}  // namespace utilgl

}  // namespace inviwo
