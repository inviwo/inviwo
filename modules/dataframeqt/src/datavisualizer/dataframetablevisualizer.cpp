/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/dataframeqt/datavisualizer/dataframetablevisualizer.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/io/datareaderfactory.h>

#include <inviwo/dataframe/processors/csvsource.h>
#include <inviwo/dataframeqt/processors/dataframetable.h>

namespace inviwo {

using GP = util::GridPos;

DataFrameTableVisualizer::DataFrameTableVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string DataFrameTableVisualizer::getName() const { return "DataFrame Table"; }

Document DataFrameTableVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Show a table view for the given DataFrame");
    return doc;
}

std::vector<FileExtension> DataFrameTableVisualizer::getSupportedFileExtensions() const {
    auto rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<DataFrame>();
    return exts;
}

bool DataFrameTableVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const DataFrameOutport*>(port) != nullptr;
}

bool DataFrameTableVisualizer::hasSourceProcessor() const { return true; }

bool DataFrameTableVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> DataFrameTableVisualizer::addSourceProcessor(
    const std::string& filename, ProcessorNetwork* network) const {

    auto source = network->addProcessor(util::makeProcessor<CSVSource>(GP{0, 0}, filename));
    auto outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> DataFrameTableVisualizer::addVisualizerNetwork(
    Outport* outport, ProcessorNetwork* network) const {

    auto table = network->addProcessor(util::makeProcessor<DataFrameTable>(GP{0, 3}));
    network->addConnection(outport, table->getInports()[0]);

    return {table};
}

std::vector<Processor*> DataFrameTableVisualizer::addSourceAndVisualizerNetwork(
    const std::string& filename, ProcessorNetwork* network) const {

    auto sourceAndOutport = addSourceProcessor(filename, network);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, network);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
