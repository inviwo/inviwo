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

#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertyfactoryobject.h>

#include <inviwo/core/properties/ordinalproperty.h>

#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/interpolation.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/interpolation.h>

#include <modules/animation/factories/interpolationfactory.h>
#include <modules/animation/factories/interpolationfactoryobject.h>


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

    floatTrack(Time{0.0}, Time{1.5});

    EXPECT_EQ(0.5f, floatProperty.get());

    floatTrack(Time{ 1.5 }, Time{ 2.5 });

    EXPECT_EQ(0.5f, floatProperty.get());

    floatTrack(Time{ 2.5 }, Time{ 3.5 });

    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);

    EXPECT_EQ(3.0f, floatProperty.get());

    floatTrack(Time{ 3.5 }, Time{ 4.5 });

    EXPECT_EQ(3.0f, floatProperty.get());

    floatTrack(Time{ 3.5 }, Time{ 0.5 });

    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack(Time{ 0.5 }, Time{ 1.0 });
    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack(Time{ 0.5 }, Time{ 2.0 });
    EXPECT_EQ(1.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack(Time{ 0.5 }, Time{ 3.0 });
    EXPECT_EQ(0.0f, floatProperty.get());

}


TEST(AnimationTests, AnimationTest) {

    FloatProperty floatProperty("float", "Float", 0.0f, 0.0f, 1.0f);

    DoubleVec3Property doubleProperty("double", "Double", dvec3(1.0), dvec3(0.0), dvec3(0.0));

    KeyframeSequenceTyped<ValueKeyframe<float>> floatSequence(
    { {Time{1.0}, 0.0f}, {Time{2.0}, 1.0f}, {Time{3.0}, 0.0f} },
        std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());

    KeyframeSequenceTyped<ValueKeyframe<dvec3>> doubleSequence(
    { {Time{1.0},  dvec3(1.0)}, {Time{2.0},  dvec3(0.0)}, {Time{3.0}, dvec3(1.0)} },
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

    animation(Time{0.0}, Time{1.5});

    EXPECT_EQ(0.5f, floatProperty.get());
    EXPECT_EQ(dvec3(0.5), doubleProperty.get());

    EXPECT_EQ(Time{1.0}, animation.firstTime());
    EXPECT_EQ(Time{3.0}, animation.lastTime());

    animation[1][0].add(ValueKeyframe<dvec3>{Time{4.0}, dvec3(2.0)});

    EXPECT_EQ(Time{ 1.0 }, animation.firstTime());
    EXPECT_EQ(Time{ 4.0 }, animation.lastTime());

    animation(Time{0.0}, Time{3.5});

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(1.5), doubleProperty.get());

    animation[1][0].remove(2);

    animation(Time{ 0.0 }, Time{ 3.0 });

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(1.0), doubleProperty.get());

    {
        KeyframeSequenceTyped<ValueKeyframe<dvec3>> doubleSequence2(
        { {Time{6.0}, dvec3(1.0)}, {Time{7.0}, dvec3(0.0)}, {Time{8.0}, dvec3(1.0)} },
            std::make_unique<LinearInterpolation<ValueKeyframe<dvec3>>>());
        animation[1].add(doubleSequence2);
    }

    EXPECT_EQ(Time{ 1.0 }, animation.firstTime());
    EXPECT_EQ(Time{ 8.0 }, animation.lastTime());

    animation(Time{ 0.0 }, Time{ 7.5 });

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(0.5), doubleProperty.get());
}


TEST(AnimationTests, KeyframeSerializationTest) {
    ValueKeyframe<dvec3> keyframe{Time{4.0}, dvec3(2.0)};

    const std::string refPath = "/tmp";

    Serializer s(refPath);
    keyframe.serialize(s);

    std::stringstream ss;
    s.writeFile(ss);

    Deserializer d(nullptr, ss, refPath);
    ValueKeyframe<dvec3> keyframe2;
    keyframe2.deserialize(d);

    EXPECT_EQ(keyframe, keyframe2);
}


