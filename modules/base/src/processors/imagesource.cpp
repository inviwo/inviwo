/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>            // for getDataReaderFactory
#include <inviwo/core/datastructures/image/image.h>    // for Image
#include <inviwo/core/io/datareader.h>                 // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>        // for DataReaderException
#include <inviwo/core/io/datareaderfactory.h>          // for DataReaderFactory
#include <inviwo/core/ports/imageport.h>               // for ImageOutport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::CPU
#include <inviwo/core/properties/fileproperty.h>       // for FileProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/optionproperty.h>     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for IntSize2Property
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics
#include <inviwo/core/util/fileextension.h>            // for FileExtension, operator==, operator<<
#include <inviwo/core/util/filesystem.h>               // for fileExists
#include <inviwo/core/util/formats.h>                  // for DataFormat, DataVec4UInt8
#include <inviwo/core/util/glmvec.h>                   // for size2_t
#include <inviwo/core/util/logcentral.h>               // for log, LogCentral
#include <inviwo/core/util/statecoordinator.h>         // for StateCoordinator
#include <modules/base/processors/datasource.h>        // for updateFilenameFilters, updateReade...

#include <map>          // for map, operator!=
#include <memory>       // for make_shared, unique_ptr, shared_ptr
#include <ostream>      // for operator<<
#include <type_traits>  // for remove_extent_t

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
