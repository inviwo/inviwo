/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorfactoryobject.h>

#include <modules/animation/datastructures/callbacktrack.h>
#include <modules/animation/datastructures/controltrack.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/valuekeyframe.h>
#include <modules/animation/interpolation/constantinterpolation.h>
#include <modules/animation/interpolation/linearinterpolation.h>
#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/animation.h>

#include <modules/animation/factories/interpolationfactory.h>
#include <modules/animation/factories/interpolationfactoryobject.h>

#include <modules/animation/factories/trackfactory.h>
#include <modules/animation/factories/trackfactoryobject.h>

#include <filesystem>

namespace inviwo {
namespace animation {

TEST(AnimationTests, FloatInterpolation) {

    FloatProperty floatProperty("float", "Float", 0.0f, 0.0f, 100.0f);

    PropertyTrack<FloatProperty, ValueKeyframe<float>> floatTrack(&floatProperty, nullptr);

    std::vector<std::unique_ptr<ValueKeyframe<float>>> fseq;
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{1.0}, 0.0f));
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{2.0}, 1.0f));
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{3.0}, 0.0f));
    auto sequence = std::make_unique<KeyframeSequenceTyped<ValueKeyframe<float>>>(
        std::move(fseq), std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());

    floatTrack.add(std::move(sequence));

    EXPECT_EQ(0.0f, floatProperty.get());

    floatTrack(Seconds{0.0}, Seconds{1.5}, AnimationState::Playing);

    EXPECT_EQ(0.5f, floatProperty.get());

    floatTrack(Seconds{1.5}, Seconds{2.5}, AnimationState::Playing);

    EXPECT_EQ(0.5f, floatProperty.get());

    floatTrack(Seconds{2.5}, Seconds{3.5}, AnimationState::Playing);

    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);

    EXPECT_EQ(3.0f, floatProperty.get());

    floatTrack(Seconds{3.5}, Seconds{4.5}, AnimationState::Playing);

    EXPECT_EQ(3.0f, floatProperty.get());

    floatTrack(Seconds{3.5}, Seconds{0.5}, AnimationState::Playing);

    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack(Seconds{0.5}, Seconds{1.0}, AnimationState::Playing);
    EXPECT_EQ(0.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack(Seconds{0.5}, Seconds{2.0}, AnimationState::Playing);
    EXPECT_EQ(1.0f, floatProperty.get());

    floatProperty.set(3.0f);
    EXPECT_EQ(3.0f, floatProperty.get());
    floatTrack(Seconds{0.5}, Seconds{3.0}, AnimationState::Playing);
    EXPECT_EQ(0.0f, floatProperty.get());
}

TEST(AnimationTests, ConstantInterpolation) {

    FloatProperty floatProperty("float", "Float", 0.0f, 0.0f, 100.0f);

    PropertyTrack<FloatProperty, ValueKeyframe<float>> floatTrack(&floatProperty, nullptr);

    std::vector<std::unique_ptr<ValueKeyframe<float>>> fseq;
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{1.0}, 1.0f));
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{2.0}, 2.0f));
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{3.0}, 3.0f));
    auto sequence = std::make_unique<KeyframeSequenceTyped<ValueKeyframe<float>>>(
        std::move(fseq), std::make_unique<ConstantInterpolation<ValueKeyframe<float>>>());

    floatTrack.add(std::move(sequence));

    EXPECT_EQ(0.0f, floatProperty.get());

    floatTrack(Seconds{0.0}, Seconds{1.5}, AnimationState::Playing);

    EXPECT_EQ(1.0f, floatProperty.get());

    floatTrack(Seconds{2.5}, Seconds{3.5}, AnimationState::Playing);

    EXPECT_EQ(3.0f, floatProperty.get());

    floatProperty.set(4.0f);

    EXPECT_EQ(4.0f, floatProperty.get());

    floatTrack(Seconds{3.5}, Seconds{4.5}, AnimationState::Playing);

    EXPECT_EQ(4.0f, floatProperty.get());

    floatTrack(Seconds{3.5}, Seconds{1.0}, AnimationState::Playing);

    EXPECT_EQ(1.0f, floatProperty.get());

    floatProperty.set(4.0f);
    EXPECT_EQ(4.0f, floatProperty.get());
    floatTrack(Seconds{0.0}, Seconds{1.0}, AnimationState::Playing);
    EXPECT_EQ(1.0f, floatProperty.get());

    floatProperty.set(4.0f);
    EXPECT_EQ(4.0f, floatProperty.get());
    floatTrack(Seconds{1.0}, Seconds{2.0}, AnimationState::Playing);
    EXPECT_EQ(2.0f, floatProperty.get());
    floatTrack(Seconds{2.0}, Seconds{3.0}, AnimationState::Playing);
    EXPECT_EQ(3.0f, floatProperty.get());

    floatProperty.set(4.0f);
    EXPECT_EQ(4.0f, floatProperty.get());
    floatTrack(Seconds{3.0}, Seconds{2.0}, AnimationState::Playing);
    EXPECT_EQ(2.0f, floatProperty.get());
    floatTrack(Seconds{2.0}, Seconds{1.0}, AnimationState::Playing);
    EXPECT_EQ(1.0f, floatProperty.get());
}

