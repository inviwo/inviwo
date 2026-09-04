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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <modules/base/io/filesequenceloader.h>

#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/glmvec.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace inviwo {

using namespace std::chrono_literals;

namespace {

// A volume reader that does not actually read the file. The frame number is parsed from the file
// stem (e.g. "frame_3.fake" -> 3) and stored as metadata so tests can confirm which file was read.
// It also counts how many times readData was called to verify lazy loading.
class FakeVolumeReader : public DataReaderType<Volume> {
public:
    explicit FakeVolumeReader(std::shared_ptr<std::atomic<int>> readCount)
        : readCount_{std::move(readCount)} {
        addExtension(FileExtension{.extension = "fake", .description = "Fake Volume"});
    }

    virtual FakeVolumeReader* clone() const override { return new FakeVolumeReader(*this); }

    virtual std::shared_ptr<Volume> readData(const std::filesystem::path& filePath) override {
        ++(*readCount_);
        auto volume = std::make_shared<Volume>(size3_t{2, 2, 2}, DataUInt8::get());

        const auto stem = filePath.stem().string();
        const auto pos = stem.find_last_of('_');
        const int number = (pos != std::string::npos) ? std::stoi(stem.substr(pos + 1)) : -1;
        volume->setMetaData<IntMetaData>("number", number);
        return volume;
    }

private:
    std::shared_ptr<std::atomic<int>> readCount_;
};

struct Fixture {
    Fixture() : readCount{std::make_shared<std::atomic<int>>(0)} {
        factory.registerObject(new FakeVolumeReader(readCount));
    }

    static std::vector<std::filesystem::path> paths(size_t count) {
        std::vector<std::filesystem::path> result;
        result.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            result.emplace_back(fmt::format("frame_{}.fake", i));
        }
        return result;
    }

    std::shared_ptr<std::atomic<int>> readCount;
    DataReaderFactory factory;
};

int numberOf(const std::shared_ptr<const Volume>& volume) {
    return volume ? volume->getMetaData<IntMetaData>("number", -1) : -1;
}

}  // namespace

TEST(FileSequenceLoaderTest, LoadsPrototypeOnce) {
    Fixture fix;
    // Constructing the loader reads exactly one file (for the prototype).
    const FileSequenceLoader loader{fix.paths(5), {}, &fix.factory};
    EXPECT_EQ(loader.size(), 5u);
    EXPECT_EQ(*fix.readCount, 1);
    EXPECT_NE(loader.prototype().format, nullptr);
    EXPECT_EQ(loader.prototype().dimensions.value(), (size3_t{2, 2, 2}));
}

TEST(FileSequenceLoaderTest, LoadsFramesByIndex) {
    Fixture fix;
    FileSequenceLoader loader{fix.paths(4), {}, &fix.factory};

    EXPECT_EQ(loader.load(0)->getMetaData<IntMetaData>("number", -1), 0);
    EXPECT_EQ(loader.load(3)->getMetaData<IntMetaData>("number", -1), 3);
}

TEST(FileSequenceLoaderTest, DefaultTimesAreIndices) {
    Fixture fix;
    const FileSequenceLoader loader{fix.paths(3), {}, &fix.factory};
    EXPECT_TRUE(loader.times().empty());
}

TEST(FileSequenceLoaderTest, CustomTimes) {
    Fixture fix;
    const FileSequenceLoader loader{fix.paths(3), {0.0s, 5.0s, 10.0s}, &fix.factory};
    ASSERT_EQ(loader.times().size(), 3u);
    EXPECT_DOUBLE_EQ(loader.times()[1].count(), 5.0);
}

TEST(FileSequenceLoaderTest, RejectsMismatchedTimes) {
    Fixture fix;
    EXPECT_THROW(FileSequenceLoader(fix.paths(3), {0.0s, 1.0s}, &fix.factory), Exception);
}

TEST(FileSequenceLoaderTest, RejectsEmptyPaths) {
    Fixture fix;
    EXPECT_THROW(FileSequenceLoader({}, {}, &fix.factory), Exception);
}

TEST(FileSequenceLoaderTest, ThrowsWithoutReader) {
    DataReaderFactory empty;
    const std::vector<std::filesystem::path> paths{"frame_0.fake"};
    EXPECT_THROW(FileSequenceLoader(paths, {}, &empty), DataReaderException);
}

TEST(FileSequenceLoaderTest, IntegratesWithTemporalVolumeLazily) {
    Fixture fix;
    auto loader =
        std::make_unique<FileSequenceLoader>(fix.paths(10), std::vector<Seconds>{}, &fix.factory);
    const TemporalVolume tv{std::move(loader), 4};

    // Only the prototype has been read so far.
    EXPECT_EQ(*fix.readCount, 1);
    EXPECT_EQ(tv.size(), 10u);

    // Accessing a frame reads exactly one more file.
    EXPECT_EQ(numberOf(tv.get(size_t{5})), 5);
    EXPECT_EQ(*fix.readCount, 2);

    // The cache keeps it, no extra read.
    EXPECT_EQ(numberOf(tv.get(size_t{5})), 5);
    EXPECT_EQ(*fix.readCount, 2);
}

}  // namespace inviwo
