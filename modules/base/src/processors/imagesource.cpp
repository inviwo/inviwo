/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/statecoordinator.h>
#include <modules/base/processors/datasource.h>

#include <map>
#include <memory>
#include <ostream>
#include <type_traits>

#include <fmt/std.h>

namespace inviwo {
class Deserializer;
class Layer;

const ProcessorInfo ImageSource::processorInfo_{"org.inviwo.ImageSource",  // Class identifier
                                                "Image Source",            // Display name
                                                "Data Input",              // Category
                                                CodeState::Stable,         // Code state
                                                Tags::CPU,                 // Tags
                                                "Loads an image from disk."_help};
const ProcessorInfo& ImageSource::getProcessorInfo() const { return processorInfo_; }

ImageSource::ImageSource(InviwoApplication* app, const std::filesystem::path& file)
    : DataSource<Image, ImageOutport, Layer>(util::getDataReaderFactory(app), file, "image")
    , dimensions_("imageDimension_", "Image Dimensions",
                  util::ordinalCount(size2_t{0}, size2_t{4096})
                      .set("Dimensions of the image file"_help)
                      .set(InvalidationLevel::Valid)
                      .set(PropertySemantics::Text)
                      .set(ReadOnly::Yes)) {

    port_.setIdentifier("image");
    port_.setHandleResizeEvents(false);
    filePath.setIdentifier("imageFileName");

    DataSource<Image, ImageOutport, Layer>::filePath.setDisplayName("Image File");
    addProperties(dimensions_);
}

std::shared_ptr<Image> ImageSource::transform(std::shared_ptr<Layer> layer) {
    return std::make_shared<Image>(layer);
}
void ImageSource::dataLoaded(std::shared_ptr<Image> data) {
    dimensions_.set(data->getDimensions());
}
void ImageSource::dataDeserialized(std::shared_ptr<Image> data) {
    dimensions_.set(data->getDimensions());
}

}  // namespace inviwo
