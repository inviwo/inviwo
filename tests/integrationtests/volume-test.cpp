/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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
#include <inviwo/core/resources/resourcemanager.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/resources/templateresource.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opengl/volume/volumegl.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <math.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {



    template<typename T>
    void testVolumeLoad(Volume *volume) {
        
        const VolumeRAMPrecision<T>* volumeRAM =
            dynamic_cast<const VolumeRAMPrecision<T>*>(volume->getRepresentation<VolumeRAM>());
        ASSERT_TRUE(volumeRAM != nullptr);

        const T* data = static_cast<const T*>(volumeRAM->getData());
        ASSERT_TRUE(data != nullptr);

        size3_t dim = volume->getDimensions();
        long long ref0;
        double ref1;
        double ref2;
        double ref3;
        double val;
        long long ty;
        long long tz;
        double mod = static_cast<double>(std::numeric_limits<T>::max())
            - static_cast<double>(std::numeric_limits<T>::min());
        double min = static_cast<double>(std::numeric_limits<T>::min());

        for (long long z = 0; z < static_cast<long long>(dim.z); z++) {
            tz = z*z*z*z*z*z*z*z;

            for (long long y = 0; y < static_cast<long long>(dim.y); y++) {
                ty = y*y*y*y;

                for (long long x = 0; x < static_cast<long long>(dim.x); x++) {
                    ref0 = x + x*ty + x*ty*tz;
                    ref1 = static_cast<double>(ref0);
                    ref2 = fmod(ref1, mod);
                    ref3 = ref2 + min;
                    val = static_cast<double>(data[x + (y*dim.x) + (z*dim.x *dim.y)]);
                    EXPECT_EQ(ref3, val);
                }
            }
        }


    }


template<typename T>
void testDatVolumeLoad(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);

    auto reader = InviwoApplication::getPtr()->getDataReaderFactory()->getReaderForTypeAndExtension<std::vector<std::shared_ptr<Volume>>>(fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volumeSeq = reader->readData(file);
    ASSERT_EQ(1,volumeSeq->size());

    auto volume = volumeSeq->front();

    testVolumeLoad<T>(volume.get());
    
}


template<typename T>
void testIvfVolumeLoad(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);
    auto reader = InviwoApplication::getPtr()->getDataReaderFactory()->getReaderForTypeAndExtension<Volume>(fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volume = reader->readData(file);

    testVolumeLoad<T>(volume.get());
}

template<typename T>
void testVolumeClone(Volume *volume) {
    Volume* volume1 = volume->clone();

    const VolumeRAMPrecision<T>* volumeRAM =
        static_cast<const VolumeRAMPrecision<T>*>(volume->getRepresentation<VolumeRAM>());
    Volume* volume2 = volume->clone();

    const T* data = static_cast<const T*>(volumeRAM->getData());
    size3_t dim = volume->getDimensions();

    const VolumeRAMPrecision<T>* volumeRAM1 =
        static_cast<const VolumeRAMPrecision<T>*>(volume1->getRepresentation<VolumeRAM>());
    const VolumeRAMPrecision<T>* volumeRAM2 =
        static_cast<const VolumeRAMPrecision<T>*>(volume2->getRepresentation<VolumeRAM>());

    const T* data1 = static_cast<const T*>(volumeRAM1->getData());
    const T* data2 = static_cast<const T*>(volumeRAM2->getData());

    size3_t dim1 = volume1->getDimensions();
    size3_t dim2 = volume2->getDimensions();

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

    long long ref0;
    double ref1;
    double ref2;
    double ref3;
    double val, val1, val2;
    long long ty;
    long long tz;
    double mod = static_cast<double>(std::numeric_limits<T>::max())
        - static_cast<double>(std::numeric_limits<T>::min());
    double min = static_cast<double>(std::numeric_limits<T>::min());

    for (long long z = 0; z < static_cast<long long>(dim.z); z++) {
        tz = z*z*z*z*z*z*z*z;

        for (long long y = 0; y < static_cast<long long>(dim.y); y++) {
            ty = y*y*y*y;

            for (long long x = 0; x < static_cast<long long>(dim.x); x++) {
                ref0 = x + x*ty + x*ty*tz;
                ref1 = static_cast<double>(ref0);
                ref2 = fmod(ref1, mod);
                ref3 = ref2 + min;
                val = static_cast<double>(data[x + (y*dim.x) + (z*dim.x *dim.y)]);
                val1 = static_cast<double>(data1[x + (y*dim.x) + (z*dim.x *dim.y)]);
                val2 = static_cast<double>(data2[x + (y*dim.x) + (z*dim.x *dim.y)]);
                ASSERT_EQ(ref3, val);
                ASSERT_EQ(ref3, val1);
                ASSERT_EQ(ref3, val2);
            }
        }
    }
    delete volume1;
    delete volume2;
}



