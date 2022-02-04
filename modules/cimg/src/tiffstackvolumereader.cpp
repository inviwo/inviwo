/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <tiffio.h>

#include <modules/cimg/cimgutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>

#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

TIFFStackVolumeReader::TIFFStackVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("tif", "TIFF Stack"));
    addExtension(FileExtension("tiff", "TIFF Stack"));
}

TIFFStackVolumeReader* TIFFStackVolumeReader::clone() const {
    return new TIFFStackVolumeReader(*this);
}

std::shared_ptr<Volume> TIFFStackVolumeReader::readData(std::string_view filePath) {
    checkExists(filePath);

    auto header = cimgutil::getTIFFHeader(filePath);
    auto volume = std::make_shared<Volume>(header.dimensions, header.format);
    auto volumeDisk = std::make_shared<VolumeDisk>(filePath, header.dimensions, header.format);
    volume->dataMap_.dataRange = dvec2{header.format->getLowest(), header.format->getMax()};
    volume->dataMap_.valueRange = dvec2{header.format->getLowest(), header.format->getMax()};

    vec3 extent{vec3{header.dimensions} / vec3{header.resolution, glm::compMin(header.resolution)}};
    if (header.resolutionUnit == cimgutil::TIFFResolutionUnit::Centimeter) {
        extent *= 2.54f;
    }
    volume->setBasis(glm::scale(extent));
    volume->setOffset(-extent * 0.5f);

    volumeDisk->setLoader(new TIFFStackVolumeRAMLoader(filePath));
    volume->addRepresentation(volumeDisk);

    return volume;
}

TIFFStackVolumeRAMLoader::TIFFStackVolumeRAMLoader(std::string_view sourceFile)
    : sourceFile_{sourceFile} {}

TIFFStackVolumeRAMLoader* TIFFStackVolumeRAMLoader::clone() const {
    return new TIFFStackVolumeRAMLoader(*this);
}

std::shared_ptr<VolumeRepresentation> TIFFStackVolumeRAMLoader::createRepresentation(
    const VolumeRepresentation& src) const {
    std::string fileName = sourceFile_;

    if (!filesystem::fileExists(fileName)) {
        const auto newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException(IVW_CONTEXT, "Error could not find input file: {}", fileName);
        }
    }
    cimgutil::TIFFHeader header;
    header.format = src.getDataFormat();
    header.dimensions = src.getDimensions();
    auto data = cimgutil::loadTIFFVolumeData(nullptr, fileName, header);

    auto volumeRAM =
        createVolumeRAM(src.getDimensions(), src.getDataFormat(), data, src.getSwizzleMask(),
                        src.getInterpolation(), src.getWrapping());

    return volumeRAM;
}

void TIFFStackVolumeRAMLoader::updateRepresentation(std::shared_ptr<VolumeRepresentation> dest,
                                                    const VolumeRepresentation& src) const {
    auto volumeDst = std::static_pointer_cast<VolumeRAM>(dest);

    std::string fileName = sourceFile_;
    if (!filesystem::fileExists(fileName)) {
        const auto newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException(IVW_CONTEXT, "Error could not find input file: {}", fileName);
        }
    }

    cimgutil::TIFFHeader header;
    header.format = src.getDataFormat();
    header.dimensions = src.getDimensions();
    cimgutil::loadTIFFVolumeData(volumeDst->getData(), fileName, header);
}

}  // namespace inviwo