TEST(AnimationTests, AnimationTest) {

    FloatProperty floatProperty("float", "Float", 0.0f, 0.0f, 100.0f);

    DoubleVec3Property doubleProperty("double", "Double", dvec3(1.0), dvec3(0.0), dvec3(100.0));

    std::vector<std::unique_ptr<ValueKeyframe<float>>> fseq;
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{1.0}, 0.0f));
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{2.0}, 1.0f));
    fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{3.0}, 0.0f));
    auto floatSequence = std::make_unique<KeyframeSequenceTyped<ValueKeyframe<float>>>(
        std::move(fseq), std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());

    std::vector<std::unique_ptr<ValueKeyframe<dvec3>>> dseq;
    dseq.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{1.0}, dvec3{1.0}));
    dseq.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{2.0}, dvec3{0.0}));
    dseq.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{3.0}, dvec3{1.0}));
    auto doubleSequence = std::make_unique<KeyframeSequenceTyped<ValueKeyframe<dvec3>>>(
        std::move(dseq), std::make_unique<LinearInterpolation<ValueKeyframe<dvec3>>>());

    Animation animation;
    {
        auto floatTrack = std::make_unique<PropertyTrack<FloatProperty, ValueKeyframe<float>>>(
            &floatProperty, nullptr);
        floatTrack->add(std::move(floatSequence));
        animation.add(std::move(floatTrack));
    }

    {
        auto doubleTrack =
            std::make_unique<PropertyTrack<DoubleVec3Property, ValueKeyframe<dvec3>>>(
                &doubleProperty, nullptr);
        doubleTrack->add(std::move(doubleSequence));
        animation.add(std::move(doubleTrack));
    }

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(1.0), doubleProperty.get());

    animation(Seconds{0.0}, Seconds{1.5}, AnimationState::Playing);

    EXPECT_EQ(0.5f, floatProperty.get());
    EXPECT_EQ(dvec3(0.5), doubleProperty.get());

    EXPECT_EQ(Seconds{1.0}, animation.getFirstTime());
    EXPECT_EQ(Seconds{3.0}, animation.getLastTime());

    EXPECT_EQ(std::nullopt, animation.getPrevTime(Seconds{0.0}));
    EXPECT_EQ(std::nullopt, animation.getPrevTime(Seconds{1.0}));
    EXPECT_EQ(Seconds{1.0}, animation.getPrevTime(Seconds{2.0}));
    EXPECT_EQ(Seconds{3.0}, animation.getPrevTime(Seconds{4.0}));

    EXPECT_EQ(Seconds{1.0}, animation.getNextTime(Seconds{0.0}));
    EXPECT_EQ(Seconds{2.0}, animation.getNextTime(Seconds{1.0}));
    EXPECT_EQ(std::nullopt, animation.getNextTime(Seconds{4.0}));

    animation[1][0].add(std::make_unique<ValueKeyframe<dvec3>>(Seconds{1.5}, dvec3(2.0)));
    animation[1][0].add(std::make_unique<ValueKeyframe<dvec3>>(Seconds{4.0}, dvec3(2.0)));

    EXPECT_EQ(Seconds{1.0}, animation.getFirstTime());
    EXPECT_EQ(Seconds{4.0}, animation.getLastTime());
    EXPECT_EQ(Seconds{1.5}, animation.getPrevTime(Seconds{2.0}));
    EXPECT_EQ(Seconds{1.5}, animation.getNextTime(Seconds{1.0}));
    EXPECT_EQ(Seconds{4.0}, animation.getNextTime(Seconds{3.0}));

    animation(Seconds{0.0}, Seconds{3.5}, AnimationState::Playing);

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(1.5), doubleProperty.get());

    animation[1][0].remove(2);

    animation(Seconds{0.0}, Seconds{3.0}, AnimationState::Playing);

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(1.0), doubleProperty.get());

    {
        std::vector<std::unique_ptr<ValueKeyframe<dvec3>>> dseq2;
        dseq2.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{6.0}, dvec3{1.0}));
        dseq2.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{7.0}, dvec3{0.0}));
        dseq2.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{8.0}, dvec3{1.0}));
        auto doubleSequence2 = std::make_unique<KeyframeSequenceTyped<ValueKeyframe<dvec3>>>(
            std::move(dseq2), std::make_unique<LinearInterpolation<ValueKeyframe<dvec3>>>());

        animation[1].add(std::move(doubleSequence2));
    }

    EXPECT_EQ(Seconds{1.0}, animation.getFirstTime());
    EXPECT_EQ(Seconds{8.0}, animation.getLastTime());

    animation(Seconds{0.0}, Seconds{7.5}, AnimationState::Playing);

    EXPECT_EQ(0.0f, floatProperty.get());
    EXPECT_EQ(dvec3(0.5), doubleProperty.get());
}

