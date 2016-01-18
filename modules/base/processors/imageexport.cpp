/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "imageexport.h"
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

const ProcessorInfo ImageExport::processorInfo_{
    "org.inviwo.ImageExport",  // Class identifier
    "Image Export",            // Display name
    "Data Output",             // Category
    CodeState::Stable,         // Code state
    Tags::CPU,                 // Tags
};
const ProcessorInfo ImageExport::getProcessorInfo() const { return processorInfo_; }

ImageExport::ImageExport()
    : Processor()
    , imagePort_("image")
    , imageFile_("imageFileName", "Image file name",
                 filesystem::getPath(PathType::Images, "/newimage.png"), "image")
    , exportImageButton_("snapshot", "Export Image", InvalidationLevel::Valid)
    , overwrite_("overwrite", "Overwrite", false)
    , exportQueued_(false) {
    auto app = InviwoApplication::getPtr();
    for (auto& ext : app->getDataWriterFactory()->getExtensionsForType<Image>()) {
        std::stringstream ss;
        ss << ext.description_ << " (*." << ext.extension_ << ")";
        imageFile_.addNameFilter(ss.str());
    }

    addPort(imagePort_);
    addProperty(imageFile_);
    imageFile_.setAcceptMode(FileProperty::AcceptMode::Save);
    exportImageButton_.onChange(this, &ImageExport::exportImage);
    addProperty(exportImageButton_);
    addProperty(overwrite_);
}

ImageExport::~ImageExport() {}

void ImageExport::exportImage() {
    if (!isValid()) {
        exportQueued_ = true;
        return;
    }
    processExport();
}

void ImageExport::process() {
    if (exportQueued_) processExport();
}

void ImageExport::processExport() {
    exportQueued_ = false;
    auto image = imagePort_.getData();

    if (image && !imageFile_.get().empty()) {
        if (auto layer = image->getColorLayer()) {
            std::string fileExtension = filesystem::getFileExtension(imageFile_.get());
            auto app = InviwoApplication::getPtr();
            if (auto writer = app->getDataWriterFactory()->getWriterForTypeAndExtension<Layer>(
                    fileExtension)) {
                try {
                    writer->setOverwrite(overwrite_.get());
                    writer->writeData(layer, imageFile_.get());
                    LogInfo("Image color layer exported to disk: " << imageFile_.get());
                } catch (DataWriterException const& e) {
                    util::log(e.getContext(), e.getMessage(), LogLevel::Error);
                }
            } else {
                LogError(
                    "Error: Could not find a writer for the specified extension and data type");
            }
        } else {
            LogError("Error: Could not find color layer to write out");
        }
    } else if (imageFile_.get().empty()) {
        LogWarn("Error: Please specify a file to write to");
    } else if (!image) {
        LogWarn("Error: Please connect an image to export");
    }
}

}  // namespace
