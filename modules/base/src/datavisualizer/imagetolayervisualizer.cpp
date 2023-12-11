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

#include <modules/base/datavisualizer/imagetolayervisualizer.h>

#include <inviwo/core/common/factoryutil.h>         // for getDataReaderFactory
#include <inviwo/core/io/datareaderfactory.h>       // for DataReaderFactory
#include <inviwo/core/network/processornetwork.h>   // for ProcessorNetwork
#include <inviwo/core/ports/imageport.h>            // for ImageOutport
#include <inviwo/core/ports/outport.h>              // for Outport
#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorutils.h>  // for makeProcessor, GridPos
#include <inviwo/core/rendering/datavisualizer.h>   // for DataVisualizer
#include <inviwo/core/util/document.h>              // for Document, Document::DocumentHandle
#include <modules/base/processors/imagetolayer.h>

namespace inviwo {

using GP = util::GridPos;

ImageToLayerVisualizer::ImageToLayerVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string ImageToLayerVisualizer::getName() const { return "Image To Layer"; }

Document ImageToLayerVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct a image to layer processor");
    return doc;
}

std::vector<FileExtension> ImageToLayerVisualizer::getSupportedFileExtensions() const { return {}; }

bool ImageToLayerVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const ImageOutport*>(port) != nullptr;
}

bool ImageToLayerVisualizer::hasSourceProcessor() const { return false; }
bool ImageToLayerVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> ImageToLayerVisualizer::addSourceProcessor(
    const std::filesystem::path&, ProcessorNetwork*) const {
    return {nullptr, nullptr};
}

std::vector<Processor*> ImageToLayerVisualizer::addVisualizerNetwork(Outport* outport,
                                                                     ProcessorNetwork* net) const {

    auto info = net->addProcessor(util::makeProcessor<ImageToLayer>(GP{0, 3}));
    net->addConnection(outport, info->getInports()[0]);

    return {info};
}

std::vector<Processor*> ImageToLayerVisualizer::addSourceAndVisualizerNetwork(
    const std::filesystem::path& filename, ProcessorNetwork* net) const {

    auto sourceAndOutport = addSourceProcessor(filename, net);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, net);

    processors.push_back(sourceAndOutport.first);

    return processors;
}

}  // namespace inviwo
