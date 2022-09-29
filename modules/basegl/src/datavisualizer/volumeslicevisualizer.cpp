/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/basegl/datavisualizer/volumeslicevisualizer.h>

#include <inviwo/core/common/inviwoapplication.h>      // for InviwoApplication
#include <inviwo/core/datastructures/volume/volume.h>  // for VolumeSequence
#include <inviwo/core/io/datareaderfactory.h>          // for DataReaderFactory
#include <inviwo/core/network/processornetwork.h>      // for ProcessorNetwork
#include <inviwo/core/ports/outport.h>                 // for Outport
#include <inviwo/core/ports/volumeport.h>              // for VolumeOutport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorutils.h>     // for makeProcessor, GridPos
#include <inviwo/core/rendering/datavisualizer.h>      // for DataVisualizer
#include <inviwo/core/util/document.h>                 // for Document, Document::DocumentHandle
#include <inviwo/core/util/fileextension.h>            // for FileExtension
#include <modules/base/processors/volumesource.h>      // for VolumeSource
#include <modules/basegl/processors/volumeslicegl.h>   // for VolumeSliceGL
#include <modules/opengl/canvasprocessorgl.h>          // for CanvasProcessorGL

#include <map>                                         // for map

namespace inviwo {
class Inport;

using GP = util::GridPos;

VolumeSliceVisualizer::VolumeSliceVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string VolumeSliceVisualizer::getName() const { return "Volume Slicer"; }

Document VolumeSliceVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct a standard volume slicer");
    return doc;
}

std::vector<FileExtension> VolumeSliceVisualizer::getSupportedFileExtensions() const {
    auto rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<Volume>();
    auto exts2 = rf->getExtensionsForType<VolumeSequence>();
    exts.insert(exts.end(), exts2.begin(), exts2.end());
    return exts;
}

bool VolumeSliceVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const VolumeOutport*>(port) != nullptr;
}

bool VolumeSliceVisualizer::hasSourceProcessor() const { return true; }
bool VolumeSliceVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> VolumeSliceVisualizer::addSourceProcessor(
    const std::string& filename, ProcessorNetwork* net) const {

    auto source = net->addProcessor(util::makeProcessor<VolumeSource>(GP{0, 0}, app_, filename));
    auto outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> VolumeSliceVisualizer::addVisualizerNetwork(Outport* outport,
                                                                    ProcessorNetwork* net) const {
    auto vsl = net->addProcessor(util::makeProcessor<VolumeSliceGL>(GP{0, 3}));
    auto cvs = net->addProcessor(util::makeProcessor<CanvasProcessorGL>(GP{0, 6}));

    net->addConnection(outport, vsl->getInports()[0]);
    net->addConnection(vsl->getOutports()[0], cvs->getInports()[0]);

    return {vsl, cvs};
}

std::vector<Processor*> VolumeSliceVisualizer::addSourceAndVisualizerNetwork(
    const std::string& filename, ProcessorNetwork* net) const {

    auto sourceAndOutport = addSourceProcessor(filename, net);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, net);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
