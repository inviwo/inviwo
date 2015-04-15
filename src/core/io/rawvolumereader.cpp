/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/io/rawvolumereader.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formatconversion.h>

namespace inviwo {

RawVolumeReader::RawVolumeReader()
    : DataReaderType<Volume>(), rawFile_(""), littleEndian_(true),
      dimensions_(0), spacing_(0.01f), format_(nullptr), parametersSet_(false) {
    addExtension(FileExtension("raw", "Raw binary file"));
}

RawVolumeReader::RawVolumeReader(const RawVolumeReader& rhs)
    : DataReaderType<Volume>(rhs)
    , rawFile_(rhs.rawFile_)
    , littleEndian_(true)
    , dimensions_(rhs.dimensions_)
    , spacing_(rhs.spacing_)
    , format_(rhs.format_)
    , parametersSet_(false) {}

RawVolumeReader& RawVolumeReader::operator=(const RawVolumeReader& that) {
    if (this != &that) {
        rawFile_ = that.rawFile_;
        littleEndian_ = that.littleEndian_;
        dimensions_ = that.dimensions_;
        spacing_ = that.spacing_;
        format_ = that.format_;
        DataReaderType<Volume>::operator=(that);
    }

    return *this;
}

RawVolumeReader* RawVolumeReader::clone() const {
    return new RawVolumeReader(*this);
}

void RawVolumeReader::setParameters(const DataFormatBase* format, ivec3 dimensions, bool littleEndian) {
    parametersSet_ = true;
    format_ = format;
    dimensions_ = dimensions;
    littleEndian_ = littleEndian;
}

Volume* RawVolumeReader::readMetaData(std::string filePath) {
    if (!filesystem::fileExists(filePath)) {
        std::string newPath = filesystem::addBasePath(filePath);

        if (filesystem::fileExists(newPath)) {
            filePath = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
        }
    }

    std::string fileDirectory = filesystem::getFileDirectory(filePath);
    std::string fileExtension = filesystem::getFileExtension(filePath);
    rawFile_ = filePath;

    if (!parametersSet_) {
        // TODO use uniqe ponter here. //Peter
        DataReaderDialog* readerDialog =
            dynamic_cast<DataReaderDialog*>(DialogFactory::getPtr()->getDialog("RawVolumeReader"));
        if (!readerDialog) {
            throw DataReaderException("No data reader dialog found.", IvwContext);
        }
        readerDialog->setFile(rawFile_);
        if (readerDialog->show()) {
            format_ = readerDialog->getFormat();
            dimensions_ = readerDialog->getDimensions();
            littleEndian_ = readerDialog->getEndianess();
            spacing_ = static_cast<glm::vec3>(readerDialog->getSpacing());
            delete readerDialog;
        } else {
            delete readerDialog;
            throw DataReaderException("Raw data import terminated by user", IvwContext);
        }
    }

    if (format_) {
        glm::mat3 basis(1.0f);
        glm::mat4 wtm(1.0f);

        basis[0][0] = dimensions_.x * spacing_.x;
        basis[1][1] = dimensions_.y * spacing_.y;
        basis[2][2] = dimensions_.z * spacing_.z;

        // Center the data around origo.
        glm::vec3 offset(-0.5f*(basis[0]+basis[1]+basis[2]));

        Volume* volume = new Volume();
        volume->setBasis(basis);
        volume->setOffset(offset);
        volume->setWorldMatrix(wtm);
        volume->setDimensions(dimensions_);
        volume->setDataFormat(format_);
        VolumeDisk* vd = new VolumeDisk(filePath, dimensions_, format_);
        vd->setDataReader(this->clone());
        volume->addRepresentation(vd);
        std::string size = formatBytesToString(dimensions_.x * dimensions_.y * dimensions_.z *
                                               (format_->getSize()));
        LogInfo("Loaded volume: " << filePath << " size: " << size);
        return volume;
    } else {
        throw DataReaderException("Raw data import could not determine format", IvwContext);
    }
}

void RawVolumeReader::readDataInto(void* destination) const {
    std::fstream fin(rawFile_.c_str(), std::ios::in | std::ios::binary);

    if (fin.good()) {
        std::size_t size = dimensions_.x*dimensions_.y*dimensions_.z*(format_->getSize());
        fin.read(static_cast<char*>(destination), size);

        if (!littleEndian_ && format_->getSize() > 1) {
            std::size_t bytes = format_->getSize();
            char* temp = new char[bytes];

            for (std::size_t i = 0; i < size; i += bytes) {
                for (std::size_t j = 0; j < bytes; j++)
                    temp[j] = static_cast<char*>(destination)[i + j];

                for (std::size_t j = 0; j < bytes; j++)
                    static_cast<char*>(destination)[i + j] = temp[bytes - j - 1];
            }

            delete [] temp;
        }
    } else
        throw DataReaderException("Error: Could not read from raw file: " + rawFile_, IvwContext);

    fin.close();
}

void* RawVolumeReader::readData() const {
    std::size_t size = dimensions_.x*dimensions_.y*dimensions_.z*(format_->getSize());
    char* data = new char[size];

    if (data)
        readDataInto(data);
    else
        throw DataReaderException("Error: Could not allocate memory for loading raw file: " + rawFile_, IvwContext);

    return data;
}

} // namespace

