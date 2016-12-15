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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/properties/ordinalproperty.h>

#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/interpolation.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/animation.h>


namespace inviwo {
namespace animation {

TEST(AnimationTests, FloatInterpolation) {

    FloatProperty floatProperty("float", "Float", 0.0f, 0.0f, 1.0f);
    
    TrackProperty<FloatProperty, ValueKeyframe<float>> floatTrack(&floatProperty);

    KeyframeSequenceTyped<ValueKeyframe<float>> sequence(
        {{Time{1}, 0.0f}, {Time{2}, 1.0f}, {Time{3}, 0.0f}},
        std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());

    floatTrack.add(sequence);

    EXPECT_EQ(0.0f, floatProperty.get());

    floatTrack.evaluate(Time{0.0}, Time{1.5});

    EXPECT_EQ(0.5f, floatProperty.get());

    floatTrack.evaluate(Time{ 1.5 }, Time{ 2.5 });

    EXPECT_EQ(0.5f, floatProperty.get());

    floatTrack.evaluate(Time{ 2.5 }, Time{ 3.5 });

    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);

    EXPECT_EQ(3.0f, floatProperty.get());

    floatTrack.evaluate(Time{ 3.5 }, Time{ 4.5 });

    EXPECT_EQ(3.0f, floatProperty.get());

    floatTrack.evaluate(Time{ 3.5 }, Time{ 0.5 });

    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack.evaluate(Time{ 0.5 }, Time{ 1.0 });
    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack.evaluate(Time{ 0.5 }, Time{ 2.0 });
    EXPECT_EQ(1.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack.evaluate(Time{ 0.5 }, Time{ 3.0 });
    EXPECT_EQ(0.0f, floatProperty.get());

}


TEST(AnimationTests, AnimationTest) {

    FloatProperty floatProperty("float", "Float", 0.0f, 0.0f, 1.0f);

    DoubleVec3Property doubleProperty("double", "Double", dvec3(1.0), dvec3(0.0), dvec3(0.0));

    KeyframeSequenceTyped<ValueKeyframe<float>> floatSequence(
    { {Time{1}, 0.0f}, {Time{2}, 1.0f}, {Time{3}, 0.0f} },
        std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());

    KeyframeSequenceTyped<ValueKeyframe<dvec3>> doubleSequence(
    { {Time{1},  dvec3(1.0)}, {Time{2},  dvec3(0.0)}, {Time{3}, dvec3(1.0)} },
        std::make_unique<LinearInterpolation<ValueKeyframe<dvec3>>>());

    Animation animation;

    {
        auto floatTrack =
            std::make_unique<TrackProperty<FloatProperty, ValueKeyframe<float>>>(&floatProperty);
        floatTrack->add(floatSequence);
        animation.add(std::move(floatTrack));
    }

    {
        auto doubleTrack =
            std::make_unique<TrackProperty<DoubleVec3Property, ValueKeyframe<dvec3>>>(
                &doubleProperty);
        doubleTrack->add(doubleSequence);
        animation.add(std::move(doubleTrack));
    }

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(1.0), doubleProperty.get());

    animation.evaluate(Time{0.0}, Time{1.5});

    EXPECT_EQ(0.5f, floatProperty.get());
    EXPECT_EQ(dvec3(0.5), doubleProperty.get());
}

}  // namespace

}  // namespace
