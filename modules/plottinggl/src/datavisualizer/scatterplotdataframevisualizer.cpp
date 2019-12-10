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

#include <modules/plottinggl/datavisualizer/scatterplotdataframevisualizer.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/io/datareaderfactory.h>

#include <inviwo/dataframe/processors/csvsource.h>
#include <modules/basegl/processors/background.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <modules/plottinggl/processors/scatterplotprocessor.h>

namespace inviwo {

using GP = util::GridPos;

ScatterPlotDataFrameVisualizer::ScatterPlotDataFrameVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string ScatterPlotDataFrameVisualizer::getName() const { return "Scatter Plot"; }

Document ScatterPlotDataFrameVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct a Scatter Plot from the given DataFrame");
    return doc;
}

std::vector<FileExtension> ScatterPlotDataFrameVisualizer::getSupportedFileExtensions() const {
    auto rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<DataFrame>();
    return exts;
}

bool ScatterPlotDataFrameVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const DataFrameOutport*>(port) != nullptr;
}

bool ScatterPlotDataFrameVisualizer::hasSourceProcessor() const { return true; }

bool ScatterPlotDataFrameVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> ScatterPlotDataFrameVisualizer::addSourceProcessor(
    const std::string& filename, ProcessorNetwork* network) const {

    auto source = network->addProcessor(util::makeProcessor<CSVSource>(GP{0, 0}, filename));
    auto outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> ScatterPlotDataFrameVisualizer::addVisualizerNetwork(
    Outport* outport, ProcessorNetwork* network) const {

    auto scatter = network->addProcessor(util::makeProcessor<plot::ScatterPlotProcessor>(GP{0, 3}));

    auto background = util::makeProcessor<Background>(GP{0, 6});
    background->backgroundStyle_.setSelectedValue(Background::BackgroundStyle::Uniform);
    background->bgColor1_ = vec4{1.0f, 1.0f, 1.0f, 1.0f};
    auto bg = network->addProcessor(std::move(background));

    auto canvas = util::makeProcessor<CanvasProcessorGL>(GP{0, 9});
    canvas->dimensions_ = ivec2{800, 400};
    auto cvs = network->addProcessor(std::move(canvas));

    network->addConnection(scatter->getOutports()[0], bg->getInports()[0]);
    network->addConnection(bg->getOutports()[0], cvs->getInports()[0]);
    network->addConnection(outport, scatter->getInports()[0]);

    return {scatter, bg, cvs};
}

std::vector<Processor*> ScatterPlotDataFrameVisualizer::addSourceAndVisualizerNetwork(
    const std::string& filename, ProcessorNetwork* network) const {

    auto sourceAndOutport = addSourceProcessor(filename, network);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, network);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