TEST(AnimationTests, ControlTrackTest) {
    Animation animation;
    {
        auto controlTrack = std::make_unique<ControlTrack>();
        std::vector<std::unique_ptr<ControlKeyframe>> controlKeys;
        controlKeys.push_back(std::make_unique<ControlKeyframe>(Seconds(1), ControlAction::Pause));
        controlKeys.push_back(
            std::make_unique<ControlKeyframe>(Seconds(2), ControlAction::Jump, Seconds(5)));
        controlKeys.push_back(std::make_unique<ControlKeyframe>(Seconds(8), ControlAction::Pause));
        auto controlSequence = std::make_unique<ControlKeyframeSequence>(std::move(controlKeys));
        controlTrack->add(std::move(controlSequence));
        animation.add(std::move(controlTrack));
    }
    {
        auto state1 = animation(Seconds{0.0}, Seconds{2.0}, AnimationState::Playing);
        EXPECT_EQ(AnimationState::Paused, state1.state);
        EXPECT_EQ(Seconds{1.0}, state1.time);

        auto state2 = animation(Seconds{1.0}, Seconds{2.0}, AnimationState::Playing);
        // Starting at control keyframe should not trigger it
        EXPECT_EQ(AnimationState::Playing, state2.state);
        EXPECT_EQ(Seconds{5}, state2.time);

        auto state3 = animation(Seconds{1.0}, animation.getLastTime(), AnimationState::Playing);
        EXPECT_EQ(AnimationState::Paused, state3.state);
        EXPECT_EQ(animation.getLastTime(), state3.time);
    }
    {
        // Reverse
        auto state1 = animation(Seconds{2.0}, Seconds{0.0}, AnimationState::Playing);
        EXPECT_EQ(AnimationState::Paused, state1.state);
        EXPECT_EQ(Seconds{1}, state1.time);

        auto state2 = animation(Seconds{2.0}, Seconds{1.0}, AnimationState::Playing);
        EXPECT_EQ(AnimationState::Paused, state2.state);
        EXPECT_EQ(Seconds{1}, state2.time);
        // Time-jump to 5 seconds, skipping the pause-control keyframe at 1
        // This behaviour might be debatable, but taking time-jumping into consideration makes
        // evaluation complex
        auto state3 = animation(Seconds(8), Seconds{0.0}, AnimationState::Playing);
        EXPECT_EQ(AnimationState::Playing, state3.state);
        EXPECT_EQ(Seconds(5), state3.time);
    }
}

