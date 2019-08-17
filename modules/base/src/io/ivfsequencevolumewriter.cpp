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

#include <modules/base/io/ivfsequencevolumewriter.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriterexception.h>

namespace inviwo {

IvfSequenceVolumeWriter::IvfSequenceVolumeWriter() : overwrite_(false) {}

void IvfSequenceVolumeWriter::writeData(const VolumeSequence *data,
                                        const std::string filePath) const {
    auto name = filesystem::getFileNameWithoutExtension(filePath);
    auto path = filesystem::getFileDirectory(filePath);
    return writeData(data, name, path);
}

void IvfSequenceVolumeWriter::writeData(const VolumeSequence *data, std::string name,
                                        std::string path,
                                        std::string reltivePathToTimesteps) const {

    util::writeIvfVolumeSequence(*data, name, path, reltivePathToTimesteps, overwrite_);
}

namespace util {
std::string writeIvfVolumeSequence(const VolumeSequence &volumes, std::string name,
                                   std::string path, std::string reltivePathToTimesteps,
                                   bool overwrite) {

    auto ivfwFile = path + "/" + name + ".ivfs";

    if (filesystem::fileExists(ivfwFile) && !overwrite)
        throw DataWriterException("Error: Output file: " + ivfwFile + " already exists",
                                  IVW_CONTEXT_CUSTOM("writeIvfVolumeSequence"));

    Serializer serializer(ivfwFile);

    auto fillLength = static_cast<int>(log10(volumes.size())) + 1;

    filesystem::createDirectoryRecursively(path + "/" + reltivePathToTimesteps);
    IvfVolumeWriter writer;
    writer.setOverwrite(overwrite);
    std::vector<std::string> filenames;
    size_t i = 0;
    for (const auto &vol : volumes) {
        std::stringstream filepath;
        filepath << reltivePathToTimesteps << "/" << name << std::setw(fillLength) << std::right
                 << std::setfill('0') << i++ << ".ivf";

        auto absFilepath = path + "/" + filepath.str();
        auto relFilepath = "./" + filepath.str();

        absFilepath = filesystem::cleanupPath(absFilepath);
        writer.writeData(vol.get(), absFilepath);
        filenames.push_back(relFilepath);
    }

    serializer.serialize("volumes", filenames, "volume");
    serializer.writeFile();
    return ivfwFile;
}
}  // namespace util

}  // namespace inviwo
