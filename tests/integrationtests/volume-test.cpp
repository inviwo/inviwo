/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opengl/volume/volumegl.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/indexmapper.h>
#include <cmath>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

template <typename T>
void testVolumeLoad(Volume* volume) {
    const auto volumeRAM =
        dynamic_cast<const VolumeRAMPrecision<T>*>(volume->getRepresentation<VolumeRAM>());
    ASSERT_TRUE(volumeRAM != nullptr);

    const auto data = volumeRAM->getDataTyped();
    const auto dim = volumeRAM->getDimensions();
    const util::IndexMapper3D im(dim);

    for (size_t z = 0; z < dim.z; z++) {
        for (size_t y = 0; y < dim.y; y++) {
            for (size_t x = 0; x < dim.x; x++) {
                const auto ref = static_cast<double>(x + y + z);
                const auto val = static_cast<double>(data[im(x, y, z)]);
                ASSERT_EQ(ref, val)
                    << "Error at Voxel: " << x << ", " << y << ", " << z << "\n"
                    << "Dimensions:     " << dim.x << ", " << dim.y << ", " << dim.z;
            }
        }
    }
}

template <typename T>
void testDatVolumeLoad(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);

    auto reader =
        InviwoApplication::getPtr()
            ->getDataReaderFactory()
            ->getReaderForTypeAndExtension<std::vector<std::shared_ptr<Volume>>>(fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volumeSeq = reader->readData(file);
    ASSERT_EQ(1, volumeSeq->size());

    auto volume = volumeSeq->front();

    testVolumeLoad<T>(volume.get());
}

template <typename T>
void testIvfVolumeLoad(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);
    auto reader =
        InviwoApplication::getPtr()->getDataReaderFactory()->getReaderForTypeAndExtension<Volume>(
            fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volume = reader->readData(file);

    testVolumeLoad<T>(volume.get());
}

template <typename T>
void testVolumeClone(Volume* volume) {
    auto volume1 = volume->clone();

    const auto volumeRAM =
        dynamic_cast<const VolumeRAMPrecision<T>*>(volume->getRepresentation<VolumeRAM>());
    ASSERT_TRUE(volumeRAM != nullptr);
    const auto data = volumeRAM->getDataTyped();
    const auto dim = volume->getDimensions();

    auto volume2 = volume->clone();

    const auto volumeRAM1 =
        static_cast<const VolumeRAMPrecision<T>*>(volume1->getRepresentation<VolumeRAM>());
    const auto volumeRAM2 =
        static_cast<const VolumeRAMPrecision<T>*>(volume2->getRepresentation<VolumeRAM>());

    const auto data1 = volumeRAM1->getDataTyped();
    const auto data2 = volumeRAM2->getDataTyped();

    const size3_t dim1 = volume1->getDimensions();
    const size3_t dim2 = volume2->getDimensions();

    EXPECT_EQ(dim, dim1);
    EXPECT_EQ(dim, dim2);

    EXPECT_EQ(volume->getModelMatrix(), volume1->getModelMatrix());
    EXPECT_EQ(volume->getModelMatrix(), volume2->getModelMatrix());

    EXPECT_EQ(volume->getWorldMatrix(), volume1->getWorldMatrix());
    EXPECT_EQ(volume->getWorldMatrix(), volume2->getWorldMatrix());

    MetaDataMap* metadata = volume->getMetaDataMap();
    MetaDataMap* metadata1 = volume1->getMetaDataMap();
    MetaDataMap* metadata2 = volume2->getMetaDataMap();

    EXPECT_EQ(*metadata, *metadata);
    EXPECT_EQ(*metadata, *metadata1);
    EXPECT_EQ(*metadata, *metadata2);

    const util::IndexMapper3D im(dim);

    for (size_t z = 0; z < dim.z; z++) {
        for (size_t y = 0; y < dim.y; y++) {
            for (size_t x = 0; x < dim.x; x++) {
                const auto ref = static_cast<double>(x + y + z);

                const auto val = static_cast<double>(data[im(x, y, z)]);
                ASSERT_EQ(ref, val)
                    << "Error at Voxel: " << x << ", " << y << ", " << z << "\n"
                    << "Dimensions:     " << dim.x << ", " << dim.y << ", " << dim.z;

                const auto val1 = static_cast<double>(data1[im(x, y, z)]);
                ASSERT_EQ(ref, val1)
                    << "Error at Voxel: " << x << ", " << y << ", " << z << "\n"
                    << "Dimensions:     " << dim.x << ", " << dim.y << ", " << dim.z;

                const auto val2 = static_cast<double>(data2[im(x, y, z)]);
                ASSERT_EQ(ref, val2)
                    << "Error at Voxel: " << x << ", " << y << ", " << z << "\n"
                    << "Dimensions:     " << dim.x << ", " << dim.y << ", " << dim.z;
            }
        }
    }

    delete volume1;
    delete volume2;
}

