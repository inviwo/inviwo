/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/pvm/mpvmvolumereader.h>

#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, DataReade...
#include <inviwo/core/datastructures/volume/volumeram.h>                // for createVolumeRAM
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/metadata/metadata.h>                              // for StringMetaData
#include <inviwo/core/metadata/metadataowner.h>                         // for MetaDataOwner
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/filesystem.h>                                // for getFileDirectory
#include <inviwo/core/util/formatconversion.h>                          // for formatBytesToString
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for size3_t
#include <inviwo/core/util/logcentral.h>                                // for LogCentral
#include <inviwo/core/util/sourcecontext.h>                             // for SourceContext
#include <inviwo/core/util/stringconversion.h>                          // for replaceInString
#include <modules/pvm/pvmvolumereader.h>                                // for PVMVolumeReader

#include <cstddef>        // for size_t
#include <fstream>        // for operator<<, basic...
#include <string>         // for string, char_traits
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for move
#include <vector>         // for vector
#include <ranges>
#include <algorithm>

#include <glm/fwd.hpp>   // for mat3
#include <glm/vec3.hpp>  // for operator+, vec<>:...

#include <fmt/std.h>

namespace inviwo {

MPVMVolumeReader::MPVMVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("mpvm", "MPVM (Multiple PVMs) file format"));
}

MPVMVolumeReader* MPVMVolumeReader::clone() const { return new MPVMVolumeReader(*this); }

namespace {

void gatherVolumes(std::shared_ptr<inviwo::VolumeRAM>& dest,
                   std::vector<std::shared_ptr<inviwo::Volume>>& volumes) {

    const auto srcPtrs = volumes | std::views::transform([](const auto& vol) {
                             return static_cast<const unsigned char*>(
                                 vol->template getRepresentation<VolumeRAM>()->getData());
                         }) |
                         std::ranges::to<std::vector>();

    auto* dstPtr = static_cast<unsigned char*>(dest->getData());

    // Copy the data from the other volumes to the new multichannel volume
    const size_t mbytes = dest->getDataFormat()->getSizeInBytes();
    const size_t bytes = volumes[0]->getDataFormat()->getSizeInBytes();
    const size_t dims = glm::compMul(volumes[0]->getDimensions());
    const size_t vsize = srcPtrs.size();
    for (size_t i = 0; i < dims; i++) {
        for (size_t j = 0; j < vsize; j++) {
            for (size_t b = 0; b < bytes; b++) {
                dstPtr[i * mbytes + (j * bytes) + b] = srcPtrs[j][i * bytes + b];
            }
        }
    }
}

}  // namespace

std::shared_ptr<Volume> MPVMVolumeReader::readData(const std::filesystem::path& filePath) {
    auto fileDirectory = filePath.parent_path();

    // Read the mpvm file content
    std::string textLine;
    std::vector<std::filesystem::path> files;
    {
        auto f = openAndCacheIfUrl(filePath);
        while (!f.eof()) {
            getline(f, textLine);
            textLine = trim(textLine);
            files.emplace_back(textLine);
        }
    }

    if (files.empty()) {
        throw DataReaderException(SourceContext{}, "Error: No PVM files found in {:?g}", filePath);
    }
    if (files.size() > 4) {
        throw DataReaderException(
            SourceContext{}, "Error: Maximum 4 pvm files are supported, file: {:?g}", filePath);
    }

    // Read all pvm volumes
    std::vector<std::shared_ptr<Volume>> volumes;
    for (size_t i = 0; i < files.size(); i++) {
        auto newVol = PVMVolumeReader::readPVMData(fileDirectory / files[i]);
        if (newVol) {
            volumes.push_back(newVol);
        } else {
            log::warn("Could not load {:?g}", fileDirectory / files[i]);
        }
    }

    if (volumes.empty()) {
        throw DataReaderException(SourceContext{}, "No PVM volumes could be read from file: {:?g}",
                                  filePath);
    }
    if (volumes.size() == 1) {
        printPVMMeta(*volumes[0], fileDirectory / files[0]);
        return volumes[0];
    }

    // Make sure dimension and format match
    const DataFormatBase* format = volumes[0]->getDataFormat();
    size3_t mdim = volumes[0]->getDimensions();
    for (size_t i = 1; i < volumes.size(); i++) {
        if (format != volumes[i]->getDataFormat() || mdim != volumes[i]->getDimensions()) {
            log::warn(
                "PVM volumes did not have the same format or dimensions, using first volume.");
            printPVMMeta(*volumes[0], fileDirectory / files[0]);
            return volumes[0];
        }
    }

    // Create new format
    const DataFormatBase* mformat =
        DataFormatBase::get(format->getNumericType(), volumes.size(), format->getSizeInBytes() * 8);

    // Create new volume
    auto volume = std::make_shared<Volume>(mdim, mformat);
    glm::mat3 basis = volumes[0]->getBasis();
    volume->setBasis(basis);
    volume->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));
    volume->copyMetaDataFrom(*volumes[0]);

    // Merge descriptions but ignore the rest
    if (auto metaData = volume->getMetaData<StringMetaData>("description")) {
        std::string descStr = metaData->get();
        for (size_t i = 1; i < volumes.size(); i++) {
            metaData = volumes[0]->getMetaData<StringMetaData>("description");
            if (metaData) descStr = descStr + ", " + metaData->get();
        }
        volume->setMetaData<StringMetaData>("description", descStr);
    }

    // Create RAM volume
    auto mvolRAM = createVolumeRAM(mdim, mformat);
    gatherVolumes(mvolRAM, volumes);

    volume->addRepresentation(mvolRAM);
    printPVMMeta(*volume, filePath);
    return volume;
}

void MPVMVolumeReader::printPVMMeta(const Volume& volume,
                                    const std::filesystem::path& fileName) const {
    const size3_t dim = volume.getDimensions();
    const size_t bytes = dim.x * dim.y * dim.z * (volume.getDataFormat()->getSizeInBytes());
    log::info("Loaded volume: '{:?g}' size: {}", fileName, ByteSize{bytes});
    printMetaInfo(volume, "description");
    printMetaInfo(volume, "courtesy");
    printMetaInfo(volume, "parameter");
    printMetaInfo(volume, "comment");
}

void MPVMVolumeReader::printMetaInfo(const MetaDataOwner& metaDataOwner,
                                     std::string_view key) const {
    if (auto metaData = metaDataOwner.getMetaData<StringMetaData>(key)) {
        std::string metaStr = metaData->get();
        replaceInString(metaStr, "\n", ", ");
        log::info("{}: {}", key, metaStr);
    }
}

}  // namespace inviwo
