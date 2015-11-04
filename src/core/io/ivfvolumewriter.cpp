/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/io/ivfvolumewriter.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

IvfVolumeWriter::IvfVolumeWriter() : DataWriterType<Volume>() {
    addExtension(FileExtension("ivf","Inviwo ivf file format"));
}

IvfVolumeWriter::IvfVolumeWriter(const IvfVolumeWriter& rhs) : DataWriterType<Volume>(rhs) {
}

IvfVolumeWriter& IvfVolumeWriter::operator=(const IvfVolumeWriter& that) {
    if (this != &that)
        DataWriterType<Volume>::operator=(that);

    return *this;
}

IvfVolumeWriter* IvfVolumeWriter::clone() const {
    return new IvfVolumeWriter(*this);
}

void IvfVolumeWriter::writeData(const Volume* volume, const std::string filePath) const {
    std::string rawPath = filesystem::replaceFileExtension(filePath, "raw");

    if (filesystem::fileExists(filePath) && !overwrite_)
        throw DataWriterException("Error: Output file: " + filePath + " already exists", IvwContext);

    if (filesystem::fileExists(rawPath) && !overwrite_)
        throw DataWriterException("Error: Output file: " + rawPath + " already exists", IvwContext);

    std::string fileName = filesystem::getFileNameWithoutExtension(filePath);
    const VolumeRAM* vr = volume->getRepresentation<VolumeRAM>();
    Serializer s(filePath);
    s.serialize("RawFile", fileName + ".raw");
    s.serialize("Format", vr->getDataFormatString());
    s.serialize("BasisAndOffset", volume->getModelMatrix());
    s.serialize("WorldTransform", volume->getWorldMatrix());
    s.serialize("Dimension", volume->getDimensions());
    s.serialize("DataRange", volume->dataMap_.dataRange);
    s.serialize("ValueRange", volume->dataMap_.valueRange);
    s.serialize("Unit", volume->dataMap_.valueUnit);

    volume->getMetaDataMap()->serialize(s);
    s.writeFile();
    std::fstream fout(rawPath.c_str(), std::ios::out | std::ios::binary);

    if (fout.good()) {
        fout.write((char*)vr->getData(),
                   vr->getDimensions().x*vr->getDimensions().x*vr->getDimensions().x
                   * vr->getDataFormat()->getSize());
    } else
        throw DataWriterException("Error: Could not write to raw file: " + rawPath, IvwContext);

    fout.close();
}

} // namespace
