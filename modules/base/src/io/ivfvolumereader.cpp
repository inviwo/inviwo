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

#include <modules/base/io/ivfvolumereader.h>

#include <inviwo/core/common/factoryutil.h>                // for getMetaDataFactory
#include <inviwo/core/datastructures/camera/camera.h>      // for mat4
#include <inviwo/core/datastructures/datamapper.h>         // for DataMapper
#include <inviwo/core/datastructures/image/imagetypes.h>   // for InterpolationType, Inte...
#include <inviwo/core/datastructures/unitsystem.h>         // for Axis, Unit
#include <inviwo/core/datastructures/volume/volume.h>      // for Volume, DataReaderType
#include <inviwo/core/datastructures/volume/volumedisk.h>  // for VolumeDisk
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/rawvolumeramloader.h>
#include <inviwo/core/io/inviwofileformattypes.h>
#include <inviwo/core/io/serialization/deserializer.h>            // for Deserializer
#include <inviwo/core/io/serialization/serializationexception.h>  // for SerializationException
#include <inviwo/core/io/serialization/versionconverter.h>        // for VersionConverter
#include <inviwo/core/io/serialization/ticpp.h>                   // for TxElement
#include <inviwo/core/metadata/metadata.h>                        // for BoolMetaData
#include <inviwo/core/metadata/metadatafactory.h>                 // for MetaDataFactory
#include <inviwo/core/metadata/metadatamap.h>                     // for MetaDataMap
#include <inviwo/core/util/fileextension.h>                       // for FileExtension
#include <inviwo/core/util/filesystem.h>                          // for getFileDirectory
#include <inviwo/core/util/formats.h>                             // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                              // for size3_t

#include <array>        // for array
#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <string>       // for string, basic_string
#include <type_traits>  // for remove_extent_t
#include <memory_resource>

#include <fmt/base.h>

namespace inviwo {

namespace {

constexpr std::string_view InviwoVolume = "InviwoVolume";

class Converter : public VersionConverter {
public:
    explicit Converter(int version);
    virtual bool convert(TxElement* root) override;

private:
    static void updateByteOrder(TxElement* root);

    int version_;
};

Converter::Converter(int version) : version_(version) {}

void Converter::updateByteOrder(TxElement* root) {
    auto metaDataMap = xml::getElement(root, "MetaDataMap");

    if (metaDataMap) {
        if (auto littleEndianNode =
                xml::getElement(metaDataMap, "MetaDataItem&key=LittleEndian/MetaData")) {
            // move byteorder to own tag
            TxElement byteOrder{"ByteOrder"};

            if (auto content = littleEndianNode->Attribute("content")) {
                byteOrder.SetAttribute("content", content == "1" ? "LittleEndian" : "BigEndian");
            } else {
                byteOrder.SetAttribute("content", "LittleEndian");
            }
            root->InsertBeforeChild(metaDataMap, byteOrder);
            metaDataMap->RemoveChild(littleEndianNode->Parent());
        }
    }
}

bool Converter::convert(TxElement* root) {
    if (root->Value() != InviwoVolume) {
        // ignore version number if the root node is not already an InviwoVolume, but for example an
        // InviwoWorkspace
        version_ = 1;
    }

    switch (version_) {
        case 0:
        case 1: {
            auto metaDataMap = xml::getElement(root, "MetaDataMap");

            if (metaDataMap) {
                if (auto littleEndianNode =
                        xml::getElement(metaDataMap, "MetaDataItem&key=LittleEndian/MetaData")) {
                    // move endianness to own tag
                    TxElement byteOrder{"ByteOrder"};

                    auto content = littleEndianNode->Attribute("content").value_or("1");
                    byteOrder.SetAttribute(
                        "content",
                        fmt::format("{}", content == "1" ? static_cast<int>(ByteOrder::LittleEndian)
                                                         : static_cast<int>(ByteOrder::BigEndian)));

                    root->InsertBeforeChild(metaDataMap, byteOrder);
                    metaDataMap->RemoveChild(littleEndianNode->Parent());
                }
            }
            if (auto rawFile = xml::getElement(root, "RawFile"); rawFile && metaDataMap) {
                // move metadata map to RawFile
                rawFile->InsertEndChild(*metaDataMap);
                root->RemoveChild(metaDataMap);
            }

            return true;
        }

        default:
            return false;  // No changes
    }
}

DataMapper deserializeDataMap(Deserializer& d) {
    DataMapper dataMap;
    d.deserialize("DataRange", dataMap.dataRange);
    d.deserialize("ValueRange", dataMap.valueRange);
    d.deserialize("ValueName", dataMap.valueAxis.name);

    std::string tmp;
    d.deserialize("ValueUnit", tmp);
    if (!tmp.empty()) {
        dataMap.valueAxis.unit = units::unit_from_string(tmp);
    }

    return dataMap;
}

std::array<Axis, 3> deserializeAxes(Deserializer& d) {
    std::array<Axis, 3> axes;
    std::string tmp;
    d.deserialize("Axis1Unit", tmp);
    if (!tmp.empty()) {
        axes[0].unit = units::unit_from_string(tmp);
    }

    tmp.clear();
    d.deserialize("Axis2Unit", tmp);
    if (!tmp.empty()) {
        axes[1].unit = units::unit_from_string(tmp);
    }

    tmp.clear();
    d.deserialize("Axis2Unit", tmp);
    if (!tmp.empty()) {
        axes[2].unit = units::unit_from_string(tmp);
    }

    d.deserialize("Axis1Name", axes[0].name);
    d.deserialize("Axis2Name", axes[1].name);
    d.deserialize("Axis3Name", axes[2].name);

    return axes;
}

struct VolumeMetaData {
    void deserialize(Deserializer& d) {
        d.deserialize("content", path, SerializationTarget::Attribute);
        metaData.deserialize(d);
    }

