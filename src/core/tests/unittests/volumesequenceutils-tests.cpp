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

#include <inviwo/core/util/volumesequenceutils.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/datastructures/volume/volume.h>


namespace {

    std::shared_ptr<inviwo::Volume> createVolume() {
        auto v = std::make_shared<inviwo::Volume>(inviwo::size3_t(5, 5, 5), inviwo::DataUInt8::get());
        return v;
    }


    std::shared_ptr<inviwo::Volume> createVolume(double t) {
        auto v = createVolume();
        v->setMetaData<inviwo::DoubleMetaData, double>("timestamp", t);
        return v;
    }
}

namespace inviwo{




    TEST(VolumeSequenceUtilsTests, hasTimestampsTest) {
        VoumeSequence s1;
        s1.push_back(createVolume(0.0));
        s1.push_back(createVolume(0.1));
        s1.push_back(createVolume(0.2));

        VoumeSequence s2;
        s2.push_back(createVolume());
        s2.push_back(createVolume());
        s2.push_back(createVolume());

        EXPECT_TRUE(util::hasTimestamps(s1));
        EXPECT_FALSE(util::hasTimestamps(s2));
    }



    TEST(VolumeSequenceUtilsTests, isSorted) {
        VoumeSequence s1;
        s1.push_back(createVolume(0.0));
        s1.push_back(createVolume(0.1));
        s1.push_back(createVolume(0.2));

        VoumeSequence s2;
        s2.push_back(createVolume(0.2));
        s2.push_back(createVolume(0.1));
        s2.push_back(createVolume(0.4));

        auto a = util::isSorted(s1);
        auto b = util::isSorted(s2);

        EXPECT_TRUE(a);
        EXPECT_FALSE(b);
    }



TEST(VolumeSequenceUtilsTests, getTimestampRangeTest) {
    VoumeSequence s1;
    s1.push_back(createVolume(0.0));
    s1.push_back(createVolume(0.1));
    s1.push_back(createVolume(0.2));

    auto range = util::getTimestampRange(s1);
    EXPECT_EQ(range.first, 0.0);
    EXPECT_EQ(range.second, 0.2);
}


TEST(VolumeSequenceUtilsTests, getVolumesForTimestepTest) {
    VoumeSequence s1;
    s1.push_back(createVolume(0.0));
    s1.push_back(createVolume(0.1));
    s1.push_back(createVolume(0.2));

    auto p0 = util::getVolumesForTimestep(s1, 0.0);
    auto p1 = util::getVolumesForTimestep(s1, 0.1);
    auto p2 = util::getVolumesForTimestep(s1, 0.2);
    auto p3 = util::getVolumesForTimestep(s1, 0.15);


    /* UNUSED
    auto A = s1[0].get();
    auto B = s1[1].get();
    auto C = s1[2].get();

    auto X1 = p0.first.get();
    auto X2 = p0.second.get();

    auto Y1 = p1.first.get();
    auto Y2 = p1.second.get();

    auto Z1 = p2.first.get();
    auto Z2 = p2.second.get();

    auto W1 = p3.first.get();
    auto W2 = p3.second.get();
    */
   
    EXPECT_EQ(s1[0].get(), p0.first.get());
    EXPECT_EQ(s1[1].get(), p0.second.get());

    EXPECT_EQ(s1[1].get(), p1.first.get());
    EXPECT_EQ(s1[2].get(), p1.second.get());

    EXPECT_EQ(s1[2].get(), p2.first.get());
    EXPECT_EQ(s1[2].get(), p2.second.get());

    EXPECT_EQ(s1[1].get(), p3.first.get());
    EXPECT_EQ(s1[2].get(), p3.second.get());






    VoumeSequence s2;
    
    s2.push_back(createVolume());
    s2.push_back(createVolume());
    s2.push_back(createVolume());
    s2.push_back(createVolume());


    auto p4 = util::getVolumesForTimestep(s2, 0.0);
    auto p5 = util::getVolumesForTimestep(s2, 1.0);
    auto p6 = util::getVolumesForTimestep(s2, 0.5);


    EXPECT_EQ(p4.first.get(), s2[0].get());
    EXPECT_EQ(p4.second.get(), s2[1].get());


    EXPECT_EQ(p5.first.get(), s2[3].get());
    EXPECT_EQ(p5.second.get(), s2[3].get());


    EXPECT_EQ(p6.first.get(), s2[1].get());
    EXPECT_EQ(p6.second.get(), s2[2].get());

}

}