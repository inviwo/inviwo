/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <modules/cimg/tiffstackvolumereader.h>

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/cimg/cimgutils.h>

#include <type_traits>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <fmt/std.h>

namespace inviwo {

TIFFStackVolumeReader::TIFFStackVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("tif", "TIFF Stack"));
    addExtension(FileExtension("tiff", "TIFF Stack"));
}

TIFFStackVolumeReader* TIFFStackVolumeReader::clone() const {
    return new TIFFStackVolumeReader(*this);
}

std::shared_ptr<Volume> TIFFStackVolumeReader::readData(const std::filesystem::path& filePath) {
    const auto localPath = downloadAndCacheIfUrl(filePath);
    checkExists(localPath);

    auto header = cimgutil::getTIFFHeader(localPath);
    auto volumeDisk = std::make_shared<VolumeDisk>(localPath, header.dimensions, header.format);
    auto volume = std::make_shared<Volume>(volumeDisk);

    volume->dataMap.dataRange = dvec2{header.format->getLowest(), header.format->getMax()};
    volume->dataMap.valueRange = dvec2{header.format->getLowest(), header.format->getMax()};

    vec3 extent{vec3{header.dimensions} / vec3{header.resolution, glm::compMin(header.resolution)}};
    if (header.resolutionUnit == cimgutil::TIFFResolutionUnit::Centimeter) {
        extent *= 2.54f;
    }
    volume->setBasis(glm::scale(extent));
    volume->setOffset(-extent * 0.5f);

    volumeDisk->setLoader(new TIFFStackVolumeRAMLoader(localPath));
    volume->addRepresentation(volumeDisk);

    return volume;
}

TIFFStackVolumeRAMLoader::TIFFStackVolumeRAMLoader(const std::filesystem::path& sourceFile)
    : sourceFile_{sourceFile} {}

TIFFStackVolumeRAMLoader* TIFFStackVolumeRAMLoader::clone() const {
    return new TIFFStackVolumeRAMLoader(*this);
}

std::shared_ptr<VolumeRepresentation> TIFFStackVolumeRAMLoader::createRepresentation(
    const VolumeRepresentation& src) const {
    const auto fileName = findFile(sourceFile_);

    auto volumeRAM = cimgutil::loadVolume(fileName, src.getDataFormat(), src.getDimensions());
    volumeRAM->setWrapping(src.getWrapping());
    volumeRAM->setInterpolation(src.getInterpolation());
    volumeRAM->setSwizzleMask(src.getSwizzleMask());
    return volumeRAM;
}

void TIFFStackVolumeRAMLoader::updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                                    const VolumeRepresentation& src) const {
    auto volumeDst = std::static_pointer_cast<VolumeRAM>(dest);

    const auto fileName = findFile(sourceFile_);
    cimgutil::updateVolume(*volumeDst, fileName);
    volumeDst->setWrapping(src.getWrapping());
    volumeDst->setInterpolation(src.getInterpolation());
    volumeDst->setSwizzleMask(src.getSwizzleMask());
}

}  // namespace inviwo
