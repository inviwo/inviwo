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

namespace inviwo {

ProcessorClassIdentifier(ImageSourceSeries, "org.inviwo.ImageSourceSeries");
ProcessorDisplayName(ImageSourceSeries,  "Image Series Source");
ProcessorTags(ImageSourceSeries, Tags::CPU);
ProcessorCategory(ImageSourceSeries, "Data Input");
ProcessorCodeState(ImageSourceSeries, CODE_STATE_EXPERIMENTAL);

ImageSourceSeries::ImageSourceSeries()
    : Processor()
    , outport_("image.outport")
    , findFilesButton_("findFiles", "Update File List")
    , imageFileDirectory_("imageFileDirectory", "Image file directory", "image" ,
                          InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_IMAGES,"images"))
    , currentImageIndex_("currentImageIndex", "Image index", 1, 1, 1, 1)
    , imageFileName_("imageFileName", "Image file name") {

    addPort(outport_);
    addProperty(imageFileDirectory_);
    addProperty(findFilesButton_);
    addProperty(currentImageIndex_);
    addProperty(imageFileName_);

    validExtensions_ = DataReaderFactory::getPtr()->getExtensionsForType<Layer>();

    imageFileDirectory_.registerFileIndexingHandle(&currentImageIndex_);
    imageFileDirectory_.onChange(this, &ImageSourceSeries::onFindFiles);
    findFilesButton_.onChange(this, &ImageSourceSeries::onFindFiles);

    imageFileName_.setReadOnly(true);
}

ImageSourceSeries::~ImageSourceSeries() {}

void ImageSourceSeries::initialize() {
    Processor::initialize();
}

void ImageSourceSeries::deinitialize() {
    Processor::deinitialize();
}

void ImageSourceSeries::onFindFiles() {
    std::vector<std::string> files = imageFileDirectory_.getFiles();
    //imageFilesInDirectory_.clearOptions();
    std::vector<std::string> ids;
    std::vector<std::string> displayNames;

    for (size_t i=0; i<files.size(); i++) {
        if (isValidImageFile(files[i])) {
            std::string displayName = filesystem::getFileNameWithExtension(files[i]);
            ids.push_back(displayName+"_id");
            displayNames.push_back(displayName);
        }
    }

    //TODO: following settings leads to crash. under investigation.
    if (ids.size())
        currentImageIndex_.setMaxValue(static_cast<const int>(ids.size()));

    if (!ids.size()) {
        ids.push_back("noImage");
        displayNames.push_back("noImage");
        files.push_back("");
    }

    currentImageIndex_.set(1);
    imageFileName_.set(displayNames[0]);
}

/**
 * Creates a ImageDisk representation if there isn't an object already defined.
 **/
void ImageSourceSeries::process() {
    Image* outImage = outport_.getData();

    if (outImage) {
        std::vector<std::string> filesInDirectory = imageFileDirectory_.getFiles();
        std::vector<std::string> fileNames;

        for (size_t i=0; i<filesInDirectory.size(); i++) {
            if (isValidImageFile(filesInDirectory[i])) {
                fileNames.push_back(filesInDirectory[i]);
            }
        }

        size_t currentIndex = currentImageIndex_.get();

        if (!fileNames.size()) {
            LogWarn("No image found in the directory.");
            return;
        }

        if (currentIndex > fileNames.size()) {
            LogWarn("Current index exceeded the number of files.");
            return;
        }

        if (currentIndex==0) {
            LogWarn("Invalid index");
            return;
        }

        std::string currentFileName = fileNames[currentIndex - 1];
        imageFileName_.set(filesystem::getFileNameWithExtension(currentFileName));

        std::string fileExtension = filesystem::getFileExtension(currentFileName);
        DataReaderType<Layer>* reader =
            DataReaderFactory::getPtr()->getReaderForTypeAndExtension<Layer>(fileExtension);

        if (reader) {
            try {
                Layer* outLayer = reader->readMetaData(currentFileName);
                // Call getRepresentation here to force read a ram representation.
                // Otherwise the default image size, i.e. 256x265, will be reported 
                // until you do the conversion. Since the LayerDisk does not have any metadata.
                outLayer->getRepresentation<LayerRAM>();
                Image* outImage = new Image(outLayer);
                outImage->getRepresentation<ImageRAM>();

                outport_.setData(outImage);

            }
            catch (DataReaderException const& e) {
                LogError("Could not load data: " << currentFileName << ", " << e.getMessage());
            }
            delete reader;
        }
        else {
            LogError("Could not find a data reader for file: " << currentFileName);
        }
    }
}

bool ImageSourceSeries::isValidImageFile(std::string fileName){
    std::string fileExtension = filesystem::getFileExtension(fileName);
    for (std::vector<FileExtension>::const_iterator it = validExtensions_.begin(); it != validExtensions_.end(); ++it) {
        if(fileExtension == it->extension_)
            return true;
    }

    return false;
}

} // namespace
