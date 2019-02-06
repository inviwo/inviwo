/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <inviwo/png/pngwriter.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <png.h>

namespace inviwo {

namespace detail {

void writeToBuffer(png_structp png_ptr, png_bytep data, png_size_t length) {
    auto buffer = static_cast<std::vector<unsigned char>*>(png_get_io_ptr(png_ptr));
    for (png_size_t i = 0; i < length; i++) {
        buffer->push_back(static_cast<unsigned char>(data[i]));
    }
}

template <typename T>
void write(const LayerRAMPrecision<T>* ram, png_voidp ioPtr, png_rw_ptr writeFunc = nullptr,
           png_flush_ptr flushFunc = nullptr) {

    // TODO better exception messages
    auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        throw PNGLayerWriterException("Internal PNG Error: Failed to create write struct");
    }
    util::OnScopeExit cleanup([&]() { png_destroy_write_struct(&png_ptr, nullptr); });

    png_set_error_fn(
        png_ptr, nullptr,
        [](png_structp, png_const_charp message) {
            throw PNGLayerWriterException(std::string("Error witting PNG: ") + message);
        },
        [](png_structp, png_const_charp message) { LogWarnCustom("PNGWriter", message); });

    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        throw PNGLayerWriterException("Internal PNG Error: Failed to create info struct");
    }
    cleanup.setAction([&]() { png_destroy_write_struct(&png_ptr, &info_ptr); });

    png_set_write_fn(png_ptr, ioPtr, writeFunc, flushFunc);

    const auto df = ram->getDataFormat();
    const auto color_type = [&]() {
        switch (df->getComponents()) {
            case 1:
                return PNG_COLOR_TYPE_GRAY;
            case 2:
                return PNG_COLOR_TYPE_GRAY_ALPHA;
            case 3:
                return PNG_COLOR_TYPE_RGB;
            case 4:
                return PNG_COLOR_TYPE_RGBA;
            default:
                // Should not ever reach this
                throw new PNGLayerWriterException("Unsupported number of channels");
        }
    }();

    const auto size = ram->getDimensions();
    const auto bit_depth = df->getPrecision();

    if (df->getNumericType() == NumericType::Float) {
        png_set_IHDR(png_ptr, info_ptr, static_cast<int>(size.x), static_cast<int>(size.y), 16,
                     color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);
        png_set_swap(png_ptr);

        const auto pixels = ram->getDataTyped();

        using T2 = typename util::same_extent<T, glm::uint16>::type;
        std::vector<T2> newData(size.x * size.y);
        for (size_t i = 0; i < size.x * size.y; i++) {
            newData[i] = util::glm_convert_normalized<T2>(glm::clamp(pixels[i], T{0}, T{1}));
        }

        std::vector<png_bytep> rows(size.y);
        for (png_uint_32 r = 0; r < size.y; ++r) {
            rows[size.y - r - 1] = reinterpret_cast<png_bytep>(newData.data() + r * size.x);
        }

        png_write_image(png_ptr, rows.data());

    } else {
        png_set_IHDR(png_ptr, info_ptr, static_cast<int>(size.x), static_cast<int>(size.y),
                     static_cast<int>(bit_depth), color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);
        png_set_swap(png_ptr);

        auto pixels = const_cast<T*>(ram->getDataTyped());
        std::vector<png_bytep> rows(size.y);
        for (png_uint_32 r = 0; r < size.y; ++r) {
            // Inviwo images are upside down compared to how libpng expects them
            rows[size.y - r - 1] = reinterpret_cast<png_bytep>(pixels + r * size.x);
        }

        png_write_image(png_ptr, rows.data());
    }

    png_write_end(png_ptr, NULL);
}

}  // namespace detail

PNGLayerWriterException::PNGLayerWriterException(const std::string& message,
                                                 ExceptionContext context)
    : Exception(message, context) {}

PNGLayerWriter::PNGLayerWriter() : DataWriterType<Layer>() {
    addExtension(FileExtension("png", "Portable Network Graphics"));
}

PNGLayerWriter* PNGLayerWriter::clone() const { return new PNGLayerWriter(*this); }

void PNGLayerWriter::writeData(const Layer* data, const std::string filePath) const {
    data->getRepresentation<LayerRAM>()->dispatch<void>([&](auto ram) {
        FILE* fp = fopen(filePath.c_str(), "wb");
        if (!fp) throw PNGLayerWriterException("Failed to open file for writing, " + filePath);
        util::OnScopeExit closeFile([&fp]() { fclose(fp); });

        detail::write(ram, static_cast<png_voidp>(fp));
    });
}

std::unique_ptr<std::vector<unsigned char>> PNGLayerWriter::writeDataToBuffer(
    const Layer* data, const std::string&) const {

    auto buffer = std::make_unique<std::vector<unsigned char>>();
    data->getRepresentation<LayerRAM>()->dispatch<void>([&](auto ram) {
        detail::write(ram, static_cast<png_voidp>(buffer.get()), &detail::writeToBuffer);
    });

    return buffer;
}

bool PNGLayerWriter::writeDataToRepresentation(const repr*, repr*) const { return false; }

}  // namespace inviwo
