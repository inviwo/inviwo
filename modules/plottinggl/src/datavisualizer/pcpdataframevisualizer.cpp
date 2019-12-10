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

#include <modules/plottinggl/datavisualizer/pcpdataframevisualizer.h>

#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/dataframe/processors/csvsource.h>
#include <modules/basegl/processors/background.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <inviwo/core/io/datareaderfactory.h>

namespace inviwo {

using GP = util::GridPos;

PCPDataFrameVisualizer::PCPDataFrameVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string PCPDataFrameVisualizer::getName() const { return "Parallel Coordinates Plot"; }

Document PCPDataFrameVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct Parallel Coordinates Plot from the given DataFrame");
    return doc;
}

std::vector<FileExtension> PCPDataFrameVisualizer::getSupportedFileExtensions() const {
    auto rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<DataFrame>();
    return exts;
}

bool PCPDataFrameVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const DataFrameOutport*>(port) != nullptr;
}

bool PCPDataFrameVisualizer::hasSourceProcessor() const { return true; }

bool PCPDataFrameVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> PCPDataFrameVisualizer::addSourceProcessor(
    const std::string& filename, ProcessorNetwork* network) const {

    auto source = network->addProcessor(util::makeProcessor<CSVSource>(GP{0, 0}, filename));
    auto outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> PCPDataFrameVisualizer::addVisualizerNetwork(
    Outport* outport, ProcessorNetwork* network) const {

    auto pcp = network->addProcessor(util::makeProcessor<plot::ParallelCoordinates>(GP{0, 3}));

    auto back = util::makeProcessor<Background>(GP{0, 6});
    back->backgroundStyle_.setSelectedValue(Background::BackgroundStyle::Uniform);
    back->bgColor1_ = vec4{1.0f, 1.0f, 1.0f, 1.0f};
    auto bak = network->addProcessor(std::move(back));

    auto canvas = util::makeProcessor<CanvasProcessorGL>(GP{0, 9});
    canvas->dimensions_ = ivec2{800, 400};
    auto cvs = network->addProcessor(std::move(canvas));

    network->addConnection(pcp->getOutports()[0], bak->getInports()[0]);
    network->addConnection(bak->getOutports()[0], cvs->getInports()[0]);
    network->addConnection(outport, pcp->getInports()[0]);

    return {pcp, bak, cvs};
}

std::vector<Processor*> PCPDataFrameVisualizer::addSourceAndVisualizerNetwork(
    const std::string& filename, ProcessorNetwork* network) const {

    auto sourceAndOutport = addSourceProcessor(filename, network);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, network);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
