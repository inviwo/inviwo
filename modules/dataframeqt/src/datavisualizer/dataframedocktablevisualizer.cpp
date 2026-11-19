/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <inviwo/dataframeqt/datavisualizer/dataframedocktablevisualizer.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/rendering/datavisualizer.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/processors/dataframesource.h>
#include <inviwo/dataframeqt/processors/dataframedocktable.h>

#include <map>

namespace inviwo {
class Inport;

using GP = util::GridPos;

DataFrameDockTableVisualizer::DataFrameDockTableVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string DataFrameDockTableVisualizer::getName() const { return "DataFrame Dock Table"; }

Document DataFrameDockTableVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Show a dockable table view for the given DataFrame");
    return doc;
}

std::vector<FileExtension> DataFrameDockTableVisualizer::getSupportedFileExtensions() const {
    auto* rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<DataFrame>();
    return exts;
}

bool DataFrameDockTableVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const DataFrameOutport*>(port) != nullptr;
}

bool DataFrameDockTableVisualizer::hasSourceProcessor() const { return true; }

bool DataFrameDockTableVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> DataFrameDockTableVisualizer::addSourceProcessor(
    const std::filesystem::path& filename, ProcessorNetwork* network, const ivec2& origin) const {

    auto* source = network->addProcessor(
        util::makeProcessor<DataFrameSource>(GP{0, 0} + origin, app_, filename));
    auto* outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> DataFrameDockTableVisualizer::addVisualizerNetwork(
    Outport* outport, ProcessorNetwork* network) const {
    const ivec2 origin = util::getPosition(outport->getProcessor());

    auto* table = network->addProcessor(util::makeProcessor<DataFrameDockTable>(GP{0, 3} + origin));
    network->addConnection(outport, table->getInports()[0]);

    return {table};
}

std::vector<Processor*> DataFrameDockTableVisualizer::addSourceAndVisualizerNetwork(
    const std::filesystem::path& filename, ProcessorNetwork* network, const ivec2& origin) const {

    const auto sourceAndOutport = addSourceProcessor(filename, network, origin);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, network);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
