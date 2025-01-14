/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>                             // for getDataReaderFactory
#include <inviwo/core/datastructures/image/layer.h>                     // for DataReaderType
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/io/datareader.h>                          // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                 // for DataReaderException
#include <inviwo/core/io/datareaderfactory.h>                   // for DataReaderFactory
#include <inviwo/core/ports/volumeport.h>                       // for VolumeOutport
#include <inviwo/core/processors/processor.h>                   // for Processor
#include <inviwo/core/processors/processorinfo.h>               // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>              // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>               // for Tags
#include <inviwo/core/properties/boolproperty.h>                // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>              // for ButtonProperty
#include <inviwo/core/properties/filepatternproperty.h>         // for FilePatternProperty
#include <inviwo/core/properties/property.h>                    // for OverwriteState
#include <inviwo/core/util/exception.h>                         // for Exception
#include <inviwo/core/util/fileextension.h>                     // for FileExtension
#include <inviwo/core/util/formatdispatching.h>                 // for PrecisionValueType
#include <inviwo/core/util/formats.h>                           // for DataFormat, DataF...
#include <inviwo/core/util/glmconvert.h>                        // for glm_convert_norma...
#include <inviwo/core/util/glmvec.h>                            // for vec3, dvec2, size2_t
#include <inviwo/core/util/logcentral.h>                        // for LogCentral, LogPr...
#include <inviwo/core/util/raiiutils.h>                         // for OnScopeExit, OnSc...
#include <inviwo/core/util/statecoordinator.h>                  // for StateCoordinator
#include <inviwo/core/util/zip.h>                               // for zipper, enumerate
#include <modules/base/properties/basisproperty.h>              // for BasisProperty
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInformation...

#include <algorithm>      // for fill, transform
#include <cstddef>        // for size_t
#include <functional>     // for __base
#include <iterator>       // for back_insert_iterator
#include <map>            // for map, operator!=
#include <string_view>    // for string_view
#include <type_traits>    // for integral_constant
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair, move
#include <vector>         // for vector

#include <fmt/core.h>                         // for format, basic_str...
#include <glm/ext/vector_float3.hpp>          // for vec3
#include <glm/gtx/component_wise.hpp>         // for compMul
#include <glm/gtx/io.hpp>                     // for operator<<
#include <glm/gtx/matrix_operation.hpp>       // for diagonal3x3
#include <glm/gtx/scalar_multiplication.hpp>  // for operator*
#include <glm/vec2.hpp>                       // for operator!=
#include <glm/vec3.hpp>                       // for operator*

#include <fmt/std.h>

namespace inviwo {
class Deserializer;
template <typename T>
class VolumeRAMPrecision;

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
const ProcessorInfo& ImageStackVolumeSource::getProcessorInfo() const { return processorInfo_; }

ImageStackVolumeSource::ImageStackVolumeSource(InviwoApplication* app)
    : Processor()
    , outport_("volume")
    , filePattern_("filePattern", "File Pattern", "####.jpeg", "")
    , reload_("reload", "Reload data")
    , skipUnsupportedFiles_("skipUnsupportedFiles", "Skip Unsupported Files", false)
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information")
    , readerFactory_{util::getDataReaderFactory(app)} {

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
            const auto overwrite =
                deserialized_ ? util::OverwriteState::Yes : util::OverwriteState::No;
            information_.updateForNewVolume(*volume_, overwrite);
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

std::shared_ptr<Volume> ImageStackVolumeSource::load() {
    const auto files = filePattern_.getFileList();
    if (files.empty()) {
        return nullptr;
    }

    std::vector<std::pair<std::filesystem::path, std::unique_ptr<DataReaderType<Layer>>>> slices;
    slices.reserve(files.size());

    std::transform(files.begin(), files.end(), std::back_inserter(slices),
                   [&](const auto& file)
                       -> std::pair<std::filesystem::path, std::unique_ptr<DataReaderType<Layer>>> {
                       return {file, std::move(readerFactory_->getReaderForTypeAndExtension<Layer>(
                                         filePattern_.getSelectedExtension(), file))};
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
        throw Exception(SourceContext{}, "No supported images found in '{}'",
                        filePattern_.getFilePatternPath());
    }

    const auto referenceLayer = first->second->readData(first->first);

    // Call getRepresentation here to enforce creating a ram representation.
    // Otherwise the default image size, i.e. 256x256, will be reported since the LayerDisk
    // does not provide meta data for all image formats.
    const auto* referenceRAM = referenceLayer->getRepresentation<LayerRAM>();
    if (glm::compMul(referenceRAM->getDimensions()) == 0) {
        throw Exception(SourceContext{}, "Could not extract valid image dimensions from {}",
                        first->first);
    }

    const auto* refFormat = referenceRAM->getDataFormat();
    if ((refFormat->getNumericType() != NumericType::Float) && (refFormat->getPrecision() > 32)) {
        throw DataReaderException(SourceContext{}, "Unsupported integer bit depth ({})",
                                  refFormat->getPrecision());
    }

    return referenceRAM->dispatch<std::shared_ptr<Volume>, FloatOrIntMax32>(
        [&](auto refLayerPrecision) {
            using ValueType = util::PrecisionValueType<decltype(refLayerPrecision)>;
            using PrimitiveType = typename DataFormat<ValueType>::primitive;

            const size2_t layerDims = refLayerPrecision->getDimensions();
            const size_t sliceOffset = glm::compMul(layerDims);

            // create matching volume representation
            auto volumeRAM =
                std::make_shared<VolumeRAMPrecision<ValueType>>(size3_t{layerDims, slices.size()});
            auto volData = volumeRAM->getDataTyped();

            const auto fill = [&](size_t s) {
                std::fill(volData + s * sliceOffset, volData + (s + 1) * sliceOffset, ValueType{0});
            };

            const auto read = [&](const auto& file, auto* reader) -> std::shared_ptr<Layer> {
                try {
                    return reader->readData(file);
                } catch (const DataReaderException& e) {
                    log::warn("Could not load image: {}, {}", file, e.getMessage());
                    return nullptr;
                }
            };

            for (auto&& elem : util::enumerate(slices)) {
                const auto slice = elem.first();
                const auto file = elem.second().first;
                auto* reader = elem.second().second.get();
                if (!reader) {
                    fill(slice);
                    continue;
                }

                const auto layer = read(file, reader);
                if (!layer) {
                    fill(slice);
                    continue;
                }
                const auto* layerRAM = layer->template getRepresentation<LayerRAM>();

                const auto* format = layerRAM->getDataFormat();
                if ((format->getNumericType() != NumericType::Float) &&
                    (format->getPrecision() > 32)) {
                    log::warn("Unsupported integer bit depth: {}, for image: {}",
                              format->getPrecision(), file);
                    fill(slice);
                    continue;
                }

                if (layerRAM->getDimensions() != layerDims) {
                    log::warn("Unexpected dimensions: {}, expected: {}, for image: {}",
                              layer->getDimensions(), layerDims, file);
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
            volume->dataMap.dataRange =
                dvec2{DataFormat<PrimitiveType>::lowest(), DataFormat<PrimitiveType>::max()};
            volume->dataMap.valueRange =
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
