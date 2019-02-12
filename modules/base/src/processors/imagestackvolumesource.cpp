/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/processors/imagestackvolumesource.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

namespace {

template <typename Format>
struct FloatOrIntMax32
    : std::integral_constant<bool, Format::numtype == NumericType::Float || Format::compsize <= 4> {
};

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageStackVolumeSource::processorInfo_{
    "org.inviwo.ImageStackVolumeSource",  // Class identifier
    "Image Stack Volume Source",          // Display name
    "Data Input",                         // Category
    CodeState::Stable,                    // Code state
    "Layer, Image, Volume",               // Tags
};
const ProcessorInfo ImageStackVolumeSource::getProcessorInfo() const { return processorInfo_; }

ImageStackVolumeSource::ImageStackVolumeSource()
    : Processor()
    , outport_("volume")
    , filePattern_("filePattern", "File Pattern", "####.jpeg", "")
    , skipUnsupportedFiles_("skipUnsupportedFiles", "Skip Unsupported Files", false)
    // volume parameters
    , voxelSpacing_("voxelSpacing", "Voxel Spacing", vec3(1.0f), vec3(0.1f), vec3(10.0f),
                    vec3(0.1f))
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information") {

    addPort(outport_);
    addProperty(filePattern_);
    addProperty(skipUnsupportedFiles_);
    addProperty(voxelSpacing_);
    addProperty(information_);
    addProperty(basis_);

    validExtensions_ =
        InviwoApplication::getPtr()->getDataReaderFactory()->getExtensionsForType<Layer>();
    filePattern_.onChange([this]() { load(); });
    voxelSpacing_.onChange([this]() {
        // update volume basis and offset
        updateVolumeBasis();
        invalidate(InvalidationLevel::InvalidOutput);
    });
    filePattern_.onChange([&]() { isReady_.update(); });
    isReady_.setUpdate([this]() { return !filePattern_.getFileList().empty(); });
}

void ImageStackVolumeSource::process() {
    if (isDeserializing_) return;

    if (dirty_) {
        load();
        dirty_ = false;
    }

    if (volume_) {
        basis_.updateEntity(*volume_);
        information_.updateVolume(*volume_);
    }
}

bool ImageStackVolumeSource::isValidImageFile(std::string fileName) {
    std::string fileExtension = toLower(filesystem::getFileExtension(fileName));
    return util::contains_if(validExtensions_,
                             [&](const FileExtension& e) { return e.extension_ == fileExtension; });
}

void ImageStackVolumeSource::load() { load(false); }

