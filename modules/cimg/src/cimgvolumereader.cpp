/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/cimg/cimgvolumereader.h>

#include <inviwo/core/datastructures/volume/volume.h>                // for Volume, DataReaderType
#include <inviwo/core/datastructures/volume/volumedisk.h>            // for VolumeDisk
#include <inviwo/core/datastructures/volume/volumeram.h>             // for createVolumeRAM
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/io/datareader.h>                               // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                      // for DataReaderException
#include <inviwo/core/metadata/metadata.h>                           // for StringMetaData, Meta...
#include <inviwo/core/metadata/metadataowner.h>                      // for MetaDataOwner
#include <inviwo/core/util/fileextension.h>                          // for FileExtension
#include <inviwo/core/util/filesystem.h>                             // for fileExists, addBasePath
#include <inviwo/core/util/formats.h>                                // for DataFormatId, DataFo...
#include <inviwo/core/util/glmvec.h>                                 // for size3_t
#include <inviwo/core/util/logcentral.h>                             // for LogCentral, LogInfo
#include <inviwo/core/util/sourcecontext.h>                          // for IVW_CONTEXT
#include <inviwo/core/util/stringconversion.h>                       // for replaceInString
#include <modules/cimg/cimgutils.h>                                  // for loadVolumeData

#include <ostream>                                                   // for operator<<, basic_os...
#include <type_traits>                                               // for remove_extent_t

namespace inviwo {

CImgVolumeReader::CImgVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("hdr", "Analyze 7.5"));
}

CImgVolumeReader* CImgVolumeReader::clone() const { return new CImgVolumeReader(*this); }

std::shared_ptr<Volume> CImgVolumeReader::readData(std::string_view filePath) {
    if (!filesystem::fileExists(filePath)) {
        throw DataReaderException(IVW_CONTEXT, "Error could not find input file: {}", filePath);
    }

    auto volumeDisk = std::make_shared<VolumeDisk>(filePath);
    volumeDisk->setLoader(new CImgVolumeRAMLoader(filePath));
    return std::make_shared<Volume>(volumeDisk);
}

void CImgVolumeReader::printMetaInfo(const MetaDataOwner& metaDataOwner,
                                     std::string_view key) const {
    if (auto metaData = metaDataOwner.getMetaData<StringMetaData>(key)) {
        std::string metaStr = metaData->get();
        replaceInString(metaStr, "\n", ", ");
        LogInfo(key << ": " << metaStr);
    }
}

CImgVolumeRAMLoader::CImgVolumeRAMLoader(std::string_view sourceFile) : sourceFile_{sourceFile} {}

CImgVolumeRAMLoader* CImgVolumeRAMLoader::clone() const { return new CImgVolumeRAMLoader(*this); }

std::shared_ptr<VolumeRepresentation> CImgVolumeRAMLoader::createRepresentation(
    const VolumeRepresentation& src) const {

    size3_t dimensions = src.getDimensions();
    DataFormatId formatId = DataFormatId::NotSpecialized;

    std::string fileName = sourceFile_;

    if (!filesystem::fileExists(fileName)) {
        const auto newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + fileName, IVW_CONTEXT);
        }
    }

    void* data = cimgutil::loadVolumeData(nullptr, fileName, dimensions, formatId);
    auto volumeRAM =
        createVolumeRAM(dimensions, DataFormatBase::get(formatId), data, src.getSwizzleMask(),
                        src.getInterpolation(), src.getWrapping());

    return volumeRAM;
}

void CImgVolumeRAMLoader::updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                               const VolumeRepresentation& src) const {
    auto volumeDst = std::static_pointer_cast<VolumeRAM>(dest);

    size3_t dimensions = src.getDimensions();
    DataFormatId formatId = DataFormatId::NotSpecialized;

    std::string fileName = sourceFile_;

    if (!filesystem::fileExists(fileName)) {
        const auto newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + fileName, IVW_CONTEXT);
        }
    }

    cimgutil::loadVolumeData(volumeDst->getData(), fileName, dimensions, formatId);
}

}  // namespace inviwo
