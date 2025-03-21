/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/base/io/ivfsequencevolumewriter.h>

#include <inviwo/core/datastructures/volume/volume.h>  // for VolumeSequence
#include <inviwo/core/io/datawriter.h>                 // for DataWriter, Overwrite
#include <inviwo/core/io/serialization/serializer.h>   // for Serializer
#include <inviwo/core/util/filesystem.h>               // for getFileDirectory, cleanupPath, cre...
#include <inviwo/core/util/typetraits.h>               // for alwaysTrue, identity
#include <modules/base/io/ivfvolumewriter.h>           // for IvfVolumeWriter

#include <cmath>    // for log10
#include <cstddef>  // for size_t
#include <memory>   // for shared_ptr
#include <vector>   // for vector
#include <memory_resource>

#include <fmt/core.h>  // for format

namespace inviwo {

void IvfSequenceVolumeWriter::writeData(const VolumeSequence* data,
                                        const std::filesystem::path& filePath) const {
    auto name = filePath.stem().string();
    auto path = filePath.parent_path();
    return writeData(data, name, path);
}

void IvfSequenceVolumeWriter::writeData(const VolumeSequence* data, std::string_view name,
                                        const std::filesystem::path& path,
                                        std::string_view relativePathToTimeSteps) const {

    util::writeIvfVolumeSequence(*data, name, path, relativePathToTimeSteps, overwrite_);
}

namespace util {
std::filesystem::path writeIvfVolumeSequence(const VolumeSequence& volumes, std::string_view name,
                                             const std::filesystem::path& path,
                                             std::string_view relativePathToTimeSteps,
                                             Overwrite overwrite) {

    auto ivfsFile = path / fmt::format("{}.ivfs", name);

    DataWriter::checkOverwrite(ivfsFile, overwrite);

    std::pmr::monotonic_buffer_resource mbr{1024 * 4};
    Serializer serializer{ivfsFile, "InviwoVolumeSequence", &mbr};

    auto fillLength = static_cast<int>(log10(volumes.size())) + 1;

    std::filesystem::create_directories(path / relativePathToTimeSteps);
    IvfVolumeWriter writer;
    writer.setOverwrite(overwrite);
    std::vector<std::filesystem::path> filenames;
    size_t i = 0;
    for (const auto& vol : volumes) {
        const auto absFilePath =
            path / relativePathToTimeSteps / fmt::format("{}{:0{}}.ivf", name, i, fillLength);
        const auto relFilePath =
            fmt::format("./{}/{}{:0{}}.ivf", relativePathToTimeSteps, name, i, fillLength);

        writer.writeData(vol.get(), absFilePath);
        filenames.push_back(relFilePath);
    }

    serializer.serialize("volumes", filenames, "volume");
    serializer.writeFile();
    return ivfsFile;
}
}  // namespace util

}  // namespace inviwo
