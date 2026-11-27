/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationfactorymanager.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <cmath>
#include <memory>
#include <typeindex>

namespace inviwo {

namespace {

class TestData;

class TestRepr : public DataRepresentation<TestData> {
public:
    virtual TestRepr* clone() const override = 0;
    double value = 0.0;
};

template <int N>
class TestReprN : public TestRepr {
public:
    explicit TestReprN(double v = 0.0) { value = v; }
    virtual TestReprN* clone() const override { return new TestReprN(*this); }
    virtual std::type_index getTypeIndex() const override { return typeid(TestReprN); }
};

// A: "RAM" like source, B: "GL" like, can be lossy, C: derived from B ("CLGL" like)
using ReprA = TestReprN<0>;
using ReprB = TestReprN<1>;
using ReprC = TestReprN<2>;

class TestData : public Data<TestData, TestRepr> {
public:
    explicit TestData(double v) { addRepresentation(std::make_shared<ReprA>(v)); }
    TestData(const TestData&) = default;
    virtual TestData* clone() const override { return new TestData(*this); }

    std::type_index lastValidType() const {
        return getLastOr([](const TestRepr& r) { return r.getTypeIndex(); },
                         std::type_index{typeid(void)});
    }

    bool lossyB = false;
};

template <typename From, typename To>
class Conv : public RepresentationConverterType<TestRepr, From, To> {
public:
    virtual std::shared_ptr<To> createFrom(std::shared_ptr<const From> src) const override {
        auto dst = std::make_shared<To>();
        update(src, dst);
        return dst;
    }
    virtual void update(std::shared_ptr<const From> src, std::shared_ptr<To> dst) const override {
        if constexpr (std::is_same_v<To, ReprB>) {
            const bool lossy = src->getOwner()->lossyB;
            dst->value = lossy ? std::round(src->value) : src->value;
            dst->setLossy(lossy);
        } else {
            dst->value = src->value;
        }
    }
};

class DataLossyTest : public ::testing::Test {
protected:
    DataLossyTest() {
        rfm.registerRepresentationConverterFactory(
            std::make_unique<RepresentationConverterFactory<TestRepr>>());
        rfm.registerRepresentationConverter<TestRepr>(std::make_unique<Conv<ReprA, ReprB>>());
        rfm.registerRepresentationConverter<TestRepr>(std::make_unique<Conv<ReprB, ReprA>>());
        rfm.registerRepresentationConverter<TestRepr>(std::make_unique<Conv<ReprB, ReprC>>());
        rfm.registerRepresentationConverter<TestRepr>(std::make_unique<Conv<ReprC, ReprB>>());
    }

    RepresentationFactoryManager rfm;
};

}  // namespace

TEST_F(DataLossyTest, LosslessConversionBecomesLastValid) {
    TestData data{1.4};
    const auto* b = data.getRepresentation<ReprB>();
    EXPECT_FALSE(b->isLossy());
    EXPECT_DOUBLE_EQ(b->value, 1.4);
    EXPECT_EQ(data.lastValidType(), std::type_index{typeid(ReprB)});
}

TEST_F(DataLossyTest, LossyRepresentationIsNeverSource) {
    TestData data{1.4};
    data.lossyB = true;

    const auto* b = data.getRepresentation<ReprB>();
    EXPECT_TRUE(b->isValid());
    EXPECT_TRUE(b->isLossy());
    EXPECT_DOUBLE_EQ(b->value, 1.0);
    EXPECT_EQ(data.lastValidType(), std::type_index{typeid(ReprA)});

    // Requesting the valid lossy representation again does not change the source
    data.getRepresentation<ReprB>();
    EXPECT_EQ(data.lastValidType(), std::type_index{typeid(ReprA)});

    // Anything derived from a lossy representation is lossy as well
    const auto* c = data.getRepresentation<ReprC>();
    EXPECT_TRUE(c->isLossy());
    EXPECT_DOUBLE_EQ(c->value, 1.0);
    EXPECT_EQ(data.lastValidType(), std::type_index{typeid(ReprA)});

    // A is still the full precision source
    EXPECT_DOUBLE_EQ(data.getRepresentation<ReprA>()->value, 1.4);

    data.getEditableRepresentation<ReprA>()->value = 2.6;
    EXPECT_FALSE(b->isValid());
    EXPECT_FALSE(c->isValid());
    EXPECT_DOUBLE_EQ(data.getRepresentation<ReprB>()->value, 3.0);
    EXPECT_DOUBLE_EQ(data.getRepresentation<ReprA>()->value, 2.6);
}

TEST_F(DataLossyTest, EditingLossyRepresentationPromotesIt) {
    TestData data{1.4};
    data.lossyB = true;
    data.getRepresentation<ReprB>();

    auto* b = data.getEditableRepresentation<ReprB>();
    EXPECT_FALSE(b->isLossy());
    EXPECT_EQ(data.lastValidType(), std::type_index{typeid(ReprB)});
    EXPECT_DOUBLE_EQ(data.getRepresentation<ReprA>()->value, 1.0);
}

TEST_F(DataLossyTest, RemovingSourcePromotesLossyRepresentation) {
    TestData data{1.4};
    data.lossyB = true;
    const auto* a = data.getRepresentation<ReprA>();
    const auto* b = data.getRepresentation<ReprB>();

    data.removeRepresentation(a);
    EXPECT_FALSE(b->isLossy());
    EXPECT_EQ(data.lastValidType(), std::type_index{typeid(ReprB)});
}

TEST_F(DataLossyTest, CopyUsesSourceRepresentation) {
    TestData data{1.4};
    data.lossyB = true;
    data.getRepresentation<ReprB>();

    const TestData copy{data};
    EXPECT_EQ(copy.lastValidType(), std::type_index{typeid(ReprA)});
    EXPECT_DOUBLE_EQ(copy.getRepresentation<ReprA>()->value, 1.4);
}

}  // namespace inviwo
