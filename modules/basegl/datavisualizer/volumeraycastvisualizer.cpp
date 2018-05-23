/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/basegl/datavisualizer/volumeraycastvisualizer.h>
#include <modules/base/processors/volumesource.h>
#include <modules/base/processors/cubeproxygeometryprocessor.h>
#include <modules/base/processors/volumeboundingbox.h>
#include <modules/basegl/processors/volumeraycaster.h>
#include <modules/basegl/processors/entryexitpointsprocessor.h>
#include <modules/basegl/processors/background.h>
#include <modules/basegl/processors/meshrenderprocessorgl.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/ports/volumeport.h>

#include <inviwo/core/io/datareaderfactory.h>

namespace inviwo {

using GP = util::GridPos;

VolumeRaycastVisualizer::VolumeRaycastVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string VolumeRaycastVisualizer::getName() const { return "Volume Raycaster"; }

Document VolumeRaycastVisualizer::getDescription() const {
    Document doc;
    using P = Document::PathComponent;
    auto b = doc.append("html").append("body");
    b.append("", "Construct a standard volume raycaster");
    return doc;
}

std::vector<FileExtension> VolumeRaycastVisualizer::getSupportedFileExtensions() const {
    auto rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<Volume>();
    auto exts2 = rf->getExtensionsForType<VolumeSequence>();
    exts.insert(exts.end(), exts2.begin(), exts2.end());
    return exts;
}

bool VolumeRaycastVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const VolumeOutport*>(port) != nullptr;
}

bool VolumeRaycastVisualizer::hasSourceProcessor() const { return true; }
bool VolumeRaycastVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> VolumeRaycastVisualizer::addSourceProcessor(
    const std::string& filename, ProcessorNetwork* net) const {

    auto source =
        net->addProcessor(util::makeProcessor<VolumeSource>(GP{0, 0}, app_, filename));
    auto outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> VolumeRaycastVisualizer::addVisualizerNetwork(Outport* outport,
                                                                      ProcessorNetwork* net) const {

    auto cpg = net->addProcessor(util::makeProcessor<CubeProxyGeometry>(GP{1, 3}));
    auto eep = net->addProcessor(util::makeProcessor<EntryExitPoints>(GP{1, 6}));
    auto vrc = net->addProcessor(util::makeProcessor<VolumeRaycaster>(GP{0, 9}));
    auto cvs = net->addProcessor(util::makeProcessor<CanvasProcessorGL>(GP{0, 12}));

    auto vbb = net->addProcessor(util::makeProcessor<VolumeBoundingBox>(GP{8, 3}));
    auto bak = net->addProcessor(util::makeProcessor<Background>(GP{15, 3}));
    auto mrp = net->addProcessor(util::makeProcessor<MeshRenderProcessorGL>(GP{8, 6}));

    net->addConnection(outport, cpg->getInports()[0]);
    net->addConnection(cpg->getOutports()[0], eep->getInports()[0]);

    net->addConnection(eep->getOutports()[0], vrc->getInports()[1]);
    net->addConnection(eep->getOutports()[1], vrc->getInports()[2]);

    net->addConnection(outport, vrc->getInports()[0]);
    net->addConnection(vrc->getOutports()[0], cvs->getInports()[0]);

    net->addConnection(outport, vbb->getInports()[0]);
    net->addConnection(vbb->getOutports()[0], mrp->getInports()[0]);
    net->addConnection(bak->getOutports()[0], mrp->getInports()[1]);
    net->addConnection(mrp->getOutports()[0], vrc->getInports()[3]);

    net->addLink(eep->getPropertyByIdentifier("camera"), vrc->getPropertyByIdentifier("camera"));
    net->addLink(vrc->getPropertyByIdentifier("camera"), eep->getPropertyByIdentifier("camera"));

    net->addLink(eep->getPropertyByIdentifier("camera"), mrp->getPropertyByIdentifier("camera"));
    net->addLink(mrp->getPropertyByIdentifier("camera"), eep->getPropertyByIdentifier("camera"));

    return {cpg, eep, vrc, cvs, vbb, bak, mrp};
}

std::vector<Processor*> VolumeRaycastVisualizer::addSourceAndVisualizerNetwork(
    const std::string& filename, ProcessorNetwork* net) const {

    auto sourceAndOutport = addSourceProcessor(filename, net);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, net);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
