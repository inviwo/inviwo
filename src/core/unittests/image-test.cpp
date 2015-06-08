/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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


#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerdisk.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/unittests/unittestsmodule.h>

namespace inviwo{

#define IMG_RGB InviwoApplication::getPtr()->getModuleByType<UnitTestsModule>()->getPath() + "/testdata/images/2x2.bmp"
#define IMG_WHITE InviwoApplication::getPtr()->getModuleByType<UnitTestsModule>()->getPath() + "/testdata/images/white.bmp"
#define IMG_RANGE InviwoApplication::getPtr()->getModuleByType<UnitTestsModule>()->getPath() + "/testdata/images/range.bmp"

TEST(ImageTests,ImageLoadWhite) {
    InviwoApplication::getPtr()->getModuleByType<UnitTestsModule>()->getPath();
    std::string imgFile = IMG_WHITE;
    ASSERT_TRUE(filesystem::fileExists(imgFile));

    LayerDisk* disk = new LayerDisk(imgFile);
    ASSERT_TRUE(disk != 0);

    std::string ext = filesystem::getFileExtension(imgFile);
    EXPECT_EQ(ext, "bmp");


    DataReader* reader = DataReaderFactory::getPtr()->getReaderForTypeAndExtension<Layer>(ext);
    ASSERT_TRUE(reader != 0);

    disk->setDataReader(reader);

    Image img;
    img.getColorLayer()->addRepresentation(disk);
    const LayerRAM* layer = img.getColorLayer()->getRepresentation<LayerRAM>();
    ASSERT_TRUE(layer!=0);

    uvec2 dim = layer->getDimensions();
    EXPECT_EQ(dim.x,2);
    EXPECT_EQ(dim.y,2);

    dvec4 a = layer->getValueAsVec4Double(uvec2(0,0));
    dvec4 b = layer->getValueAsVec4Double(uvec2(0,1));
    dvec4 c = layer->getValueAsVec4Double(uvec2(1,0));
    dvec4 d = layer->getValueAsVec4Double(uvec2(1,1));
    EXPECT_DOUBLE_EQ(a.r, 1.0);
    EXPECT_DOUBLE_EQ(a.g, 1.0);
    EXPECT_DOUBLE_EQ(a.b, 1.0);
    EXPECT_DOUBLE_EQ(a.a, 1.0);
    EXPECT_DOUBLE_EQ(b.r, 1.0);
    EXPECT_DOUBLE_EQ(b.g, 1.0);
    EXPECT_DOUBLE_EQ(b.b, 1.0);
    EXPECT_DOUBLE_EQ(b.a, 1.0);
    EXPECT_DOUBLE_EQ(c.r, 1.0);
    EXPECT_DOUBLE_EQ(c.g, 1.0);
    EXPECT_DOUBLE_EQ(c.b, 1.0);
    EXPECT_DOUBLE_EQ(c.a, 1.0);
    EXPECT_DOUBLE_EQ(d.r, 1.0);
    EXPECT_DOUBLE_EQ(d.g, 1.0);
    EXPECT_DOUBLE_EQ(d.b, 1.0);
    EXPECT_DOUBLE_EQ(d.a, 1.0);
}


TEST(ImageTests, ImageLoadRGB) {
    std::string imgFile = IMG_RGB;
    ASSERT_TRUE(filesystem::fileExists(imgFile));

    LayerDisk* disk = new LayerDisk(imgFile);
    ASSERT_TRUE(disk != 0);

    std::string ext = filesystem::getFileExtension(imgFile);
    EXPECT_EQ(ext, "bmp");


    DataReader* reader = DataReaderFactory::getPtr()->getReaderForTypeAndExtension<Layer>(ext);
    ASSERT_TRUE(reader != 0);

    disk->setDataReader(reader);

    Image img;
    img.getColorLayer()->addRepresentation(disk);
    const LayerRAM* layer = img.getColorLayer()->getRepresentation<LayerRAM>();
    ASSERT_TRUE(layer!=0);
    uvec2 dim = layer->getDimensions();
    EXPECT_EQ(dim.x,2);
    EXPECT_EQ(dim.y,2);
    dvec4 a = layer->getValueAsVec4Double(uvec2(0,0));
    dvec4 b = layer->getValueAsVec4Double(uvec2(1,0));
    dvec4 c = layer->getValueAsVec4Double(uvec2(0,1));
    dvec4 d = layer->getValueAsVec4Double(uvec2(1,1));
    EXPECT_DOUBLE_EQ(a.r, 1.0);
    EXPECT_DOUBLE_EQ(a.g, 0.0);
    EXPECT_DOUBLE_EQ(a.b, 0.0);
    EXPECT_DOUBLE_EQ(a.a, 1.0);
    EXPECT_DOUBLE_EQ(b.r, 0.0);
    EXPECT_DOUBLE_EQ(b.g, 1.0);
    EXPECT_DOUBLE_EQ(b.b, 0.0);
    EXPECT_DOUBLE_EQ(b.a, 1.0);
    EXPECT_DOUBLE_EQ(c.r, 0.0);
    EXPECT_DOUBLE_EQ(c.g, 0.0);
    EXPECT_DOUBLE_EQ(c.b, 1.0);
    EXPECT_DOUBLE_EQ(c.a, 1.0);
    EXPECT_DOUBLE_EQ(d.r, 50/255.0);
    EXPECT_DOUBLE_EQ(d.g, 100/255.0);
    EXPECT_DOUBLE_EQ(d.b, 150/255.0);
    EXPECT_DOUBLE_EQ(d.a, 1.0);
}



TEST(ImageTests, ImageLoadRange) {
    std::string imgFile = IMG_RANGE;
    ASSERT_TRUE(filesystem::fileExists(imgFile));

    LayerDisk* disk = new LayerDisk(imgFile);
    ASSERT_TRUE(disk != 0);

    std::string ext = filesystem::getFileExtension(imgFile);
    EXPECT_EQ(ext, "bmp");


    DataReader* reader = DataReaderFactory::getPtr()->getReaderForTypeAndExtension<Layer>(ext);
    ASSERT_TRUE(reader != 0);

    disk->setDataReader(reader);

    Image img;
    img.getColorLayer()->addRepresentation(disk);
    const LayerRAM* layer = img.getColorLayer()->getRepresentation<LayerRAM>();
    ASSERT_TRUE(layer!=0);
    uvec2 dim = layer->getDimensions();
    EXPECT_EQ(dim.x,256);
    EXPECT_EQ(dim.y,1);

    for (int i = 0; i<255; i++) {
        dvec4 a = layer->getValueAsVec4Double(uvec2(i,0));
        EXPECT_DOUBLE_EQ(a.r, i/255.0);
        EXPECT_DOUBLE_EQ(a.g, i/255.0);
        EXPECT_DOUBLE_EQ(a.b, i/255.0);
        EXPECT_DOUBLE_EQ(a.a, 1.0);
    }
}





TEST(ImageTests, ImageResize) {
    std::string imgFile = IMG_RGB;
    ASSERT_TRUE(filesystem::fileExists(imgFile));

    LayerDisk* disk = new LayerDisk(imgFile);
    ASSERT_TRUE(disk != 0);

    std::string ext = filesystem::getFileExtension(imgFile);
    EXPECT_EQ(ext, "bmp");


    DataReader* reader = DataReaderFactory::getPtr()->getReaderForTypeAndExtension<Layer>(ext);
    ASSERT_TRUE(reader != 0);

    disk->setDataReader(reader);

    Image img;
    img.getColorLayer()->addRepresentation(disk);
    const LayerRAM* layer = img.getColorLayer()->getRepresentation<LayerRAM>();
    ASSERT_TRUE(layer!=0);
    uvec2 dim = layer->getDimensions();
    EXPECT_EQ(dim.x,2);
    EXPECT_EQ(dim.y,2);


    img.setDimensions(uvec2(10,10));


    const LayerRAM* layer2 = img.getColorLayer()->getRepresentation<LayerRAM>();

    dim = layer->getDimensions();

    EXPECT_EQ(dim.x,10);
    EXPECT_EQ(dim.y,10);
    
    dim = layer2->getDimensions();

    EXPECT_EQ(dim.x,10);
    EXPECT_EQ(dim.y,10);

    EXPECT_TRUE(layer == layer2);


}



}