/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

#include "imagesourceseries.h"
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/imagedisk.h>
#include <inviwo/core/datastructures/image/layerdisk.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

const ProcessorInfo ImageSourceSeries::processorInfo_{
    "org.inviwo.ImageSourceSeries",  // Class identifier
    "Image Series Source",           // Display name
    "Data Input",                    // Category
    CodeState::Experimental,         // Code state
    Tags::CPU,                       // Tags
};
const ProcessorInfo ImageSourceSeries::getProcessorInfo() const { return processorInfo_; }

ImageSourceSeries::ImageSourceSeries()
    : Processor()
    , outport_("outputImage", DataVec4UInt8::get(), false)
    , findFilesButton_("findFiles", "Update File List")
    , imageFilePattern_("imageFilePattern", "File Pattern", filesystem::getPath(PathType::Images, "/*"), "")
    , currentImageIndex_("currentImageIndex", "Image Index", 1, 1, 1, 1)
    , imageFileName_("imageFileName", "Image File Name")
    , ready_(false) {
    

    addPort(outport_);
    addProperty(imageFilePattern_);
    addProperty(findFilesButton_);
    addProperty(currentImageIndex_);
    addProperty(imageFileName_);

    validExtensions_ =
        InviwoApplication::getPtr()->getDataReaderFactory()->getExtensionsForType<Layer>();
    imageFilePattern_.addNameFilters(validExtensions_);

    imageFilePattern_.onChange([&]() { onFindFiles(); });
    findFilesButton_.onChange([&]() { onFindFiles(); });
    
    imageFileName_.setReadOnly(true);
}

void ImageSourceSeries::process() {
    if (fileList_.empty()) return;

    if (imageFilePattern_.isModified()) {
        // check all matching files whether they have a supported file extension, 
        // i.e. a data reader exists
        fileList_ = imageFilePattern_.getFileList();
        const auto numElems = fileList_.size();
        util::erase_remove_if(fileList_,
                              [this](std::string& file) { return !isValidImageFile(file); });
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
        LogError("Invalid image index. Exceeded number of files.");
        return;
    }

    const std::string currentFileName = fileList_[index];
    const std::string ext = filesystem::getFileExtension(currentFileName);

    auto factory = getNetwork()->getApplication()->getDataReaderFactory();
    auto reader = factory->getReaderForTypeAndExtension<Layer>(ext);

    // there should always be a reader since we asked the reader for valid extensions
    ivwAssert(reader != nullptr, "Could not find reader for \"" << currentFileName << "\"");

    try {
        auto layer = reader->readData(currentFileName);
        // Call getRepresentation here to force read a ram representation.
        // Otherwise the default image size, i.e. 256x256, will be reported
        // until you do the conversion. Since the LayerDisk does not have any metadata.
        auto ram = layer->getRepresentation<LayerRAM>();
        // Hack needs to set format here since LayerDisk does not have a format.
        layer->setDataFormat(ram->getDataFormat());

        auto outImage = std::make_shared<Image>(layer);
        outImage->getRepresentation<ImageRAM>();

        outport_.setData(outImage);
    } catch (DataReaderException const& e) {
        LogError(e.getMessage());
    }
}

bool ImageSourceSeries::isReady() const {
    return ready_;
}

void ImageSourceSeries::onFindFiles() {
    // this processor will only be ready if at least one matching file exists
    fileList_ = imageFilePattern_.getFileList();
    if (fileList_.empty() && !imageFilePattern_.getFilePattern().empty()) {
        if (imageFilePattern_.hasOutOfRangeMatches()) {
            LogError("All matching files are outside the specified range (\""
                        << imageFilePattern_.getFilePattern() << "\", "
                        << imageFilePattern_.getMinRange() << " - "
                        << imageFilePattern_.getMaxRange()
                        << ").");
        }
        else {
            LogError("No images found matching \"" << imageFilePattern_.getFilePattern() << "\" in " 
                     << imageFilePattern_.getFilePatternPath() << ".");
        }
    }
    updateProperties();
}

void ImageSourceSeries::updateProperties() {
    ready_ = !fileList_.empty();

    currentImageIndex_.setReadOnly(fileList_.size() <= 1);

    if (fileList_.size() < static_cast<std::size_t>(currentImageIndex_.get())) {
        currentImageIndex_.set(1);
    }
    currentImageIndex_.setMaxValue(std::max(static_cast<const int>(fileList_.size()), 1));
    updateFileName();
}

void ImageSourceSeries::updateFileName() {
    int index = currentImageIndex_.get() - 1;
    if ((index < 0) || (static_cast<std::size_t>(index) >= fileList_.size())) {
        imageFileName_.set("<no images found>");
    } else {
        imageFileName_.set(fileList_[index]);
    }
}

bool ImageSourceSeries::isValidImageFile(std::string fileName) {
    std::string fileExtension = filesystem::getFileExtension(fileName);
    return util::contains_if(validExtensions_, [&](const FileExtension& f){
        return f.extension_ == fileExtension;});
}

}  // namespace
