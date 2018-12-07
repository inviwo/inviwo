/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/basegl/processors/coloroffset.h>

#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

const ProcessorInfo ColorOffset::processorInfo_{
    "org.inviwo.ColorOffset",  // Class identifier
    "Color Offset",            // Display name
    "Image Processing",        // Category
    CodeState::Experimental,   // Code state
    Tags::CPU,                 // Tags
};

const ProcessorInfo ColorOffset::getProcessorInfo() const { return processorInfo_; }

ColorOffset::ColorOffset() : Processor()
    , mesh_inport_("mesh_in")
    , mesh_outport_("mesh_out")
    , offset_("offset", "Offset", vec4(0.0f), vec4(-1.0f), vec4(1.0f), vec4(0.001f))
    , clamp_color_("clamp_color", "Clamp Color", true)
{
    addPort(mesh_inport_);
    addPort(mesh_outport_);

    addProperty(offset_);
    addProperty(clamp_color_);
}

void ColorOffset::process() {
    // TODO: replace clone, buffer IDs also get copied, which should not happen
    auto mesh_out = mesh_inport_.getData()->clone();

    if (mesh_out) {
        auto color_buffer = reinterpret_cast<Vec4BufferRAM*>(
            mesh_out->getBuffer(static_cast<size_t>(BufferType::ColorAttrib))
            ->getEditableRepresentation<BufferRAM>());

        if (color_buffer) {
            for (size_t idx = 0; idx < color_buffer->getSize(); ++idx) {
                auto& color = color_buffer->get(idx);
                const auto new_color = color + offset_.get();
                color =
                    clamp_color_.get() ? glm::clamp(new_color, vec4(0.0f), vec4(1.0f)) : new_color;
            }
        }
    }

    mesh_outport_.setData(mesh_out);
}

}  // namespace inviwo
