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
#include <modules/base/processors/datasource.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/layerdisk.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

#include <algorithm>

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
    , reader_("reader", "Data Reader")
    , reload_("reload", "Reload data")
    , imageDimension_("imageDimension_", "Dimension", size2_t(0), size2_t(0), size2_t(10000),
                      size2_t(1), InvalidationLevel::Valid, PropertySemantics("Text")) {

    addPort(outport_);
    addProperties(file_, reader_, reload_, imageDimension_);
    imageDimension_.setReadOnly(true);

    util::updateFilenameFilters<Layer>(*rf_, file_, reader_);
    util::updateReaderFromFile(file_, reader_);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
    isReady_.setUpdate([this]() {
        return !loadingFailed_ && filesystem::fileExists(file_.get()) &&
               !reader_.getSelectedValue().empty();
    });
    file_.onChange([this]() {
        loadingFailed_ = false;
        util::updateReaderFromFile(file_, reader_);
        isReady_.update();
    });
    reader_.onChange([this]() {
        loadingFailed_ = false;
        isReady_.update();
    });
}

void ImageSource::process() {
    if (file_.get().empty()) return;

    const auto sext = reader_.getSelectedValue();
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
            outport_.detachData();
            isReady_.update();
        }
    } else {
        LogError("Could not find a data reader for file: " << file_.get());
        loadingFailed_ = true;
        outport_.detachData();
        isReady_.update();
    }
}

void ImageSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    util::updateFilenameFilters<Layer>(*rf_, file_, reader_);
}

}  // namespace inviwo
