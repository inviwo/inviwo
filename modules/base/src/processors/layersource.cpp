/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <modules/base/processors/layersource.h>

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/datastructures/image/layer.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerSource::processorInfo_{
    "org.inviwo.LayerSource",                  // Class identifier
    "Layer Source",                            // Display name
    "Data Input",                              // Category
    CodeState::Stable,                         // Code state
    Tags::CPU | Tag{"Layer"} | Tag{"Source"},  // Tags
    R"(Loads a Layer from an image file on disk.)"_unindentHelp};

const ProcessorInfo LayerSource::getProcessorInfo() const { return processorInfo_; }

LayerSource::LayerSource(InviwoApplication* app, const std::filesystem::path& filePath)
    : DataSource<Layer, LayerOutport>(util::getDataReaderFactory(app), filePath, "image")
    , dimensions_("dimensions", "Layer Dimensions",
                  util::ordinalCount(ivec2{0}, ivec2{4096})
                      .set("Dimensions of the image file"_help)
                      .set(InvalidationLevel::Valid)
                      .set(PropertySemantics::Text))
    , basis_("Basis", "Basis and offset") {

    DataSource<Layer, LayerOutport>::filePath.setDisplayName("Image File");
    addProperties(dimensions_, basis_);
}

void LayerSource::dataLoaded(std::shared_ptr<Layer> data) {
    dimensions_.set(data->getDimensions());
    basis_.updateForNewEntity(*data, false);
}
void LayerSource::dataDeserialized(std::shared_ptr<Layer> data) {
    dimensions_.set(data->getDimensions());
    basis_.updateForNewEntity(*data, true);
}

}  // namespace inviwo
