/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/io/inviwofileformattypes.h>
#include <inviwo/core/io/bytewriterutil.h>
#include <inviwo/core/io/datawriter.h>                // for DataWriterType
#include <inviwo/core/io/datawriterexception.h>       // for DataWriterException
#include <inviwo/core/io/serialization/serializer.h>  // for Serializer
#include <inviwo/core/metadata/metadatamap.h>         // for MetaDataMap
#include <inviwo/core/util/fileextension.h>           // for FileExtension
#include <inviwo/core/util/filesystem.h>              // for getFileNameWithou...
#include <inviwo/core/util/formats.h>                 // for DataFormatBase
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/volumesequenceutils.h>

#include <array>          // for array
#include <fstream>        // for basic_ofstream, ios
#include <memory>         // for unique_ptr
#include <string>         // for basic_string, string
#include <unordered_set>  // for unordered_set

#include <glm/gtx/component_wise.hpp>  // for compMul

#include <fmt/std.h>
#include <fmt/core.h>
#include <memory_resource>

namespace inviwo {

constexpr std::string_view InviwoVolume = "InviwoVolume";
constexpr int InviwoVolumeVersion = 2;

IvfVolumeWriter::IvfVolumeWriter() : DataWriterType<Volume>() {
    addExtension(FileExtension("ivf", "Inviwo Volume Format"));
}

IvfVolumeWriter::IvfVolumeWriter(const IvfVolumeWriter& rhs) = default;

IvfVolumeWriter::IvfVolumeWriter(IvfVolumeWriter&& rhs) = default;

IvfVolumeWriter& IvfVolumeWriter::operator=(const IvfVolumeWriter& that) = default;

IvfVolumeWriter& IvfVolumeWriter::operator=(IvfVolumeWriter&& that) = default;

IvfVolumeWriter* IvfVolumeWriter::clone() const { return new IvfVolumeWriter(*this); }

void IvfVolumeWriter::writeData(const Volume* volume, const std::filesystem::path& filePath) const {
    util::writeIvfVolume(*volume, filePath, getOverwrite());
}

IvfVolumeSequenceWriter::IvfVolumeSequenceWriter() : DataWriterType<VolumeSequence>() {
    addExtension(FileExtension("ivfs", "Inviwo Volume Sequence"));
}

IvfVolumeSequenceWriter::IvfVolumeSequenceWriter(const IvfVolumeSequenceWriter& rhs) = default;

IvfVolumeSequenceWriter::IvfVolumeSequenceWriter(IvfVolumeSequenceWriter&& rhs) = default;

IvfVolumeSequenceWriter& IvfVolumeSequenceWriter::operator=(const IvfVolumeSequenceWriter& that) =
    default;

IvfVolumeSequenceWriter& IvfVolumeSequenceWriter::operator=(IvfVolumeSequenceWriter&& that) =
    default;

IvfVolumeSequenceWriter* IvfVolumeSequenceWriter::clone() const {
    return new IvfVolumeSequenceWriter(*this);
}

void IvfVolumeSequenceWriter::writeData(const VolumeSequence* volumes,
                                        const std::filesystem::path& filePath) const {
    util::writeIvfVolumeSequence(*volumes, filePath.stem().generic_string(), filePath.parent_path(),
                                 {}, getOverwrite());
}

namespace util {

void writeIvfVolume(const Volume& data, const std::filesystem::path& filePath,
                    Overwrite overwrite) {
    const Compression compression = Compression::Enabled;
    const std::string_view extension = compression == Compression::Enabled ? "raw.gz" : "raw";
    const auto rawPath = filesystem::replaceFileExtension(filePath, extension);

    DataWriter::checkOverwrite(filePath, overwrite);
    DataWriter::checkOverwrite(rawPath, overwrite);

    const auto fileName = filePath.stem().string();
    const VolumeRAM* vr = data.getRepresentation<VolumeRAM>();

    std::pmr::monotonic_buffer_resource mbr{1024 * 4};
    Serializer s{filePath, InviwoVolume, InviwoVolumeVersion, &mbr};
    {
        const auto nodeSwitch = s.switchToNewNode("RawFile");
        s.serialize("content", fmt::format("{}.{}", fileName, extension),
                    SerializationTarget::Attribute);
        data.getMetaDataMap()->serialize(s);
    }
    s.serialize("Format", vr->getDataFormatString());
    s.serialize("ByteOffset", 0u);
    s.serialize("ByteOrder", ByteOrder::LittleEndian);
    s.serialize("Compression", compression);
    s.serialize("BasisAndOffset", data.getModelMatrix());
    s.serialize("WorldTransform", data.getWorldMatrix());
    s.serialize("Dimension", data.getDimensions());
    s.serialize("DataRange", data.dataMap.dataRange);
    s.serialize("ValueRange", data.dataMap.valueRange);
    s.serialize("ValueName", data.dataMap.valueAxis.name);
    s.serialize("ValueUnit", units::to_string(data.dataMap.valueAxis.unit));

    s.serialize("Axis1Name", data.axes[0].name);
    s.serialize("Axis1Unit", units::to_string(data.axes[0].unit));

    s.serialize("Axis2Name", data.axes[1].name);
    s.serialize("Axis2Unit", units::to_string(data.axes[1].unit));

    s.serialize("Axis3Name", data.axes[2].name);
    s.serialize("Axis3Unit", units::to_string(data.axes[2].unit));

    s.serialize("SwizzleMask", vr->getSwizzleMask());
    s.serialize("Interpolation", vr->getInterpolation());
    s.serialize("Wrapping", vr->getWrapping());

    s.writeFile();

    const size_t bytes = glm::compMul(vr->getDimensions()) * vr->getDataFormat()->getSizeInBytes();
    util::writeBytes(rawPath, vr->getData(), bytes, compression);
}

namespace {

void serializeAllVolumeData(Serializer& s, const Volume& volume) {
    s.serialize("Format", volume.getDataFormat()->getString());
    s.serialize("BasisAndOffset", volume.getModelMatrix());
    s.serialize("WorldTransform", volume.getWorldMatrix());
    s.serialize("Dimension", volume.getDimensions());
    s.serialize("DataRange", volume.dataMap.dataRange);
    s.serialize("ValueRange", volume.dataMap.valueRange);
    s.serialize("ValueName", volume.dataMap.valueAxis.name);
    s.serialize("ValueUnit", units::to_string(volume.dataMap.valueAxis.unit));

    s.serialize("Axis1Name", volume.axes[0].name);
    s.serialize("Axis1Unit", units::to_string(volume.axes[0].unit));

    s.serialize("Axis2Name", volume.axes[1].name);
    s.serialize("Axis2Unit", units::to_string(volume.axes[1].unit));

    s.serialize("Axis3Name", volume.axes[2].name);
    s.serialize("Axis3Unit", units::to_string(volume.axes[2].unit));
}

void serializeDissimilarVolumeData(Serializer& s, const SharedSequenceData& shared,
                                   const Volume& volume) {
    if (!shared.format) {
        s.serialize("Format", volume.getDataFormat()->getString());
    }
    if (!shared.basis) {
        s.serialize("BasisAndOffset", volume.getModelMatrix());
    }
    if (!shared.worldTransform) {
        s.serialize("WorldTransform", volume.getWorldMatrix());
    }
    if (!shared.dimensions) {
        s.serialize("Dimension", volume.getDimensions());
    }
    if (!shared.dataMap) {
        s.serialize("DataRange", volume.dataMap.dataRange);
        s.serialize("ValueRange", volume.dataMap.valueRange);
        s.serialize("ValueName", volume.dataMap.valueAxis.name);
        s.serialize("ValueUnit", units::to_string(volume.dataMap.valueAxis.unit));
    }
    if (!shared.axes) {
        s.serialize("Axis1Name", volume.axes[0].name);
        s.serialize("Axis1Unit", units::to_string(volume.axes[0].unit));

        s.serialize("Axis2Name", volume.axes[1].name);
        s.serialize("Axis2Unit", units::to_string(volume.axes[1].unit));

        s.serialize("Axis3Name", volume.axes[2].name);
        s.serialize("Axis3Unit", units::to_string(volume.axes[2].unit));
    }
}

struct VolumeMetaData {
    VolumeMetaData(const SharedSequenceData& shared, std::filesystem::path relativePath,
                   std::shared_ptr<const Volume> volume)
        : shared{shared}, relativePath{std::move(relativePath)}, volume{std::move(volume)} {}

