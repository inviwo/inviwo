/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/pathtype.h>
#include <inviwo/core/util/statecoordinator.h>
#include <inviwo/core/util/stdextensions.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <string_view>

#include <fmt/std.h>

namespace inviwo {
class Layer;

const ProcessorInfo ImageSourceSeries::processorInfo_{
    "org.inviwo.ImageSourceSeries",  // Class identifier
    "Image Series Source",           // Display name
    "Data Input",                    // Category
    CodeState::Experimental,         // Code state
    Tags::CPU,                       // Tags
    R"(Provides functionality to pick a single image from a 
    list of files matching a pattern or selection)"_unindentHelp};

const ProcessorInfo& ImageSourceSeries::getProcessorInfo() const { return processorInfo_; }

ImageSourceSeries::ImageSourceSeries(InviwoApplication* app)
    : Processor()
    , outport_("outputImage", "Selected image"_help, DataVec4UInt8::get(), HandleResizeEvents::No)
    , findFilesButton_("findFiles", "Update File List", "Reload the list of matching images"_help)
    , imageFilePattern_("imageFilePattern", "File Pattern",
                        filesystem::getPath(PathType::Images, "/*"), "")
    , currentImageIndex_("currentImageIndex", "Image Index", "Index of selected image file"_help, 1,
                         {1, ConstraintBehavior::Immutable}, {1, ConstraintBehavior::Immutable}, 1)
    , imageFileName_("imageFileName", "Image File Name",
                     "Name of the selected file (read-only)"_help) {

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
        throw Exception("Invalid image index. Exceeded number of files.");
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
