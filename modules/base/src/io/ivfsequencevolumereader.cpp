/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>   // for VolumeSequence
#include <inviwo/core/io/datareader.h>                  // for DataReaderType
#include <inviwo/core/io/serialization/deserializer.h>  // for Deserializer
#include <inviwo/core/util/filesystem.h>                // for cleanupPath, getFileDirectory
#include <modules/base/io/ivfvolumereader.h>            // for IvfVolumeReader

#include <functional>                                   // for __base
#include <string>                                       // for string, basic_string<>::value_type
#include <type_traits>                                  // for remove_extent_t
#include <vector>                                       // for vector

namespace inviwo {
IvfSequenceVolumeReader::IvfSequenceVolumeReader() {
    addExtension({"ivfs", "Sequence of Inviwo ivf volumes"});
}

std::shared_ptr<VolumeSequence> IvfSequenceVolumeReader::readData(std::string_view filePath) {
    checkExists(filePath);

    auto dir = filesystem::getFileDirectory(filePath);

    std::vector<std::string> filenames;
    Deserializer d(filePath);
    d.deserialize("volumes", filenames, "volume");
    auto volumes = std::make_shared<VolumeSequence>();
    for (auto filename : filenames) {
        auto abs = filesystem::cleanupPath(dir + "/" + filename);
        volumes->push_back(reader_.readData(abs));
    }

    return volumes;
}

}  // namespace inviwo
