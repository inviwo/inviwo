/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/indexmapper.h>
#include <modules/opengl/openglcapabilities.h>
#include <modules/basegl/algorithm/dataminmaxgl.h>
#include <modules/base/algorithm/dataminmax.h>

#include <cmath>
#include <ranges>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

class DataMinMaxGLTest : public ::testing::Test {
public:
    DataMinMaxGLTest() = default;

protected:
    virtual void SetUp() override {
        if (!utilgl::DataMinMaxGL::isSuppportedByGPU()) {
            GTEST_SKIP() << "Compute Shaders and/or necessary extensions not available. Skipping "
                            "DataMinMaxGL tests.";
        }
        minmaxGL_ = std::make_unique<utilgl::DataMinMaxGL>();
    }
    void testVolume(std::string_view filename);
    // void testVolume(std::string_view filename);
    void test(std::shared_ptr<Volume> volume);
    void test(std::shared_ptr<Layer> layer);
    void test(std::shared_ptr<BufferBase> buffer);

private:
    std::unique_ptr<utilgl::DataMinMaxGL> minmaxGL_;
};

void DataMinMaxGLTest::testVolume(std::string_view filename) {
    auto file = filesystem::getPath(PathType::Tests) / "volumes" / filename;
    auto reader = InviwoApplication::getPtr()
                      ->getDataReaderFactory()
                      ->getReaderForTypeAndExtension<VolumeSequence>(file);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volumeSeq = reader->readData(file);
    ASSERT_EQ(1, volumeSeq->size());

    test(volumeSeq->front());
}

void DataMinMaxGLTest::test(std::shared_ptr<Volume> volume) {
    auto [refMin, refMax] = util::volumeMinMax(volume.get(), IgnoreSpecialValues::No);
    auto [glMin, glMax] = minmaxGL_->minMax(volume.get());

    EXPECT_EQ(refMin, glMin) << "Minimum values differ";
    EXPECT_EQ(refMax, glMax) << "Maximum values differ";
}

void DataMinMaxGLTest::test(std::shared_ptr<Layer> layer) {
    auto [refMin, refMax] = util::layerMinMax(layer.get(), IgnoreSpecialValues::No);
    auto [glMin, glMax] = minmaxGL_->minMax(layer.get());

    EXPECT_EQ(refMin, glMin) << "Minimum values differ";
    EXPECT_EQ(refMax, glMax) << "Maximum values differ";
}

void DataMinMaxGLTest::test(std::shared_ptr<BufferBase> buffer) {
    auto [refMin, refMax] = util::bufferMinMax(buffer.get(), IgnoreSpecialValues::No);
    auto [glMin, glMax] = minmaxGL_->minMax(buffer.get());

    EXPECT_EQ(refMin, glMin) << "Minimum values differ";
    EXPECT_EQ(refMax, glMax) << "Maximum values differ";
}

template <typename T>
[[nodiscard]] std::shared_ptr<Volume> createVolume(size3_t dims) {
    using Type = T::type;
    using ValueType = T::primitive;
    auto volumeRam = std::make_shared<VolumeRAMPrecision<Type>>(VolumeReprConfig{
        .dimensions = dims,
        .format = T::get(),
    });
    auto data = volumeRam->getView();
    util::IndexMapper3D im{dims};
    for (auto i : std::views::iota(0u, data.size())) {
        data[i] = Type{static_cast<ValueType>(1 + glm::compAdd(im(i)))};
    }

    return std::make_shared<Volume>(volumeRam);
}

template <typename T>
[[nodiscard]] std::shared_ptr<Layer> createLayer(size2_t dims) {
    using Type = T::type;
    using ValueType = T::primitive;
    auto layerRam = std::make_shared<LayerRAMPrecision<Type>>(LayerReprConfig{
        .dimensions = dims,
        .format = T::get(),
    });
    auto data = layerRam->getView();
    util::IndexMapper2D im{dims};
    for (auto i : std::views::iota(0u, data.size())) {
        data[i] = Type{static_cast<ValueType>(1 + glm::compAdd(im(i)))};
    }

    return std::make_shared<Layer>(layerRam);
}

template <typename T>
[[nodiscard]] std::shared_ptr<BufferBase> createBuffer(size_t size) {
    using Type = T::type;
    using ValueType = T::primitive;

    std::vector<Type> data(size);
    for (auto i : std::views::iota(0u, data.size())) {
        data[i] = Type{static_cast<ValueType>(1 + i)};
    }
    auto bufferRam = createBufferRAM(data);
    return std::make_shared<Buffer<Type>>(bufferRam);
}

