/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/processors/meshcolorfromnormals.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshColorFromNormals::processorInfo_{
    "org.inviwo.MeshColorFromNormals",  // Class identifier
    "Mesh Color From Normals",          // Display name
    "Mesh Processing",                  // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};
const ProcessorInfo MeshColorFromNormals::getProcessorInfo() const { return processorInfo_; }

MeshColorFromNormals::MeshColorFromNormals() : Processor(), inport_("inport"), outport_("outport") {
    addPort(inport_);
    addPort(outport_);
}

void MeshColorFromNormals::process() {
    auto inMesh = inport_.getData();

    auto mesh = std::shared_ptr<Mesh>(inMesh->clone());
    while (auto cbuf = mesh->getBuffer(BufferType::ColorAttrib)) {
        mesh->removeBuffer(cbuf);
    }

    if (auto normalsBuffer = mesh->getBuffer(BufferType::NormalAttrib)) {
        mesh->addBuffer(Mesh::BufferInfo{BufferType::ColorAttrib},
                        std::shared_ptr<BufferBase>(normalsBuffer->clone()));
    } else {
        throw Exception("Input mesh has no normals", IVW_CONTEXT);
    }

    outport_.setData(mesh);
}

}  // namespace inviwo
