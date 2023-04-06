/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <modules/basegl/datavisualizer/meshvisualizer.h>

#include <inviwo/core/common/inviwoapplication.h>             // for InviwoApplication
#include <inviwo/core/io/datareaderfactory.h>                 // for DataReaderFactory
#include <inviwo/core/network/processornetwork.h>             // for ProcessorNetwork
#include <inviwo/core/ports/meshport.h>                       // for MeshOutport
#include <inviwo/core/ports/outport.h>                        // for Outport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorutils.h>            // for makeProcessor, GridPos
#include <inviwo/core/rendering/datavisualizer.h>             // for DataVisualizer
#include <inviwo/core/util/document.h>                        // for Document, Document::Documen...
#include <inviwo/core/util/fileextension.h>                   // for FileExtension
#include <modules/base/processors/meshsource.h>               // for MeshSource
#include <modules/basegl/processors/background.h>             // for Background
#include <modules/basegl/processors/meshrenderprocessorgl.h>  // for MeshRenderProcessorGL
#include <modules/opengl/canvasprocessorgl.h>                 // for CanvasProcessorGL

#include <map>  // for map

namespace inviwo {
class Inport;
class Mesh;

using GP = util::GridPos;

MeshVisualizer::MeshVisualizer(InviwoApplication* app) : DataVisualizer{}, app_(app) {}

std::string MeshVisualizer::getName() const { return "Mesh Renderer"; }

Document MeshVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct a standard mesh renderer");
    return doc;
}

std::vector<FileExtension> MeshVisualizer::getSupportedFileExtensions() const {
    auto rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<Mesh>();
    return exts;
}

bool MeshVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const MeshOutport*>(port) != nullptr;
}

bool MeshVisualizer::hasSourceProcessor() const { return true; }
bool MeshVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> MeshVisualizer::addSourceProcessor(
    const std::filesystem::path& filename, ProcessorNetwork* net) const {

    auto source = net->addProcessor(util::makeProcessor<MeshSource>(GP{0, 0}, app_, filename));
    auto outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> MeshVisualizer::addVisualizerNetwork(Outport* outport,
                                                             ProcessorNetwork* net) const {

    auto bak = net->addProcessor(util::makeProcessor<Background>(GP{1, 3}));
    auto mrp = net->addProcessor(util::makeProcessor<MeshRenderProcessorGL>(GP{0, 6}));
    auto cvs = net->addProcessor(util::makeProcessor<CanvasProcessorGL>(GP{0, 9}));

    net->addConnection(bak->getOutports()[0], mrp->getInports()[1]);
    net->addConnection(mrp->getOutports()[0], cvs->getInports()[0]);

    net->addConnection(outport, mrp->getInports()[0]);

    return {bak, mrp, cvs};
}

std::vector<Processor*> MeshVisualizer::addSourceAndVisualizerNetwork(
    const std::filesystem::path& filename, ProcessorNetwork* net) const {

    auto sourceAndOutport = addSourceProcessor(filename, net);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, net);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
