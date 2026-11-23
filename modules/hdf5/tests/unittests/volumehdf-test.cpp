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

#include <modules/hdf5/datastructures/volumehdf.h>
#include <modules/hdf5/datastructures/volumehdfconverter.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/datastructures/hdf5selection.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/hdf5temporalvolumeloader.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/formats.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <H5Cpp.h>
#include <warn/pop>

#include <cstddef>
#include <filesystem>
#include <numeric>
#include <span>
#include <vector>

namespace inviwo::hdf5 {

namespace {

constexpr hsize_t nx = 4;
constexpr hsize_t ny = 3;
constexpr hsize_t nz = 2;
constexpr hsize_t nt = 3;

// Column major voxel index, matches the memory layout of VolumeRAM
constexpr size_t index(size_t x, size_t y, size_t z) { return x + nx * (y + ny * z); }

// Writes "/data/field" with HDF (row major) shape [nt, nz, ny, nx] where the voxel (x, y, z) at
// time step t has the value t * 1000 + index(x, y, z).
void writeFile(const std::filesystem::path& path) {
    const H5::H5File file(path.generic_string(), H5F_ACC_TRUNC);
    const auto group = file.createGroup("/data");
    const std::array<hsize_t, 4> dims{nt, nz, ny, nx};
    const H5::DataSpace space(4, dims.data());
    const auto dataset = group.createDataSet("field", H5::PredType::NATIVE_FLOAT, space);

    std::vector<float> data(nt * nz * ny * nx);
    for (size_t t = 0; t < nt; ++t) {
        for (size_t z = 0; z < nz; ++z) {
            for (size_t y = 0; y < ny; ++y) {
                for (size_t x = 0; x < nx; ++x) {
                    data[t * nz * ny * nx + index(x, y, z)] =
                        static_cast<float>(t * 1000 + index(x, y, z));
                }
            }
        }
    }
    dataset.write(data.data(), H5::PredType::NATIVE_FLOAT);
}

// Column major selection [x, y, z, t] selecting time step @p t
std::vector<Selection> selectTime(size_t t) {
    return {Selection{0, 0, 1}, Selection{0, 0, 1}, Selection{0, 0, 1}, Selection{t, 1, 1}};
}

class VolumeHDFTest : public ::testing::Test {
protected:
    void SetUp() override {
        dir = std::filesystem::temp_directory_path() / "inviwo-hdf5-volumehdf-unittest";
        std::filesystem::create_directories(dir);
        file = dir / "volume.h5";
        writeFile(file);
    }
    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }

    Handle handle() const { return Handle{file} + Path{"/data/field"}; }

    std::filesystem::path dir;
    std::filesystem::path file;
};

}  // namespace

TEST_F(VolumeHDFTest, ResolvesDimensionsAndFormatFromDataset) {
    const VolumeHDF repr{handle(), selectTime(0)};

    EXPECT_EQ(repr.getDimensions(), size3_t(nx, ny, nz));
    EXPECT_EQ(repr.getDataFormatId(), DataFormatId::Float32);
    EXPECT_EQ(repr.getSizeInBytes(DataUInt8::get()), nx * ny * nz);
    EXPECT_EQ(repr.getSizeInBytes(DataFloat32::get()), nx * ny * nz * sizeof(float));
    EXPECT_EQ(repr.getHandle().getFilename(), file);
    EXPECT_EQ(repr.getSelection().size(), 4u);
}

TEST_F(VolumeHDFTest, HonorsRequestedFormatAndSamplingConfig) {
    const VolumeHDF repr{handle(), selectTime(0),
                         VolumeReprConfig{.formatId = DataFormatId::UInt16,
                                          .swizzleMask = swizzlemasks::luminance,
                                          .interpolation = InterpolationType::Nearest,
                                          .wrapping = wrapping3d::repeatAll}};

    EXPECT_EQ(repr.getDataFormatId(), DataFormatId::UInt16);
    EXPECT_EQ(repr.getSwizzleMask(), swizzlemasks::luminance);
    EXPECT_EQ(repr.getInterpolation(), InterpolationType::Nearest);
    EXPECT_EQ(repr.getWrapping(), wrapping3d::repeatAll);
}

