/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/image/imagetypes.h>                // for enumToStr, Swizzl...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Axis, Unit
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, DataWrite...
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/io/datawriter.h>                                  // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>                         // for DataWriterException
#include <inviwo/core/metadata/metadata.h>                              // for StringMetaData
#include <inviwo/core/metadata/metadatamap.h>                           // for MetaDataMap
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/filesystem.h>                                // for ofstream, getFile...
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/stdextensions.h>                             // for overloaded

#include <array>          // for array
#include <fstream>        // for stringstream, bas...
#include <memory>         // for unique_ptr
#include <string>         // for char_traits, string
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <fmt/core.h>            // for basic_string_view
#include <fmt/format.h>          // for join
#include <fmt/ostream.h>         // for print
#include <glm/fwd.hpp>           // for mat4, mat3, vec3
#include <glm/gtc/type_ptr.hpp>  // for value_ptr
#include <glm/gtx/range.hpp>     // for begin, end
#include <glm/mat3x3.hpp>        // for mat<>::col_type
#include <glm/mat4x4.hpp>        // for mat<>::col_type
#include <glm/matrix.hpp>        // for transpose

namespace inviwo {

DatVolumeWriter::DatVolumeWriter() : DataWriterType<Volume>() {
    addExtension(FileExtension("dat", "Inviwo dat file format"));
}

DatVolumeWriter::DatVolumeWriter(const DatVolumeWriter& rhs) = default;

DatVolumeWriter& DatVolumeWriter::operator=(const DatVolumeWriter& that) = default;

DatVolumeWriter* DatVolumeWriter::clone() const { return new DatVolumeWriter(*this); }

void DatVolumeWriter::writeData(const Volume* data, std::string_view filePath) const {
    util::writeDatVolume(*data, filePath, getOverwrite());
}

namespace util {
void writeDatVolume(const Volume& data, std::string_view filePath, Overwrite overwrite) {
    std::string rawPath = filesystem::replaceFileExtension(filePath, "raw");

    DataWriter::checkOverwrite(filePath, overwrite);
    DataWriter::checkOverwrite(rawPath, overwrite);

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
        [&](std::string_view key, const Unit& unit) { fmt::print(ss, "{}: {}\n", key, unit); },
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
    print("ValueUnit", data.dataMap_.valueAxis.unit);
    print("ValueName", data.dataMap_.valueAxis.name);

    print("Axis1Name", data.axes[0].name);
    print("Axis2Name", data.axes[1].name);
    print("Axis3Name", data.axes[2].name);

    print("Axis1Unit", data.axes[0].unit);
    print("Axis2Unit", data.axes[1].unit);
    print("Axis3Unit", data.axes[2].unit);

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
        throw DataWriterException(IVW_CONTEXT_CUSTOM("util::writeDatVolume"),
                                  "Could not write to dat file: {}", filePath);
    }

    if (auto f = filesystem::ofstream(rawPath, std::ios::out | std::ios::binary)) {
        f.write(static_cast<const char*>(vr->getData()), vr->getNumberOfBytes());
    } else {
        throw DataWriterException(IVW_CONTEXT_CUSTOM("util::writeDatVolume"),
                                  "Could not write to raw file: {}", rawPath);
    }
}
}  // namespace util

}  // namespace inviwo
