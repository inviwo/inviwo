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

#include <modules/hdf5/datastructures/layerhdf.h>
#include <modules/hdf5/datastructures/layerhdfconverter.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/datastructures/hdf5selection.h>
#include <modules/hdf5/hdf5read.h>

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formats.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <H5Cpp.h>
#include <warn/pop>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace inviwo::hdf5 {

namespace {

constexpr hsize_t nx = 5;
constexpr hsize_t ny = 3;
constexpr hsize_t nz = 2;

constexpr size_t index(size_t x, size_t y) { return x + nx * y; }

// Writes "/data/image" with HDF (row major) shape [nz, ny, nx] where the pixel (x, y) in slice z
// has the value z * 100 + index(x, y).
void writeFile(const std::filesystem::path& path) {
    const H5::H5File file(path.generic_string(), H5F_ACC_TRUNC);
    const auto group = file.createGroup("/data");
    const std::array<hsize_t, 3> dims{nz, ny, nx};
    const H5::DataSpace space(3, dims.data());
    const auto dataset = group.createDataSet("image", H5::PredType::NATIVE_UINT16, space);

    std::vector<std::uint16_t> data(nz * ny * nx);
    for (size_t z = 0; z < nz; ++z) {
        for (size_t y = 0; y < ny; ++y) {
            for (size_t x = 0; x < nx; ++x) {
                data[z * ny * nx + index(x, y)] = static_cast<std::uint16_t>(z * 100 + index(x, y));
            }
        }
    }
    dataset.write(data.data(), H5::PredType::NATIVE_UINT16);
}

// Column major selection [x, y, z] selecting slice @p z
std::vector<Selection> selectSlice(size_t z) {
    return {Selection{0, 0, 1}, Selection{0, 0, 1}, Selection{z, 1, 1}};
}

class LayerHDFTest : public ::testing::Test {
protected:
    void SetUp() override {
        dir = std::filesystem::temp_directory_path() / "inviwo-hdf5-layerhdf-unittest";
        std::filesystem::create_directories(dir);
        file = dir / "layer.h5";
        writeFile(file);
    }
    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }

    Handle handle() const { return Handle{file} + Path{"/data/image"}; }

    std::filesystem::path dir;
    std::filesystem::path file;
};

}  // namespace

TEST_F(LayerHDFTest, ResolvesDimensionsAndFormatFromDataset) {
    const LayerHDF repr{handle(), selectSlice(0)};

    EXPECT_EQ(repr.getDimensions(), size2_t(nx, ny));
    EXPECT_EQ(repr.getDataFormatId(), DataFormatId::UInt16);
    EXPECT_EQ(repr.getLayerType(), LayerType::Color);
    EXPECT_EQ(repr.getSizeInBytes(DataFloat32::get()), nx * ny * sizeof(float));
}

TEST_F(LayerHDFTest, HonorsConfig) {
    const LayerHDF repr{handle(), selectSlice(0),
                        LayerReprConfig{.formatId = DataFormatId::Float32,
                                        .type = LayerType::Depth,
                                        .interpolation = InterpolationType::Nearest,
                                        .wrapping = wrapping2d::repeatAll}};

    EXPECT_EQ(repr.getDataFormatId(), DataFormatId::Float32);
    EXPECT_EQ(repr.getLayerType(), LayerType::Depth);
    EXPECT_EQ(repr.getInterpolation(), InterpolationType::Nearest);
    EXPECT_EQ(repr.getWrapping(), wrapping2d::repeatAll);
}

TEST_F(LayerHDFTest, IsReadOnly) {
    LayerHDF repr{handle(), selectSlice(0)};
    EXPECT_THROW(repr.setDimensions(size2_t{1, 1}), Exception);
    EXPECT_FALSE(repr.copyRepresentationsTo(nullptr));
}

TEST_F(LayerHDFTest, RejectsSelectionOfTooManyDimensions) {
    EXPECT_THROW((LayerHDF{handle(), std::vector<Selection>(3, Selection{0, 0, 1})}), Exception);
}

TEST_F(LayerHDFTest, ConvertsToLayerRAMWithFormatConversion) {
    Layer layer{std::make_shared<LayerHDF>(handle(), selectSlice(1),
                                           LayerReprConfig{.formatId = DataFormatId::Float32})};

    EXPECT_TRUE(layer.hasRepresentation<LayerHDF>());
    EXPECT_FALSE(layer.hasRepresentation<LayerRAM>());
    EXPECT_EQ(layer.getDimensions(), size2_t(nx, ny));

    const auto* ram = layer.getRepresentation<LayerRAM>();
    ASSERT_TRUE(ram);
    EXPECT_EQ(ram->getDataFormatId(), DataFormatId::Float32);
    EXPECT_EQ(ram->getDimensions(), size2_t(nx, ny));
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size2_t{0, 0}), 100.0);
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size2_t{4, 2}), 100.0 + index(4, 2));
}

TEST_F(LayerHDFTest, UpdatesExistingLayerRAM) {
    Layer layer{std::make_shared<LayerHDF>(handle(), selectSlice(0))};
    const auto* ram = layer.getRepresentation<LayerRAM>();
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size2_t{2, 1}), static_cast<double>(index(2, 1)));

    auto hdf = std::make_shared<LayerHDF>(handle(), selectSlice(1));
    layer.addRepresentation(hdf);
    layer.invalidateAllOther(hdf.get());

    const auto* updated = layer.getRepresentation<LayerRAM>();
    EXPECT_EQ(updated, ram);
    EXPECT_DOUBLE_EQ(updated->getAsDouble(size2_t{2, 1}), 100.0 + index(2, 1));
}

TEST_F(LayerHDFTest, LazyGetLayerAtPathCreatesHDFRepresentation) {
    auto layer = getLayerAtPathAsType(handle(), selectSlice(1), LayerConfig{}, Loading::Lazy);

    ASSERT_TRUE(layer);
    EXPECT_TRUE(layer->hasRepresentation<LayerHDF>());
    EXPECT_FALSE(layer->hasRepresentation<LayerRAM>());
    EXPECT_EQ(layer->getDimensions(), size2_t(nx, ny));
    EXPECT_EQ(layer->getDataFormatId(), DataFormatId::UInt16);
    EXPECT_DOUBLE_EQ(layer->getRepresentation<LayerRAM>()->getAsDouble(size2_t{1, 2}),
                     100.0 + index(1, 2));
}

TEST_F(LayerHDFTest, EagerGetLayerAtPathCreatesRAMRepresentation) {
    auto layer = getLayerAtPathAsType(handle(), selectSlice(0), LayerConfig{}, Loading::Eager);

    ASSERT_TRUE(layer);
    EXPECT_FALSE(layer->hasRepresentation<LayerHDF>());
    EXPECT_TRUE(layer->hasRepresentation<LayerRAM>());
    EXPECT_DOUBLE_EQ(layer->getRepresentation<LayerRAM>()->getAsDouble(size2_t{1, 2}),
                     static_cast<double>(index(1, 2)));
}

}  // namespace inviwo::hdf5
