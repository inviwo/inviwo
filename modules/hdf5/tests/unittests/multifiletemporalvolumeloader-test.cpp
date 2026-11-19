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

#include <modules/hdf5/hdf5multifiletemporalvolumeloader.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/datastructures/hdf5selection.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/exception.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <H5Cpp.h>
#include <warn/pop>

#include <filesystem>
#include <numeric>
#include <vector>

namespace inviwo::hdf5 {

namespace {

constexpr hsize_t nx = 2;
constexpr hsize_t ny = 3;
constexpr hsize_t nz = 4;

// Writes "/data/field" with HDF (row major) shape [steps, nz, ny, nx] where every voxel of the
// local time step i has the value `firstValue + i`.
void writeFile(const std::filesystem::path& path, hsize_t steps, float firstValue,
               hsize_t sizeX = nx) {
    const H5::H5File file(path.generic_string(), H5F_ACC_TRUNC);
    const auto group = file.createGroup("/data");
    const std::array<hsize_t, 4> dims{steps, nz, ny, sizeX};
    const H5::DataSpace space(4, dims.data());
    const auto dataset = group.createDataSet("field", H5::PredType::NATIVE_FLOAT, space);

    const size_t frameSize = nz * ny * sizeX;
    std::vector<float> data(steps * frameSize);
    for (hsize_t t = 0; t < steps; ++t) {
        std::fill_n(data.begin() + static_cast<std::ptrdiff_t>(t * frameSize), frameSize,
                    firstValue + static_cast<float>(t));
    }
    dataset.write(data.data(), H5::PredType::NATIVE_FLOAT);
}

// Column major selection [x, y, z, t], all dimensions fully selected
std::vector<Selection> fullSelection() { return std::vector<Selection>(4, Selection{0, 0, 1}); }
constexpr size_t timeDim = 3;

class MultiFileLoader : public ::testing::Test {
protected:
    void SetUp() override {
        dir = std::filesystem::temp_directory_path() / "inviwo-hdf5-multifile-unittest";
        std::filesystem::create_directories(dir);
        // 2 + 3 + 1 = 6 frames, global frame index == stored value
        files = {dir / "a.h5", dir / "b.h5", dir / "c.h5"};
        writeFile(files[0], 2, 0.0f);
        writeFile(files[1], 3, 2.0f);
        writeFile(files[2], 1, 5.0f);
    }
    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }

    static double valueAt(const Volume& volume) {
        return volume.getRepresentation<VolumeRAM>()->getAsDouble(size3_t{1, 2, 3});
    }

    std::filesystem::path dir;
    std::vector<std::filesystem::path> files;
};

}  // namespace

TEST_F(MultiFileLoader, FrameCountAndMapping) {
    const HDF5MultiFileTemporalVolumeLoader loader{files,   Path{"/data/field"}, fullSelection(),
                                                   timeDim, Seconds{0.5},        VolumeConfig{}};

    EXPECT_EQ(loader.size(), 6u);
    EXPECT_EQ(loader.prototype().dimensions, size3_t(nx, ny, nz));
    EXPECT_EQ(loader.prototype().formatId, DataFormatId::Float32);
    EXPECT_DOUBLE_EQ(loader.time(4).count(), 2.0);

    EXPECT_EQ(loader.locate(0), (std::pair<size_t, size_t>{0, 0}));
    EXPECT_EQ(loader.locate(1), (std::pair<size_t, size_t>{0, 1}));
    EXPECT_EQ(loader.locate(2), (std::pair<size_t, size_t>{1, 0}));
    EXPECT_EQ(loader.locate(4), (std::pair<size_t, size_t>{1, 2}));
    EXPECT_EQ(loader.locate(5), (std::pair<size_t, size_t>{2, 0}));
    EXPECT_THROW(loader.locate(6), Exception);
}

TEST_F(MultiFileLoader, LoadsFramesAcrossFiles) {
    const HDF5MultiFileTemporalVolumeLoader loader{files,   Path{"/data/field"}, fullSelection(),
                                                   timeDim, Seconds{1.0},        VolumeConfig{}};

    for (size_t i = 0; i < loader.size(); ++i) {
        const auto volume = loader.load(i, nullptr, {});
        ASSERT_TRUE(volume);
        EXPECT_EQ(volume->getDimensions(), size3_t(nx, ny, nz));
        EXPECT_DOUBLE_EQ(valueAt(*volume), static_cast<double>(i)) << "frame " << i;
    }
}

TEST_F(MultiFileLoader, ReusesVolume) {
    const HDF5MultiFileTemporalVolumeLoader loader{files,   Path{"/data/field"}, fullSelection(),
                                                   timeDim, Seconds{1.0},        VolumeConfig{}};

    auto first = loader.load(0, nullptr, {});
    auto second = loader.load(3, first, {});
    EXPECT_EQ(first, second);
    EXPECT_DOUBLE_EQ(valueAt(*second), 3.0);
}

TEST_F(MultiFileLoader, TimeSubSelection) {
    auto selection = fullSelection();
    selection[timeDim] = Selection{.start = 1, .count = 0, .stride = 1};
    const HDF5MultiFileTemporalVolumeLoader loader{files,   Path{"/data/field"}, selection,
                                                   timeDim, Seconds{1.0},        VolumeConfig{}};

    // a.h5 contributes frame 1 only, b.h5 frames 1 and 2, c.h5 has only frame 0 so its time
    // selection is clamped to that frame
    EXPECT_EQ(loader.size(), 4u);
    EXPECT_DOUBLE_EQ(valueAt(*loader.load(0, nullptr, {})), 1.0);
    EXPECT_DOUBLE_EQ(valueAt(*loader.load(1, nullptr, {})), 3.0);
    EXPECT_DOUBLE_EQ(valueAt(*loader.load(2, nullptr, {})), 4.0);
    EXPECT_DOUBLE_EQ(valueAt(*loader.load(3, nullptr, {})), 5.0);
}

TEST_F(MultiFileLoader, MismatchingFileFailsOnlyWhenLoaded) {
    files.push_back(dir / "d.h5");
    writeFile(files.back(), 1, 6.0f, nx + 1);

    const HDF5MultiFileTemporalVolumeLoader loader{files,   Path{"/data/field"}, fullSelection(),
                                                   timeDim, Seconds{1.0},        VolumeConfig{}};

    EXPECT_EQ(loader.size(), 7u);
    EXPECT_NO_THROW(loader.load(5, nullptr, {}));
    EXPECT_THROW(loader.load(6, nullptr, {}), Exception);
}

TEST_F(MultiFileLoader, EmptyFileListThrows) {
    EXPECT_THROW(
        (HDF5MultiFileTemporalVolumeLoader{
            {}, Path{"/data/field"}, fullSelection(), timeDim, Seconds{1.0}, VolumeConfig{}}),
        Exception);
}

}  // namespace inviwo::hdf5