template<typename T>
void testDatVolumeClone(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);
    auto reader = InviwoApplication::getPtr()->getDataReaderFactory()->getReaderForTypeAndExtension<std::vector<std::shared_ptr<Volume>>>(fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volumeSeq = reader->readData(file);
    ASSERT_EQ(1, volumeSeq->size());

    auto volume = volumeSeq->front();

    testVolumeClone<T>(volume.get());
}



template<typename T>
void testIvfVolumeClone(std::string filename) {
    std::string file = filesystem::getPath(PathType::Tests) + "/volumes/" + filename;
    std::string fileExtension = filesystem::getFileExtension(file);
    auto reader = InviwoApplication::getPtr()->getDataReaderFactory()->getReaderForTypeAndExtension<Volume>(fileExtension);
    ASSERT_TRUE(reader.get() != nullptr);
    auto volume = reader->readData(file);

    testVolumeClone<T>(volume.get());
}




// Test the .dat reader

TEST(VolumeTest, DatReaderLoadTypeUINT8) {
    testDatVolumeLoad<glm::uint8>("testdata.UINT8.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT16) {
    testDatVolumeLoad<glm::uint16>("testdata.UINT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT32) {
    testDatVolumeLoad<glm::uint32>("testdata.UINT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT64) {
    testDatVolumeLoad<glm::uint64>("testdata.UINT64.LittleEndian.dat");
}

TEST(VolumeTest, DatReaderLoadTypeINT8) {
    testDatVolumeLoad<glm::int8>("testdata.INT8.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT16) {
    testDatVolumeLoad<glm::int16>("testdata.INT16.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT32) {
    testDatVolumeLoad<glm::int32>("testdata.INT32.LittleEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT64) {
    testDatVolumeLoad<glm::int64>("testdata.INT64.LittleEndian.dat");
}

TEST(VolumeTest, DatReaderLoadTypeUINT8BigEndian) {
    testDatVolumeLoad<glm::uint8>("testdata.UINT8.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT16BigEndian) {
    testDatVolumeLoad<glm::uint16>("testdata.UINT16.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT32BigEndian) {
    testDatVolumeLoad<glm::uint32>("testdata.UINT32.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeUINT64BigEndian) {
    testDatVolumeLoad<glm::uint64>("testdata.UINT64.BigEndian.dat");
}

TEST(VolumeTest, DatReaderLoadTypeINT8BigEndian) {
    testDatVolumeLoad<glm::int8>("testdata.INT8.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT16BigEndian) {
    testDatVolumeLoad<glm::int16>("testdata.INT16.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT32BigEndian) {
    testDatVolumeLoad<glm::int32>("testdata.INT32.BigEndian.dat");
}
TEST(VolumeTest, DatReaderLoadTypeINT64BigEndian) {
    testDatVolumeLoad<glm::int64>("testdata.INT64.BigEndian.dat");
}


// Test the .ivf reader

TEST(VolumeTest, IvfReaderLoadTypeUINT8) {
    testIvfVolumeLoad<glm::uint8>("testdata.UINT8.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT16) {
    testIvfVolumeLoad<glm::uint16>("testdata.UINT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT32) {
    testIvfVolumeLoad<glm::uint32>("testdata.UINT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT64) {
    testIvfVolumeLoad<glm::uint64>("testdata.UINT64.LittleEndian.ivf");
}

TEST(VolumeTest, IvfReaderLoadTypeINT8) {
    testIvfVolumeLoad<glm::int8>("testdata.INT8.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT16) {
    testIvfVolumeLoad<glm::int16>("testdata.INT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT32) {
    testIvfVolumeLoad<glm::int32>("testdata.INT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT64) {
    testIvfVolumeLoad<glm::int64>("testdata.INT64.LittleEndian.ivf");
}

TEST(VolumeTest, IvfReaderLoadTypeUINT8BigEndian) {
    testIvfVolumeLoad<glm::uint8>("testdata.UINT8.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT16BigEndian) {
    testIvfVolumeLoad<glm::uint16>("testdata.UINT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT32BigEndian) {
    testIvfVolumeLoad<glm::uint32>("testdata.UINT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeUINT64BigEndian) {
    testIvfVolumeLoad<glm::uint64>("testdata.UINT64.BigEndian.ivf");
}

TEST(VolumeTest, IvfReaderLoadTypeINT8BigEndian) {
    testIvfVolumeLoad<glm::int8>("testdata.INT8.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT16BigEndian) {
    testIvfVolumeLoad<glm::int16>("testdata.INT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT32BigEndian) {
    testIvfVolumeLoad<glm::int32>("testdata.INT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfReaderLoadTypeINT64BigEndian) {
    testIvfVolumeLoad<glm::int64>("testdata.INT64.BigEndian.ivf");
}


// Test cloning ////////////////////////////////////////////////////////////////////////

TEST(VolumeTest, DatVolumeCloneTypeUINT8) {
    testDatVolumeClone<glm::uint8>("testdata.UINT8.LittleEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeUINT16) {
    testDatVolumeClone<glm::uint16>("testdata.UINT16.LittleEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeUINT32) {
    testDatVolumeClone<glm::uint32>("testdata.UINT32.LittleEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeUINT64) {
    testDatVolumeClone<glm::uint64>("testdata.UINT64.LittleEndian.dat");
}

TEST(VolumeTest, DatVolumeCloneTypeINT8) {
    testDatVolumeClone<glm::int8>("testdata.INT8.LittleEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeINT16) {
    testDatVolumeClone<glm::int16>("testdata.INT16.LittleEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeINT32) {
    testDatVolumeClone<glm::int32>("testdata.INT32.LittleEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeINT64) {
    testDatVolumeClone<glm::int64>("testdata.INT64.LittleEndian.dat");
}

TEST(VolumeTest, DatVolumeCloneTypeUINT8BigEndian) {
    testDatVolumeClone<glm::uint8>("testdata.UINT8.BigEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeUINT16BigEndian) {
    testDatVolumeClone<glm::uint16>("testdata.UINT16.BigEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeUINT32BigEndian) {
    testDatVolumeClone<glm::uint32>("testdata.UINT32.BigEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeUINT64BigEndian) {
    testDatVolumeClone<glm::uint64>("testdata.UINT64.BigEndian.dat");
}

TEST(VolumeTest, DatVolumeCloneTypeINT8BigEndian) {
    testDatVolumeClone<glm::int8>("testdata.INT8.BigEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeINT16BigEndian) {
    testDatVolumeClone<glm::int16>("testdata.INT16.BigEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeINT32BigEndian) {
    testDatVolumeClone<glm::int32>("testdata.INT32.BigEndian.dat");
}
TEST(VolumeTest, DatVolumeCloneTypeINT64BigEndian) {
    testDatVolumeClone<glm::int64>("testdata.INT64.BigEndian.dat");
}





TEST(VolumeTest, IvfVolumeCloneTypeUINT8) {
    testIvfVolumeClone<glm::uint8>("testdata.UINT8.LittleEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeUINT16) {
    testIvfVolumeClone<glm::uint16>("testdata.UINT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeUINT32) {
    testIvfVolumeClone<glm::uint32>("testdata.UINT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeUINT64) {
    testIvfVolumeClone<glm::uint64>("testdata.UINT64.LittleEndian.ivf");
}

TEST(VolumeTest, IvfVolumeCloneTypeINT8) {
    testIvfVolumeClone<glm::int8>("testdata.INT8.LittleEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeINT16) {
    testIvfVolumeClone<glm::int16>("testdata.INT16.LittleEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeINT32) {
    testIvfVolumeClone<glm::int32>("testdata.INT32.LittleEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeINT64) {
    testIvfVolumeClone<glm::int64>("testdata.INT64.LittleEndian.ivf");
}

TEST(VolumeTest, IvfVolumeCloneTypeUINT8BigEndian) {
    testIvfVolumeClone<glm::uint8>("testdata.UINT8.BigEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeUINT16BigEndian) {
    testIvfVolumeClone<glm::uint16>("testdata.UINT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeUINT32BigEndian) {
    testIvfVolumeClone<glm::uint32>("testdata.UINT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeUINT64BigEndian) {
    testIvfVolumeClone<glm::uint64>("testdata.UINT64.BigEndian.ivf");
}

TEST(VolumeTest, IvfVolumeCloneTypeINT8BigEndian) {
    testIvfVolumeClone<glm::int8>("testdata.INT8.BigEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeINT16BigEndian) {
    testIvfVolumeClone<glm::int16>("testdata.INT16.BigEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeINT32BigEndian) {
    testIvfVolumeClone<glm::int32>("testdata.INT32.BigEndian.ivf");
}
TEST(VolumeTest, IvfVolumeCloneTypeINT64BigEndian) {
    testIvfVolumeClone<glm::int64>("testdata.INT64.BigEndian.ivf");
}

}