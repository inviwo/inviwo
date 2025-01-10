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

#include <modules/basegl/processors/firstivwprocessor.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, makeBuffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/imageport.h>                                // for ImageOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatVec3Property
#include <inviwo/core/util/glmvec.h>                                    // for vec2, vec3, vec4
#include <modules/opengl/buffer/buffergl.h>                             // for BufferGL
#include <modules/opengl/inviwoopengl.h>                                // for glDrawArrays, GL_...
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/texture/textureutils.h>                        // for activateAndClearT...

#include <functional>     // for __base
#include <sstream>        // for basic_stringbuf<>...
#include <string>         // for operator==, string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for move

#include <glm/vec2.hpp>  // for vec
#include <glm/vec3.hpp>  // for vec, vec<>::(anon...

namespace inviwo {

const ProcessorInfo FirstIvwProcessor::processorInfo_{
    "org.inviwo.FirstIVWProcessor",  // Class identifier
    "First Processor",               // Display name
    "Various",                       // Category
    CodeState::Stable,               // Code state
    Tags::GL,                        // Tags
};
const ProcessorInfo& FirstIvwProcessor::getProcessorInfo() const { return processorInfo_; }

FirstIvwProcessor::FirstIvwProcessor()
    : Processor()
    , color_("color", "Color", vec3(1.0f), vec3(0.0f), vec3(1.0f), vec3(0.1f))
    , outport_("outport")
    , shader_("minimal.vert", "img_color.frag") {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addProperty(color_);
    addPort(outport_);

    quad_ = util::makeBuffer<vec2>({{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}});

    triangle_ = util::makeBuffer<vec2>({{0.0f, 1.0f}, {-1.0f, -1.0f}, {1.0f, -1.0f}});

    quadGL_ = quad_->getRepresentation<BufferGL>();
    triangleGL_ = triangle_->getRepresentation<BufferGL>();
}

void FirstIvwProcessor::process() {
    utilgl::activateAndClearTarget(outport_, ImageType::ColorOnly);
    shader_.activate();

    // Render Quad
    shader_.setUniform("color", vec4(color_.get().x, color_.get().y, color_.get().z, 1.f));
    quadGL_->enable();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    quadGL_->disable();

    // Render Triangle
    shader_.setUniform("color", vec4(0.f, 1.f, 0.f, 1.f));
    triangleGL_->enable();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    triangleGL_->disable();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
