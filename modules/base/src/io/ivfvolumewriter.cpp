/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <modules/base/io/ivfvolumewriter.h>

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/unitsystem.h>                      // for Axis
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, DataWrite...
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/io/datawriter.h>                                  // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>                         // for DataWriterException
#include <inviwo/core/io/serialization/serializer.h>                    // for Serializer
#include <inviwo/core/metadata/metadatamap.h>                           // for MetaDataMap
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/filesystem.h>                                // for getFileNameWithou...
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT_CUSTOM

#include <array>          // for array
#include <fstream>        // for basic_ofstream, ios
#include <memory>         // for unique_ptr
#include <string>         // for basic_string, string
#include <unordered_set>  // for unordered_set

#include <glm/gtx/component_wise.hpp>  // for compMul
#include <llnl-units/units.hpp>        // for to_string

#include <fmt/std.h>

namespace inviwo {

IvfVolumeWriter::IvfVolumeWriter() : DataWriterType<Volume>() {
    addExtension(FileExtension("ivf", "Inviwo ivf file format"));
}

IvfVolumeWriter::IvfVolumeWriter(const IvfVolumeWriter& rhs) = default;

IvfVolumeWriter& IvfVolumeWriter::operator=(const IvfVolumeWriter& that) = default;

IvfVolumeWriter* IvfVolumeWriter::clone() const { return new IvfVolumeWriter(*this); }

void IvfVolumeWriter::writeData(const Volume* volume, const std::filesystem::path& filePath) const {
    util::writeIvfVolume(*volume, filePath, getOverwrite());
}

namespace util {
void writeIvfVolume(const Volume& data, const std::filesystem::path& filePath,
                    Overwrite overwrite) {
    const auto rawPath = filesystem::replaceFileExtension(filePath, "raw");

    DataWriter::checkOverwrite(filePath, overwrite);
    DataWriter::checkOverwrite(rawPath, overwrite);

    const auto fileName = filePath.stem().string();
    const VolumeRAM* vr = data.getRepresentation<VolumeRAM>();
    Serializer s(filePath);
    s.serialize("RawFile", fmt::format("{}.raw", fileName));
    s.serialize("Format", vr->getDataFormatString());
    s.serialize("ByteOffset", 0u);
    s.serialize("BasisAndOffset", data.getModelMatrix());
    s.serialize("WorldTransform", data.getWorldMatrix());
    s.serialize("Dimension", data.getDimensions());
    s.serialize("DataRange", data.dataMap_.dataRange);
    s.serialize("ValueRange", data.dataMap_.valueRange);
    s.serialize("ValueName", data.dataMap_.valueAxis.name);
    s.serialize("ValueUnit", units::to_string(data.dataMap_.valueAxis.unit));

    s.serialize("Axis1Name", data.axes[0].name);
    s.serialize("Axis1Unit", units::to_string(data.axes[0].unit));

    s.serialize("Axis2Name", data.axes[1].name);
    s.serialize("Axis2Unit", units::to_string(data.axes[1].unit));

    s.serialize("Axis3Name", data.axes[2].name);
    s.serialize("Axis3Unit", units::to_string(data.axes[2].unit));

    s.serialize("SwizzleMask", vr->getSwizzleMask());
    s.serialize("Interpolation", vr->getInterpolation());
    s.serialize("Wrapping", vr->getWrapping());

    data.getMetaDataMap()->serialize(s);
    s.writeFile();

    if (auto fout = std::ofstream(rawPath, std::ios::out | std::ios::binary)) {
        fout.write(static_cast<const char*>(vr->getData()),
                   glm::compMul(vr->getDimensions()) * vr->getDataFormat()->getSize());
    } else {
        throw DataWriterException(IVW_CONTEXT_CUSTOM("util::writeIvfVolume"),
                                  "Could not write to raw file: {}", rawPath);
    }
}
}  // namespace util

}  // namespace inviwo
