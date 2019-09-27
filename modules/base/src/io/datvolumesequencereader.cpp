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

#include <modules/base/io/datvolumesequencereader.h>
#include <modules/base/algorithm/dataminmax.h>

#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/rawvolumeramloader.h>

#include <iterator>

namespace inviwo {

DatVolumeSequenceReader::DatVolumeSequenceReader()
    : DataReaderType<VolumeSequence>(), enableLogOutput_(true) {
    addExtension(FileExtension("dat", "Inviwo dat file format"));
}

DatVolumeSequenceReader::DatVolumeSequenceReader(const DatVolumeSequenceReader& rhs)
    : DataReaderType<VolumeSequence>(rhs), enableLogOutput_(rhs.enableLogOutput_) {}

DatVolumeSequenceReader& DatVolumeSequenceReader::operator=(const DatVolumeSequenceReader& that) {
    if (this != &that) {
        enableLogOutput_ = that.enableLogOutput_;
        DataReaderType<VolumeSequence>::operator=(that);
    }

    return *this;
}

DatVolumeSequenceReader* DatVolumeSequenceReader::clone() const {
    return new DatVolumeSequenceReader(*this);
}

std::shared_ptr<DatVolumeSequenceReader::VolumeSequence> DatVolumeSequenceReader::readData(
    const std::string& filePath) {
    std::string fileName = filePath;
    if (!filesystem::fileExists(fileName)) {
        std::string newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + fileName, IVW_CONTEXT);
        }
    }

    std::string fileDirectory = filesystem::getFileDirectory(fileName);
    std::string fileExtension = filesystem::getFileExtension(fileName);
    // Read the dat file content

    auto f = filesystem::ifstream(fileName);

    if (!f.is_open()) {
        throw DataReaderException("Error could open file: " + fileName, IVW_CONTEXT);
    }

    std::string rawFile;
    size3_t dimensions{0u};
    size_t byteOffset = 0u;
    const DataFormatBase* format = nullptr;
    bool littleEndian = true;

    std::string formatFlag = "";
    glm::mat3 basis(2.0f);
    glm::vec3 offset(0.0f);
    bool hasOffset = false;
    glm::vec3 spacing(0.0f);
    glm::mat4 wtm(1.0f);
    glm::vec3 a(0.0f), b(0.0f), c(0.0f);
    std::string key;
    std::string value;
    dvec2 datarange(0);
    dvec2 valuerange(0);
    std::string unit("");
    size_t sequences = 1;

    std::unordered_map<std::string, std::string> metadata;

    // For dat file containing references to multiple datfiles
    std::vector<std::string> datFiles;

    std::string textLine;
    std::vector<std::string> parts;
    while (!f.eof()) {
        getline(f, textLine);

        textLine = trim(textLine);

        if (textLine == "" || textLine[0] == '#' || textLine[0] == '/') continue;
        parts = splitString(splitString(textLine, '#')[0], ':');
        if (parts.size() != 2) continue;

        key = toLower(trim(parts[0]));
        value = trim(parts[1]);

        std::stringstream ss(value);

        if (key == "objectfilename" || key == "rawfile") {
            rawFile = fileDirectory + "/" + value;
        } else if (key == "datfile") {
            datFiles.push_back(fileDirectory + "/" + value);
        } else if (key == "byteorder") {
            if (toLower(value) == "bigendian") {
                littleEndian = false;
            }
        } else if (key == "byteoffset") {
            ss >> byteOffset;
        } else if (key == "sequences") {
            ss >> sequences;
        } else if (key == "resolution" || key == "dimensions") {
            ss >> dimensions.x;
            ss >> dimensions.y;
            ss >> dimensions.z;
        } else if (key == "spacing" || key == "slicethickness") {
            ss >> spacing.x;
            ss >> spacing.y;
            ss >> spacing.z;
        } else if (key == "basisvector1") {
            ss >> a.x;
            ss >> a.y;
            ss >> a.z;
        } else if (key == "basisvector2") {
            ss >> b.x;
            ss >> b.y;
            ss >> b.z;
        } else if (key == "basisvector3") {
            ss >> c.x;
            ss >> c.y;
            ss >> c.z;
        } else if (key == "offset") {
            hasOffset = true;
            ss >> offset.x;
            ss >> offset.y;
            ss >> offset.z;
        } else if (key == "worldvector1") {
            ss >> wtm[0][0];
            ss >> wtm[1][0];
            ss >> wtm[2][0];
            ss >> wtm[3][0];
        } else if (key == "worldvector2") {
            ss >> wtm[0][1];
            ss >> wtm[1][1];
            ss >> wtm[2][1];
            ss >> wtm[3][1];
        } else if (key == "worldvector3") {
            ss >> wtm[0][2];
            ss >> wtm[1][2];
            ss >> wtm[2][2];
            ss >> wtm[3][2];
        } else if (key == "worldvector4") {
            ss >> wtm[0][3];
            ss >> wtm[1][3];
            ss >> wtm[2][3];
            ss >> wtm[3][3];
        } else if (key == "format") {
            ss >> formatFlag;
            // Backward support for USHORT_12 key
            if (formatFlag == "USHORT_12") {
                format = DataUInt16::get();
                // Check so that data range has not been set before
                if (glm::all(glm::equal(datarange, dvec2(0)))) {
                    datarange.y = 4095;
                }
            } else {
                format = DataFormatBase::get(formatFlag);
            }
        } else if (key == "datarange") {
            ss >> datarange.x;
            ss >> datarange.y;
        } else if (key == "valuerange") {
            ss >> valuerange.x;
            ss >> valuerange.y;
        } else if (key == "unit") {
            unit = value;
        } else {
            metadata[key] = value;
        }
    };

    // Check if other dat files where specified, and then only consider them as a sequence
    auto volumes = std::make_shared<VolumeSequence>();

    if (!datFiles.empty()) {
        for (size_t t = 0; t < datFiles.size(); ++t) {
            auto datVolReader = std::make_unique<DatVolumeSequenceReader>();
            datVolReader->enableLogOutput_ = false;
            auto v = datVolReader->readData(datFiles[t]);

            std::copy(v->begin(), v->end(), std::back_inserter(*volumes));
        }
        if (enableLogOutput_) {
            LogInfo("Loaded multiple volumes: " << fileName << " volumes: " << datFiles.size());
        }

    } else {
        if (dimensions == size3_t(0))
            throw DataReaderException(
                "Error: Unable to find \"Resolution\" tag in .dat file: " + fileName, IVW_CONTEXT);
        else if (format == nullptr)
            throw DataReaderException(
                "Error: Unable to find \"Format\" tag in .dat file: " + fileName, IVW_CONTEXT);
        else if (format->getId() == DataFormatId::NotSpecialized)
            throw DataReaderException(
                "Error: Invalid format string found: " + formatFlag + " in " + fileName +
                    "\nThe valid formats are:\n" +
                    "FLOAT16, FLOAT32, FLOAT64, INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, "
                    "UINT64, Vec2FLOAT16, Vec2FLOAT32, Vec2FLOAT64, Vec2INT8, Vec2INT16, "
                    "Vec2INT32, Vec2INT64, Vec2UINT8, Vec2UINT16, Vec2UINT32, Vec2UINT64, "
                    "Vec3FLOAT16, Vec3FLOAT32, Vec3FLOAT64, Vec3INT8, Vec3INT16, Vec3INT32, "
                    "Vec3INT64, Vec3UINT8, Vec3UINT16, Vec3UINT32, Vec3UINT64, Vec4FLOAT16, "
                    "Vec4FLOAT32, Vec4FLOAT64, Vec4INT8, Vec4INT16, Vec4INT32, Vec4INT64, "
                    "Vec4UINT8, Vec4UINT16, Vec4UINT32, Vec4UINT64",
                IVW_CONTEXT);

        else if (rawFile == "")
            throw DataReaderException(
                "Error: Unable to find \"ObjectFilename\" tag in .dat file: " + fileName,
                IVW_CONTEXT);

        if (spacing != vec3(0.0f)) {
            basis[0][0] = dimensions.x * spacing.x;
            basis[1][1] = dimensions.y * spacing.y;
            basis[2][2] = dimensions.z * spacing.z;
        }

        if (a != vec3(0.0f) && b != vec3(0.0f) && c != vec3(0.0f)) {
            basis[0][0] = a.x;
            basis[1][0] = a.y;
            basis[2][0] = a.z;
            basis[0][1] = b.x;
            basis[1][1] = b.y;
            basis[2][1] = b.z;
            basis[0][2] = c.x;
            basis[1][2] = c.y;
            basis[2][2] = c.z;
        }

        // If not specified, center the data around origo.
        if (!hasOffset) {
            offset = -0.5f * (basis[0] + basis[1] + basis[2]);
        }

        auto volume = std::make_shared<Volume>(dimensions, format);
        volume->setBasis(basis);
        volume->setOffset(offset);
        volume->setWorldMatrix(wtm);

        if (datarange != dvec2(0)) {
            volume->dataMap_.dataRange = datarange;
        }
        if (valuerange != dvec2(0)) {
            volume->dataMap_.valueRange = valuerange;
        } else {
            volume->dataMap_.valueRange = volume->dataMap_.dataRange;
        }
        if (unit != "") {
            volume->dataMap_.valueUnit = unit;
        }

        size_t bytes = dimensions.x * dimensions.y * dimensions.z * (format->getSize());

        for (auto elem : metadata) volume->setMetaData<StringMetaData>(elem.first, elem.second);

        for (size_t t = 0; t < sequences; ++t) {
            if (t == 0)
                volumes->push_back(std::move(volume));
            else
                volumes->push_back(std::shared_ptr<Volume>(volumes->front()->clone()));
            auto diskRepr = std::make_shared<VolumeDisk>(fileName, dimensions, format);
            size_t filePos = t * bytes + byteOffset;

            auto loader = std::make_unique<RawVolumeRAMLoader>(rawFile, filePos, dimensions,
                                                               littleEndian, format);
            diskRepr->setLoader(loader.release());
            volumes->back()->addRepresentation(diskRepr);
            // Compute data range if not specified
            if (t == 0 && datarange == dvec2(0)) {
                // Use min/max value in data as data range if none is given
                // Only consider first time step since it can be time consuming
                // to compute for all time steps
                auto minmax = util::volumeMinMax(volumes->front().get(), IgnoreSpecialValues::No);
                // minmax always have four components, unused components are set to zero.
                // Hence, only consider components used by the data format
                dvec2 computedRange(minmax.first[0], minmax.second[0]);
                for (size_t component = 1; component < format->getComponents(); ++component) {
                    computedRange = dvec2(glm::min(computedRange[0], minmax.first[component]),
                                          glm::max(computedRange[1], minmax.second[component]));
                }
                // Set value range
                volumes->front()->dataMap_.dataRange = computedRange;
                // Also set value range if not specified
                if (valuerange == dvec2(0)) {
                    volumes->front()->dataMap_.valueRange = computedRange;
                }
                // Performance warning for larger volumes (rougly > 2MB)
                if (bytes < 128 * 128 * 128 || sequences > 1) {
                    LogWarn(
                        "Performance warning: Using min/max of data since DataRange was not "
                        "specified. "
                        << "Data range refer to the range of the data type, i.e. [0 4095] for "
                           "12-bit unsigned integer data. "
                        << "It is important that the data range is specified for data types with a "
                           "large range "
                        << "(for example 32/64-bit float and integer) since the data is often "
                           "normalized to [0 1], "
                        << "when for example performing color mapping, i.e. applying a transfer "
                           "function."
                        << std::endl
                        << "Value range refer to the physical meaning of the value, i.e. "
                           "Hounsfield "
                           "value range is from [-1000 3000]. "
                        << "Improve volume read performance by adding for example: " << std::endl
                        << "DataRange: " << computedRange[0] << " " << computedRange[1] << std::endl
                        << "ValueRange: " << computedRange[0] << " " << computedRange[1]
                        << std::endl
                        << "in file: " << fileName);
                }
                if (sequences > 1) {
                    LogWarn(
                        "Multiple volumes in file but we only computed DataRange of the first "
                        "volume sequence due to performance consideration. "
                        << std::endl
                        << "We strongly recommend setting the DataRange, for example to min(Volume "
                           "sequence), max(Volume sequence). ");
                }
            }
        }

        std::string size = util::formatBytesToString(bytes * sequences);
        if (enableLogOutput_) {
            LogInfo("Loaded volume sequence: " << fileName << " size: " << size);
        }
    }
    return volumes;
}

}  // namespace inviwo
