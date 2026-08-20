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

#include <gtest/gtest.h>

#include <inviwo/cloudfiles/cloudfiles.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace inviwo::cloudfiles {

namespace {

// Creates a unique regular file in the temp directory and removes it on destruction.
class TempFile {
public:
    TempFile() {
        path_ = std::filesystem::temp_directory_path() /
                ("inviwo-cloudfiles-test-" +
                 std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ".tmp");
        std::ofstream out{path_, std::ios::binary};
        out << "hello cloud files";
    }
    ~TempFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }
    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST(CloudFiles, ToStringCoversAllValues) {
    EXPECT_EQ(toString(Availability::NotACloudFile), "NotACloudFile");
    EXPECT_EQ(toString(Availability::Available), "Available");
    EXPECT_EQ(toString(Availability::NotAvailable), "NotAvailable");
    EXPECT_EQ(toString(Availability::Downloading), "Downloading");
    EXPECT_EQ(toString(Availability::Unknown), "Unknown");
}

TEST(CloudFiles, StatusOnMissingFileThrows) {
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "inviwo-cloudfiles-does-not-exist.xyz";
    EXPECT_THROW(inviwo::cloudfiles::status(missing), CloudFilesError);
}

TEST(CloudFiles, RegularFileIsLocallyAvailable) {
    TempFile file;

    const auto s = inviwo::cloudfiles::status(file.path());
    EXPECT_FALSE(s.isPlaceholder);
    EXPECT_TRUE(s.availability == Availability::NotACloudFile ||
                s.availability == Availability::Available);

    EXPECT_TRUE(isLocallyAvailable(file.path()));
    EXPECT_FALSE(isCloudFile(file.path()));

    ASSERT_TRUE(s.size.has_value());
    EXPECT_EQ(*s.size, std::filesystem::file_size(file.path()));
}

TEST(CloudFiles, DownloadRegularFileMatchesPlatformSupport) {
    TempFile file;
    if (isSupported()) {
        // A regular local file is already available; downloading should be a no-op.
        EXPECT_NO_THROW(download(file.path()));
    } else {
        EXPECT_THROW(download(file.path()), CloudFilesError);
    }
}

TEST(CloudFiles, OffloadRegularFileThrows) {
    TempFile file;
    // A regular local file is not a cloud placeholder, so offloading is never valid.
    EXPECT_THROW(offload(file.path()), CloudFilesError);
}

}  // namespace inviwo::cloudfiles
