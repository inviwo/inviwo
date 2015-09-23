/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "imagesource.h"
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/imagedisk.h>
#include <inviwo/core/datastructures/image/layerdisk.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>


namespace inviwo {

ProcessorClassIdentifier(ImageSource, "org.inviwo.ImageSource");
ProcessorDisplayName(ImageSource, "Image Source");
ProcessorTags(ImageSource, Tags::CPU);
ProcessorCategory(ImageSource, "Data Input");
ProcessorCodeState(ImageSource, CODE_STATE_STABLE);

ImageSource::ImageSource()
    : Processor()
    , outport_("image.outport", DataVec4UINT8::get(), false)
    , imageFileName_("imageFileName", "File name", "", "image")
    , imageDimension_("imageDimension_", "Dimension", ivec2(0), ivec2(0), ivec2(10000),
                      ivec2(1), VALID, PropertySemantics("Text"))
    , isDeserializing_(false) {
    addPort(outport_);

    imageFileName_.onChange(this, &ImageSource::load);
    std::vector<FileExtension> ext = DataReaderFactory::getPtr()->getExtensionsForType<Layer>();
    for (std::vector<FileExtension>::const_iterator it = ext.begin(); it != ext.end(); ++it) {
        std::stringstream ss;
        ss << it->description_ << " (*." << it->extension_ << ")";
        imageFileName_.addNameFilter(ss.str());
    }

    addProperty(imageFileName_);

    imageDimension_.setReadOnly(true);
    addProperty(imageDimension_);
}

ImageSource::~ImageSource() {}

bool ImageSource::isReady() const { return filesystem::fileExists(imageFileName_.get()); }

void ImageSource::process() {}

void ImageSource::load() {
    if (isDeserializing_ || imageFileName_.get() == "") {
        return;
    }

    std::string fileExtension = filesystem::getFileExtension(imageFileName_.get());
    auto reader = DataReaderFactory::getPtr()->getReaderForTypeAndExtension<Layer>(fileExtension);

    if (reader) {
        try {
            auto outLayer = reader->readData(imageFileName_.get());
            // Call getRepresentation here to force read a ram representation.
            // Otherwise the default image size, i.e. 256x265, will be reported
            // until you do the conversion. Since the LayerDisk does not have any metadata.
            outLayer->getRepresentation<LayerRAM>();
            auto outImage = std::make_shared<Image>(outLayer);
            outImage->getRepresentation<ImageRAM>();

            outport_.setData(outImage);
            imageDimension_.set(outLayer->getDimensions());

        } catch (DataReaderException const& e) {
            util::log(e.getContext(),
                      "Could not load data: " + imageFileName_.get() + ", " + e.getMessage(),
                      LogLevel::Error);
            imageFileName_.set("");
        }
    } else {
        LogError("Could not find a data reader for file: " << imageFileName_.get());
        imageFileName_.set("");
    }
}

/**
 * Deserialize everything first then load the data
 */
void ImageSource::deserialize(IvwDeserializer& d) {
    isDeserializing_ = true;
    Processor::deserialize(d);
    isDeserializing_ = false;
    load();
}

}  // namespace
