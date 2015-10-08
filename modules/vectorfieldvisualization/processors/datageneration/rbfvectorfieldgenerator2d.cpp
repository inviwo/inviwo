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

#include "rbfvectorfieldgenerator2d.h"

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

#include <warn/push>
#include <warn/ignore/all>
#include <Eigen/Dense>
#include <warn/pop>

namespace inviwo {

ProcessorClassIdentifier(RBFVectorFieldGenerator2D, "org.inviwo.RBFVectorFieldGenerator2D");
ProcessorDisplayName(RBFVectorFieldGenerator2D, "RBF Based 2D Vector Field Generator");
ProcessorTags(RBFVectorFieldGenerator2D, Tags::CPU);
ProcessorCategory(RBFVectorFieldGenerator2D, "Data Creation");
ProcessorCodeState(RBFVectorFieldGenerator2D, CODE_STATE_EXPERIMENTAL);

RBFVectorFieldGenerator2D::RBFVectorFieldGenerator2D()
    : Processor()
    , vectorField_("vectorField", DataVec4FLOAT32::get(), false)
    , size_("size", "Volume size", ivec2(700, 700), ivec2(1, 1), ivec2(1024, 1024))
    , seeds_("seeds", "Number of seeds", 9, 1, 100)
    , shape_("shape", "Shape Parameter", 1.2f, 0.0001f, 10.0f, 0.0001f)
    , gaussian_("gaussian", "Gaussian")
    , randomness_("randomness", "Randomness")
    , useSameSeed_("useSameSeed", "Use same seed", true)
    , seed_("seed", "Seed", 1, 0, std::numeric_limits<int>::max())
    , rd_()
    , mt_(rd_())
    , theta_(0, 2 * M_PI)
    , x_(-1.0, 1.0) {
    addPort(vectorField_);

    addProperty(size_);
    addProperty(seeds_);
    addProperty(shape_);
    addProperty(gaussian_);

    addProperty(randomness_);
    randomness_.addProperty(useSameSeed_);
    randomness_.addProperty(seed_);
    useSameSeed_.onChange([&]() { seed_.setVisible(useSameSeed_.get()); });
}

RBFVectorFieldGenerator2D::~RBFVectorFieldGenerator2D() {}

void RBFVectorFieldGenerator2D::process() {
    if (useSameSeed_.get()) {
        mt_.seed(seed_.get());
    }

    std::vector<std::pair<dvec2, dvec2>> samples(seeds_.get());
    std::generate(samples.begin(), samples.end(),
                  [&]() { return std::make_pair(dvec2(x_(mt_), x_(mt_)), randomVector()); });

    Eigen::MatrixXd A(seeds_.get(), seeds_.get());
    Eigen::VectorXd bx(seeds_.get()), by(seeds_.get());
    Eigen::VectorXd xx(seeds_.get()), xy(seeds_.get());

    int row = 0;
    for (auto &a : samples) {
        int col = 0;
        for (auto &b : samples) {
            auto r = glm::distance(a.first, b.first);
            A(row, col++) = shape_.get() + gaussian_.evaluate(r);
        }
        bx(row) = a.second.x;
        by(row++) = a.second.y;
    }

    auto solverX = A.llt();
    auto solverY = A.llt();

    xx = solverX.solve(bx);
    xy = solverY.solve(by);

    auto testX = solverX.info() == Eigen::Success;
    auto testY = solverY.info() == Eigen::Success;
    if (!(testX && testY)) {
        LogError("Fuck, didn't work" << (testX ? "1" : "0") << (testY ? "1" : "0"));
    }

    auto img = std::make_shared<Image>(size_.get(), DataVec4FLOAT32::get());

    vec4 *data =
        static_cast<vec4 *>(img->getColorLayer()->getEditableRepresentation<LayerRAM>()->getData());

    int i = 0;
    for (int y = 0; y < size_.get().y; y++) {
        for (int x = 0; x < size_.get().x; x++) {
            dvec2 p(x, y);
            p /= size_.get();
            p *= 2;
            p -= 1;

            vec3 v(0, 0, 0);
            int s = 0;
            for (; s < seeds_.get(); s++) {
                double r = glm::distance(p, samples[s].first);
                auto w = gaussian_.evaluate(r);
                v.x += static_cast<float>(xx(s) * w);
                v.y += static_cast<float>(xy(s) * w);
            }
            data[i++] = vec4(v, 1.0f);
        }
    }
    vectorField_.setData(img);
}

dvec2 RBFVectorFieldGenerator2D::randomVector() {
    dvec2 v(1, 0);

    auto d = theta_(mt_);
    auto c = std::cos(d);
    auto s = std::sin(d);
    return (mat2(c, s, -s, c) * v) * static_cast<float>((x_(mt_) * 2 - 1));
}

}  // namespace