TEST_F(VolumeHDFTest, IsReadOnly) {
    VolumeHDF repr{handle(), selectTime(0)};
    EXPECT_THROW(repr.setDimensions(size3_t{1, 1, 1}), Exception);
    EXPECT_EQ(repr.getDimensions(), size3_t(nx, ny, nz));
}

TEST_F(VolumeHDFTest, RejectsInvalidSelections) {
    // rank mismatch
    EXPECT_THROW((VolumeHDF{handle(), std::vector<Selection>(3, Selection{0, 0, 1})}), Exception);
    // selects four dimensions
    EXPECT_THROW((VolumeHDF{handle(), std::vector<Selection>(4, Selection{0, 0, 1})}), Exception);
}

TEST_F(VolumeHDFTest, ReadIntoConvertsFormat) {
    const VolumeHDF repr{handle(), selectTime(1)};

    std::vector<std::byte> bytes(repr.getSizeInBytes(DataInt32::get()));
    repr.readInto(bytes, DataInt32::get());

    const std::span<const std::int32_t> values{reinterpret_cast<const std::int32_t*>(bytes.data()),
                                               nx * ny * nz};
    for (size_t z = 0; z < nz; ++z) {
        for (size_t y = 0; y < ny; ++y) {
            for (size_t x = 0; x < nx; ++x) {
                EXPECT_EQ(values[index(x, y, z)], static_cast<std::int32_t>(1000 + index(x, y, z)));
            }
        }
    }
}

TEST_F(VolumeHDFTest, ReadIntoRejectsWrongBufferSize) {
    const VolumeHDF repr{handle(), selectTime(0)};
    std::vector<std::byte> tooSmall(repr.getSizeInBytes(DataFloat32::get()) - 1);
    EXPECT_THROW(repr.readInto(tooSmall), Exception);
}

TEST_F(VolumeHDFTest, ConvertsToVolumeRAM) {
    Volume volume{std::make_shared<VolumeHDF>(handle(), selectTime(2))};

    EXPECT_TRUE(volume.hasRepresentation<VolumeHDF>());
    EXPECT_FALSE(volume.hasRepresentation<VolumeRAM>());
    EXPECT_EQ(volume.getDimensions(), size3_t(nx, ny, nz));
    EXPECT_EQ(volume.getDataFormatId(), DataFormatId::Float32);

    const auto* ram = volume.getRepresentation<VolumeRAM>();
    ASSERT_TRUE(ram);
    EXPECT_EQ(ram->getDimensions(), size3_t(nx, ny, nz));
    EXPECT_EQ(ram->getDataFormatId(), DataFormatId::Float32);
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{0, 0, 0}), 2000.0);
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{3, 2, 1}), 2000.0 + index(3, 2, 1));
}

TEST_F(VolumeHDFTest, StridedSubSelection) {
    // x: every second starting at 1 -> {1, 3}; y: 1..2 -> {1, 2}; z: all -> {0, 1}
    const std::vector<Selection> selection{Selection{1, 0, 2}, Selection{1, 2, 1},
                                           Selection{0, 0, 1}, Selection{0, 1, 1}};
    Volume volume{std::make_shared<VolumeHDF>(handle(), selection)};
    EXPECT_EQ(volume.getDimensions(), size3_t(2, 2, 2));

    const auto* ram = volume.getRepresentation<VolumeRAM>();
    for (size_t z = 0; z < 2; ++z) {
        EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{0, 0, z}), static_cast<double>(index(1, 1, z)));
        EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{1, 0, z}), static_cast<double>(index(3, 1, z)));
        EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{0, 1, z}), static_cast<double>(index(1, 2, z)));
        EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{1, 1, z}), static_cast<double>(index(3, 2, z)));
    }
}

TEST_F(VolumeHDFTest, UpdatesExistingVolumeRAM) {
    Volume volume{std::make_shared<VolumeHDF>(handle(), selectTime(0))};
    const auto* ram = volume.getRepresentation<VolumeRAM>();
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{1, 1, 1}), static_cast<double>(index(1, 1, 1)));

    // Replace the HDF representation with one pointing at another time step. The RAM
    // representation becomes invalid and is updated in place on the next request.
    auto hdf = std::make_shared<VolumeHDF>(handle(), selectTime(1));
    volume.addRepresentation(hdf);
    volume.invalidateAllOther(hdf.get());

    const auto* updated = volume.getRepresentation<VolumeRAM>();
    EXPECT_EQ(updated, ram);
    EXPECT_DOUBLE_EQ(updated->getAsDouble(size3_t{1, 1, 1}), 1000.0 + index(1, 1, 1));
}

