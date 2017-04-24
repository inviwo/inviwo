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

class TempFileHandle {
public:
    explicit TempFileHandle(const std::string& prefix = "", const std::string& suffix = "") {

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
        auto uRetVal = GetTempFileName(tempPath.data(),   // directory for tmp files
                                       prefixW.c_str(),   // temp file name prefix
                                       0,                 // create unique name
                                       tempFile.data());  // buffer for name
        if (uRetVal == 0) {
            throw Exception("could not create temporary file name");
        }

        filename_.assign(tempFile.begin(),
                        tempFile.begin() + std::min<size_t>(wcslen(tempFile.data()), MAX_PATH));
        filename_ += suffix;
        
        handle_ = fopen(filename_.c_str(), "w");
        if (!handle_) {
            throw Exception("could not open temporary file");
        }
#else

        static const std::string unqiue = "XXXXXX";

        const int suffixlen = suffix.size();

        std::vector<char> fileTemplate;
        fileTemplate.insert(fileTemplate.end(), prefix.begin(), prefix.end());
        fileTemplate.insert(fileTemplate.end(), unqiue.begin(), unqiue.end());
        fileTemplate.insert(fileTemplate.end(), suffix.begin(), suffix.end());
        fileTemplate.push_back('\0');

        int fd = mkstemps(fileTemplate.data(), suffixlen);
        if (fd == -1) {
            throw Exception("could not create temporary file");
        }
        handle_ = fdopen(fd, "w");
        if (!handle_) {
            throw Exception("could not open temporary file");
        }
        filename_.assign(fileTemplate.begin(), fileTemplate.end() - 1);
#endif
    }

    TempFileHandle(const TempFileHandle&) = delete;
    TempFileHandle& operator=(const TempFileHandle&) = delete;

    TempFileHandle(TempFileHandle&& rhs)
        : handle_{rhs.handle_}, filename_{std::move(rhs.filename_)} {
        rhs.handle_ = nullptr;
        rhs.filename_ = "";
    }
    TempFileHandle& operator=(TempFileHandle&& rhs) {
        if (this != &rhs) {
            cleanup();
            handle_ = rhs.handle_;
            filename_ = std::move(rhs.filename_);
            rhs.handle_ = nullptr;
            rhs.filename_ = "";
        }
        return *this;
    }

    ~TempFileHandle() { cleanup(); }

    const std::string& getFileName() const { return filename_; }

    FILE* getHandle() { return handle_; }
    operator FILE*() { return handle_; };

private:
    void cleanup() {
        if (handle_) fclose(handle_);

        if (!filename_.empty()) {

#ifdef WIN32
            std::wstring fileW(filename_.begin(), filename_.end());
            DeleteFile(fileW.c_str());
#else
            remove(filename_.c_str());
#endif
        }
    }

    FILE* handle_;
    std::string filename_;
};

TEST(CImgUtils, cimgToBuffer) {
    // load source image
    const auto filename = filesystem::getPath(PathType::Tests, "/images/swirl.png");
    CImgLayerReader reader;
    auto layer = reader.readData(filename);

    const std::string testExtension = "png";

    // write layer to a temporary png file
    TempFileHandle tmpFile("cimg", std::string(".") + testExtension);

    cimgutil::saveLayer(tmpFile.getFileName(), layer.get());

    // read file contents
    std::vector<unsigned char> fileContents;
    std::ifstream in(tmpFile.getFileName().c_str(), std::ifstream::binary);
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
