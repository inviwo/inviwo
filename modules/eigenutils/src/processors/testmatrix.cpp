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

#include <modules/eigenutils/processors/testmatrix.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TestMatrix::processorInfo_{
    "org.inviwo.TestMatrix",  // Class identifier
    "Test Matrix",            // Display name
    "Eigen",                  // Category
    CodeState::Stable,        // Code state
    "Testing",                // Tags
};
const ProcessorInfo TestMatrix::getProcessorInfo() const { return processorInfo_; }

TestMatrix::TestMatrix()
    : Processor(), matrix_("matrix_"), size_("size", "Matrix size (square)", 10, 1, 1000) {
    addPort(matrix_);
    addProperty(size_);
}

void TestMatrix::process() {

    auto m = std::make_shared<Eigen::MatrixXf>(size_.get(), size_.get());
    for (size_t y = 0; y < size_.get(); y++) {
        float Y = y / static_cast<float>(size_.get() - 1);
        Y *= Y * 0.5f;
        for (size_t x = 0; x < size_.get(); x++) {
            float X = x / static_cast<float>(size_.get() - 1);
            X *= X;
            (*m)(y, x) = glm::mix(Y, 1.0f, X);
        }
    }
    matrix_.setData(m);
}

}  // namespace inviwo