TEST_F(VolumeHDFTest, CloneKeepsSource) {
    const VolumeHDF repr{handle(), selectTime(0),
                         VolumeReprConfig{.formatId = DataFormatId::UInt8}};
    const std::unique_ptr<VolumeHDF> clone{repr.clone()};

    EXPECT_EQ(clone->getDimensions(), repr.getDimensions());
    EXPECT_EQ(clone->getDataFormatId(), DataFormatId::UInt8);
    EXPECT_EQ(clone->getHandle().getFilename(), file);

    std::vector<std::byte> bytes(clone->getSizeInBytes(DataUInt8::get()));
    clone->readInto(bytes);
    EXPECT_EQ(std::to_integer<unsigned>(bytes[index(3, 2, 1)]), index(3, 2, 1));
}

TEST_F(VolumeHDFTest, LazyGetVolumeAtPathCreatesHDFRepresentation) {
    auto volume = getVolumeAtPathAsType(
        handle(), selectTime(1), VolumeConfig{.formatId = DataFormatId::Float64},
        [](const VolumeConfig& cfg) { return std::make_shared<Volume>(cfg); }, Loading::Lazy);

    ASSERT_TRUE(volume);
    EXPECT_TRUE(volume->hasRepresentation<VolumeHDF>());
    EXPECT_FALSE(volume->hasRepresentation<VolumeRAM>());
    EXPECT_EQ(volume->getDimensions(), size3_t(nx, ny, nz));
    EXPECT_EQ(volume->getDataFormatId(), DataFormatId::Float64);

    const auto* ram = volume->getRepresentation<VolumeRAM>();
    EXPECT_EQ(ram->getDataFormatId(), DataFormatId::Float64);
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{2, 1, 0}), 1000.0 + index(2, 1, 0));
}

TEST_F(VolumeHDFTest, LazyGetVolumeAtPathReusesVolumeAndInvalidatesRAM) {
    auto first = getVolumeAtPathAsType(
        handle(), selectTime(0), VolumeConfig{},
        [](const VolumeConfig& cfg) { return std::make_shared<Volume>(cfg); }, Loading::Lazy);
    const auto* ram = first->getRepresentation<VolumeRAM>();
    EXPECT_DOUBLE_EQ(ram->getAsDouble(size3_t{0, 0, 0}), 0.0);

    auto second = getVolumeAtPathAsType(
        handle(), selectTime(2), VolumeConfig{},
        [&](const VolumeConfig& cfg) { return reuseOrCreateVolume(cfg, first); }, Loading::Lazy);
    EXPECT_EQ(first, second);
    EXPECT_DOUBLE_EQ(second->getRepresentation<VolumeRAM>()->getAsDouble(size3_t{0, 0, 0}), 2000.0);
}

TEST_F(VolumeHDFTest, LazyTemporalLoaderProducesHDFBackedFrames) {
    const HDF5TemporalVolumeLoader loader{handle(),
                                          std::vector<Selection>(4, Selection{0, 0, 1}),
                                          3,
                                          Seconds{1.0},
                                          VolumeConfig{},
                                          Loading::Lazy};
    EXPECT_EQ(loader.size(), nt);

    auto frame = loader.load(1, nullptr, {});
    ASSERT_TRUE(frame);
    EXPECT_TRUE(frame->hasRepresentation<VolumeHDF>());
    EXPECT_FALSE(frame->hasRepresentation<VolumeRAM>());
    EXPECT_DOUBLE_EQ(frame->getRepresentation<VolumeRAM>()->getAsDouble(size3_t{0, 0, 0}), 1000.0);

    auto reused = loader.load(2, frame, {});
    EXPECT_EQ(reused, frame);
    EXPECT_DOUBLE_EQ(reused->getRepresentation<VolumeRAM>()->getAsDouble(size3_t{0, 0, 0}), 2000.0);
}

}  // namespace inviwo::hdf5
