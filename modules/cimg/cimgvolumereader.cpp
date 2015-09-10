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

#include <modules/cimg/cimgvolumereader.h>
#include <modules/cimg/cimgutils.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

CImgVolumeReader::CImgVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("hdr", "Analyze 7.5"));
}

CImgVolumeReader::CImgVolumeReader(const CImgVolumeReader& rhs) : DataReaderType<Volume>(rhs){};

CImgVolumeReader& CImgVolumeReader::operator=(const CImgVolumeReader& that) {
    if (this != &that) {
        DataReaderType<Volume>::operator=(that);
    }

    return *this;
}

CImgVolumeReader* CImgVolumeReader::clone() const { return new CImgVolumeReader(*this); }

Volume* CImgVolumeReader::readMetaData(std::string filePath) {
    if (!filesystem::fileExists(filePath)) {
        std::string newPath = filesystem::addBasePath(filePath);

        if (filesystem::fileExists(newPath)) {
            filePath = newPath;
        }
        else {
            throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
        }
    }

    Volume* volume = new Volume();

    VolumeDisk* volumeDisk = new VolumeDisk(filePath);
    volumeDisk->setDataReader(this->clone());

    volume->addRepresentation(volumeDisk);

    return volume;
}

void CImgVolumeReader::readDataInto(void* destination) const {
    VolumeDisk* volumeDisk = dynamic_cast<VolumeDisk*>(owner_);
    if (volumeDisk){
        size3_t dimensions = volumeDisk->getDimensions();
        DataFormatEnums::Id formatId = DataFormatEnums::NOT_SPECIALIZED;

        std::string filePath = volumeDisk->getSourceFile();

        if (!filesystem::fileExists(filePath)) {
            std::string newPath = filesystem::addBasePath(filePath);

            if (filesystem::fileExists(newPath)) {
                filePath = newPath;
            }
            else {
                throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
            }
        }

        CImgUtils::loadVolumeData(destination, filePath, dimensions, formatId);
        volumeDisk->setDimensions(dimensions);
    }
}

void* CImgVolumeReader::readData() const {
    void* data = nullptr;

    VolumeDisk* volumeDisk = dynamic_cast<VolumeDisk*>(owner_);
    if (volumeDisk){
        size3_t dimensions = volumeDisk->getDimensions();
        DataFormatEnums::Id formatId = DataFormatEnums::NOT_SPECIALIZED;

        std::string filePath = volumeDisk->getSourceFile();

        if (!filesystem::fileExists(filePath)) {
            std::string newPath = filesystem::addBasePath(filePath);

            if (filesystem::fileExists(newPath)) {
                filePath = newPath;
            }
            else {
                throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
            }
        }

        data = CImgUtils::loadVolumeData(nullptr, filePath, dimensions, formatId);
        volumeDisk->setDimensions(dimensions);
    }

    return data;
}

void CImgVolumeReader::printMetaInfo(MetaDataOwner* metaDataOwner, std::string key) {
    StringMetaData* metaData = metaDataOwner->getMetaData<StringMetaData>(key);
    if (metaData) {
        std::string metaStr = metaData->get();
        replaceInString(metaStr, "\n", ", ");
        key[0] = static_cast<char>(toupper(key[0]));
        LogInfo(key << ": " << metaStr);
    }
}

}  // namespace
