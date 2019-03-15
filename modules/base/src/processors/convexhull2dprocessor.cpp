/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/base/processors/convexhull2dprocessor.h>

#include <modules/base/algorithm/convexhull.h>
#include <modules/base/algorithm/convexhullmesh.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>

#include <glm/gtc/epsilon.hpp>

namespace inviwo {

const ProcessorInfo ConvexHull2DProcessor::processorInfo_{
    "org.inviwo.ConvexHull2DProcessor",  // Class identifier
    "Convex Hull2D Processor",           // Display name
    "Geometry",                          // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
};
const ProcessorInfo ConvexHull2DProcessor::getProcessorInfo() const { return processorInfo_; }

ConvexHull2DProcessor::ConvexHull2DProcessor()
    : Processor()
    , inport_("geometry")
    , outport_("convexhull")
    , normal_("normal", "Normal", vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f), vec3(1.0f)) {
    addPort(inport_);
    addPort(outport_);
    addProperty(normal_);
}

void ConvexHull2DProcessor::process() {
    auto data = inport_.getData();

    // build a rotation matrix based on the normal
    const vec3 normal(glm::normalize(normal_.get()));
    mat3 rotMatrix(1.0f);
    if (1.0f - std::abs(glm::dot(normal, vec3(0.0f, 0.0f, 1.0f))) > glm::epsilon<float>()) {
        // build right-hand coordinate system around normal
        vec3 right = glm::normalize(glm::cross(normal, vec3(0.0f, 0.0f, 1.0f)));
        vec3 up = glm::normalize(glm::cross(normal, right));
        right = glm::cross(up, normal);
        rotMatrix[0] = right;
        rotMatrix[1] = up;
        rotMatrix[2] = normal;
    }
    // build projection matrix
    glm::dmat3x2 proj(glm::transpose(glm::dmat2x3(rotMatrix[0], rotMatrix[1])));

    // TODO: check data format of all buffers! And find common denominator instead of dvec2
    std::vector<vec2> points;
    // collect all positions of the input geometry
    for (auto item : data->getBuffers()) {
        if (item.first.type == BufferType::PositionAttrib) {
            auto buffer = item.second->getRepresentation<BufferRAM>();
            for (std::size_t i = 0; i < buffer->getSize(); ++i) {
                auto p = buffer->getAsDVec3(i);
                // project point p onto plane given by normal
                auto q = proj * p;
                points.push_back(q);
            }
        }
    }

    std::vector<vec2> hull;
    hull = util::convexHull(points);
    if (!util::isConvex(hull)) {
        LogWarn("Hull returned by Monotone Chain algorithm (convexHull) is _not_ convex");
    }

    // convert hull into a line strip mesh
    auto mesh = util::convertHullToMesh(hull);
    mesh->setModelMatrix(mat4(rotMatrix));
    outport_.setData(mesh);
}

}  // namespace inviwo
