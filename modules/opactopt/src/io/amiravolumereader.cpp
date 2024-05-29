/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/opactopt/io/amiravolumereader.h>

#include <inviwo/core/io/datareader.h>           // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>  // for DataReaderException
#include <inviwo/core/util/fileextension.h>      // for FileExtension
#include <inviwo/core/util/glmvec.h>             // for vec3, vec4
#include <inviwo/core/util/logcentral.h>         // for LogVerbosity, Log...
#include <inviwo/core/io/rawvolumeramloader.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/util/formatconversion.h>

#include <cstring>  // for strlen
#include <format>
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>    // type conversion
namespace py = pybind11;

namespace inviwo {

AmiraVolumeReader::AmiraVolumeReader() : DataReaderType<Volume>() {
    addExtension(FileExtension("am", "AMIRA volume reader"));
}

AmiraVolumeReader* AmiraVolumeReader::clone() const { return new AmiraVolumeReader(*this); }

const char* FindAndJump(const char* buffer, const char* SearchString) {
    const char* FoundLoc = strstr(buffer, SearchString);
    if (FoundLoc) return FoundLoc + strlen(SearchString);
    return buffer;
}

std::shared_ptr<Volume> AmiraVolumeReader::readData(const std::filesystem::path& filePath) {
    auto& path = filePath;

    FILE* fp = fopen(path.string().c_str(), "rb");
    if (!fp) {
        LogError(std::format("Could not find %s", path.string()));
        return nullptr;
    }

    // We read the first 2k bytes into memory to parse the header.
    // The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
    char buffer[2048];
    fread(buffer, sizeof(char), 2047, fp);
    buffer[2047] = '\0';  // The following string routines prefer null-terminated strings

    if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1")) {
        LogError("Not a proper AmiraMesh file.");
        fclose(fp);
        return nullptr;
    }

    // Find the Lattice definition, i.e., the dimensions of the uniform grid
    int xDim(0), yDim(0), zDim(0);
    sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);

    // Find the BoundingBox
    float xmin(1.0f), ymin(1.0f), zmin(1.0f);
    float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
    sscanf(FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax,
           &zmin, &zmax);

    // Is it a uniform grid? We need this only for the sanity check below.
    const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);

    // Type of the field: scalar, vector
    int NumComponents(0);
    if (strstr(buffer, "Lattice { float Data }")) {
        // Scalar field
        NumComponents = 1;
    } else {
        // A field with more than one component, i.e., a vector field
        sscanf(FindAndJump(buffer, "Lattice { float["), "%d", &NumComponents);
    }
    printf("\tNumber of Components: %d\n", NumComponents);

    const DataFormatBase* format = [NumComponents, filePath]() -> const DataFormatBase* {
        switch (NumComponents) {
            case 1:
                return DataFloat32::get();
            case 2:
                return DataFloat64::get();
            case 3:
                return DataVec3Float32::get();
            default:
                throw DataReaderException(
                    IVW_CONTEXT_CUSTOM("AmiraVolumeReader"),
                    "Error: Unsupported format (bytes per voxel) in .am file: {}", filePath);
        }
    }();

    // Sanity check
    if (xDim <= 0 || yDim <= 0 || zDim <= 0 || xmin > xmax || ymin > ymax || zmin > zmax ||
        !bIsUniform || NumComponents <= 0) {
        LogError("Something went wrong");
        fclose(fp);
        return nullptr;
    }

    // Find the beginning of the data section
    const long idxStartData = strstr(buffer, "# Data section follows") - buffer;

    // Read the data
    //  - how much to read
    const size_t NumToRead = xDim * yDim * zDim * NumComponents;
    float* data = new float[NumToRead];
    if (idxStartData > 0) {
        // Set the file pointer to the beginning of "# Data section follows"
        fseek(fp, idxStartData, SEEK_SET);
        // Consume this line, which is "# Data section follows"
        fgets(buffer, 2047, fp);
        // Consume the next line, which is "@1"
        fgets(buffer, 2047, fp);

        if (data) {
            // - do it
            const size_t ActRead = fread((void*)data, sizeof(float), NumToRead, fp);
            // - ok?
            if (NumToRead != ActRead) {
                printf(
                    "Something went wrong while reading the binary data section.\nPremature end of "
                    "file?\n");
                delete[] data;
                fclose(fp);
                return nullptr;
            }
        }
    }
    fclose(fp);

    glm::mat3 basis(1.0f);
    glm::mat4 wtm(1.0f);

    basis[0][0] = (xmax - xmin) / (float)xDim;
    basis[1][1] = (ymax - ymin) / (float)yDim;
    basis[2][2] = (zmax - zmin) / (float)zDim;

    // Center the data around origo.
    glm::vec3 offset(xmin, ymin, zmin);

    auto volRAM = createVolumeRAM(size3_t{xDim, yDim, zDim}, format, data);
    auto volume = std::make_shared<Volume>(volRAM);
    volume->setBasis(basis);
    volume->setOffset(offset);
    volume->setWorldMatrix(wtm);
    auto loader = std::make_unique<RawVolumeRAMLoader>(path, idxStartData, true);
    auto minmax =
        volume->getRepresentation<VolumeRAM>()->dispatch<std::pair<dvec4, dvec4>>([](auto ram) {
            using ValueType = util::PrecisionValueType<decltype(ram)>;
            const size3_t dims = ram->getDimensions();
            using Res = std::pair<ValueType, ValueType>;
            // Initialize result with max and min values of given data type
            Res minmax{DataFormat<ValueType>::max(), DataFormat<ValueType>::lowest()};
            const ValueType* data = ram->getDataTyped();
            // Iterate over buffer, accumulating the min and max components
            minmax = std::accumulate(data, data + glm::compMul(dims) / 2, minmax,
                                     [](const Res& mm, const ValueType& v) -> Res {
                                         // Take component-wise min and max
                                         return {glm::max((ValueType)-100, glm::min(mm.first, v)),
                                                 glm::min((ValueType)100, glm::max(mm.second, v))};
                                     });
            // Return pair of dvec4, as we specified in dispatch's type parameter
            return std::pair<dvec4, dvec4>{util::glm_convert<dvec4>(minmax.first),
                                           util::glm_convert<dvec4>(minmax.second)};
        });
    LogInfo("Min: " << minmax.first << ", Max: " << minmax.second);

    std::string size = util::formatBytesToString(xDim * yDim * zDim * (format->getSizeInBytes()));
    LogInfo("Loaded volume: " << filePath << " size: " << size);
    return volume;
}

}  // namespace inviwo
