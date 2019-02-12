/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/base/processors/meshconverterprocessor.h>
#include <modules/base/algorithm/mesh/meshconverter.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshConverterProcessor::processorInfo_{
    "org.inviwo.MeshConverter",  // Class identifier
    "Mesh Converter",            // Display name
    "Mesh Operation",            // Category
    CodeState::Stable,           // Code state
    Tags::CPU,                   // Tags
};
const ProcessorInfo MeshConverterProcessor::getProcessorInfo() const { return processorInfo_; }

MeshConverterProcessor::MeshConverterProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , type_{"type",
            "Type",
            {{"lines", "To Lines", Type::ToLines}, {"points", "To Points", Type::ToPoints}},
            0} {

    addPort(inport_);
    addPort(outport_);
    addProperty(type_);
}

void MeshConverterProcessor::process() {
    auto meshes = std::make_shared<std::vector<std::shared_ptr<Mesh>>>();
    for (auto mesh : inport_) {
        if (mesh) {
            switch (type_.get()) {
                case Type::ToLines: {
                    if (auto lines = meshutil::toLineMesh(*mesh)) {
                        meshes->emplace_back(std::move(lines));
                    }
                    break;
                }
                case Type::ToPoints: {
                    if (auto points = meshutil::toPointMesh(*mesh)) {
                        meshes->emplace_back(std::move(points));
                    }
                    break;
                }
            }
        }
    }
    if (!meshes->empty()) {
        outport_.setData(meshes);
    } else {
        outport_.setData(nullptr);
    }
}

}  // namespace inviwo
