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

#include <modules/hdf5/datastructures/hdf5path.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <string>

namespace inviwo {

namespace hdf5 {

TEST(HDF5Path, RoundTrip) {
    const Path path{"/group/subgroup/dataset"};
    EXPECT_EQ(path.toString(), "/group/subgroup/dataset");
}

TEST(HDF5Path, SkipsEmptySegments) {
    // Leading, trailing and repeated separators produce no empty components.
    const Path path{"//group///dataset//"};
    EXPECT_EQ(path.toString(), "/group/dataset");
}

TEST(HDF5Path, EmptyIsRoot) {
    const Path path{""};
    EXPECT_EQ(path.toString(), "/");
}

TEST(HDF5Path, PushAndPop) {
    Path path{"/a/b"};
    path.push("c");
    EXPECT_EQ(path.toString(), "/a/b/c");
    path.pop();
    EXPECT_EQ(path.toString(), "/a/b");
}

TEST(HDF5Path, Concatenation) {
    const Path combined = Path{"/a"} + Path{"/b/c"};
    EXPECT_EQ(combined.toString(), "/a/b/c");
}

TEST(HDF5Path, PlusEqualsMergesSegments) {
    Path path{"/a"};
    path += Path{"/b"};
    path += "c/d";
    EXPECT_EQ(path.toString(), "/a/b/c/d");
}

TEST(HDF5Path, ImplicitStringConversion) {
    const Path path{"/a/b"};
    const std::string& str = path;
    EXPECT_EQ(str, "/a/b");
}

}  // namespace hdf5

}  // namespace inviwo
