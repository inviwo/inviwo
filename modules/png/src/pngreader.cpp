/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/png/pngreader.h>

#include <inviwo/core/datastructures/image/imagetypes.h>                // for SwizzleMask, lumi...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer, DataReader...
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/filesystem.h>                                // for fileExists, fopen
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for size2_t
#include <inviwo/core/util/logcentral.h>                                // for LogCentral, LogWa...
#include <inviwo/core/util/raiiutils.h>                                 // for OnScopeExit, OnSc...
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT, IVW_...

#include <functional>     // for __base
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <png.h>      // for png_structp, png_...
#include <pngconf.h>  // for png_bytep, png_co...

#include <fmt/std.h>

namespace inviwo {

PNGLayerReader::PNGLayerReader() : DataReaderType<Layer>() {
    addExtension(FileExtension("png", "Portable Network Graphics"));
}

PNGLayerReader* PNGLayerReader::clone() const { return new PNGLayerReader(*this); }

std::shared_ptr<inviwo::Layer> PNGLayerReader::readData(const std::filesystem::path& filePath) {
    checkExists(filePath);

    auto* fp = filesystem::fopen(filePath, "rb");
    if (!fp) {
        throw DataReaderException(IVW_CONTEXT, "Failed to open file for reading, {}", filePath);
    }
    util::OnScopeExit closeFile([fp]() { fclose(fp); });

    return readData(fp, filePath.string());
}

std::shared_ptr<inviwo::Layer> PNGLayerReader::readData(FILE* fp, std::string_view name) {

    unsigned char header[8];
    const auto read = fread(header, 1, 8, fp);
    if (read != 8 || png_sig_cmp(header, 0, 8)) {
        throw DataReaderException(IVW_CONTEXT, "File is not a PNG, {}", name);
    }

    auto png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        throw DataReaderException(IVW_CONTEXT,
                                  "Internal PNG Error: Failed to create read struct: {}", name);
    }
    util::OnScopeExit cleanup1([&]() { png_destroy_read_struct(&png_ptr, NULL, NULL); });
    png_set_error_fn(
        png_ptr, NULL,
        [](png_structp, png_const_charp message) {
            throw DataReaderException(IVW_CONTEXT_CUSTOM("PNGLayerReader::readData"),
                                      "Error reading PNG: {}", message);
        },
        [](png_structp, png_const_charp message) { log::report(LogLevel::Warn, message); });

    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        throw DataReaderException(IVW_CONTEXT,
                                  "Internal PNG Error: Failed to create info struct {}", name);
    }
    util::OnScopeExit cleanup2([&]() { png_destroy_read_struct(NULL, &info_ptr, NULL); });

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    auto width = png_get_image_width(png_ptr, info_ptr);
    auto height = png_get_image_height(png_ptr, info_ptr);
    auto color_type = png_get_color_type(png_ptr, info_ptr);
    auto bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    SwizzleMask swizzleMask;
    size_t channels;
    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            swizzleMask = swizzlemasks::luminanceAlpha;
            channels = 2;
            break;
        case PNG_COLOR_TYPE_GRAY:
            swizzleMask = swizzlemasks::luminance;
            channels = 1;
            break;
        case PNG_COLOR_TYPE_PALETTE:
            [[fallthrough]];
        case PNG_COLOR_TYPE_RGB:
            swizzleMask = swizzlemasks::rgb;
            channels = 3;
            break;
        case PNG_COLOR_TYPE_RGBA:
            swizzleMask = swizzlemasks::rgba;
            channels = 4;
            break;
        default:
            throw DataReaderException(IVW_CONTEXT, "PNG has unsupported color type {}", name);
            break;
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        if (bit_depth <= 8) {
            png_set_expand(png_ptr);
            bit_depth = 8;
        } else {
            png_set_expand_16(png_ptr);
        }
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
        bit_depth = 8;
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
        channels++;
    }

    if (bit_depth == 16) {
        png_set_expand_16(png_ptr);
    }

    png_set_swap(png_ptr);

    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    const auto dimensions = size2_t(width, height);
    auto layer = std::make_shared<Layer>(LayerConfig{
        .dimensions = dimensions,
        .format = DataFormatBase::get(inviwo::NumericType::UnsignedInteger, channels, bit_depth),
        .swizzleMask = swizzleMask,
        .model = LayerConfig::aspectPreservingModelMatrixFromDimensions(dimensions)});

    layer->getEditableRepresentation<LayerRAM>()->dispatch<void>([&](auto ram) {
        auto data = ram->getDataTyped();

        std::vector<png_bytep> rows(height);

        for (png_uint_32 rownum = 0; rownum < height; ++rownum) {
            // Need to flip images in Inviwo
            rows[height - rownum - 1] = reinterpret_cast<png_bytep>(data + rownum * width);
        }

        png_read_image(png_ptr, rows.data());
    });

    return layer;
}

}  // namespace inviwo
