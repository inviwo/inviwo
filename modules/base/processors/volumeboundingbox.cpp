/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "volumeboundingbox.h"
#include <inviwo/core/datastructures/geometry/basicmesh.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeBoundingBox::processorInfo_{
    "org.inviwo.VolumeBoundingBox",      // Class identifier
    "Volume Bounding Box",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo VolumeBoundingBox::getProcessorInfo() const {
    return processorInfo_;
}

VolumeBoundingBox::VolumeBoundingBox()
    : Processor()
    , volume_("volume")
    , mesh_("mesh") 
    , color_("color","Color", vec4(1.0f), vec4(0.0f), vec4(1.0f))

    , showFrontFace_("showFrontFace", "Show Front Face", false)
    , showBackFace_("showBackFace", "Show Back Face", false)
    , showRightFace_("showRightFace", "Show Right Face", false)
    , showLeftFace_("showLeftFace", "Show Left Face", false)
    , showTopFace_("showTopFace", "Show Top Face", false)
    , showBottomFace_("showBottomFace", "Show Bottom Face", false)
{
    
    addPort(volume_);
    addPort(mesh_);

    color_.setSemantics(PropertySemantics::Color);
    addProperty(color_);


    addProperty(showFrontFace_);
    addProperty(showBackFace_);
    addProperty(showRightFace_);
    addProperty(showLeftFace_);
    addProperty(showTopFace_);
    addProperty(showBottomFace_);
}
    
void VolumeBoundingBox::process() {
    auto mesh = BasicMesh::boundingBoxAdjacency(volume_.getData()->getModelMatrix(), color_);
    
    auto a = showBackFace_.get();
    auto b = showFrontFace_.get();
    auto c = showRightFace_.get();
    auto d = showLeftFace_.get();
    auto e = showTopFace_.get();
    auto f = showBottomFace_.get();

    if (a || b || c || d || e || f) {
        auto ib = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
        if (a) {  // back face
            ib->add({4, 1, 2, 2, 1, 0});
        }
        if (b) {  // front face
            ib->add({5, 6, 7, 5, 3, 6});
        }
        if (c) {  // right face
            ib->add({4, 6, 1, 7, 6, 4});
        }
        if (d) {  // left face
            ib->add({ 3, 2, 0, 5,2,3 });
        }
        if (e) {  // top face
            ib->add({5, 4, 2, 4, 5, 7});
        }
        if (f) {  // bottom face
            ib->add({0, 1, 3, 6, 3, 1});
        }
    }

    

    mesh_.setData(mesh);
    
}

} // namespace

