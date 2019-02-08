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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/datastructures/transferfunction.h>

#include <iostream>

namespace inviwo {

TEST(TFSampling, empty) {
    TransferFunction tf;

    EXPECT_EQ(vec4(0.0f), tf.sample(0.0));
    EXPECT_EQ(vec4(0.0f), tf.sample(0.3));
    EXPECT_EQ(vec4(0.0f), tf.sample(1.2));
}

TEST(TFSampling, constant) {
    vec4 color{1.0f, 0.0f, 0.5f, 1.0f};
    TransferFunction tf{{{0.5, color}}};

    EXPECT_EQ(color, tf.sample(0.0));
    EXPECT_EQ(color, tf.sample(0.3));
    EXPECT_EQ(color, tf.sample(0.5));
    EXPECT_EQ(color, tf.sample(1.0));

    // check outside of TF boundaries
    EXPECT_EQ(color, tf.sample(-0.3)) << "TF sampling < 0 not clamped to [0,1]";
    EXPECT_EQ(color, tf.sample(1.3)) << "TF sampling > 1 not clamped to [0,1]";
}

TEST(TFSampling, step) {
    vec4 color1{0.0f, 1.0f, 0.0f, 0.5f};
    vec4 color2{1.0f, 0.0f, 0.5f, 1.0f};
    TransferFunction tf{{{0.4999, color1}, {0.5, color2}}};

    EXPECT_EQ(color1, tf.sample(0.4));
    EXPECT_EQ(color2, tf.sample(0.6));
}

TEST(TFSampling, linear) {
    vec4 color1{0.0f, 1.0f, 0.0f, 0.5f};
    vec4 color2{1.0f, 0.0f, 0.5f, 1.0f};
    TransferFunction tf{{{0.25, color1}, {0.75, color2}}};

    EXPECT_EQ(color1, tf.sample(0.0));
    EXPECT_EQ(color1, tf.sample(0.25));
    EXPECT_EQ(glm::mix(color1, color2, 0.5), tf.sample(0.5));
    EXPECT_EQ(color2, tf.sample(1.0));
}

}  // namespace inviwo
