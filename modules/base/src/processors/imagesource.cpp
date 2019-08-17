/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/base/processors/imagesource.h>
#include <inviwo/core/common/inviwoapplication.h>
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

ImageSource::ImageSource(InviwoApplication* app, const std::string& file)
    : Processor()
    , rf_(app->getDataReaderFactory())
    , outport_("image", DataVec4UInt8::get(), false)
    , file_("imageFileName", "File name", file, "image")
    , imageDimension_("imageDimension_", "Dimension", ivec2(0), ivec2(0), ivec2(10000), ivec2(1),
                      InvalidationLevel::Valid, PropertySemantics("Text")) {

    addPort(outport_);

    file_.clearNameFilters();
    file_.addNameFilter(FileExtension::all());
    file_.addNameFilters(rf_->getExtensionsForType<Layer>());

    addProperty(file_);

    imageDimension_.setReadOnly(true);
    addProperty(imageDimension_);

    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() { return !loadingFailed_ && filesystem::fileExists(file_.get()); });
    file_.onChange([this]() {
        loadingFailed_ = false;
        isReady_.update();
    });
}

void ImageSource::process() {
    if (file_.get().empty()) return;

    const auto sext = file_.getSelectedExtension();
    const auto fext = filesystem::getFileExtension(file_.get());
    if (auto reader = rf_->getReaderForTypeAndExtension<Layer>(sext, fext)) {
        try {
            auto outLayer = reader->readData(file_.get());
            outport_.setData(std::make_shared<Image>(outLayer));
            imageDimension_.set(outLayer->getDimensions());
        } catch (DataReaderException const& e) {
            util::log(e.getContext(), "Could not load data: " + file_.get() + ", " + e.getMessage(),
                      LogLevel::Error);
            loadingFailed_ = true;
            isReady_.update();
        }
    } else {
        LogError("Could not find a data reader for file: " << file_.get());
        loadingFailed_ = true;
        isReady_.update();
    }
}

void ImageSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension::all());
    file_.addNameFilters(rf_->getExtensionsForType<Layer>());
}

}  // namespace inviwo