void ImageStackVolumeSource::load(bool deserialized) {
    if (isDeserializing_) return;

    std::vector<std::string> filesInDirectory = filePattern_.getFileList();
    if (filesInDirectory.empty()) return;

    std::vector<std::string> fileNames;
    for (size_t i = 0; i < filesInDirectory.size(); i++) {
        if (isValidImageFile(filesInDirectory[i])) {
            fileNames.push_back(filesInDirectory[i]);
        }
    }
    if (!fileNames.size()) {
        LogWarn("No images found in '" << filePattern_.getFilePatternPath() << "'");
        return;
    }

    using ReaderMap = std::map<std::string, std::unique_ptr<DataReaderType<Layer>>>;
    ReaderMap readerMap;

    size_t numSlices = 0;
    // determine number of slices and file readers for all images
    for (auto file : fileNames) {
        std::string fileExtension = toLower(filesystem::getFileExtension(file));
        if (readerMap.find(fileExtension) != readerMap.end()) {
            // reader already exists for this format
            ++numSlices;
            continue;
        }
        auto reader = InviwoApplication::getPtr()
                          ->getDataReaderFactory()
                          ->getReaderForTypeAndExtension<Layer>(fileExtension);
        if (!reader && skipUnsupportedFiles_.get()) {
            // ignore file entirely
            continue;
        }
        readerMap[fileExtension] = std::move(reader);

        ++numSlices;
    }

    if (readerMap.empty()) {
        // could not find any suitable data reader for the images
        LogWarn("Image formats not supported");
        return;
    }

    auto getReaderIt = [&readerMap](const std::string& filename) {
        return readerMap.find(toLower(filesystem::getFileExtension(filename)));
    };

    // use first image to determine the volume size
    size2_t layerDims{0u, 0u};
    //    const DataFormatBase* format = nullptr;
    try {
        size_t slice = 0;

        // skip slices with unsupported image formats
        while (getReaderIt(fileNames[slice]) == readerMap.end()) {
            ++slice;
        }

        auto imgLayer = getReaderIt(fileNames[slice])->second->readData(fileNames[slice]);
        // Call getRepresentation here to force read a ram representation.
        // Otherwise the default image size, i.e. 256x265, will be reported
        // until you do the conversion since the LayerDisk does not provide any metadata.
        auto layerRAM = imgLayer->getRepresentation<LayerRAM>();
        layerDims = imgLayer->getDimensions();

        if ((layerDims.x == 0) || (layerDims.y == 0)) {
            LogError("Could not extract valid image dimensions from '" << fileNames[slice] << "'");
            return;
        }

        volume_ = layerRAM->dispatch<std::shared_ptr<Volume>, FloatOrIntMax32>(
            [&](auto baselayerprecision) {
                using ValueType = util::PrecsionValueType<decltype(baselayerprecision)>;
                using PrimitiveType = typename DataFormat<ValueType>::primitive;

                const size3_t volSize(layerDims, numSlices);
                // create matching volume representation
                auto volumeRAM = std::make_shared<VolumeRAMPrecision<ValueType>>(volSize);

                auto volume = std::make_shared<Volume>(volumeRAM);
                volume->dataMap_.dataRange =
                    dvec2{DataFormat<PrimitiveType>::lowest(), DataFormat<PrimitiveType>::max()};
                volume->dataMap_.valueRange =
                    dvec2{DataFormat<PrimitiveType>::lowest(), DataFormat<PrimitiveType>::max()};

                auto volData = volumeRAM->getDataTyped();
                const size_t sliceOffset = glm::compMul(layerDims);
                // initalize volume slices before current one
                std::fill(volData, volData + slice * sliceOffset, ValueType{0});

                // copy first image layer into volume
                auto layerDataBase = baselayerprecision->getDataTyped();
                std::copy(layerDataBase, layerDataBase + sliceOffset,
                          volData + slice * sliceOffset);

                // load remaining slices
                ++slice;
                while (slice < numSlices) {
                    auto it = getReaderIt(fileNames[slice]);
                    if (it == readerMap.end()) {
                        // reader does not exist for this format
                        std::fill(volData + slice * sliceOffset,
                                  volData + (slice + 1) * sliceOffset, ValueType{0});
                        ++slice;
                        continue;
                    }

                    try {
                        auto layer = (*it).second->readData(fileNames[slice]);
                        if (layer->getDimensions() != layerDims) {
                            // rescale layer if necessary
                            layer->setDimensions(layerDims);
                        }
                        layer->getRepresentation<LayerRAM>()->dispatch<void, FloatOrIntMax32>(
                            [&](auto layerpr) {
                                using ValueTypeLayer = util::PrecsionValueType<decltype(layerpr)>;
                                if ((DataFormat<ValueType>::numtype != NumericType::Float) &&
                                    (DataFormat<ValueType>::compsize > 4)) {
                                    throw DataReaderException(
                                        std::string{"Unsupported integer bit depth ("} +
                                        std::to_string(DataFormat<ValueType>::precision()) + ")");
                                }

                                auto layerData = layerpr->getDataTyped();
                                std::transform(
                                    layerData, layerData + sliceOffset,
                                    volData + slice * sliceOffset, [](auto value) {
                                        return util::glm_convert_normalized<ValueType>(value);
                                    });
                            });

                    } catch (DataReaderException const& e) {
                        LogProcessorError("Could not load image: " << fileNames[slice] << ", "
                                                                   << e.getMessage());
                        std::fill(volData + slice * sliceOffset,
                                  volData + (slice + 1) * sliceOffset, ValueType{0});
                    }
                    ++slice;
                }
                return volume;
            });

    } catch (DataReaderException const& e) {
        LogProcessorError("Could not load image: " << fileNames.front() << ", " << e.getMessage());
        return;
    }
    /*
    if ((layerDims.x == 0) || (layerDims.y == 0) || !format) {
        LogError("Invalid layer dimensions/format for volume");
        return;
    }

    // create a volume GL representation
    auto volumeRAM = createVolumeRAM(volSize, format);
    volume_ = std::make_shared<Volume>(volumeRAM);
    volume_->dataMap_.dataRange = dvec2{format->getLowest(), format->getMax()};
    volume_->dataMap_.valueRange = dvec2{format->getLowest(), format->getMax()};

    // set physical size of volume
    updateVolumeBasis(deserialized);

    volumeRAM->dispatch<void, FloatOrIntMax32>([&](auto volumeprecision) {
        using ValueType = util::PrecsionValueType<decltype(volumeprecision)>;
        using PrimitiveType = typename DataFormat<ValueType>::primitive;

        auto volData = volumeprecision->getDataTyped();
        const size_t sliceOffset = glm::compMul(layerDims);

        size_t slice = 0;
        for (auto file : fileNames) {
            // load image into texture
            std::string fileExtension = toLower(filesystem::getFileExtension(file));

            auto it = readerMap.find(fileExtension);
            if (it == readerMap.end()) {
                // reader does not exist for this format
                std::fill(volData + slice * sliceOffset, volData + (slice + 1) * sliceOffset,
                          ValueType{0});
                ++slice;
                continue;
            }

            try {
                auto layer = (*it).second->readData(file);
                if (layer->getDimensions() != layerDims) {
                    // rescale layer if necessary
                    layer->setDimensions(layerDims);
                }
                layer->getRepresentation<LayerRAM>()->dispatch<void, FloatOrIntMax32>(
                    [&](auto layerpr) {
                        auto layerData = layerpr->getDataTyped();
                        std::transform(
                            layerData, layerData + sliceOffset, volData + slice * sliceOffset,
                            [](auto value) {
                                return util::glm_convert_normalized<typename ValueType>(value);
                            });
                    });

            } catch (DataReaderException const& e) {
                LogProcessorError("Could not load image: " << file << ", " << e.getMessage());
                std::fill(volData + slice * sliceOffset, volData + (slice + 1) * sliceOffset,
                          ValueType{0});
            }

            ++slice;
        }
    });
    */

    basis_.updateForNewEntity(*volume_, deserialized);
    information_.updateForNewVolume(*volume_, deserialized);

    outport_.setData(volume_);
}

