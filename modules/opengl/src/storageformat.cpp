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

#include <modules/opengl/storageformat.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/openglexception.h>
#include <modules/opengl/texture/texture.h>
#include <modules/opengl/volume/volumegl.h>

#include <algorithm>
#include <array>
#include <span>
#include <string>

namespace inviwo::utilgl {

namespace {

constexpr std::array<StorageFormat, 5> storageFormats{
    StorageFormat::Default, StorageFormat::Float32, StorageFormat::Float16, StorageFormat::UNorm8,
    StorageFormat::SNorm8};

constexpr std::array<GLenum, 4> float16InternalFormats{GL_R16F, GL_RG16F, GL_RGB16F, GL_RGBA16F};
constexpr std::array<std::string_view, 4> float16LayoutQualifiers{"r16f", "rg16f", "", "rgba16f"};

std::optional<TextureStorage> lossyTextureStorage(const DataFormatBase* format,
                                                  StorageFormat storage) {
    const auto comp = format->getComponents();
    const auto type = format->getNumericType();
    const auto precision = format->getPrecision();
    if (comp < 1 || comp > 4) return std::nullopt;

    // The format with the internal format replaced by the one of @p storageId
    const auto withStorageOf = [&](const DataFormatBase* client, DataFormatId storageId) {
        auto glFormat = GLFormats::get(client->getId());
        const auto& storageFormat = GLFormats::get(storageId);
        glFormat.internalFormat = storageFormat.internalFormat;
        glFormat.layoutQualifier = storageFormat.layoutQualifier;
        return TextureStorage{client, glFormat, true};
    };

    switch (storage) {
        case StorageFormat::Float32:
            if (type == NumericType::Float && precision == 64) {
                const auto* f32 = DataFormatBase::get(NumericType::Float, comp, 32);
                return TextureStorage{f32, GLFormats::get(f32->getId()), true};
            }
            break;
        case StorageFormat::Float16:
            if (type == NumericType::Float && precision >= 32) {
                const auto* f32 = DataFormatBase::get(NumericType::Float, comp, 32);
                auto glFormat = GLFormats::get(f32->getId());
                glFormat.internalFormat = float16InternalFormats[comp - 1];
                glFormat.layoutQualifier = float16LayoutQualifiers[comp - 1];
                return TextureStorage{f32, glFormat, true};
            }
            break;
        case StorageFormat::UNorm8:
            if (type == NumericType::UnsignedInteger && precision == 16) {
                return withStorageOf(
                    format, DataFormatBase::get(NumericType::UnsignedInteger, comp, 8)->getId());
            }
            break;
        case StorageFormat::SNorm8:
            if (type == NumericType::SignedInteger && precision == 16) {
                return withStorageOf(
                    format, DataFormatBase::get(NumericType::SignedInteger, comp, 8)->getId());
            }
            break;
        case StorageFormat::Default: break;
    }
    return std::nullopt;
}

template <typename RAM, typename GL, typename D>
void setStorageFormatImpl(D& data, StorageFormat format) {
    if (getStorageFormat(&data) == format) return;

    if (format == StorageFormat::Default) {
        data.template unsetMetaData<StringMetaData>(storageFormatMetaDataKey);
    } else {
        data.template setMetaData<StringMetaData>(storageFormatMetaDataKey,
                                                  std::string{enumToStr(format)});
    }

    if (data.template hasRepresentation<GL>()) {
        // Make sure we have an up to date RAM representation before removing the GL one
        const auto* ram = data.template getRepresentation<RAM>();
        data.removeOtherRepresentations(ram);
    }
}

}  // namespace

std::string_view enumToStr(StorageFormat format) {
    switch (format) {
        case StorageFormat::Default: return "Default";
        case StorageFormat::Float32: return "Float32";
        case StorageFormat::Float16: return "Float16";
        case StorageFormat::UNorm8:  return "UNorm8";
        case StorageFormat::SNorm8:  return "SNorm8";
    }
    return "Default";
}

StorageFormat getStorageFormat(const MetaDataOwner* owner) {
    if (!owner) return StorageFormat::Default;
    if (const auto* meta = owner->getMetaData<StringMetaData>(storageFormatMetaDataKey)) {
        const auto it = std::ranges::find(storageFormats, std::string_view{meta->get()},
                                          [](StorageFormat f) { return enumToStr(f); });
        if (it != storageFormats.end()) {
            return *it;
        }
        log::warn("Unknown GL storage format '{}'", meta->get());
    }
    return StorageFormat::Default;
}

void setStorageFormat(Volume& volume, StorageFormat format) {
    setStorageFormatImpl<VolumeRAM, VolumeGL>(volume, format);
}
void setStorageFormat(Layer& layer, StorageFormat format) {
    setStorageFormatImpl<LayerRAM, LayerGL>(layer, format);
}

TextureStorage textureStorage(const DataFormatBase* format, StorageFormat storage) {
    if (storage != StorageFormat::Default) {
        if (auto lossy = lossyTextureStorage(format, storage)) {
            return *lossy;
        }
        log::warn("Unsupported GL storage format '{}' for data format '{}', using default",
                  enumToStr(storage), *format);
    }
    try {
        return TextureStorage{format, GLFormats::get(format->getId()), false};
    } catch (const OpenGLException&) {
        throw ConverterException(SourceContext{},
                                 "No GL texture format for '{}', a lossy storage format can be "
                                 "requested using utilgl::setStorageFormat",
                                 *format);
    }
}

GLFormat getGLFormat(const Texture& texture) {
    const auto* format = texture.getDataFormat();
    const auto& glFormat = GLFormats::get(format->getId());
    if (glFormat.internalFormat != texture.getInternalFormat()) {
        for (const auto storage : storageFormats) {
            const auto lossy = lossyTextureStorage(format, storage);
            if (lossy && lossy->glFormat.internalFormat == texture.getInternalFormat()) {
                return lossy->glFormat;
            }
        }
    }
    return glFormat;
}

bool isLossy(const DataFormatBase* format, const Texture& texture) {
    if (texture.getDataFormat() != format) return true;
    return std::ranges::any_of(storageFormats, [&](StorageFormat storage) {
        const auto lossy = lossyTextureStorage(format, storage);
        return lossy && lossy->glFormat.internalFormat == texture.getInternalFormat();
    });
}

void convertFloatPrecision(const void* src, const DataFormatBase* srcFormat, void* dst,
                           const DataFormatBase* dstFormat, size_t count) {
    if (srcFormat->getComponents() != dstFormat->getComponents() ||
        srcFormat->getNumericType() != NumericType::Float ||
        dstFormat->getNumericType() != NumericType::Float) {
        throw ConverterException(SourceContext{}, "Unsupported data conversion from {} to {}",
                                 *srcFormat, *dstFormat);
    }

    const auto convert = [&]<typename From, typename To>() {
        const auto values = count * srcFormat->getComponents();
        std::ranges::transform(std::span{static_cast<const From*>(src), values},
                               static_cast<To*>(dst), [](From v) { return static_cast<To>(v); });
    };

    if (srcFormat->getPrecision() == 64 && dstFormat->getPrecision() == 32) {
        convert.template operator()<double, float>();
    } else if (srcFormat->getPrecision() == 32 && dstFormat->getPrecision() == 64) {
        convert.template operator()<float, double>();
    } else {
        throw ConverterException(SourceContext{}, "Unsupported data conversion from {} to {}",
                                 *srcFormat, *dstFormat);
    }
}

}  // namespace inviwo::utilgl