    void serialize(Serializer& s) const {
        s.serialize("content", relativePath, SerializationTarget::Attribute);
        serializeDissimilarVolumeData(s, shared, *volume);
        volume->getMetaDataMap()->serialize(s);
    }

    const SharedSequenceData& shared;
    std::filesystem::path relativePath;
    std::shared_ptr<const Volume> volume;
};

}  // namespace

std::filesystem::path writeIvfVolumeSequence(const VolumeSequence& data, std::string_view name,
                                             const std::filesystem::path& parentFolder,
                                             const std::filesystem::path& relativePathToElements,
                                             Overwrite overwrite) {
    if (data.empty()) {
        throw DataWriterException(SourceContext{}, "Expected non-empty volume sequence");
    }

    auto filePath = parentFolder / name;
    if (!name.ends_with(".ivfs")) {
        filePath += ".ivfs";
    }
    DataWriter::checkOverwrite(filePath, overwrite);

    const auto rawBaseName = name.ends_with(".ivfs") ? name.substr(0, name.size() - 5) : name;
    const Compression compression = Compression::Enabled;
    const std::string_view extension = compression == Compression::Enabled ? "raw.gz" : "raw";

    const util::SharedSequenceData sharedData{data};

    std::vector<VolumeMetaData> volumeData;
    StrBuffer rawFile;
    const auto numDigits = static_cast<int>(log10(data.size())) + 1;
    for (const auto& [index, volume] : util::enumerate<int>(data)) {
        rawFile.replace("{}{:0{}}.{}", rawBaseName, index, numDigits, extension);
        const auto relativePath =
            std::filesystem::path{"."} / relativePathToElements / rawFile.view();
        volumeData.emplace_back(sharedData, relativePath, volume);
    }

    std::pmr::monotonic_buffer_resource mbr{1024 * 4};
    Serializer s{filePath, InviwoVolume, InviwoVolumeVersion, &mbr};

    s.serialize("RawFiles", volumeData, "RawFile");

    s.serialize("ByteOffset", 0u);
    s.serialize("ByteOrder", ByteOrder::LittleEndian);
    s.serialize("Compression", compression);
    // serialize default/shared volume data
    serializeAllVolumeData(s, *data.front());

    s.writeFile();

    // write raw data
    for (const auto& v : volumeData) {
        const auto rawFilePath = parentFolder / v.relativePath;
        DataWriter::checkOverwrite(rawFilePath, overwrite);

        const auto* vr = v.volume->getRepresentation<VolumeRAM>();
        const size_t bytes =
            glm::compMul(vr->getDimensions()) * vr->getDataFormat()->getSizeInBytes();
        util::writeBytes(rawFilePath, vr->getData(), bytes, compression);
    }

    return filePath;
}

}  // namespace util

}  // namespace inviwo