TEST(AnimationTests, CallbackTrackTest) {
    Animation animation;
    bool calledForward = false;
    bool calledBackward = false;

    {
        auto track = std::make_unique<CallbackTrack>();
        std::vector<std::unique_ptr<CallbackKeyframe>> keys;
        keys.push_back(std::make_unique<CallbackKeyframe>(
            Seconds(0), [&calledForward]() { calledForward = true; },
            [&calledBackward]() { calledBackward = true; }));
        auto seq = std::make_unique<CallbackKeyframeSequence>(std::move(keys));
        track->add(std::move(seq));
        animation.add(std::move(track));
    }
    animation(Seconds{0.0}, Seconds{1.0}, AnimationState::Playing);
    EXPECT_EQ(true, calledForward);
    EXPECT_EQ(false, calledBackward);

    calledForward = false;
    animation(Seconds{1.0}, Seconds{0.0}, AnimationState::Playing);
    EXPECT_EQ(false, calledForward);
    EXPECT_EQ(true, calledBackward);
}

TEST(AnimationTests, KeyframeSerializationTest) {
    ValueKeyframe<dvec3> keyframe{Seconds{4.0}, dvec3(2.0)};

    const std::filesystem::path refPath = "/tmp";

    Serializer s(refPath);
    keyframe.serialize(s);

    std::stringstream ss;
    s.writeFile(ss);

    Deserializer d(ss, refPath);
    ValueKeyframe<dvec3> keyframe2;
    keyframe2.deserialize(d);

    EXPECT_EQ(keyframe, keyframe2);
}

TEST(AnimationTests, InterpolationSerializationTest) {
    InterpolationFactory factory;

    InterpolationFactoryObjectTemplate<LinearInterpolation<ValueKeyframe<dvec3>>> linearIFO;
    factory.registerObject(&linearIFO);

    const std::filesystem::path refPath = "/tmp";

    LinearInterpolation<ValueKeyframe<dvec3>> linear;

    Interpolation* iptr = &linear;

    Serializer s(refPath);
    s.serialize("interpolation", iptr);

    std::stringstream ss;
    s.writeFile(ss);

    Deserializer d(ss, refPath);
    d.setExceptionHandler([](SourceContext context) { throw; });
    d.registerFactory(&factory);

    std::unique_ptr<Interpolation> iptr2;
    d.deserialize("interpolation", iptr2);

    factory.unRegisterObject(&linearIFO);
}

TEST(AnimationTests, KeyframeSequenceSerializationTest) {
    InterpolationFactory factory;

    InterpolationFactoryObjectTemplate<LinearInterpolation<ValueKeyframe<dvec3>>> linearIFO;
    factory.registerObject(&linearIFO);

    std::vector<std::unique_ptr<ValueKeyframe<dvec3>>> dseq;
    dseq.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{1.0}, dvec3{1.0}));
    dseq.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{2.0}, dvec3{0.0}));
    dseq.push_back(std::make_unique<ValueKeyframe<dvec3>>(Seconds{3.0}, dvec3{0.0}));
    KeyframeSequenceTyped<ValueKeyframe<dvec3>> doubleSequence{
        std::move(dseq), std::make_unique<LinearInterpolation<ValueKeyframe<dvec3>>>()};

    const std::filesystem::path refPath = "/tmp";

    Serializer s(refPath);
    doubleSequence.serialize(s);

    std::stringstream ss;
    s.writeFile(ss);

    Deserializer d(ss, refPath);
    d.setExceptionHandler([](SourceContext context) { throw; });
    d.registerFactory(&factory);

    KeyframeSequenceTyped<ValueKeyframe<dvec3>> doubleSequence2(
        {}, std::make_unique<LinearInterpolation<ValueKeyframe<dvec3>>>());

    doubleSequence2.deserialize(d);

    EXPECT_EQ(doubleSequence, doubleSequence2);

    factory.unRegisterObject(&linearIFO);
}