// Test volume min/max
TEST_F(DataMinMaxGLTest, VolumeUInt8) { test(createVolume<DataUInt8>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec2UInt8) { test(createVolume<DataVec2UInt8>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec3UInt8) { test(createVolume<DataVec3UInt8>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec4UInt8) { test(createVolume<DataVec4UInt8>({64, 20, 10})); }

TEST_F(DataMinMaxGLTest, VolumeInt8) { test(createVolume<DataInt8>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec2Int8) { test(createVolume<DataVec2Int8>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec3Int8) { test(createVolume<DataVec3Int8>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec4Int8) { test(createVolume<DataVec4Int8>({64, 20, 10})); }

TEST_F(DataMinMaxGLTest, VolumeFloat32) { test(createVolume<DataFloat32>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec2Float32) { test(createVolume<DataVec2Float32>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec3Float32) { test(createVolume<DataVec3Float32>({64, 20, 10})); }
TEST_F(DataMinMaxGLTest, VolumeVec4Float32) { test(createVolume<DataVec4Float32>({64, 20, 10})); }

// 8 bit
TEST_F(DataMinMaxGLTest, VolumeTypeUINT8) { testVolume("testdata.UINT8.dat"); }
TEST_F(DataMinMaxGLTest, VolumeTypeINT8) { testVolume("testdata.INT8.dat"); }

// 16 bit
TEST_F(DataMinMaxGLTest, VolumeTypeUINT16) { testVolume("testdata.UINT16.LittleEndian.dat"); }
TEST_F(DataMinMaxGLTest, VolumeTypeINT16) { testVolume("testdata.INT16.LittleEndian.dat"); }

// 32 bit
TEST_F(DataMinMaxGLTest, VolumeTypeUINT32) { testVolume("testdata.UINT32.LittleEndian.dat"); }
TEST_F(DataMinMaxGLTest, VolumeTypeINT32) { testVolume("testdata.INT32.LittleEndian.dat"); }
TEST_F(DataMinMaxGLTest, VolumeTypeFLOAT32) { testVolume("testdata.FLOAT32.LittleEndian.dat"); }

// Test layer min/max
TEST_F(DataMinMaxGLTest, LayerUInt8) { test(createLayer<DataUInt8>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec2UInt8) { test(createLayer<DataVec2UInt8>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec3UInt8) { test(createLayer<DataVec3UInt8>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec4UInt8) { test(createLayer<DataVec4UInt8>({64, 20})); }

TEST_F(DataMinMaxGLTest, LayerInt8) { test(createLayer<DataInt8>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec2Int8) { test(createLayer<DataVec2Int8>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec3Int8) { test(createLayer<DataVec3Int8>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec4Int8) { test(createLayer<DataVec4Int8>({64, 20})); }

TEST_F(DataMinMaxGLTest, LayerFloat32) { test(createLayer<DataFloat32>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec2Float32) { test(createLayer<DataVec2Float32>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec3Float32) { test(createLayer<DataVec3Float32>({64, 20})); }
TEST_F(DataMinMaxGLTest, LayerVec4Float32) { test(createLayer<DataVec4Float32>({64, 20})); }

// Test buffer min/max
TEST_F(DataMinMaxGLTest, BufferUInt8) { test(createBuffer<DataUInt8>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec2UInt8) { test(createBuffer<DataVec2UInt8>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec3UInt8) { test(createBuffer<DataVec3UInt8>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec4UInt8) { test(createBuffer<DataVec4UInt8>(64)); }

TEST_F(DataMinMaxGLTest, BufferInt8) { test(createBuffer<DataInt8>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec2Int8) { test(createBuffer<DataVec2Int8>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec3Int8) { test(createBuffer<DataVec3Int8>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec4Int8) { test(createBuffer<DataVec4Int8>(64)); }

TEST_F(DataMinMaxGLTest, BufferFloat32) { test(createBuffer<DataFloat32>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec2Float32) { test(createBuffer<DataVec2Float32>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec3Float32) { test(createBuffer<DataVec3Float32>(64)); }
TEST_F(DataMinMaxGLTest, BufferVec4Float32) { test(createBuffer<DataVec4Float32>(64)); }

}  // namespace inviwo
