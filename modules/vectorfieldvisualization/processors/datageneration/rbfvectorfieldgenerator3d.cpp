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

#include "rbfvectorfieldgenerator3d.h"

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

#include <warn/push>
#include <warn/ignore/all>
#include <Eigen/Dense>
#include <warn/pop>

namespace inviwo {
ProcessorClassIdentifier(RBFVectorFieldGenerator3D, "org.inviwo.RBFBased3DVectorFieldGenerator");
ProcessorDisplayName(RBFVectorFieldGenerator3D, "RBF Based 3D Vector Field Generator");
ProcessorTags(RBFVectorFieldGenerator3D, Tags::CPU);
ProcessorCategory(RBFVectorFieldGenerator3D, "Data Creation");
ProcessorCodeState(RBFVectorFieldGenerator3D, CODE_STATE_EXPERIMENTAL);
RBFVectorFieldGenerator3D::RBFVectorFieldGenerator3D()
    : Processor()
    , volume_("volume")
    , mesh_("mesh")
    , size_("size", "Volume size", size3_t(32, 32, 32), size3_t(1, 1, 1), size3_t(1024, 1024, 1024))
    , seeds_("seeds", "Number of seeds", 6, 1, 100)
    , shape_("shape", "Shape Parameter", 1.2f, 0.0001f, 10.0f, 0.0001f)
    , gaussian_("gaussian", "Gaussian")
    , randomness_("randomness", "Randomness")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, std::numeric_limits<int>::max())
    , debugMesh_("debug", "Debug Mesh Settings")
    , sphereRadius_("radius", "Radius", 0.1f)
    , arrowLength_("arrowLength", "Arrow Length", 0.3f)
    , sphereColor_("sphereColor", "Sphere Color", vec4(1, 0, 0, 1))
    , arrowColor_("arrowColor", "Arrow Color", vec4(0, 0, 1, 1))
    , rd_()
    , mt_(rd_())
    , theta_(0, 2 * M_PI)
    , x_(-1.0, 1.0) {
    addPort(volume_);
    addPort(mesh_);

    addProperty(size_);
    addProperty(seeds_);
    addProperty(shape_);
    addProperty(gaussian_);

    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });

    addProperty(debugMesh_);
    debugMesh_.addProperty(sphereRadius_);
    debugMesh_.addProperty(arrowLength_);
    debugMesh_.addProperty(sphereColor_);
    debugMesh_.addProperty(arrowColor_);

    sphereColor_.setSemantics(PropertySemantics::Color);
    arrowColor_.setSemantics(PropertySemantics::Color);

    setAllPropertiesCurrentStateAsDefault();
}

RBFVectorFieldGenerator3D::~RBFVectorFieldGenerator3D() {}

void RBFVectorFieldGenerator3D::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    mat3 basis(2);
    vec3 offset = vec3(-1);

    std::vector<std::pair<dvec3, dvec3>> samples(seeds_.get());
    std::generate(samples.begin(), samples.end(), [&]() {
        return std::make_pair(dvec3(x_(mt_), x_(mt_), x_(mt_)), randomVector());
    });

    if (mesh_.isConnected()) {
        auto mesh = std::make_shared<BasicMesh>();
        for (auto &p : samples) {
            vec3 p0 = vec3(p.first);
            vec3 p1 = p0 + glm::normalize(vec3(p.second)) * arrowLength_.get();
            auto sphere = BasicMesh::colorsphere(p0, sphereRadius_.get());
            auto arrow = BasicMesh::arrow(p0, p1, arrowColor_.get(), sphereRadius_.get() * 0.5f,
                                          0.15f, sphereRadius_.get());
            mesh->append(sphere.get());
            mesh->append(arrow.get());
        }
        mesh_.setData(mesh);
    }

    Eigen::MatrixXd A(seeds_.get(), seeds_.get());
    Eigen::VectorXd bx(seeds_.get()), by(seeds_.get()), bz(seeds_.get());
    Eigen::VectorXd xx(seeds_.get()), xy(seeds_.get()), xz(seeds_.get());

    int row = 0;
    for (auto &a : samples) {
        int col = 0;
        for (auto &b : samples) {
            auto r = glm::distance(a.first, b.first);
            A(row, col++) = shape_.get() + gaussian_.evaluate(r);
        }
        bx(row) = a.second.x;
        by(row) = a.second.y;
        bz(row++) = a.second.z;
    }

    auto solverX = A.llt();
    auto solverY = A.llt();
    auto solverZ = A.llt();

    xx = solverX.solve(bx);
    xy = solverY.solve(by);
    xz = solverZ.solve(bz);

    auto testX = solverX.info() == Eigen::Success;
    auto testY = solverY.info() == Eigen::Success;
    auto testZ = solverZ.info() == Eigen::Success;
    if (!(testX && testY && testZ)) {
        LogError("Fuck, didn't work" << (testX ? "1" : "0") << (testY ? "1" : "0")
                                     << (testZ ? "1" : "0"));
    }

    auto volume = std::make_shared<Volume>(size_.get(), DataVec4FLOAT32::get());
    volume->dataMap_.dataRange = vec2(0, 1);
    volume->dataMap_.valueRange = vec2(-1, 1);
    volume->setBasis(basis);
    volume->setOffset(offset);

    vec4 *data = static_cast<vec4 *>(volume->getEditableRepresentation<VolumeRAM>()->getData());

    int i = 0;
    for (size_t z = 0; z < size_.get().z; z++) {
        for (size_t y = 0; y < size_.get().y; y++) {
            for (size_t x = 0; x < size_.get().x; x++) {
                dvec3 p(x, y, z);
                p /= size_.get();
                p *= 2;
                p -= 1;

                vec3 v(0, 0, 0);
                int s = 0;
                for (; s < seeds_.get(); s++) {
                    double r = glm::distance(p, samples[s].first);
                    v.x += static_cast<float>(xx(s) * gaussian_.evaluate(r));
                    v.y += static_cast<float>(xy(s) * gaussian_.evaluate(r));
                    v.z += static_cast<float>(xz(s) * gaussian_.evaluate(r));
                }

                data[i++] = vec4(v, glm::length(v));
            }
        }
    }

    volume_.setData(volume);
}

dvec3 RBFVectorFieldGenerator3D::randomVector() {
    dvec3 v;
    v.z = x_(mt_);
    auto t = theta_(mt_);
    v.y = v.x = sqrt(1 - v.z * v.z);
    v.x *= cos(t);
    v.y *= sin(t);

    return v;
}

}  // namespace
