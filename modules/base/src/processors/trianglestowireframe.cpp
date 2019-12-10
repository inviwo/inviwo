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

#include <modules/base/processors/trianglestowireframe.h>
#include <modules/base/algorithm/meshutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TrianglesToWireframe::processorInfo_{
    "org.inviwo.TrianglesToWireframe",  // Class identifier
    "Triangles To Wireframe",           // Display name
    "Mesh Operation",                   // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo TrianglesToWireframe::getProcessorInfo() const { return processorInfo_; }

TrianglesToWireframe::TrianglesToWireframe() : Processor() {
    addPort(mesh_);
    addPort(wireframe_);
}

void TrianglesToWireframe::process() {
    auto mesh = mesh_.getData();
    auto wireframe = std::make_shared<Mesh>();
    wireframe->setModelMatrix(mesh->getModelMatrix());
    wireframe->setWorldMatrix(mesh->getWorldMatrix());

    for (auto &buf : mesh->getBuffers()) {
        wireframe->addBuffer(buf.first, std::shared_ptr<BufferBase>(buf.second->clone()));
    }

    auto indicesRam = std::make_shared<IndexBufferRAM>();
    auto &vec = indicesRam->getDataContainer();
    wireframe->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None),
                          std::make_shared<IndexBuffer>(indicesRam));

    for (auto &ib : mesh->getIndexBuffers()) {
        if (ib.first.dt == DrawType::Triangles) {
            meshutil::forEachTriangle(ib.first, *ib.second,
                                      [&vec](uint32_t i0, uint32_t i1, uint32_t i2) {
                                          vec.push_back(i0);
                                          vec.push_back(i1);

                                          vec.push_back(i1);
                                          vec.push_back(i2);

                                          vec.push_back(i2);
                                          vec.push_back(i0);
                                      });
        } else if (ib.first.dt == DrawType::Lines) {
            meshutil::forEachLineSegment(ib.first, *ib.second, [&vec](uint32_t i0, uint32_t i1) {
                vec.push_back(i0);
                vec.push_back(i1);
            });
        }
    }

    wireframe_.setData(wireframe);
}

}  // namespace inviwo
