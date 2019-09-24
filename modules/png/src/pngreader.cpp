/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <png.h>

#include <sstream>

namespace inviwo {

PNGLayerReaderException::PNGLayerReaderException(const std::string& message,
                                                 ExceptionContext context)
    : DataReaderException(message, context) {}

PNGLayerReader::PNGLayerReader() : DataReaderType<Layer>() {
    addExtension(FileExtension("png", "Portable Network Graphics"));
}

PNGLayerReader* PNGLayerReader::clone() const { return new PNGLayerReader(*this); }

std::shared_ptr<inviwo::Layer> PNGLayerReader::readData(const std::string& filePath) {
    if (!filesystem::fileExists(filePath)) throw PNGLayerReaderException(filePath);

    auto* fp = filesystem::fopen(filePath, "rb");
    if (!fp) throw PNGLayerReaderException("Failed to open file for reading, " + filePath);
    util::OnScopeExit closeFile([fp]() { fclose(fp); });

    unsigned char header[8];
    const auto read = fread(header, 1, 8, fp);
    if (read != 8 || png_sig_cmp(header, 0, 8)) {
        throw PNGLayerReaderException("File is not a PNG, " + filePath);
    }

    auto png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        throw PNGLayerReaderException("Internal PNG Error: Failed to create read struct");
    }
    util::OnScopeExit cleanup([&]() { png_destroy_read_struct(&png_ptr, NULL, NULL); });
    png_set_error_fn(
        png_ptr, NULL,
        [](png_structp, png_const_charp message) {
            throw PNGLayerReaderException(std::string("Error reading PNG: ") + message);
        },
        [](png_structp, png_const_charp message) { LogWarnCustom("PNGReader", message); });

    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        throw PNGLayerReaderException("Internal PNG Error: Failed to create info struct");
    }
    cleanup.setAction([&]() { png_destroy_read_struct(&png_ptr, &info_ptr, NULL); });

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
            throw PNGLayerReaderException("Unsupported color type");
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

    const DataFormatBase* df =
        DataFormatBase::get(inviwo::NumericType::UnsignedInteger, channels, bit_depth);

    auto layer = std::make_shared<Layer>(size2_t(width, height), df);

    layer->setSwizzleMask(swizzleMask);

    layer->getEditableRepresentation<LayerRAM>()->dispatch<void>([&](auto ram) {
        auto data = ram->getDataTyped();

        std::vector<png_bytep> rows(height);

        for (png_uint_32 rownum = 0; rownum < height; ++rownum) {
            // Need to flip images in Inviwo
            rows[height - rownum - 1] = (png_bytep)(data + rownum * width);
        }

        png_read_image(png_ptr, rows.data());
    });

    return layer;
}

}  // namespace inviwo
