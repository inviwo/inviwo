/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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
#include <modules/cimg/cimgutils.h>
#include <modules/cimg/cimglayerreader.h>

#include <fstream>
#include <array>
#include <cstdio>
#include <algorithm>

namespace inviwo {

#ifdef WIN32
std::wstring get_utf16(const std::string &str, int codepage) {
    if (str.empty()) return std::wstring();
    int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
    return res;
}
#endif

std::string getTempFilename(const std::string& prefix, const std::string& suffix) {
#ifdef WIN32
    // get temp directory
    std::array<wchar_t, MAX_PATH> tempPath;
    std::array<wchar_t, MAX_PATH> tempFile;
    auto retVal = GetTempPath(MAX_PATH, tempPath.data());
    if ((retVal > MAX_PATH) || (retVal == 0)) {
        throw Exception("could not locate temp folder");
    }
    // generate temp file name
    std::wstring prefixW(prefix.begin(), prefix.end());
    auto uRetVal = GetTempFileName(tempPath.data(), // directory for tmp files
                                   prefixW.c_str(),     // temp file name prefix 
                                   0,                // create unique name 
                                   tempFile.data());  // buffer for name 
    if (uRetVal == 0) {
        throw Exception("could not create temporary file name");
    }

    return std::string(tempFile.begin(),
                       tempFile.begin() + std::min<size_t>(wcslen(tempFile.data()), MAX_PATH)) +
           suffix;
#else
    std::array<char, L_tmpnam> tempFile;
    auto retVal = tmpnam_r(tempFile.data());
    if (retVal == nullptr) {
        throw Exception("could not create temporary file name");
    }

    return std::string(tempFile.begin(),
                       tempFile.begin() + std::min<size_t>(strlen(tempFile.data()), L_tmpnam)) +
           suffix;
#endif
}

struct TempFile {
    TempFile(const std::string& prefix, const std::string& suffix) 
    : filename(getTempFilename(prefix, suffix)) {}
    ~TempFile() {
#ifdef WIN32
        std::wstring fileW(filename.begin(), filename.end());
        DeleteFile(fileW.c_str());
#else
        remove(filename.c_str());
#endif
    }

    std::string filename;
};



TEST(CImgUtils, cimgToBuffer) {
    // load source image
    const auto filename = filesystem::getPath(PathType::Tests, "/images/swirl.png");
    CImgLayerReader reader;
    auto layer = reader.readData(filename);

    const std::string testExtension = "png";

    // write layer to a temporary png file
    TempFile tmpFile("cimg", std::string(".") + testExtension);

    cimgutil::saveLayer(tmpFile.filename, layer.get());

    // read file contents
    std::vector<unsigned char> fileContents;
    std::ifstream in(tmpFile.filename.c_str(), std::ifstream::binary);
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

}