TEST(AnimationTests, InterpolationSerializationTest) {
    InterpolationFactory factory;

    InterpolationFactoryObjectTemplate<LinearInterpolation<ValueKeyframe<dvec3>>> linearIFO(
        LinearInterpolation<ValueKeyframe<dvec3>>::classIdentifier());
    factory.registerObject(&linearIFO);

    const std::string refPath = "/tmp";

    LinearInterpolation<ValueKeyframe<dvec3>> linear;

    Interpolation* iptr = &linear;


    Serializer s(refPath);
    s.serialize("interpolation", iptr);

    std::stringstream ss;
    s.writeFile(ss);

    Deserializer d(nullptr, ss, refPath);
    d.setExceptionHandler([](ExceptionContext context) {throw;});
    d.registerFactory(&factory);
    
    Interpolation* iptr2 = nullptr;
    d.deserialize("interpolation", iptr2);


    delete iptr2;
    factory.unRegisterObject(&linearIFO);
}




TEST(AnimationTests, KeyframeSequenceSerializationTest) {
    InterpolationFactory factory;

    InterpolationFactoryObjectTemplate<LinearInterpolation<ValueKeyframe<dvec3>>> linearIFO(
        LinearInterpolation<ValueKeyframe<dvec3>>::classIdentifier());
    factory.registerObject(&linearIFO);


    KeyframeSequenceTyped<ValueKeyframe<dvec3>> doubleSequence(
    { {Time{1.0},  dvec3(1.0)}, {Time{2.0},  dvec3(0.0)}, {Time{3.0}, dvec3(1.0)} },
        std::make_unique<LinearInterpolation<ValueKeyframe<dvec3>>>());

    const std::string refPath = "/tmp";

    Serializer s(refPath);
    doubleSequence.serialize(s);

    std::stringstream ss;
    s.writeFile(ss);

    Deserializer d(nullptr, ss, refPath);
    d.setExceptionHandler([](ExceptionContext context) {throw;});
    d.registerFactory(&factory);
    
    KeyframeSequenceTyped<ValueKeyframe<dvec3>> doubleSequence2;

    doubleSequence2.deserialize(d);

    EXPECT_EQ(doubleSequence, doubleSequence2);

    factory.unRegisterObject(&linearIFO);
}

TEST(AnimationTests, TrackSerializationTest) {
    InterpolationFactory interpolationFactory;
    InterpolationFactoryObjectTemplate<LinearInterpolation<ValueKeyframe<float>>> linearIFO(
        LinearInterpolation<ValueKeyframe<float>>::classIdentifier());
    interpolationFactory.registerObject(&linearIFO);

    PropertyFactory propertyFactory;
    PropertyFactoryObjectTemplate<FloatProperty> floatPFO;
    propertyFactory.registerObject(&floatPFO);


    FloatProperty floatProperty("float", "Float", 0.0f, 0.0f, 1.0f);
    TrackProperty<FloatProperty, ValueKeyframe<float>> floatTrack(&floatProperty);
    KeyframeSequenceTyped<ValueKeyframe<float>> sequence(
    { {Time{1}, 0.0f}, {Time{2}, 1.0f}, {Time{3}, 0.0f} },
        std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());
    floatTrack.add(sequence);

    const std::string refPath = "/tmp";

    Serializer s(refPath);
    s.serialize("Property", &floatProperty);
    s.serialize("Track", floatTrack);
    
    std::stringstream ss;
    s.writeFile(ss);

    Deserializer d(nullptr, ss, refPath);
    d.setExceptionHandler([](ExceptionContext context) {throw; });
    d.registerFactory(&interpolationFactory);
    d.registerFactory(&propertyFactory);


    Property* floatProperty2 = nullptr;
    TrackProperty<FloatProperty, ValueKeyframe<float>> floatTrack2;
    
    d.deserialize("Property", floatProperty2);
    d.deserialize("Track", floatTrack2);

    EXPECT_EQ(floatTrack[0], floatTrack2[0]);

    interpolationFactory.unRegisterObject(&linearIFO);
    propertyFactory.unRegisterObject(&floatPFO);
}


}  // namespace

}  // namespace