template <typename T>
void testDatVolumeClone(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);
    auto reader =
        InviwoApplication::getPtr()
            ->getDataReaderFactory()
            ->getReaderForTypeAndExtension<std::vector<std::shared_ptr<Volume>>>(fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volumeSeq = reader->readData(file);
    ASSERT_EQ(1, volumeSeq->size());

    auto volume = volumeSeq->front();

    testVolumeClone<T>(volume.get());
}

template <typename T>
void testIvfVolumeClone(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);
    auto reader =
        InviwoApplication::getPtr()->getDataReaderFactory()->getReaderForTypeAndExtension<Volume>(
            fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volume = reader->readData(file);

    testVolumeClone<T>(volume.get());
}

// Test the .dat reader

// 8 bit
TEST(VolumeTest, DatReaderLoadTypeUINT8) { testDatVolumeLoad<glm::uint8>("testdata.UINT8.dat"); }
TEST(VolumeTest, DatReaderLoadTypeINT8) { testDatVolumeLoad<glm::int8>("testdata.INT8.dat"); }

// 16 bit
TEST(VolumeTest, DatReaderLoadTypeUINT16LittleEndian) {
    testDatVolumeLoad<glm::uint16>("testdata.UINT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT16LittleEndian) {
    testDatVolumeLoad<glm::int16>("testdata.INT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT16BigEndian) {
    testDatVolumeLoad<glm::uint16>("testdata.UINT16.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT16BigEndian) {
    testDatVolumeLoad<glm::int16>("testdata.INT16.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeFLOAT16LittleEndian) {
    testDatVolumeLoad<half_float::half>("testdata.FLOAT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeFLOAT16BigEndian) {
    testDatVolumeLoad<half_float::half>("testdata.FLOAT16.BigEndian.dat");
}

// 32 bit
TEST(VolumeTest, DatReaderLoadTypeUINT32) {
    testDatVolumeLoad<glm::uint32>("testdata.UINT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT32) {
    testDatVolumeLoad<glm::int32>("testdata.INT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT32BigEndian) {
    testDatVolumeLoad<glm::uint32>("testdata.UINT32.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT32BigEndian) {
    testDatVolumeLoad<glm::int32>("testdata.INT32.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeFLOAT32LittleEndian) {
    testDatVolumeLoad<float>("testdata.FLOAT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeFLOAT32BigEndian) {
    testDatVolumeLoad<float>("testdata.FLOAT32.BigEndian.dat");
}

// 64 bit
TEST(VolumeTest, DatReaderLoadTypeUINT64) {
    testDatVolumeLoad<glm::uint64>("testdata.UINT64.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT64) {
    testDatVolumeLoad<glm::int64>("testdata.INT64.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT64BigEndian) {
    testDatVolumeLoad<glm::uint64>("testdata.UINT64.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT64BigEndian) {
    testDatVolumeLoad<glm::int64>("testdata.INT64.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeFLOAT64LittleEndian) {
    testDatVolumeLoad<double>("testdata.FLOAT64.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeFLOAT64BigEndian) {
    testDatVolumeLoad<double>("testdata.FLOAT64.BigEndian.dat");
}

// Test the .ivf reader
// 8 bit
TEST(VolumeTest, IvfReaderLoadTypeUINT8) { testIvfVolumeLoad<glm::uint8>("testdata.UINT8.ivf"); }
TEST(VolumeTest, IvfReaderLoadTypeINT8) { testIvfVolumeLoad<glm::int8>("testdata.INT8.ivf"); }

// 16 bit
TEST(VolumeTest, IvfReaderLoadTypeUINT16LittleEndian) {
    testIvfVolumeLoad<glm::uint16>("testdata.UINT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT16LittleEndian) {
    testIvfVolumeLoad<glm::int16>("testdata.INT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT16BigEndian) {
    testIvfVolumeLoad<glm::uint16>("testdata.UINT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT16BigEndian) {
    testIvfVolumeLoad<glm::int16>("testdata.INT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeFLOAT16LittleEndian) {
    testIvfVolumeLoad<half_float::half>("testdata.FLOAT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeFLOAT16BigEndian) {
    testIvfVolumeLoad<half_float::half>("testdata.FLOAT16.BigEndian.ivf");
}

// 32 bit
TEST(VolumeTest, IvfReaderLoadTypeUINT32) {
    testIvfVolumeLoad<glm::uint32>("testdata.UINT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT32) {
    testIvfVolumeLoad<glm::int32>("testdata.INT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT32BigEndian) {
    testIvfVolumeLoad<glm::uint32>("testdata.UINT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT32BigEndian) {
    testIvfVolumeLoad<glm::int32>("testdata.INT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeFLOAT32LittleEndian) {
    testIvfVolumeLoad<float>("testdata.FLOAT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeFLOAT32BigEndian) {
    testIvfVolumeLoad<float>("testdata.FLOAT32.BigEndian.ivf");
}

// 64 bit
TEST(VolumeTest, IvfReaderLoadTypeUINT64) {
    testIvfVolumeLoad<glm::uint64>("testdata.UINT64.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT64) {
    testIvfVolumeLoad<glm::int64>("testdata.INT64.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT64BigEndian) {
    testIvfVolumeLoad<glm::uint64>("testdata.UINT64.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT64BigEndian) {
    testIvfVolumeLoad<glm::int64>("testdata.INT64.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeFLOAT64LittleEndian) {
    testIvfVolumeLoad<double>("testdata.FLOAT64.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeFLOAT64BigEndian) {
    testIvfVolumeLoad<double>("testdata.FLOAT64.BigEndian.ivf");
}

// Test cloning ////////////////////////////////////////////////////////////////////////
// Test the .dat reader

// 8 bit
TEST(VolumeTest, DatReaderCloneTypeUINT8) { testDatVolumeClone<glm::uint8>("testdata.UINT8.dat"); }
TEST(VolumeTest, DatReaderCloneTypeINT8) { testDatVolumeClone<glm::int8>("testdata.INT8.dat"); }

// 16 bit
TEST(VolumeTest, DatReaderCloneTypeUINT16LittleEndian) {
    testDatVolumeClone<glm::uint16>("testdata.UINT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeINT16LittleEndian) {
    testDatVolumeClone<glm::int16>("testdata.INT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeUINT16BigEndian) {
    testDatVolumeClone<glm::uint16>("testdata.UINT16.BigEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeINT16BigEndian) {
    testDatVolumeClone<glm::int16>("testdata.INT16.BigEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeFLOAT16LittleEndian) {
    testDatVolumeClone<half_float::half>("testdata.FLOAT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeFLOAT16BigEndian) {
    testDatVolumeClone<half_float::half>("testdata.FLOAT16.BigEndian.dat");
}

// 32 bit
TEST(VolumeTest, DatReaderCloneTypeUINT32) {
    testDatVolumeClone<glm::uint32>("testdata.UINT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeINT32) {
    testDatVolumeClone<glm::int32>("testdata.INT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeUINT32BigEndian) {
    testDatVolumeClone<glm::uint32>("testdata.UINT32.BigEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeINT32BigEndian) {
    testDatVolumeClone<glm::int32>("testdata.INT32.BigEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeFLOAT32LittleEndian) {
    testDatVolumeClone<float>("testdata.FLOAT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeFLOAT32BigEndian) {
    testDatVolumeClone<float>("testdata.FLOAT32.BigEndian.dat");
}

// 64 bit
TEST(VolumeTest, DatReaderCloneTypeUINT64) {
    testDatVolumeClone<glm::uint64>("testdata.UINT64.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeINT64) {
    testDatVolumeClone<glm::int64>("testdata.INT64.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeUINT64BigEndian) {
    testDatVolumeClone<glm::uint64>("testdata.UINT64.BigEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeINT64BigEndian) {
    testDatVolumeClone<glm::int64>("testdata.INT64.BigEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeFLOAT64LittleEndian) {
    testDatVolumeClone<double>("testdata.FLOAT64.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderCloneTypeFLOAT64BigEndian) {
    testDatVolumeClone<double>("testdata.FLOAT64.BigEndian.dat");
}

// Test the .ivf reader
// 8 bit
TEST(VolumeTest, IvfReaderCloneTypeUINT8) { testIvfVolumeClone<glm::uint8>("testdata.UINT8.ivf"); }
TEST(VolumeTest, IvfReaderCloneTypeINT8) { testIvfVolumeClone<glm::int8>("testdata.INT8.ivf"); }

// 16 bit
TEST(VolumeTest, IvfReaderCloneTypeUINT16LittleEndian) {
    testIvfVolumeClone<glm::uint16>("testdata.UINT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeINT16LittleEndian) {
    testIvfVolumeClone<glm::int16>("testdata.INT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeUINT16BigEndian) {
    testIvfVolumeClone<glm::uint16>("testdata.UINT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeINT16BigEndian) {
    testIvfVolumeClone<glm::int16>("testdata.INT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeFLOAT16LittleEndian) {
    testIvfVolumeClone<half_float::half>("testdata.FLOAT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeFLOAT16BigEndian) {
    testIvfVolumeClone<half_float::half>("testdata.FLOAT16.BigEndian.ivf");
}

// 32 bit
TEST(VolumeTest, IvfReaderCloneTypeUINT32) {
    testIvfVolumeClone<glm::uint32>("testdata.UINT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeINT32) {
    testIvfVolumeClone<glm::int32>("testdata.INT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeUINT32BigEndian) {
    testIvfVolumeClone<glm::uint32>("testdata.UINT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeINT32BigEndian) {
    testIvfVolumeClone<glm::int32>("testdata.INT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeFLOAT32LittleEndian) {
    testIvfVolumeClone<float>("testdata.FLOAT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeFLOAT32BigEndian) {
    testIvfVolumeClone<float>("testdata.FLOAT32.BigEndian.ivf");
}

// 64 bit
TEST(VolumeTest, IvfReaderCloneTypeUINT64) {
    testIvfVolumeClone<glm::uint64>("testdata.UINT64.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeINT64) {
    testIvfVolumeClone<glm::int64>("testdata.INT64.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeUINT64BigEndian) {
    testIvfVolumeClone<glm::uint64>("testdata.UINT64.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeINT64BigEndian) {
    testIvfVolumeClone<glm::int64>("testdata.INT64.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeFLOAT64LittleEndian) {
    testIvfVolumeClone<double>("testdata.FLOAT64.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderCloneTypeFLOAT64BigEndian) {
    testIvfVolumeClone<double>("testdata.FLOAT64.BigEndian.ivf");
}

}  // namespace inviwo