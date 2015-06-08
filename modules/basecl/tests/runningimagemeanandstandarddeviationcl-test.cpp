/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/basecl/runningimagemeanandstandarddeviationcl.h>

float rnd() {
    return rand() / (float)RAND_MAX;
}

vec4* genSamples(const int size)
{
    srand(0);
    vec4* data = new vec4[size];
    for(int i=0; i<size; ++i)
        data[i] = vec4(rnd(), rnd(), rnd(), 1.f);
    return data;
}

namespace inviwo {

    void meanAndStd(const int size, vec4& mean, vec4& std, vec4& cl_mean, vec4& cl_std)
    {
        Layer* layer_sample = new Layer(uvec2(1), DataVec4FLOAT32::get());
        Layer* layer_mean;
        Layer* layer_std;

        vec4* data = genSamples(size);

        mean = vec4(0.f);
        for(int i=0; i<size; ++i)
            mean += data[i];
        mean = mean / (float)size;
        mean.w = 1.f;

        std = vec4(0.f);
        for(int i=0; i<size; ++i)
            std += (data[i] - mean)*(data[i] - mean);
        std = (size > 1) ? std / (float)(size - 1) : vec4(0,0,0,1);
        std.w = 1.f;

        RunningImageMeanAndStandardDeviationCL obj;

        for(int i=0; i<size; ++i)
        {
            LayerRAM* ram_sample = layer_sample->getEditableRepresentation<LayerRAM>();
            vec4* data_sample = static_cast<vec4*>(ram_sample->getData());
            *data_sample = data[i];
            obj.computeMeanAndStandardDeviation(layer_sample, i, layer_mean, layer_std, true);
        }

        LayerRAM* ram_mean = layer_mean->getEditableRepresentation<LayerRAM>();
        vec4* data_mean = static_cast<vec4*>(ram_mean->getData());

        LayerRAM* ram_std = layer_std->getEditableRepresentation<LayerRAM>();
        vec4* data_std = static_cast<vec4*>(ram_std->getData());

        cl_mean = *data_mean;
        cl_std = *data_std;

        delete[] data;
        delete layer_sample;
    }

    TEST(MeanAndStdTests, test1) {
        vec4 mean, std, cl_mean, cl_std;
        meanAndStd(1, mean, std, cl_mean, cl_std);

        EXPECT_EQ(mean, cl_mean);
        EXPECT_EQ(std, cl_std);
    }

    TEST(MeanAndStdTests, test10) {
        vec4 mean, std, cl_mean, cl_std;
        meanAndStd(10, mean, std, cl_mean, cl_std);

        EXPECT_EQ(mean, cl_mean);
        EXPECT_EQ(std, cl_std);
    }

    TEST(MeanAndStdTests, test100) {
        vec4 mean, std, cl_mean, cl_std;
        meanAndStd(100, mean, std, cl_mean, cl_std);

        EXPECT_EQ(mean, cl_mean);
        EXPECT_EQ(std, cl_std);
    }

    TEST(MeanAndStdTests, test1000) {
        vec4 mean, std, cl_mean, cl_std;
        meanAndStd(1000, mean, std, cl_mean, cl_std);

        EXPECT_EQ(mean, cl_mean);
        EXPECT_EQ(std, cl_std);
    }

} // namespace

