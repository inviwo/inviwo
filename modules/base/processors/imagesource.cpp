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

const ProcessorInfo ImageSource::processorInfo_{
    "org.inviwo.ImageSource",  // Class identifier
    "Image Source",            // Display name
    "Data Input",              // Category
    CodeState::Stable,         // Code state
    Tags::CPU,                 // Tags
};
const ProcessorInfo ImageSource::getProcessorInfo() const { return processorInfo_; }

ImageSource::ImageSource()
    : Processor()
    , outport_("image.outport", DataVec4UInt8::get(), false)
    , file_("imageFileName", "File name", "", "image")
    , imageDimension_("imageDimension_", "Dimension", ivec2(0), ivec2(0), ivec2(10000), ivec2(1),
                      InvalidationLevel::Valid, PropertySemantics("Text"))
    , isDeserializing_(false) {

    addPort(outport_);

    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    file_.addNameFilters(rf->getExtensionsForType<Layer>());

    addProperty(file_);

    imageDimension_.setReadOnly(true);
    addProperty(imageDimension_);
}

bool ImageSource::isReady() const {
    return Processor::isReady() && filesystem::fileExists(file_.get());
}

void ImageSource::process() { 
    if (file_.get().empty()) return;

    std::string ext = filesystem::getFileExtension(file_.get());
    auto factory = getNetwork()->getApplication()->getDataReaderFactory();
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>(ext)) {
        try {
            auto outLayer = reader->readData(file_.get());
            // Call getRepresentation here to force read a ram representation.
            // Otherwise the default image size, i.e. 256x265, will be reported
            // until you do the conversion. Since the LayerDisk does not have any metadata.
            auto ram = outLayer->getRepresentation<LayerRAM>();
            // Hack needs to set format here since LayerDisk does not have a format.
            outLayer->setDataFormat(ram->getDataFormat());

            auto outImage = std::make_shared<Image>(outLayer);
            outImage->getRepresentation<ImageRAM>();

            outport_.setData(outImage);
            imageDimension_.set(outLayer->getDimensions());

        } catch (DataReaderException const& e) {
            util::log(e.getContext(), "Could not load data: " + file_.get() + ", " + e.getMessage(),
                      LogLevel::Error);
            file_.set("");
        }
    } else {
        LogError("Could not find a data reader for file: " << file_.get());
        file_.set("");
    }
}

void ImageSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    auto rf = InviwoApplication::getPtr()->getDataReaderFactory();
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension("*", "All Files"));
    file_.addNameFilters(rf->getExtensionsForType<Layer>());
}

}  // namespace
