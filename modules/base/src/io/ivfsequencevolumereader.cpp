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

#include <modules/base/io/ivfsequencevolumereader.h>
#include <modules/base/io/ivfvolumewriter.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {
IvfSequenceVolumeReader::IvfSequenceVolumeReader() {
    addExtension({"ivfs", "Sequence of Inviwo ivf volumes"});
}

std::shared_ptr<VolumeSequence> IvfSequenceVolumeReader::readData(const std::string &filePath) {
    std::string fileName = filePath;
    if (!filesystem::fileExists(fileName)) {
        std::string newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + fileName, IVW_CONTEXT);
        }
    }

    auto volumes = std::make_shared<VolumeSequence>();

    auto dir = filesystem::getFileDirectory(fileName);

    std::vector<std::string> filenames;
    Deserializer d(fileName);
    d.deserialize("volumes", filenames, "volume");
    for (auto filename : filenames) {
        auto abs = filesystem::cleanupPath(dir + "/" + filename);
        volumes->push_back(reader_.readData(abs));
    }

    return volumes;
}

}  // namespace inviwo
