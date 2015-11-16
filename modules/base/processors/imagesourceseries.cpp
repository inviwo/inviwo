/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
const ProcessorInfo ImageSourceSeries::getProcessorInfo() const {
    return processorInfo_;
}

ImageSourceSeries::ImageSourceSeries()
    : Processor()
    , outport_("image.outport")
    , findFilesButton_("findFiles", "Update File List")
    , imageFileDirectory_(
          "imageFileDirectory", "Image file directory", "",
          InviwoApplication::getPtr()->getPath(PathType::Images, "/images"))
    , currentImageIndex_("currentImageIndex", "Image index", 1, 1, 1, 1)
    , imageFileName_("imageFileName", "Image file name") {
    addPort(outport_);
    addProperty(imageFileDirectory_);
    addProperty(findFilesButton_);
    addProperty(currentImageIndex_);
    addProperty(imageFileName_);

    validExtensions_ = InviwoApplication::getPtr()->getDataReaderFactory()->getExtensionsForType<Layer>();

    imageFileDirectory_.onChange(this, &ImageSourceSeries::onFindFiles);
    findFilesButton_.onChange(this, &ImageSourceSeries::onFindFiles);

    imageFileName_.setReadOnly(true);

    this->onFindFiles();
}

void ImageSourceSeries::onFindFiles() {
    std::string path{imageFileDirectory_.get()};
    if (!path.empty()) {
        std::vector<std::string> files = filesystem::getDirectoryContents(path);

        fileList_.clear();
        for (std::size_t i = 0; i < files.size(); i++) {
            if (isValidImageFile(files[i])) {
                std::string fileName = filesystem::getFileNameWithExtension(files[i]);
                fileList_.push_back(fileName);
            }
        }
        if (fileList_.empty()) {
            LogWarn("No images found in \"" << imageFileDirectory_.get() << "\"");
        }
    }

    updateProperties();
}

/**
 * Creates a ImageDisk representation if there isn't an object already defined.
 **/
void ImageSourceSeries::process() {
    if (fileList_.empty()) return;

    std::string basePath{imageFileDirectory_.get()};
    long currentIndex = currentImageIndex_.get() - 1;
    if ((currentIndex < 0) || (currentIndex >= static_cast<long>(fileList_.size()))) {
        LogError("Invalid image index. Exceeded number of files.");
        return;
    }

    std::string currentFileName{basePath + "/" + fileList_[currentIndex]};
    imageFileName_.set(fileList_[currentIndex]);

    std::string fileExtension = filesystem::getFileExtension(currentFileName);
    auto factory = InviwoApplication::getPtr()->getDataReaderFactory();
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>(fileExtension)) {
        try {
            auto outLayer = reader->readData(currentFileName);
            // Call getRepresentation here to force read a ram representation.
            // Otherwise the default image size, i.e. 256x265, will be reported
            // until you do the conversion. Since the LayerDisk does not have any metadata.
            outLayer->getRepresentation<LayerRAM>();
            auto newOutImage = std::make_shared<Image>(outLayer);
            newOutImage->getRepresentation<ImageRAM>();

            outport_.setData(newOutImage);
        } catch (DataReaderException const& e) {
            util::log(e.getContext(),
                      "Could not load data: " + imageFileName_.get() + ", " + e.getMessage(),
                      LogLevel::Error);
        }
    } else {
        LogWarn("Could not find a data reader for file: " << currentFileName);
        // remove file from list
        fileList_.erase(fileList_.begin() + currentIndex);
        // adjust index property
        updateProperties();
    }
}

void ImageSourceSeries::updateProperties() {
    currentImageIndex_.setReadOnly(fileList_.empty());

    if (fileList_.size() < static_cast<std::size_t>(currentImageIndex_.get()))
        currentImageIndex_.set(1);

    // clamp the number of files since setting the maximum to 0 will reset the min value to 0
    int numFiles = std::max(static_cast<const int>(fileList_.size()), 1);
    currentImageIndex_.setMaxValue(numFiles);
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
    for (std::vector<FileExtension>::const_iterator it = validExtensions_.begin();
         it != validExtensions_.end(); ++it) {
        if (fileExtension == it->extension_) return true;
    }

    return false;
}

}  // namespace

