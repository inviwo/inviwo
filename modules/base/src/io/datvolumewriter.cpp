/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/io/datawriterexception.h>

#include <glm/gtx/range.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/ostream.h>

namespace inviwo {

DatVolumeWriter::DatVolumeWriter() : DataWriterType<Volume>() {
    addExtension(FileExtension("dat", "Inviwo dat file format"));
}

DatVolumeWriter::DatVolumeWriter(const DatVolumeWriter& rhs) = default;

DatVolumeWriter& DatVolumeWriter::operator=(const DatVolumeWriter& that) = default;

DatVolumeWriter* DatVolumeWriter::clone() const { return new DatVolumeWriter(*this); }

void DatVolumeWriter::writeData(const Volume* data, const std::string filePath) const {
    util::writeDatVolume(*data, filePath, getOverwrite());
}

namespace util {
void writeDatVolume(const Volume& data, const std::string filePath, bool overwrite) {
    std::string rawPath = filesystem::replaceFileExtension(filePath, "raw");

    if (filesystem::fileExists(filePath) && !overwrite)
        throw DataWriterException("Output file: " + filePath + " already exists",
                                  IVW_CONTEXT_CUSTOM("util::writeDatVolume"));

    if (filesystem::fileExists(rawPath) && !overwrite)
        throw DataWriterException("Output file: " + rawPath + " already exists",
                                  IVW_CONTEXT_CUSTOM("util::writeDatVolume"));

    std::string fileName = filesystem::getFileNameWithoutExtension(filePath);
    // Write the .dat file content
    std::stringstream ss;
    const VolumeRAM* vr = data.getRepresentation<VolumeRAM>();
    glm::mat3 basis = glm::transpose(data.getBasis());
    glm::vec3 offset = data.getOffset();
    glm::mat4 wtm = glm::transpose(data.getWorldMatrix());

    auto print = util::overloaded{
        [&](std::string_view key, const std::string& val) { fmt::print(ss, "{}: {}\n", key, val); },
        [&](std::string_view key, InterpolationType val) { fmt::print(ss, "{}: {}\n", key, val); },
        [&](std::string_view key, const SwizzleMask& mask) {
            fmt::print(ss, "{}: {}{}{}{}\n", key, mask[0], mask[1], mask[2], mask[3]);
        },
        [&](std::string_view key, const Wrapping3D& wrapping) {
            fmt::print(ss, "{}: {} {} {}\n", key, wrapping[0], wrapping[1], wrapping[2]);
        },
        [&](std::string_view key, const auto& vec) {
            fmt::print(ss, "{}: {}\n", key, fmt::join(begin(vec), end(vec), " "));
        }};

    print("RawFile", fileName + ".raw");
    print("Resolution", vr->getDimensions());
    print("Format", vr->getDataFormatString());
    print("ByteOffset", std::string("0"));
    print("BasisVector1", basis[0]);
    print("BasisVector2", basis[1]);
    print("BasisVector3", basis[2]);
    print("Offset", offset);
    print("WorldVector1", wtm[0]);
    print("WorldVector2", wtm[1]);
    print("WorldVector3", wtm[2]);
    print("WorldVector4", wtm[3]);
    print("DataRange", data.dataMap_.dataRange);
    print("ValueRange", data.dataMap_.valueRange);
    print("Unit", data.dataMap_.valueUnit);

    print("SwizzleMask", vr->getSwizzleMask());
    print("Interpolation", vr->getInterpolation());
    print("Wrapping", vr->getWrapping());

    for (auto& key : data.getMetaDataMap()->getKeys()) {
        auto m = data.getMetaDataMap()->get(key);
        if (auto sm = dynamic_cast<const StringMetaData*>(m)) print(key, sm->get());
    }

    if (auto f = filesystem::ofstream(filePath)) {
        f << ss.str();
    } else {
        throw DataWriterException("Could not write to dat file: " + filePath,
                                  IVW_CONTEXT_CUSTOM("util::writeDatVolume"));
    }

    if (auto f = filesystem::ofstream(rawPath, std::ios::out | std::ios::binary)) {
        f.write(static_cast<const char*>(vr->getData()), vr->getNumberOfBytes());
    } else {
        throw DataWriterException("Could not write to raw file: " + rawPath,
                                  IVW_CONTEXT_CUSTOM("util::writeDatVolume"));
    }
}
}  // namespace util

}  // namespace inviwo