    std::filesystem::path path;
    MetaDataMap metaData;
};

}  // namespace

IvfVolumeReader::IvfVolumeReader() : DataReaderType<VolumeSequence>{} {
    addExtension(FileExtension("ivf", "Inviwo ivf file format"));
}

IvfVolumeReader* IvfVolumeReader::clone() const { return new IvfVolumeReader(*this); }

std::shared_ptr<VolumeSequence> IvfVolumeReader::readData(const std::filesystem::path& filePath) {
    const auto localPath = downloadAndCacheIfUrl(filePath);

    checkExists(localPath);
    const auto fileDirectory = filePath.parent_path();

    std::pmr::monotonic_buffer_resource mbr{1024 * 4};
    Deserializer d{localPath, "InviwoVolume", &mbr};

    Converter converter{d.getVersion()};
    d.convertVersion(&converter);

    d.registerFactory(util::getMetaDataFactory());

    size_t byteOffset = 0u;
    ByteOrder byteOrder = ByteOrder::LittleEndian;
    Compression compression = Compression::Disabled;
    d.deserialize("ByteOffset", byteOffset);
    d.deserialize("ByteOrder", byteOrder);
    d.deserialize("Compression", compression);

    std::string formatFlag;
    const DataFormatBase* format = nullptr;
    d.deserialize("Format", formatFlag);
    format = DataFormatBase::get(formatFlag);

    size3_t dimensions{0u};
    d.deserialize("Dimension", dimensions);

    SwizzleMask swizzleMask{swizzlemasks::rgba};
    InterpolationType interpolation{InterpolationType::Linear};
    Wrapping3D wrapping{wrapping3d::clampAll};
    d.deserialize("SwizzleMask", swizzleMask);
    d.deserialize("Interpolation", interpolation);
    d.deserialize("Wrapping", wrapping);

    mat4 basisAndOffset{1.0f};
    mat4 worldTransform{1.0f};
    d.deserialize("BasisAndOffset", basisAndOffset);
    d.deserialize("WorldTransform", worldTransform);

    auto dataMap = deserializeDataMap(d);
    auto axes = deserializeAxes(d);

    std::vector<VolumeMetaData> volumeData;
    if (d.hasElement("RawFiles")) {
        d.deserialize("RawFiles", volumeData, "RawFile");
    } else {
        VolumeMetaData metaData;
        d.deserialize("RawFile", metaData);
        volumeData.push_back(metaData);
    }

    auto sequence = std::make_shared<VolumeSequence>();
    for (const auto& [path, metaData] : volumeData) {
        auto volume =
            std::make_shared<Volume>(dimensions, format, swizzleMask, interpolation, wrapping);
        sequence->push_back(volume);

        volume->setModelMatrix(basisAndOffset);
        volume->setWorldMatrix(worldTransform);
        volume->dataMap = dataMap;
        volume->axes = axes;
        *(volume->getMetaDataMap()) = metaData;

        auto volumeDisk = std::make_shared<VolumeDisk>(localPath, dimensions, format, swizzleMask,
                                                       interpolation, wrapping);
        auto loader = std::make_unique<RawVolumeRAMLoader>(fileDirectory / path, byteOffset,
                                                           byteOrder, compression);
        volumeDisk->setLoader(loader.release());
        volume->addRepresentation(volumeDisk);
    }

    return sequence;
}

}  // namespace inviwo
