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

#include <modules/base/processors/randommeshgenerator.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo RandomMeshGenerator::processorInfo_{
    "org.inviwo.RandomMeshGenerator",  // Class identifier
    "Random Mesh Generator",           // Display name
    "Mesh Creation",                   // Category
    CodeState::Experimental,           // Code state
    Tags::None,                        // Tags
};

const ProcessorInfo RandomMeshGenerator::getProcessorInfo() const { return processorInfo_; }

RandomMeshGenerator::RandomMeshGenerator()
    : Processor()
    , mesh_("mesh")
    , seed_("seed", "Seed", 0, 0, std::mt19937::max())
    , rand_()
    , reseed_("reseed_", "Seed")
    , numberOfBoxes_("numberOf_", "Number of Boxes", 1, 0, 100)
    , numberOfSpheres_("numberOfSpheres_", "Number of Spheres", 1, 0, 100)
    , numberOfCylinders_("numberOfCylinders_", "Number of cylinders", 1, 0, 100)
    , numberOfCones_("numberOfCones_", "Number of Cones", 1, 0, 100)
    , numberOfToruses_("numberOfToruses", "Number of Toruses", 1, 0, 100)

{

    addPort(mesh_);
    addProperty(numberOfBoxes_);
    addProperty(numberOfSpheres_);
    addProperty(numberOfCylinders_);
    addProperty(numberOfCones_);
    addProperty(numberOfToruses_);

    addProperty(seed_);
    addProperty(reseed_);

    reseed_.onChange([&]() {
        seed_.set(rand());
        rand_.seed(static_cast<std::mt19937::result_type>(seed_.get()));
    });
}

float RandomMeshGenerator::randD() { return dis_(rand_); }

float RandomMeshGenerator::randD(const float min, const float max) {
    return min + randD() * (max - min);
}

inviwo::vec4 RandomMeshGenerator::randColor() {
    float r = randD(0.5, 1.0);
    float g = randD(0.5, 1.0);
    float b = randD(0.5, 1.0);
    return vec4(r, g, b, 1);
}

vec3 RandomMeshGenerator::randVec3(const float min, const float max) {
    float x = randD(min, max);
    float y = randD(min, max);
    float z = randD(min, max);
    return vec3(x, y, z);
}

void RandomMeshGenerator::process() {
    auto mesh = std::make_shared<BasicMesh>();
    mesh_.setData(mesh);

    rand_.seed(static_cast<std::mt19937::result_type>(seed_.get()));

    for (int i = 0; i < numberOfBoxes_.get(); i++) {
        mat4 o = glm::rotate(mat4(1.0f), randD(0, 6.28f), vec3(1, 0, 0));
        o = glm::rotate(o, randD(0, 6.28f), vec3(0, 1, 0));
        o = glm::rotate(o, randD(0, 6.28f), vec3(0, 0, 1));
        o = glm::translate(o, randVec3());
        o = glm::scale(o, randVec3());
        auto mesh2 = BasicMesh::cube(o, randColor());
        mesh->append(mesh2.get());
    }
    for (int i = 0; i < numberOfSpheres_.get(); i++) {
        auto radius = randD(0.1f, 1.0f);
        auto color = randColor();
        auto center = randVec3();
        BasicMesh::sphere(center, radius, color, mesh);
    }
    for (int i = 0; i < numberOfCylinders_.get(); i++) {
        auto radius = randD(0.1f, 1.0f);
        auto color = randColor();
        auto end = randVec3();
        auto start = randVec3();
        auto mesh2 = BasicMesh::cylinder(start, end, color, radius);
        mesh->append(mesh2.get());
    }
    for (int i = 0; i < numberOfCones_.get(); i++) {
        auto radius = randD(0.1f, 1.0f);
        auto color = randColor();
        auto end = randVec3();
        auto start = randVec3();
        auto mesh2 = BasicMesh::cone(start, end, color, radius);
        mesh->append(mesh2.get());
    }

    for (int i = 0; i < numberOfToruses_.get(); i++) {
        auto center = randVec3();
        auto up = glm::normalize(randVec3());
        auto r2 = randD(0.1, 0.5f);
        auto r1 = randD(0.1 + r2, 1.0f + r2);
        auto color = randColor();
        auto mesh2 = BasicMesh::torus(center, up, r1, r2, ivec2(32, 8), color);
        mesh->append(mesh2.get());
    }
}

}  // namespace
