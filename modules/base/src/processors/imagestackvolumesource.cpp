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
#include <inviwo/core/util/zip.h>
#include <inviwo/core/io/datareaderexception.h>

#include <algorithm>

#include <fmt/format.h>
#include <fmt/ostream.h>

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

ImageStackVolumeSource::ImageStackVolumeSource(InviwoApplication* app)
    : Processor()
    , outport_("volume")
    , filePattern_("filePattern", "File Pattern", "####.jpeg", "")
    , skipUnsupportedFiles_("skipUnsupportedFiles", "Skip Unsupported Files", false)
    // volume parameters
    , voxelSpacing_("voxelSpacing", "Voxel Spacing", vec3(1.0f), vec3(0.1f), vec3(10.0f),
                    vec3(0.1f))
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , app_{app} {

    addPort(outport_);
    addProperty(filePattern_);
    addProperty(skipUnsupportedFiles_);
    addProperty(voxelSpacing_);
    addProperty(information_);
    addProperty(basis_);

    validExtensions_ = app_->getDataReaderFactory()->getExtensionsForType<Layer>();
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

    const auto files = filePattern_.getFileList();

    using ReaderMap = std::map<std::string, std::unique_ptr<DataReaderType<Layer>>>;
    ReaderMap readerMap;

    const auto getReader = [&](const std::string& filename) {
        const auto fileExtension = toLower(filesystem::getFileExtension(filename));
        const auto it = readerMap.find(fileExtension);
        if (it != readerMap.end()) {
            return it->second.get();
        }
        auto reader =
            app_->getDataReaderFactory()->getReaderForTypeAndExtension<Layer>(fileExtension);
        auto ptr = reader.get();
        readerMap.insert({fileExtension, std::move(reader)});
        return ptr;
    };

    std::vector<std::pair<std::string, DataReaderType<Layer>*>> slices;
    slices.reserve(files.size());

    std::transform(files.begin(), files.end(), std::back_inserter(slices),
                   [&](const auto& file) -> std::pair<std::string, DataReaderType<Layer>*> {
                       return {file, getReader(file)};
                   });
    if (skipUnsupportedFiles_) {
        slices.erase(std::remove_if(slices.begin(), slices.end(),
                                    [](auto& elem) { return elem.second == nullptr; }),
                     slices.end());
    }

    // identify first slice with a reader
    const auto first = std::find_if(slices.begin(), slices.end(),
                                    [](auto& item) { return item.second != nullptr; });
    if (first == slices.end()) {  // could not find any suitable data reader for the images
        throw Exception(
            fmt::format("No supported images found in '{}'", filePattern_.getFilePatternPath()),
            IVW_CONTEXT);
    }

    const auto referenceLayer = first->second->readData(first->first);

    // Call getRepresentation here to enforce creating a ram representation.
    // Otherwise the default image size, i.e. 256x256, will be reported since the LayerDisk
    // does not provide meta data for all image formats.
    const auto layerRAM = referenceLayer->getRepresentation<LayerRAM>();
    if (glm::compMul(layerRAM->getDimensions()) == 0) {
        throw Exception(
            fmt::format("Could not extract valid image dimensions from '{}'", first->first),
            IVW_CONTEXT);
    }

    const auto refFormat = layerRAM->getDataFormat();
    if ((refFormat->getNumericType() != NumericType::Float) && (refFormat->getPrecision() > 32)) {
        throw DataReaderException(
            fmt::format("Unsupported integer bit depth ({})", refFormat->getPrecision()),
            IVW_CONTEXT);
    }

    volume_ =
        layerRAM->dispatch<std::shared_ptr<Volume>, FloatOrIntMax32>([&](auto reflayerprecision) {
            using ValueType = util::PrecsionValueType<decltype(reflayerprecision)>;
            using PrimitiveType = typename DataFormat<ValueType>::primitive;

            const size2_t layerDims = reflayerprecision->getDimensions();
            const size_t sliceOffset = glm::compMul(layerDims);

            // create matching volume representation
            auto volumeRAM =
                std::make_shared<VolumeRAMPrecision<ValueType>>(size3_t{layerDims, slices.size()});
            auto volData = volumeRAM->getDataTyped();

            const auto fill = [&](size_t s) {
                std::fill(volData + s * sliceOffset, volData + (s + 1) * sliceOffset, ValueType{0});
            };

            const auto read = [&](const auto& file, auto reader) -> std::shared_ptr<Layer> {
                try {
                    return reader->readData(file);
                } catch (DataReaderException const& e) {
                    LogProcessorWarn(
                        fmt::format("Could not load image: {}, {}", file, e.getMessage()));
                    return nullptr;
                }
            };

            for (auto&& elem : util::enumerate(slices)) {
                const auto slice = elem.first();
                const auto file = elem.second().first;
                const auto reader = elem.second().second;
                if (!reader) {
                    fill(slice);
                    continue;
                }

                const auto layer = read(file, reader);
                if (!layer) {
                    fill(slice);
                    continue;
                }
                const auto layerRAM = layer->template getRepresentation<LayerRAM>();

                const auto format = layerRAM->getDataFormat();
                if ((format->getNumericType() != NumericType::Float) &&
                    (format->getPrecision() > 32)) {
                    LogProcessorWarn(fmt::format("Unsupported integer bit depth: {}, for image: {}",
                                                 format->getPrecision(), file));
                    fill(slice);
                    continue;
                }

                if (layerRAM->getDimensions() != layerDims) {
                    LogProcessorWarn(
                        fmt::format("Unexpected dimensions: {} , expected: {}, for image: {}",
                                    layer->getDimensions(), layerDims, file));
                    fill(slice);
                    continue;
                }
                layerRAM->template dispatch<void, FloatOrIntMax32>([&](auto layerpr) {
                    const auto data = layerpr->getDataTyped();
                    std::transform(
                        data, data + sliceOffset, volData + slice * sliceOffset,
                        [](auto value) { return util::glm_convert_normalized<ValueType>(value); });
                });
            }

            auto volume = std::make_shared<Volume>(volumeRAM);
            volume->dataMap_.dataRange =
                dvec2{DataFormat<PrimitiveType>::lowest(), DataFormat<PrimitiveType>::max()};
            volume->dataMap_.valueRange =
                dvec2{DataFormat<PrimitiveType>::lowest(), DataFormat<PrimitiveType>::max()};

            return volume;
        });

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
    vec3 extent{voxelSpacing_.get() * vec3{volume_->getDimensions()}};
    volume_->setBasis(glm::scale(extent));
    volume_->setOffset(-extent * 0.5f);

    basis_.updateForNewEntity(*volume_, deserialized);
}

}  // namespace inviwo