void ImageStackVolumeSource::deserialize(Deserializer& d) {
    isDeserializing_ = true;
    Processor::deserialize(d);
    isDeserializing_ = false;
    dirty_ = true;
}

void ImageStackVolumeSource::updateVolumeBasis(bool deserialized) {
    if (!volume_) {
        return;
    }
    size3_t layerDims{volume_->getDimensions()};
    vec3 spacing{voxelSpacing_.get()};

    double aspectRatio = static_cast<double>(layerDims.x) / static_cast<double>(layerDims.y);
    // consider voxel spacing
    aspectRatio *= spacing.x / spacing.y;

    vec3 scale{2.0f};
    if (aspectRatio > 1.0) {
        scale.y /= static_cast<float>(aspectRatio);
        scale.z = scale.x / (layerDims.x * spacing.x) * (layerDims.z * spacing.z);
    } else {
        scale.x *= static_cast<float>(aspectRatio);
        scale.z = scale.y / (layerDims.y * spacing.y) * (layerDims.z * spacing.z);
    }

    mat3 basis(glm::scale(scale));
    vec3 offset(-scale);
    offset *= 0.5;
    volume_->setBasis(basis);
    volume_->setOffset(offset);

    basis_.updateForNewEntity(*volume_, deserialized);
}

}  // namespace inviwo
