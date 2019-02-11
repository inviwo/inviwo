/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/tempfilehandle.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/cimg/cimgutils.h>
#include <modules/cimg/cimglayerreader.h>

#include <fstream>
#include <array>
#include <cstdio>
#include <algorithm>

namespace inviwo {

TEST(CImgUtils, cimgToBuffer) {
    // load source image

    std::string testExtension = "bmp";
    const auto filename = filesystem::getPath(PathType::Tests, "/images/swirl." + testExtension);
    CImgLayerReader reader;
    auto layer = reader.readData(filename);

    // write layer to a temporary file
    util::TempFileHandle tmpFile("cimg", std::string(".") + testExtension);

    cimgutil::saveLayer(tmpFile.getFileName(), layer.get());

    // read file contents
    std::vector<unsigned char> fileContents;
    auto in = filesystem::ifstream(tmpFile.getFileName(), std::ios::binary);
    if (in.is_open()) {
        in.seekg(0, in.end);
        size_t fileLen = in.tellg();
        in.seekg(0, in.beg);

        fileContents.resize(fileLen);
        in.read(reinterpret_cast<char*>(fileContents.data()), fileLen);

        in.close();
    }

    // write layer to buffer
    auto imgBuffer = cimgutil::saveLayerToBuffer(testExtension, layer.get());
    ASSERT_TRUE(imgBuffer != nullptr) << "buffer is empty";

    // compare buffer and file contents

    ASSERT_TRUE(fileContents.size() == imgBuffer->size()) << "buffer and file size does not match";
    EXPECT_EQ(*imgBuffer.get(), fileContents) << "buffer and file contents do not match";
}

}  // namespace inviwo
