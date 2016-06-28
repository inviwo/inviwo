/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/integrallinevectortomesh.h>
#include <modules/vectorfieldvisualization/processors/3d/pathlines.h>
#include <modules/vectorfieldvisualization/processors/3d/streamlines.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo IntegralLineVectorToMesh::processorInfo_{
    "org.inviwo.IntegralLineVectorToMesh",      // Class identifier
    "Integral Line Vector To Mesh",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo IntegralLineVectorToMesh::getProcessorInfo() const {
    return processorInfo_;
}

IntegralLineVectorToMesh::IntegralLineVectorToMesh()
    : Processor()
    , lines_("lines")
    , mesh_("mesh")
    , ignoreBrushingList_("ignoreBrushingList", "Ignore Brushing List", false)
    , brushingList_("brushingList", "BrushingList")

    //    , coloringMethod_("coloringMethod", "Color by")
    , tf_("transferFunction", "Transfer Function")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)
    , stride_("stride","Vertex stride",1,1,10)

{
    
    addPort(lines_);
    addPort(mesh_);

    addProperty(ignoreBrushingList_);
    addProperty(brushingList_);

    addProperty(tf_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);

    addProperty(stride_);


    tf_.autoLinkToProperty<PathLines>("transferFunction");
    tf_.autoLinkToProperty<StreamLines>("transferFunction");

    tf_.get().clearPoints();
    tf_.get().addPoint(vec2(0, 1), vec4(0, 0, 1, 1));
    tf_.get().addPoint(vec2(0.5, 1), vec4(1, 1, 0, 1));
    tf_.get().addPoint(vec2(1, 1), vec4(1, 0, 0, 1));

    setAllPropertiesCurrentStateAsDefault();
    


}
    
void IntegralLineVectorToMesh::process() {
    auto mesh = std::make_shared<BasicMesh>();


    mesh->setModelMatrix(lines_.getData()->getModelMatrix());

    std::vector<BasicMesh::Vertex> vertices;
    float maxVelocity = 0;

    vertices.reserve(lines_.getData()->size()*2000);

    for (auto &line : (*lines_.getData())) {
        auto size = line.getPositions().size();
        if (size == 0) continue;

        if (!ignoreBrushingList_.get() && brushingList_.isBrushed(line.getIndex())) {
            continue;
        }


        auto position = line.getPositions().begin();
        auto velocity = line.getMetaData("velocity").begin();

        auto indexBuffer =
            mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);

        indexBuffer->getDataContainer()->reserve(size + 2);

        vec4 c(1, 1, 1, 1);

        indexBuffer->add(0);

        for (size_t ii = 0; ii < size; ii++) {
            vec3 pos(*position);
            vec3 v(*velocity);

            position++;
            velocity++;


            if (!(ii == size - 1 || ii%stride_.get() == 0) ) {
                continue;
            }

            float l = glm::length(v);
            float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
            maxVelocity = std::max(maxVelocity, l);
            c = tf_.get().sample(d);

            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

            vertices.push_back({ pos,glm::normalize(v),pos,c });

        }
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size() - 1));
    }
    mesh->addVertices(vertices);

    mesh_.setData(mesh);
    maxVelocity_.set(toString(maxVelocity));
}

} // namespace

