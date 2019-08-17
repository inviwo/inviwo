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
#include <inviwo/core/util/raiiutils.h>
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
    , reload_("reload", "Reload data")
    , skipUnsupportedFiles_("skipUnsupportedFiles", "Skip Unsupported Files", false)
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , readerFactory_{app->getDataReaderFactory()} {

    addPort(outport_);
    addProperty(filePattern_);
    addProperty(reload_);
    addProperty(skipUnsupportedFiles_);
    addProperty(basis_);
    addProperty(information_);

    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() { return !filePattern_.getFileList().empty(); });
    filePattern_.onChange([&]() { isReady_.update(); });

    addFileNameFilters();
}

void ImageStackVolumeSource::addFileNameFilters() {
    filePattern_.clearNameFilters();
    filePattern_.addNameFilter(FileExtension::all());
    filePattern_.addNameFilters(readerFactory_->getExtensionsForType<Layer>());
}

void ImageStackVolumeSource::process() {
    util::OnScopeExit guard{[&]() { outport_.setData(nullptr); }};

    if (filePattern_.isModified() || reload_.isModified() || skipUnsupportedFiles_.isModified()) {
        volume_ = load();
        if (volume_) {
            basis_.updateForNewEntity(*volume_, deserialized_);
            information_.updateForNewVolume(*volume_, deserialized_);
        }
        deserialized_ = false;
    }

    if (volume_) {
        basis_.updateEntity(*volume_);
        information_.updateVolume(*volume_);
    }
    outport_.setData(volume_);
    guard.release();
}

bool ImageStackVolumeSource::isValidImageFile(std::string fileName) {
    return readerFactory_->hasReaderForTypeAndExtension<Layer>(
        filesystem::getFileExtension(fileName));
}

std::shared_ptr<Volume> ImageStackVolumeSource::load() {
    const auto files = filePattern_.getFileList();
    if (files.empty()) {
        return nullptr;
    }

    using ReaderMap = std::map<std::string, std::unique_ptr<DataReaderType<Layer>>>;
    ReaderMap readerMap;

    const auto getReader = [&](const std::string& filename) {
        const auto fext = toLower(filesystem::getFileExtension(filename));
        const auto it = readerMap.find(fext);
        if (it != readerMap.end()) {
            return it->second.get();
        }
        const auto sext = filePattern_.getSelectedExtension();
        auto reader = readerFactory_->getReaderForTypeAndExtension<Layer>(sext, fext);
        auto ptr = reader.get();
        readerMap.emplace(fext, std::move(reader));
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
    const auto referenceRAM = referenceLayer->getRepresentation<LayerRAM>();
    if (glm::compMul(referenceRAM->getDimensions()) == 0) {
        throw Exception(
            fmt::format("Could not extract valid image dimensions from '{}'", first->first),
            IVW_CONTEXT);
    }

    const auto refFormat = referenceRAM->getDataFormat();
    if ((refFormat->getNumericType() != NumericType::Float) && (refFormat->getPrecision() > 32)) {
        throw DataReaderException(
            fmt::format("Unsupported integer bit depth ({})", refFormat->getPrecision()),
            IVW_CONTEXT);
    }

    return referenceRAM->dispatch<std::shared_ptr<Volume>, FloatOrIntMax32>(
        [&](auto reflayerprecision) {
            using ValueType = util::PrecisionValueType<decltype(reflayerprecision)>;
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

            const auto size = vec3(0.01f) * static_cast<vec3>(volumeRAM->getDimensions());
            volume->setBasis(glm::diagonal3x3(size));
            volume->setOffset(-0.5 * size);

            return volume;
        });
}

void ImageStackVolumeSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    addFileNameFilters();
    deserialized_ = true;
}

}  // namespace inviwo
