/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/basegl/processors/linerendererprocessor.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/inportiterable.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <modules/basegl/properties/linesettingsproperty.h>
#include <modules/basegl/rendering/linerenderer.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>

#include <memory>
#include <string>
#include <string_view>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LineRendererProcessor::processorInfo_{
    "org.inviwo.LineRenderer",  // Class identifier
    "Line Renderer",            // Display name
    "Mesh Rendering",           // Category
    CodeState::Stable,          // Code state
    Tags::GL,                   // Tags
    R"(Render input meshes as 2D lines using OpenGL.)"_unindentHelp,
};
const ProcessorInfo& LineRendererProcessor::getProcessorInfo() const { return processorInfo_; }

LineRendererProcessor::LineRendererProcessor()
    : Processor()
    , inport_{"geometry", "Input meshes"_help}
    , imageInport_{"imageInport", "Optional background image"_help}
    , outport_{"image"}
    , lineSettings_{"lineSettings", "Line Settings"}
    , writeDepth_{"writeDepth", "Write Depth Layer",
                  "If enabled, line depths are rendered onto the background image"_help, true}
    , camera_{"camera", "Camera", util::boundingBox(inport_)}
    , trackball_{&camera_}
    , bnl_{}
    , lineRenderer_{{bnl_.getRequirement()}} {

    addPort(inport_);
    addPort(imageInport_).setOptional(true);
    addPort(bnl_.inport);
    addPort(outport_);

    addProperties(lineSettings_, writeDepth_, bnl_.highlight, bnl_.select, bnl_.filter, camera_,
                  trackball_);
}

void LineRendererProcessor::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);

    const utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const utilgl::DepthMaskState depthMask(writeDepth_.get());
    const utilgl::DepthFuncState depthFunc(GL_LEQUAL);

    bnl_.update();
    TextureUnitContainer cont;
    utilgl::bind(cont, bnl_);

    drawMeshes();

    utilgl::deactivateCurrentTarget();
}

void LineRendererProcessor::drawMeshes() {
    for (const auto& mesh : inport_) {
        lineRenderer_.renderWithUniforms(*mesh, camera_.get(), outport_.getDimensions(),
                                         lineSettings_, bnl_);
    }
}

}  // namespace inviwo
