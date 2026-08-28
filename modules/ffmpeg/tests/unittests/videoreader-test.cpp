/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/ffmpeg/ffmpegvideoreader.h>
#include <inviwo/ffmpeg/video.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/formats.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace inviwo {

namespace {

constexpr int width = 64;
constexpr int height = 48;
constexpr int frames = 6;
constexpr int frameRate = 10;

constexpr uint8_t topRow = 250;
constexpr uint8_t bottomRow = 5;

/// Luma of the interior of frame @p index
constexpr uint8_t lumaOf(int index) { return static_cast<uint8_t>(20 + 20 * index); }

/**
 * Write an uncompressed YUV4MPEG2 file. Each frame is filled with lumaOf(index), except for the
 * first and last row of the frame which get distinct values so that the vertical flip can be
 * verified. @p colorspace is "mono" for grayscale or "420" for color.
 */
std::filesystem::path writeVideo(const std::filesystem::path& path,
                                 std::string_view colorspace = "mono") {
    std::ofstream file{path, std::ios::binary};
    file << "YUV4MPEG2 W" << width << " H" << height << " F" << frameRate << ":1 Ip A1:1 C"
         << colorspace << "\n";

    for (int i = 0; i < frames; ++i) {
        file << "FRAME\n";
        std::vector<uint8_t> luma(static_cast<size_t>(width) * height, lumaOf(i));
        std::fill_n(luma.begin(), width, topRow);  // row 0 of a y4m frame is the top row
        std::fill_n(luma.end() - width, width, bottomRow);
        file.write(reinterpret_cast<const char*>(luma.data()),
                   static_cast<std::streamsize>(luma.size()));

        if (colorspace == "420") {
            const std::vector<uint8_t> chroma(static_cast<size_t>(width / 2) * (height / 2), 128);
            file.write(reinterpret_cast<const char*>(chroma.data()),
                       static_cast<std::streamsize>(chroma.size()));
            file.write(reinterpret_cast<const char*>(chroma.data()),
                       static_cast<std::streamsize>(chroma.size()));
        }
    }
    return path;
}

/// Fixture creating one grayscale and one color video in a temporary directory
class VideoReader : public ::testing::Test {
protected:
    void SetUp() override {
        dir = std::filesystem::temp_directory_path() / "inviwo-ffmpeg-unittest";
        std::filesystem::create_directories(dir);
        gray = writeVideo(dir / "gray.y4m", "mono");
        color = writeVideo(dir / "color.y4m", "420");
    }
    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }

    static double valueAt(const Layer& layer, size2_t pos) {
        return layer.getRepresentation<LayerRAM>()->getAsDVec4(pos).x;
    }
    static double center(const Layer& layer) {
        return valueAt(layer, size2_t{width / 2, height / 2});
    }

    std::filesystem::path dir;
    std::filesystem::path gray;
    std::filesystem::path color;
};

}  // namespace

TEST_F(VideoReader, GrayscaleFirstFrame) {
    FFmpegLayerReader reader;
    auto layer = reader.readData(gray);

    ASSERT_TRUE(layer);
    EXPECT_EQ(layer->getDimensions(), size2_t(width, height));
    EXPECT_EQ(layer->getDataFormat(), DataUInt8::get());
    EXPECT_EQ(layer->getLayerType(), LayerType::Color);
    EXPECT_DOUBLE_EQ(center(*layer), lumaOf(0));
}

TEST_F(VideoReader, LayersAreFlippedVertically) {
    FFmpegLayerReader reader;
    auto layer = reader.readData(gray);

    // Layers have their origin in the lower left, the first row of a y4m frame is its top row
    EXPECT_DOUBLE_EQ(valueAt(*layer, size2_t{0, 0}), bottomRow);
    EXPECT_DOUBLE_EQ(valueAt(*layer, size2_t{0, height - 1}), topRow);
}

TEST_F(VideoReader, ColorFramesAreRGBA) {
    FFmpegLayerReader reader;
    auto layer = reader.readData(color);

    ASSERT_TRUE(layer);
    EXPECT_EQ(layer->getDataFormat(), DataVec4UInt8::get());
    const auto pixel = layer->getRepresentation<LayerRAM>()->getAsDVec4(size2_t{0, 0});
    EXPECT_DOUBLE_EQ(pixel.w, 255.0);
    EXPECT_LT(pixel.x, layer->getRepresentation<LayerRAM>()->getAsDVec4({0, height - 1}).x);
}

TEST_F(VideoReader, IndexOption) {
    FFmpegLayerReader reader;
    for (auto index : {std::ptrdiff_t{3}, std::ptrdiff_t{1}, std::ptrdiff_t{5}}) {
        ASSERT_TRUE(reader.setOption(reader::option::index, index));
        EXPECT_DOUBLE_EQ(center(*reader.readData(gray)), lumaOf(static_cast<int>(index)));
    }
}

