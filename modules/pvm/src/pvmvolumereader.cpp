/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datareaderexception.h>
#include <tidds/ddsbase.h>

namespace inviwo {

PVMVolumeReader::PVMVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("pvm", "PVM file format"));
}

PVMVolumeReader* PVMVolumeReader::clone() const { return new PVMVolumeReader(*this); }

std::shared_ptr<Volume> PVMVolumeReader::readData(const std::string& filePath) {
    if (!filesystem::fileExists(filePath)) {
        throw DataReaderException("Error could not find input file: " + filePath, IVW_CONTEXT);
    }
    auto volume = readPVMData(filePath);

    if (!volume) return std::shared_ptr<Volume>();

    // Print information
    size3_t dim = volume->getDimensions();
    size_t bytes = dim.x * dim.y * dim.z * (volume->getDataFormat()->getSize());
    std::string size = util::formatBytesToString(bytes);
    LogInfo("Loaded volume: " << filePath << " size: " << size);
    printMetaInfo(*volume, "description");
    printMetaInfo(*volume, "courtesy");
    printMetaInfo(*volume, "parameter");
    printMetaInfo(*volume, "comment");

    return volume;
}

std::shared_ptr<Volume> PVMVolumeReader::readPVMData(std::string filePath) {
    uvec3 udim{0};
    vec3 spacing(0.0f);
    unsigned int bytesPerVoxel = 0;

    // these pointers refer to positions within pvmdata
    unsigned char* description = nullptr;
    unsigned char* courtesy = nullptr;
    unsigned char* parameter = nullptr;
    unsigned char* comment = nullptr;

    unsigned char* pvmdata =
        readPVMvolume(filePath.c_str(), &udim.x, &udim.y, &udim.z, &bytesPerVoxel, &spacing.x,
                      &spacing.y, &spacing.z, &description, &courtesy, &parameter, &comment);

    util::OnScopeExit release([&]() { free(pvmdata); });

    if (!pvmdata) {
        throw DataReaderException("Error: Could not read data in PVM file: " + filePath,
                                  IVW_CONTEXT_CUSTOM("PVMVolumeReader"));
    }
    if (udim == uvec3{0}) {
        throw DataReaderException("Error: Unable to find dimensions in .pvm file: " + filePath,
                                  IVW_CONTEXT_CUSTOM("PVMVolumeReader"));
    }
    const size_t volsize = glm::compMul(udim) * bytesPerVoxel;
    if (bytesPerVoxel == 2) {
        // swap byte order for DataUInt16,
        swapbytes(pvmdata, static_cast<unsigned int>(volsize));
    }

    // re-allocate the volume using "new" and copy the data
    auto data = std::make_unique<unsigned char[]>(volsize);
    std::copy(pvmdata, pvmdata + volsize, data.get());

    const DataFormatBase* format = [bytesPerVoxel, filePath]() -> const DataFormatBase* {
        switch (bytesPerVoxel) {
            case 1:
                return DataUInt8::get();
            case 2:
                return DataUInt16::get();
            case 3:
                return DataVec3UInt8::get();
            default:
                throw DataReaderException(
                    "Error: Unsupported format (bytes per voxel) in .pvm file: " + filePath,
                    IVW_CONTEXT_CUSTOM("PVMVolumeReader"));
        }
    }();

    auto volRAM = createVolumeRAM(size3_t(udim), format, data.release());
    auto volume = std::make_shared<Volume>(volRAM);

    mat3 basis(2.0f);
    if (spacing != vec3(0.0f)) {
        basis[0][0] = udim.x * spacing.x;
        basis[1][1] = udim.y * spacing.y;
        basis[2][2] = udim.z * spacing.z;
    }
    volume->setBasis(basis);
    volume->setOffset(-0.5f * (basis[0] + basis[1] + basis[2]));

    // Additional information
    if (description) {
        volume->setMetaData<StringMetaData>("description", toString(description));
    }
    if (courtesy) {
        volume->setMetaData<StringMetaData>("courtesy", toString(courtesy));
    }
    if (parameter) {
        volume->setMetaData<StringMetaData>("parameter", toString(parameter));
    }
    if (comment) {
        volume->setMetaData<StringMetaData>("comment", toString(comment));
    }

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

}  // namespace inviwo
