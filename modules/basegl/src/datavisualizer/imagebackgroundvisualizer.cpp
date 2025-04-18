/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/basegl/datavisualizer/imagebackgroundvisualizer.h>

#include <inviwo/core/common/inviwoapplication.h>    // for InviwoApplication
#include <inviwo/core/io/datareaderfactory.h>        // for DataReaderFactory
#include <inviwo/core/network/processornetwork.h>    // for ProcessorNetwork
#include <inviwo/core/ports/imageport.h>             // for ImageOutport
#include <inviwo/core/ports/outport.h>               // for Outport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorutils.h>   // for makeProcessor, GridPos
#include <inviwo/core/properties/optionproperty.h>   // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatVec4Property
#include <inviwo/core/rendering/datavisualizer.h>    // for DataVisualizer
#include <inviwo/core/util/document.h>               // for Document, Document::DocumentHandle
#include <inviwo/core/util/fileextension.h>          // for FileExtension
#include <inviwo/core/util/glmvec.h>                 // for vec4
#include <modules/base/processors/imagesource.h>     // for ImageSource
#include <modules/basegl/processors/background.h>    // for Background, Background::BackgroundStyle
#include <modules/opengl/canvasprocessorgl.h>        // for CanvasProcessorGL

#include <map>     // for map
#include <memory>  // for unique_ptr

namespace inviwo {
class Inport;
class Layer;

using GP = util::GridPos;

ImageBackgroundVisualizer::ImageBackgroundVisualizer(InviwoApplication* app)
    : DataVisualizer{}, app_(app) {}

std::string ImageBackgroundVisualizer::getName() const { return "Image Canvas with Background"; }

Document ImageBackgroundVisualizer::getDescription() const {
    Document doc;
    auto b = doc.append("html").append("body");
    b.append("", "Construct an image source, background, and canvas");
    return doc;
}

std::vector<FileExtension> ImageBackgroundVisualizer::getSupportedFileExtensions() const {
    auto rf = app_->getDataReaderFactory();
    auto exts = rf->getExtensionsForType<Layer>();
    return exts;
}

bool ImageBackgroundVisualizer::isOutportSupported(const Outport* port) const {
    return dynamic_cast<const ImageOutport*>(port) != nullptr;
}

bool ImageBackgroundVisualizer::hasSourceProcessor() const { return true; }
bool ImageBackgroundVisualizer::hasVisualizerNetwork() const { return true; }

std::pair<Processor*, Outport*> ImageBackgroundVisualizer::addSourceProcessor(
    const std::filesystem::path& filename, ProcessorNetwork* net, const ivec2& origin) const {

    auto* source =
        net->addProcessor(util::makeProcessor<ImageSource>(GP{0, 0} + origin, app_, filename));
    auto* outport = source->getOutports().front();
    return {source, outport};
}

std::vector<Processor*> ImageBackgroundVisualizer::addVisualizerNetwork(
    Outport* outport, ProcessorNetwork* net) const {
    const ivec2 origin = util::getPosition(outport->getProcessor());

    auto proc = util::makeProcessor<Background>(GP{0, 3} + origin);
    proc->backgroundStyle_.setSelectedValue(Background::BackgroundStyle::Uniform);
    proc->bgColor1_ = vec4{1.0f, 1.0f, 1.0f, 1.0f};
    proc->blendMode_.setSelectedValue(Background::BlendMode::AlphaMixing);
    auto* bg = net->addProcessor(std::move(proc));
    auto* cvs = net->addProcessor(util::makeProcessor<CanvasProcessorGL>(GP{0, 6} + origin));
    net->addConnection(outport, bg->getInports()[0]);
    net->addConnection(bg->getOutports()[0], cvs->getInports()[0]);

    if (auto* canvas = dynamic_cast<CanvasProcessor*>(cvs)) {
        canvas->setCanvasSize(size2_t{768, 768});
    }

    return {bg, cvs};
}

std::vector<Processor*> ImageBackgroundVisualizer::addSourceAndVisualizerNetwork(
    const std::filesystem::path& filename, ProcessorNetwork* net, const ivec2& origin) const {

    auto sourceAndOutport = addSourceProcessor(filename, net, origin);
    auto processors = addVisualizerNetwork(sourceAndOutport.second, net);

    net->addLink(sourceAndOutport.first->getPropertyByIdentifier("imageDimension_"),
                 processors.back()->getPropertyByPath("inputSize.dimensions"));

    net->evaluateLinksFromProperty(
        sourceAndOutport.first->getPropertyByIdentifier("imageDimension_"));

    processors.push_back(sourceAndOutport.first);

    return processors;
}

}  // namespace inviwo