struct TestProcessor : Processor {
    inline static const ProcessorInfo processorInfo_{
        "org.inviwo.animation.TestProcessor",  // Class identifier
        "TestProcessor",                       // Display name
        "Test",                                // Category
        CodeState::Stable,                     // Code state
        Tags::CPU,                             // Tags
    };

    virtual const ProcessorInfo& getProcessorInfo() const override { return processorInfo_; }

    TestProcessor(std::string_view id, std::string_view name) : Processor{id, name} {
        addProperty(floatProperty);
    }

    FloatProperty floatProperty{"float", "Float", 0.0f, 0.0f, 1.0f};
};

TEST(AnimationTests, TrackSerializationTest) {
    const std::string_view refPath = "/tmp";
    std::stringstream ss;

    InterpolationFactory interpolationFactory;
    InterpolationFactoryObjectTemplate<LinearInterpolation<ValueKeyframe<float>>> linearIFO;
    interpolationFactory.registerObject(&linearIFO);

    PropertyFactory propertyFactory;
    PropertyFactoryObjectTemplate<FloatProperty> floatPFO;
    propertyFactory.registerObject(&floatPFO);

    ProcessorFactory processorFactory{nullptr};
    ProcessorFactoryObjectTemplate<TestProcessor> testPFO;
    processorFactory.registerObject(&testPFO);

    {
        ProcessorNetwork net{nullptr};

        TrackFactory trackFactory{&net};
        TrackFactoryObjectTemplate<PropertyTrack<FloatProperty, ValueKeyframe<float>>> floatTFO;
        trackFactory.registerObject(&floatTFO);

        auto floatProcessor = static_cast<TestProcessor*>(
            net.addProcessor(std::make_unique<TestProcessor>("float", "Float")));
        PropertyTrack<FloatProperty, ValueKeyframe<float>> floatTrack(
            &(floatProcessor->floatProperty));

        std::vector<std::unique_ptr<ValueKeyframe<float>>> fseq;
        fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{1.0}, 0.0f));
        fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{2.0}, 1.0f));
        fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{3.0}, 0.0f));
        auto floatSequence = std::make_unique<KeyframeSequenceTyped<ValueKeyframe<float>>>(
            std::move(fseq), std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());
        floatTrack.add(std::move(floatSequence));

        Serializer s(refPath);
        s.serialize("Network", net);
        s.serialize("Track", floatTrack);
        s.writeFile(ss);

        trackFactory.unRegisterObject(&floatTFO);
    }
    {
        ProcessorNetwork net{nullptr};
        TrackFactory trackFactory{&net};
        TrackFactoryObjectTemplate<PropertyTrack<FloatProperty, ValueKeyframe<float>>> floatTFO;
        trackFactory.registerObject(&floatTFO);

        Deserializer d(ss, refPath);
        d.setExceptionHandler([](SourceContext context) { throw; });
        d.registerFactory(&interpolationFactory);
        d.registerFactory(&propertyFactory);
        d.registerFactory(&processorFactory);
        d.registerFactory(&trackFactory);

        std::unique_ptr<Track> track;

        d.deserialize("Network", net);
        d.deserialize("Track", track);

        auto floatTrack2 =
            dynamic_cast<PropertyTrack<FloatProperty, ValueKeyframe<float>>*>(track.get());
        EXPECT_NE(nullptr, floatTrack2);

        auto& floatProperty = net.getProcessorsByType<TestProcessor>().front()->floatProperty;

        EXPECT_EQ(*floatProperty, 0.0f);
        (*floatTrack2)(Seconds{0.0}, Seconds{2.0}, AnimationState::Playing);
        EXPECT_EQ(*floatProperty, 1.0f);
        (*floatTrack2)(Seconds{2.0}, Seconds{3.0}, AnimationState::Playing);
        EXPECT_EQ(*floatProperty, 0.0f);

        trackFactory.unRegisterObject(&floatTFO);
    }
    interpolationFactory.unRegisterObject(&linearIFO);
    propertyFactory.unRegisterObject(&floatPFO);
    processorFactory.unRegisterObject(&testPFO);
}

