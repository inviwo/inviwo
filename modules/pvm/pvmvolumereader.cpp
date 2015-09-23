/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <modules/pvm/pvmvolumereader.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datareaderexception.h>
#include "ext/tidds/ddsbase.h"

namespace inviwo {

PVMVolumeReader::PVMVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("pvm", "PVM file format"));
}

PVMVolumeReader* PVMVolumeReader::clone() const { return new PVMVolumeReader(*this); }

std::shared_ptr<Volume> PVMVolumeReader::readData(const std::string filePath) {
    auto volume = readPVMData(filePath);

    if (!volume) return std::shared_ptr<Volume>();

    // Print information
    size3_t dim = volume->getDimensions();
    size_t bytes = dim.x * dim.y * dim.z * (volume->getDataFormat()->getSize());
    std::string size = formatBytesToString(bytes);
    LogInfo("Loaded volume: " << filePath << " size: " << size);
    printMetaInfo(*volume, "description");
    printMetaInfo(*volume, "courtesy");
    printMetaInfo(*volume, "parameter");
    printMetaInfo(*volume, "comment");

    return volume;
}

std::shared_ptr<Volume> PVMVolumeReader::readPVMData(std::string filePath) {
    if (!filesystem::fileExists(filePath)) {
        std::string newPath = filesystem::addBasePath(filePath);

        if (filesystem::fileExists(newPath)) {
            filePath = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + filePath,
                                      IvwContextCustom("PVMVolumeReader"));
        }
    }

    size3_t dim(0);
    glm::mat3 basis(2.0f);
    glm::vec3 spacing(0.0f);

    // Reading MPVM volume
    unsigned char* data = nullptr;
    unsigned int bytesPerVoxel;
    unsigned char* description;
    unsigned char* courtesy;
    unsigned char* parameter;
    unsigned char* comment;

    try {
        uvec3 udim{0};
        data = readPVMvolume(filePath.c_str(), &udim.x, &udim.y, &udim.z, &bytesPerVoxel, &spacing.x,
                             &spacing.y, &spacing.z, &description, &courtesy, &parameter, &comment);
        dim = udim;

    } catch (Exception& e) {
        LogErrorCustom("PVMVolumeReader", e.what());
    }

    if (data == nullptr) {
        throw DataReaderException("Error: Could not read data in PVM file: " + filePath,
                                  IvwContextCustom("PVMVolumeReader"));
    }

    const DataFormatBase* format = nullptr;

    switch (bytesPerVoxel) {
        case 1:
            format = DataUINT8::get();
            break;
        case 2:
            format = DataUINT16::get();
            break;
        case 3:
            format = DataVec3UINT8::get();
            break;
        default:
            throw DataReaderException(
                "Error: Unsupported format (bytes per voxel) in .pvm file: " + filePath,
                IvwContextCustom("PVMVolumeReader"));
    }

    if (dim == size3_t(0)) {
        throw DataReaderException("Error: Unable to find dimensions in .pvm file: " + filePath,
                                  IvwContextCustom("PVMVolumeReader"));
    }

    auto volume = std::make_shared<Volume>();

    if (format == DataUINT16::get()) {
        size_t bytes = format->getSize();
        size_t size = dim.x * dim.y * dim.z * bytes;
        swapbytes(data, static_cast<unsigned int>(size));
        // This format does not contain information about data range
        // so we need to compute it for correct results
        auto minmax = std::minmax_element(reinterpret_cast<DataUINT16::type*>(data),
                                          reinterpret_cast<DataUINT16::type*>(data + size));
        volume->dataMap_.dataRange = dvec2(*minmax.first, *minmax.second);
    }

    // Additional information
    std::stringstream ss;

    if (description) {
        ss << description;
        volume->setMetaData<StringMetaData>("description", ss.str());
    }

    if (courtesy) {
        ss.clear();
        ss.str("");
        ss << courtesy;
        volume->setMetaData<StringMetaData>("courtesy", ss.str());
    }

    if (parameter) {
        ss.clear();
        ss.str("");
        ss << parameter;
        volume->setMetaData<StringMetaData>("parameter", ss.str());
    }

    if (comment) {
        ss.clear();
        ss.str("");
        ss << comment;
        volume->setMetaData<StringMetaData>("comment", ss.str());
    }

    if (spacing != vec3(0.0f)) {
        basis[0][0] = dim.x * spacing.x;
        basis[1][1] = dim.y * spacing.y;
        basis[2][2] = dim.z * spacing.z;
    }

    volume->setBasis(basis);
    volume->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));
    volume->setDimensions(dim);

    volume->dataMap_.initWithFormat(format);

    volume->setDataFormat(format);

    // Create RAM volume s all data has already is in memory
    auto volRAM = createVolumeRAM(dim, format, data);
    volume->addRepresentation(volRAM);

    return volume;
}

void PVMVolumeReader::printMetaInfo(const MetaDataOwner& metaDataOwner, std::string key) const {
    if (auto metaData = metaDataOwner.getMetaData<StringMetaData>(key)) {
        std::string metaStr = metaData->get();
        replaceInString(metaStr, "\n", ", ");
        key[0] = static_cast<char>(toupper(key[0]));
        LogInfo(key << ": " << metaStr);
    }
}

}  // namespace
