/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo{


TEST(filesystemTest,fileExistsTest) {
#ifdef __FILE__
    EXPECT_TRUE(filesystem::fileExists(__FILE__));
#endif
    EXPECT_TRUE(filesystem::fileExists(InviwoApplication::getPtr()->getCommandLineParser()->getARGV()[0]));//Cant find current executable
}

TEST(filesystemTest,fileExtensionTest) {
    EXPECT_STREQ("",filesystem::getFileExtension("").c_str());
    EXPECT_STREQ("txt",filesystem::getFileExtension("test.txt").c_str());
    EXPECT_STREQ("txt",filesystem::getFileExtension("test.dobule.txt").c_str());
    EXPECT_STREQ("",filesystem::getFileExtension("noExtensions").c_str());
    EXPECT_STREQ("",filesystem::getFileExtension("C:/a/directory/for/test/noExtensions").c_str());
    EXPECT_STREQ("", filesystem::getFileExtension("C:/a/directory/for/test.test/noExtensions").c_str());
#ifdef __FILE__
    EXPECT_STREQ("cpp",filesystem::getFileExtension(__FILE__).c_str());
#endif
}



TEST(filesystemTest,FileDirectoryTest) {
    EXPECT_STREQ("C:/a/directory/for/test/",filesystem::getFileDirectory("C:/a/directory/for/test/file.txt").c_str());
    EXPECT_STREQ("C:\\a\\directory\\for\\test\\",filesystem::getFileDirectory("C:\\a\\directory\\for\\test\\file.txt").c_str());
    EXPECT_STREQ("",filesystem::getFileDirectory("justafile.txt").c_str());
    // EXPECT_STREQ("C:/a/directory/for/test/",filesystem::getFileDirectory("C:/a/directory/for/test//withdoubleslahs.txt").c_str());
}

}