TEST(AnimationTests, AnimationSerializationTest) {
    InterpolationFactory interpolationFactory;
    InterpolationFactoryObjectTemplate<LinearInterpolation<ValueKeyframe<float>>> linearFloatIFO;
    interpolationFactory.registerObject(&linearFloatIFO);

    PropertyFactory propertyFactory;
    PropertyFactoryObjectTemplate<FloatProperty> floatPFO;
    propertyFactory.registerObject(&floatPFO);

    ProcessorFactory processorFactory{nullptr};
    ProcessorFactoryObjectTemplate<TestProcessor> testPFO;
    processorFactory.registerObject(&testPFO);

    const std::filesystem::path refPath = "/tmp";
    std::stringstream ss;
    ProcessorNetwork net{nullptr};
    Animation animation;

    {
        TrackFactory trackFactory{&net};
        TrackFactoryObjectTemplate<PropertyTrack<FloatProperty, ValueKeyframe<float>>> floatTFO;
        trackFactory.registerObject(&floatTFO);

        auto floatProcessor = static_cast<TestProcessor*>(
            net.addProcessor(std::make_unique<TestProcessor>("float", "Float")));

        std::vector<std::unique_ptr<ValueKeyframe<float>>> fseq;
        fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{1.0}, 0.0f));
        fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{2.0}, 1.0f));
        fseq.push_back(std::make_unique<ValueKeyframe<float>>(Seconds{3.0}, 0.0f));
        auto floatSequence = std::make_unique<KeyframeSequenceTyped<ValueKeyframe<float>>>(
            std::move(fseq), std::make_unique<LinearInterpolation<ValueKeyframe<float>>>());

        {
            auto floatTrack = std::make_unique<PropertyTrack<FloatProperty, ValueKeyframe<float>>>(
                &(floatProcessor->floatProperty));
            floatTrack->add(std::move(floatSequence));
            animation.add(std::move(floatTrack));
        }

        Serializer s(refPath);
        s.serialize("Network", net);
        s.serialize("animation", animation);
        s.writeFile(ss);

        trackFactory.unRegisterObject(&floatTFO);
    }

    ProcessorNetwork net2{nullptr};
    Animation animation2;

    {

        TrackFactory trackFactory{&net2};
        TrackFactoryObjectTemplate<PropertyTrack<FloatProperty, ValueKeyframe<float>>> floatTFO;
        trackFactory.registerObject(&floatTFO);

        Deserializer d(ss, refPath);
        d.setExceptionHandler([](SourceContext context) { throw; });
        d.registerFactory(&interpolationFactory);
        d.registerFactory(&propertyFactory);
        d.registerFactory(&trackFactory);
        d.registerFactory(&processorFactory);

        d.deserialize("Network", net2);
        d.deserialize("animation", animation2);

        trackFactory.unRegisterObject(&floatTFO);
    }

    const auto& ft1 =
        static_cast<PropertyTrack<FloatProperty, ValueKeyframe<float>>&>(animation[0]);
    const auto& ft2 =
        static_cast<PropertyTrack<FloatProperty, ValueKeyframe<float>>&>(animation2[0]);

    EXPECT_EQ(ft1[0], ft2[0]);

    Seconds start = animation.getFirstTime();
    Seconds end = animation.getLastTime();

    auto& floatProperty = net.getProcessorsByType<TestProcessor>().front()->floatProperty;
    auto& floatProperty2 = net2.getProcessorsByType<TestProcessor>().front()->floatProperty;

    Seconds from = start;
    for (Seconds to = start; to <= end; to += (end - start) / 100) {
        animation(from, to, AnimationState::Playing);
        animation2(from, to, AnimationState::Playing);

        auto oldfloat = floatProperty.get();
        auto newfloat = floatProperty2.get();
        EXPECT_EQ(oldfloat, newfloat);

        from = to;
    }

    interpolationFactory.unRegisterObject(&linearFloatIFO);
    propertyFactory.unRegisterObject(&floatPFO);
    processorFactory.unRegisterObject(&testPFO);
}

}  // namespace animation

}  // namespace inviwo
