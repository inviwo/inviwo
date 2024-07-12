/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <modules/base/datavisualizer/layertoimagevisualizer.h>

#include <inviwo/core/common/factoryutil.h>         // for getDataReaderFactory
#include <inviwo/core/io/datareaderfactory.h>       // for DataReaderFactory
#include <inviwo/core/network/processornetwork.h>   // for ProcessorNetwork
#include <inviwo/core/ports/layerport.h>            // for ImageOutport
#include <inviwo/core/ports/outport.h>              // for Outport
#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorutils.h>  // for makeProcessor, GridPos
#include <inviwo/core/rendering/datavisualizer.h>   // for DataVisualizer
#include <inviwo/core/util/document.h>              // for Document, Document::DocumentHandle
#include <modules/base/processors/layertoimage.h>

namespace inviwo {

using GP = util::GridPos;

LayerToImageVisualizer::LayerToImageVisualizer(InviwoApplication*) : DataVisualizer{} {}

std::string LayerToImageVisualizer::getName() const { return "Layer To Image"; }

Document LayerToImageVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct a layer to image processor");
    return doc;
}

std::vector<FileExtension> LayerToImageVisualizer::getSupportedFileExtensions() const { return {}; }

bool LayerToImageVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const LayerOutport*>(port) != nullptr;
}

bool LayerToImageVisualizer::hasSourceProcessor() const { return false; }
bool LayerToImageVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> LayerToImageVisualizer::addSourceProcessor(
    const std::filesystem::path&, ProcessorNetwork*, const ivec2&) const {
    return {nullptr, nullptr};
}

std::vector<Processor*> LayerToImageVisualizer::addVisualizerNetwork(Outport* outport,
                                                                     ProcessorNetwork* net) const {
    const ivec2 initialPos = util::getPosition(outport->getProcessor());

    auto* info = net->addProcessor(util::makeProcessor<LayerToImage>(GP{0, 3} + initialPos));
    net->addConnection(outport, info->getInports()[0]);

    return {info};
}

std::vector<Processor*> LayerToImageVisualizer::addSourceAndVisualizerNetwork(
    const std::filesystem::path&, ProcessorNetwork*, const ivec2&) const {
    return {};
}

}  // namespace inviwo
