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

#include <modules/base/io/datvolumewriter.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/io/datawriterexception.h>

namespace inviwo {

DatVolumeWriter::DatVolumeWriter() : DataWriterType<Volume>() {
    addExtension(FileExtension("dat", "Inviwo dat file format"));
}

DatVolumeWriter::DatVolumeWriter(const DatVolumeWriter& rhs) : DataWriterType<Volume>(rhs) {}

DatVolumeWriter& DatVolumeWriter::operator=(const DatVolumeWriter& that) {
    if (this != &that) DataWriterType<Volume>::operator=(that);

    return *this;
}

DatVolumeWriter* DatVolumeWriter::clone() const { return new DatVolumeWriter(*this); }

void DatVolumeWriter::writeData(const Volume* data, const std::string filePath) const {
    std::string rawPath = filesystem::replaceFileExtension(filePath, "raw");

    if (filesystem::fileExists(filePath) && !overwrite_)
        throw DataWriterException("Error: Output file: " + filePath + " already exists",
                                  IVW_CONTEXT);

    if (filesystem::fileExists(rawPath) && !overwrite_)
        throw DataWriterException("Error: Output file: " + rawPath + " already exists",
                                  IVW_CONTEXT);

    std::string fileName = filesystem::getFileNameWithoutExtension(filePath);
    // Write the .dat file content
    std::stringstream ss;
    const VolumeRAM* vr = data->getRepresentation<VolumeRAM>();
    glm::mat3 basis = glm::transpose(data->getBasis());
    glm::vec3 offset = data->getOffset();
    glm::mat4 wtm = glm::transpose(data->getWorldMatrix());
    writeKeyToString(ss, "RawFile", fileName + ".raw");
    writeKeyToString(ss, "Resolution", vr->getDimensions());
    writeKeyToString(ss, "Format", vr->getDataFormatString());
    writeKeyToString(ss, "ByteOffset", "0");
    writeKeyToString(ss, "BasisVector1", basis[0]);
    writeKeyToString(ss, "BasisVector2", basis[1]);
    writeKeyToString(ss, "BasisVector3", basis[2]);
    writeKeyToString(ss, "Offset", offset);
    writeKeyToString(ss, "WorldVector1", wtm[0]);
    writeKeyToString(ss, "WorldVector2", wtm[1]);
    writeKeyToString(ss, "WorldVector3", wtm[2]);
    writeKeyToString(ss, "WorldVector4", wtm[3]);
    writeKeyToString(ss, "DataRange", data->dataMap_.dataRange);
    writeKeyToString(ss, "ValueRange", data->dataMap_.valueRange);
    writeKeyToString(ss, "Unit", data->dataMap_.valueUnit);

    std::vector<std::string> keys = data->getMetaDataMap()->getKeys();

    for (auto& key : keys) {
        auto m = data->getMetaDataMap()->get(key);
        if (auto sm = dynamic_cast<const StringMetaData*>(m)) writeKeyToString(ss, key, sm->get());
    }

    if (auto f = filesystem::ofstream(filePath)) {
        f << ss.str();
    } else {
        throw DataWriterException("Error: Could not write to dat file: " + filePath, IVW_CONTEXT);
    }

    if (auto f = filesystem::ofstream(rawPath, std::ios::out | std::ios::binary)) {
        f.write((char*)vr->getData(), vr->getNumberOfBytes());
    } else {
        throw DataWriterException("Error: Could not write to raw file: " + rawPath, IVW_CONTEXT);
    }
}

void DatVolumeWriter::writeKeyToString(std::stringstream& ss, const std::string& key,
                                       const std::string& str) const {
    ss << key << ": " << str << std::endl;
}

}  // namespace inviwo
