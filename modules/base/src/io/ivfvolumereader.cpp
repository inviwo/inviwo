/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/base/io/ivfvolumereader.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/rawvolumeramloader.h>

namespace inviwo {

IvfVolumeReader::IvfVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("ivf", "Inviwo ivf file format"));
}

IvfVolumeReader* IvfVolumeReader::clone() const { return new IvfVolumeReader(*this); }

std::shared_ptr<Volume> IvfVolumeReader::readData(const std::string& filePath) {
    if (!filesystem::fileExists(filePath)) {
        throw DataReaderException("Error could not find input file: " + filePath, IVW_CONTEXT);
    }

    std::string fileDirectory = filesystem::getFileDirectory(filePath);

    Deserializer d(filePath);

    std::string rawFile;
    size3_t dimensions{0u};
    size_t byteOffset = 0u;
    const DataFormatBase* format = nullptr;
    bool littleEndian = true;

    d.registerFactory(InviwoApplication::getPtr()->getMetaDataFactory());
    d.deserialize("RawFile", rawFile);
    rawFile = fileDirectory + "/" + rawFile;
    d.deserialize("ByteOffset", byteOffset);
    std::string formatFlag;
    d.deserialize("Format", formatFlag);
    format = DataFormatBase::get(formatFlag);
    d.deserialize("Dimension", dimensions);

    auto volume = std::make_shared<Volume>(dimensions, format);
    mat4 basisAndOffset = volume->getModelMatrix();
    mat4 worldTransform = volume->getWorldMatrix();
    d.deserialize("BasisAndOffset", basisAndOffset);
    d.deserialize("WorldTransform", worldTransform);
    volume->setModelMatrix(basisAndOffset);
    volume->setWorldMatrix(worldTransform);

    d.deserialize("DataRange", volume->dataMap_.dataRange);
    d.deserialize("ValueRange", volume->dataMap_.valueRange);
    d.deserialize("Unit", volume->dataMap_.valueUnit);

    volume->getMetaDataMap()->deserialize(d);
    littleEndian = volume->getMetaData<BoolMetaData>("LittleEndian", littleEndian);
    auto vd = std::make_shared<VolumeDisk>(filePath, dimensions, format);

    auto loader =
        std::make_unique<RawVolumeRAMLoader>(rawFile, byteOffset, dimensions, littleEndian, format);
    vd->setLoader(loader.release());

    volume->addRepresentation(vd);
    return volume;
}

}  // namespace inviwo
