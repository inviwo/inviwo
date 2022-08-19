/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/opengl/canvasprocessorgl.h>
#include <modules/opengl/canvasgl.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/util/rendercontext.h>

namespace inviwo {

const ProcessorInfo CanvasProcessorGL::processorInfo_{
    "org.inviwo.CanvasGL",  // Class identifier
    "Canvas",               // Display name
    "Data Output",          // Category
    CodeState::Stable,      // Code state
    Tags::GL,               // Tags
    R"(The canvas processor has an associated processor widget window to render an
       image. The processor tracks the window size and position, and can also render
       different image layers. By default the first color layer is rendered.
       The processor also has functionality to save images to disk.)"_unindentHelp  // Help
};
const ProcessorInfo CanvasProcessorGL::getProcessorInfo() const { return processorInfo_; }

CanvasProcessorGL::CanvasProcessorGL(InviwoApplication* app) : CanvasProcessor(app) {}

void CanvasProcessorGL::process() {
    // ensure that the image inport has a GL representation
    // otherwise the canvas widget will request a new one which in turn creates an fbo within the
    // context of the canvas, i.e. not the default canvas.
    //
    // FIXME: this is not ideal
    if (!inport_.getData()->hasRepresentation<ImageGL>()) {
        RenderContext::getPtr()->activateDefaultRenderContext();
        inport_.getData()->getRepresentation<ImageGL>();
    }
    CanvasProcessor::process();
}

}  // namespace inviwo
