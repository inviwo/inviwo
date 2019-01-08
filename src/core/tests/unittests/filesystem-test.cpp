/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formats.h>
#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

/* This will not work with ccache.
TEST(filesystemTest, fileExistsTest) {
#ifdef __FILE__
    EXPECT_TRUE(filesystem::fileExists(__FILE__));
#endif
}
*/

TEST(filesystemTest, fileExtensionTest) {
    EXPECT_STREQ("", filesystem::getFileExtension("").c_str());
    EXPECT_STREQ("txt", filesystem::getFileExtension("test.txt").c_str());
    EXPECT_STREQ("txt", filesystem::getFileExtension("test.dobule.txt").c_str());
    EXPECT_STREQ("", filesystem::getFileExtension("noExtensions").c_str());
    EXPECT_STREQ("", filesystem::getFileExtension("C:/a/directory/for/test/noExtensions").c_str());
    EXPECT_STREQ("",
                 filesystem::getFileExtension("C:/a/directory/for/test.test/noExtensions").c_str());
#ifdef __FILE__
    EXPECT_STREQ("cpp", filesystem::getFileExtension(__FILE__).c_str());
#endif
}

TEST(filesystemTest, fileNameWithExtension) {
    EXPECT_STREQ("file.txt",
                 filesystem::getFileNameWithExtension("C:/a/directory/for/test/file.txt").c_str());
    EXPECT_STREQ(
        "file.txt",
        filesystem::getFileNameWithExtension("C:\\a\\directory\\for\\test\\file.txt").c_str());
    EXPECT_STREQ("justafile.txt", filesystem::getFileNameWithExtension("justafile.txt").c_str());
}

TEST(filesystemTest, fileDirectoryTest) {
    EXPECT_STREQ("C:/a/directory/for/test",
                 filesystem::getFileDirectory("C:/a/directory/for/test/file.txt").c_str());
    EXPECT_STREQ("C:/a/directory/for/test",
                 filesystem::getFileDirectory("C:\\a\\directory\\for\\test\\file.txt").c_str());
    EXPECT_STREQ("", filesystem::getFileDirectory("justafile.txt").c_str());
}

TEST(filesystemTest, relativePathTest) {

    const std::string a = "/a/b/c/d";
    const std::string b = "/a/b/c/d/e/f";
    const std::string c = "/a/b";
    const std::string d = "/a/b/c/d/p/q";

    auto AB = filesystem::getRelativePath(a, b);
    EXPECT_STREQ("e/f", AB.c_str());

    auto AC = filesystem::getRelativePath(a, c);
    EXPECT_STREQ("../../../b", AC.c_str());

    auto BA = filesystem::getRelativePath(b, a);
    EXPECT_STREQ("../../../d", BA.c_str());

    auto BC = filesystem::getRelativePath(b, c);
    EXPECT_STREQ("../../../../../b", BC.c_str());

    auto CA = filesystem::getRelativePath(c, a);
    EXPECT_STREQ("c/d", CA.c_str());

    auto CB = filesystem::getRelativePath(c, b);
    EXPECT_STREQ("c/d/e/f", CB.c_str());

    auto DA = filesystem::getRelativePath(d, a);
    EXPECT_STREQ("../../../d", DA.c_str());

    auto AD = filesystem::getRelativePath(a, d);
    EXPECT_STREQ("p/q", AD.c_str());

    EXPECT_STREQ("../test/file.txt",
                 filesystem::getRelativePath("C:/foo/bar", "C:/foo/test/file.txt").c_str());
}

TEST(filesystemTest, pathCleanupTest) {
    // cleanupPath should replace all '\' with '/'
    EXPECT_STREQ("C:/a/directory/for/test/file.txt",
                 filesystem::cleanupPath("C:\\a\\directory/for/test\\file.txt").c_str());
    // cleanupPath should remove surrounding double quotes '"'
    EXPECT_STREQ("C:/a/directory/for/test/file.txt",
                 filesystem::cleanupPath("\"C:/a/directory/for/test/file.txt\"").c_str());
    // remove quotes and backslashes
    EXPECT_STREQ("C:/a/directory/for/test/file.txt",
                 filesystem::cleanupPath("\"C:\\a\\directory\\for\\test/file.txt\"").c_str());

    // unmatched quote
    EXPECT_STREQ("\"C:/test/file.txt", filesystem::cleanupPath("\"C:\\test/file.txt").c_str());
}

}  // namespace inviwo
