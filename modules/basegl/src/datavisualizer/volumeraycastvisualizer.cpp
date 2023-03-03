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

#include <modules/basegl/datavisualizer/volumeraycastvisualizer.h>

#include <inviwo/core/common/factoryutil.h>                      // for getDataReaderFactory
#include <inviwo/core/datastructures/light/lightingstate.h>      // for ShadingMode, ShadingMode...
#include <inviwo/core/datastructures/volume/volume.h>            // for VolumeSequence
#include <inviwo/core/io/datareaderfactory.h>                    // for DataReaderFactory
#include <inviwo/core/network/processornetwork.h>                // for ProcessorNetwork
#include <inviwo/core/ports/outport.h>                           // for Outport
#include <inviwo/core/ports/volumeport.h>                        // for VolumeOutport
#include <inviwo/core/processors/processor.h>                    // for Processor
#include <inviwo/core/processors/processorutils.h>               // for makeProcessor, trySetPro...
#include <inviwo/core/properties/optionproperty.h>               // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>              // for FloatVec4Property, Float...
#include <inviwo/core/properties/ordinalrefproperty.h>           // for FloatVec3RefProperty
#include <inviwo/core/rendering/datavisualizer.h>                // for DataVisualizer
#include <inviwo/core/util/document.h>                           // for Document, Document::Docu...
#include <inviwo/core/util/fileextension.h>                      // for FileExtension
#include <inviwo/core/util/glmvec.h>                             // for vec4, vec3
#include <modules/base/processors/cubeproxygeometryprocessor.h>  // for CubeProxyGeometry
#include <modules/base/processors/volumeboundingbox.h>           // for VolumeBoundingBox
#include <modules/base/processors/volumesource.h>                // for VolumeSource
#include <modules/basegl/processors/background.h>                // for Background
#include <modules/basegl/processors/entryexitpointsprocessor.h>  // for EntryExitPoints
#include <modules/basegl/processors/linerendererprocessor.h>     // for LineRendererProcessor
#include <modules/basegl/processors/volumeraycaster.h>           // for VolumeRaycaster
#include <modules/opengl/canvasprocessorgl.h>                    // for CanvasProcessorGL

#include <map>  // for map

namespace inviwo {
class Inport;
class InviwoApplication;

using GP = util::GridPos;

VolumeRaycastVisualizer::VolumeRaycastVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string VolumeRaycastVisualizer::getName() const { return "Volume Raycaster"; }

Document VolumeRaycastVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct a standard volume raycaster");
    return doc;
}

std::vector<FileExtension> VolumeRaycastVisualizer::getSupportedFileExtensions() const {
    auto rf = util::getDataReaderFactory(app_);
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

    auto source = net->addProcessor(util::makeProcessor<VolumeSource>(GP{0, 0}, app_, filename));
    auto outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> VolumeRaycastVisualizer::addVisualizerNetwork(Outport* outport,
                                                                      ProcessorNetwork* net) const {

    auto cpg = net->addProcessor(util::makeProcessor<CubeProxyGeometry>(GP{1, 3}));
    auto eep = net->addProcessor(util::makeProcessor<EntryExitPoints>(GP{1, 6}));
    auto vrc = net->addProcessor(util::makeProcessor<VolumeRaycaster>(GP{0, 9}));
    auto bak = net->addProcessor(util::makeProcessor<Background>(GP{0, 12}));
    auto cvs = net->addProcessor(util::makeProcessor<CanvasProcessorGL>(GP{0, 15}));

    auto vbb = net->addProcessor(util::makeProcessor<VolumeBoundingBox>(GP{8, 3}));
    auto lrp = net->addProcessor(util::makeProcessor<LineRendererProcessor>(GP{8, 6}));

    util::trySetProperty<FloatVec4Property>(bak, "bgColor1", vec4(0.443f, 0.482f, 0.600f, 1.0f));
    util::trySetProperty<FloatVec4Property>(bak, "bgColor2", vec4(0.831f, 0.831f, 0.831f, 1.0f));

    util::trySetProperty<OptionProperty<ShadingMode>>(vrc, "shadingMode", ShadingMode::None, true);
    util::trySetProperty<FloatVec3RefProperty>(vrc, "lookFrom", vec3(0.0f, 0.0f, 30.0f), true);
    util::trySetProperty<FloatProperty>(lrp, "lineSettings.lineWidth", 1.5f);

    net->addConnection(outport, cpg->getInports()[0]);
    net->addConnection(cpg->getOutports()[0], eep->getInports()[0]);

    net->addConnection(eep->getOutports()[0], vrc->getInports()[1]);
    net->addConnection(eep->getOutports()[1], vrc->getInports()[2]);

    net->addConnection(outport, vrc->getInports()[0]);
    net->addConnection(vrc->getOutports()[0], bak->getInports()[0]);
    net->addConnection(bak->getOutports()[0], cvs->getInports()[0]);

    net->addConnection(outport, vbb->getInports()[0]);
    net->addConnection(vbb->getOutports()[0], lrp->getInports()[0]);
    net->addConnection(lrp->getOutports()[0], vrc->getInports()[3]);

    net->addLink(eep->getPropertyByIdentifier("camera"), vrc->getPropertyByIdentifier("camera"));
    net->addLink(vrc->getPropertyByIdentifier("camera"), eep->getPropertyByIdentifier("camera"));

    net->addLink(eep->getPropertyByIdentifier("camera"), lrp->getPropertyByIdentifier("camera"));
    net->addLink(lrp->getPropertyByIdentifier("camera"), eep->getPropertyByIdentifier("camera"));

    net->evaluateLinksFromProperty(vrc->getPropertyByIdentifier("camera"));

    return {cpg, eep, vrc, cvs, vbb, bak, lrp};
}

std::vector<Processor*> VolumeRaycastVisualizer::addSourceAndVisualizerNetwork(
    const std::string& filename, ProcessorNetwork* net) const {

    auto sourceAndOutport = addSourceProcessor(filename, net);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, net);

    processors.push_back(sourceAndOutport.first);
    return processors;
}

}  // namespace inviwo
