/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/base/processors/imagesourceseries.h>

#include <inviwo/core/common/factoryutil.h>              // for getDataReaderFactory
#include <inviwo/core/datastructures/image/image.h>      // for Image
#include <inviwo/core/io/datareader.h>                   // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>          // for DataReaderException
#include <inviwo/core/io/datareaderfactory.h>            // for DataReaderFactory
#include <inviwo/core/ports/imageport.h>                 // for ImageOutport
#include <inviwo/core/processors/processor.h>            // for Processor
#include <inviwo/core/processors/processorinfo.h>        // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>       // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>        // for Tags, Tags::CPU
#include <inviwo/core/properties/buttonproperty.h>       // for ButtonProperty
#include <inviwo/core/properties/filepatternproperty.h>  // for FilePatternProperty
#include <inviwo/core/properties/ordinalproperty.h>      // for IntProperty
#include <inviwo/core/properties/stringproperty.h>       // for StringProperty
#include <inviwo/core/util/assertion.h>                  // for ivwAssert
#include <inviwo/core/util/fileextension.h>              // for FileExtension
#include <inviwo/core/util/filesystem.h>                 // for getPath
#include <inviwo/core/util/formats.h>                    // for DataFormat, DataVec4UInt8
#include <inviwo/core/util/logcentral.h>                 // for LogCentral
#include <inviwo/core/util/pathtype.h>                   // for PathType, PathType::Images
#include <inviwo/core/util/statecoordinator.h>           // for StateCoordinator
#include <inviwo/core/util/stdextensions.h>              // for contains_if, erase_remove_if

#include <algorithm>    // for max
#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <map>          // for map, operator!=
#include <memory>       // for make_shared, operator!=, unique_ptr
#include <ostream>      // for operator<<, basic_ostream
#include <string_view>  // for string_view

namespace inviwo {
class Layer;

const ProcessorInfo ImageSourceSeries::processorInfo_{
    "org.inviwo.ImageSourceSeries",  // Class identifier
    "Image Series Source",           // Display name
    "Data Input",                    // Category
    CodeState::Experimental,         // Code state
    Tags::CPU,                       // Tags
};
const ProcessorInfo& ImageSourceSeries::getProcessorInfo() const { return processorInfo_; }

ImageSourceSeries::ImageSourceSeries(InviwoApplication* app)
    : Processor()
    , outport_("outputImage", DataVec4UInt8::get(), false)
    , findFilesButton_("findFiles", "Update File List")
    , imageFilePattern_("imageFilePattern", "File Pattern",
                        filesystem::getPath(PathType::Images, "/*"), "")
    , currentImageIndex_("currentImageIndex", "Image Index", 1, 1, 1, 1)
    , imageFileName_("imageFileName", "Image File Name") {

    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() { return !fileList_.empty(); });

    addPort(outport_);
    addProperty(imageFilePattern_);
    addProperty(findFilesButton_);
    addProperty(currentImageIndex_);
    addProperty(imageFileName_);

    validExtensions_ = util::getDataReaderFactory(app)->getExtensionsForType<Layer>();
    imageFilePattern_.addNameFilters(validExtensions_);

    imageFilePattern_.onChange([&]() {
        onFindFiles();
        isReady_.update();
    });
    findFilesButton_.onChange([&]() {
        onFindFiles();
        isReady_.update();
    });

    imageFileName_.setReadOnly(true);
}

void ImageSourceSeries::process() {
    if (fileList_.empty()) return;

    if (imageFilePattern_.isModified()) {
        // check all matching files whether they have a supported file extension,
        // i.e. a data reader exists
        fileList_ = imageFilePattern_.getFileList();
        const auto numElems = fileList_.size();
        std::erase_if(fileList_, [this](const auto& file) { return !isValidImageFile(file); });
        if (numElems != fileList_.size()) {
            // number of valid files has changed, need to update properties
            updateProperties();
            return;
        }
    }

    if (currentImageIndex_.isModified()) {
        updateFileName();
    }

    // sanity check for valid index
    const auto index = currentImageIndex_.get() - 1;
    if ((index < 0) || (index >= static_cast<int>(fileList_.size()))) {
        throw Exception(IVW_CONTEXT, "Invalid image index. Exceeded number of files.");
    }

    const auto currentFileName = fileList_[index];
    const auto sext = imageFilePattern_.getSelectedExtension();

    auto factory = util::getDataReaderFactory(getInviwoApplication());
    auto reader = factory->getReaderForTypeAndExtension<Layer>(sext, currentFileName);

    // there should always be a reader since we asked the reader for valid extensions
    IVW_ASSERT(reader != nullptr, "Could not find reader for \"" << currentFileName << "\"");

    auto layer = reader->readData(currentFileName);
    outport_.setData(std::make_shared<Image>(layer));
}

void ImageSourceSeries::onFindFiles() {
    // this processor will only be ready if at least one matching file exists
    fileList_ = imageFilePattern_.getFileList();
    if (fileList_.empty() && !imageFilePattern_.getFilePattern().empty()) {
        if (imageFilePattern_.hasOutOfRangeMatches()) {
            log::error("All matching files are outside the specified range (\"{}\", {} - {}).",
                       imageFilePattern_.getFilePattern(), imageFilePattern_.getMinRange(),
                       imageFilePattern_.getMaxRange());
        } else {
            log::error("No images found matching \"{}\" in {}.", imageFilePattern_.getFilePattern(),
                       imageFilePattern_.getFilePatternPath());
        }
    }
    updateProperties();
}

void ImageSourceSeries::updateProperties() {
    currentImageIndex_.setReadOnly(fileList_.size() <= 1);

    if (fileList_.size() < static_cast<std::size_t>(currentImageIndex_.get())) {
        currentImageIndex_.set(1);
    }
    currentImageIndex_.setMaxValue(std::max(static_cast<int>(fileList_.size()), 1));
    updateFileName();
    isReady_.update();
}

void ImageSourceSeries::updateFileName() {
    int index = currentImageIndex_.get() - 1;
    if ((index < 0) || (static_cast<std::size_t>(index) >= fileList_.size())) {
        imageFileName_.set("<no images found>");
    } else {
        imageFileName_.set(fileList_[index].string());
    }
}

bool ImageSourceSeries::isValidImageFile(const std::filesystem::path& fileName) {
    return util::contains_if(validExtensions_,
                             [&](const FileExtension& f) { return f.matches(fileName); });
}

}  // namespace inviwo
