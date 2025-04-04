/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>         // for operator""_unindentHelp
#include <inviwo/core/processors/processorinfo.h>   // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>  // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>   // for Tags, Tags::GL

#include <memory>       // for shared_ptr, shared_ptr<>::element_type
#include <string>       // for string
#include <type_traits>  // for remove_extent_t

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
const ProcessorInfo& CanvasProcessorGL::getProcessorInfo() const { return processorInfo_; }

CanvasProcessorGL::CanvasProcessorGL(InviwoApplication* app) : CanvasProcessor(app) {}

}  // namespace inviwo