TEST_F(VideoReader, NegativeIndexCountsFromTheEnd) {
    FFmpegLayerReader reader;
    ASSERT_TRUE(reader.setOption(reader::option::index, std::ptrdiff_t{-1}));
    EXPECT_DOUBLE_EQ(center(*reader.readData(gray)), lumaOf(frames - 1));
}

TEST_F(VideoReader, TimeOption) {
    FFmpegLayerReader reader;
    ASSERT_TRUE(reader.setOption(videoreader::option::time, 0.2));
    EXPECT_DOUBLE_EQ(center(*reader.readData(gray)), lumaOf(2));
}

TEST_F(VideoReader, GetOptionReflectsSetOption) {
    FFmpegLayerReader reader;
    EXPECT_EQ(std::any_cast<std::ptrdiff_t>(reader.getOption(reader::option::index)), 0);
    reader.setOption(reader::option::index, std::ptrdiff_t{4});
    EXPECT_EQ(std::any_cast<std::ptrdiff_t>(reader.getOption(reader::option::index)), 4);
    EXPECT_FALSE(reader.setOption("nonexistent", 1));
}

TEST_F(VideoReader, IndexOutOfRangeThrows) {
    FFmpegLayerReader reader;
    reader.setOption(reader::option::index, std::ptrdiff_t{frames + 10});
    EXPECT_THROW(reader.readData(gray), DataReaderException);
}

TEST_F(VideoReader, SequenceReadsAllFrames) {
    FFmpegLayerSequenceReader reader;
    auto sequence = reader.readData(gray);

    ASSERT_TRUE(sequence);
    ASSERT_EQ(sequence->size(), frames);
    for (int i = 0; i < frames; ++i) {
        EXPECT_DOUBLE_EQ(center(*(*sequence)[i]), lumaOf(i)) << "frame " << i;
    }
}

TEST_F(VideoReader, SequenceIndexCountAndStride) {
    FFmpegLayerSequenceReader reader;
    ASSERT_TRUE(reader.setOption(reader::option::index, std::ptrdiff_t{1}));
    ASSERT_TRUE(reader.setOption(reader::option::count, std::ptrdiff_t{2}));
    ASSERT_TRUE(reader.setOption(reader::option::stride, std::ptrdiff_t{2}));

    auto sequence = reader.readData(gray);
    ASSERT_EQ(sequence->size(), 2);
    EXPECT_DOUBLE_EQ(center(*(*sequence)[0]), lumaOf(1));
    EXPECT_DOUBLE_EQ(center(*(*sequence)[1]), lumaOf(3));
}

TEST_F(VideoReader, SequenceCountIsClampedToTheVideoLength) {
    FFmpegLayerSequenceReader reader;
    reader.setOption(reader::option::index, std::ptrdiff_t{4});
    reader.setOption(reader::option::count, std::ptrdiff_t{-1});

    auto sequence = reader.readData(gray);
    EXPECT_EQ(sequence->size(), 2);
}

TEST_F(VideoReader, VideoInfo) {
    ffmpeg::Video video{gray};
    const auto& info = video.info();

    EXPECT_EQ(info.dimensions, size2_t(width, height));
    EXPECT_EQ(info.format, DataUInt8::get());
    EXPECT_DOUBLE_EQ(info.frameRate, frameRate);
    EXPECT_EQ(info.frames, frames);
    EXPECT_EQ(video.frameAt(0.3), 3);
}

TEST_F(VideoReader, ReadNextFrameEndsWithNullptr) {
    ffmpeg::Video video{gray};
    for (int i = 0; i < frames; ++i) {
        auto layer = video.readNextFrame();
        ASSERT_TRUE(layer) << "frame " << i;
        EXPECT_DOUBLE_EQ(center(*layer), lumaOf(i));
    }
    EXPECT_FALSE(video.readNextFrame());
}

TEST_F(VideoReader, MatchingDestinationLayerIsReused) {
    ffmpeg::Video video{gray};

    auto first = video.readFrame(0);
    auto second = video.readFrame(1, first);
    EXPECT_EQ(first.get(), second.get());
    EXPECT_DOUBLE_EQ(center(*second), lumaOf(1));

    auto mismatched = std::make_shared<Layer>(size2_t{4, 4}, DataUInt8::get());
    auto third = video.readFrame(2, mismatched);
    EXPECT_NE(third.get(), mismatched.get());
    EXPECT_EQ(third->getDimensions(), size2_t(width, height));
}

}  // namespace inviwo
