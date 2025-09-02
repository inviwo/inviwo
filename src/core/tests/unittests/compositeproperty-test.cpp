/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <gmock/gmock.h>
#include <warn/pop>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

struct MockCompositePropertyObserver : CompositePropertyObserver {
    MOCK_METHOD(void, onSetCollapsed, (bool value), (override));
};

struct MockPropertyOwnerObserver : PropertyOwnerObserver {
    MOCK_METHOD(void, onWillAddProperty, (PropertyOwner * owner, Property* property, size_t index),
                (override));
    MOCK_METHOD(void, onDidAddProperty, (Property * property, size_t index), (override));


    MOCK_METHOD(void, onWillRemoveProperty, (Property * property, size_t index), (override));
    MOCK_METHOD(void, onDidRemoveProperty,
                (PropertyOwner * owner, Property* property, size_t index), (override));
};

TEST(CompositeProperty, Collapse) {

    CompositeProperty org{"org", "name", "help"_help};

    MockCompositePropertyObserver obs1;
    org.CompositePropertyObservable::addObserver(&obs1);

    EXPECT_CALL(obs1, onSetCollapsed(true)).Times(1);
    EXPECT_CALL(obs1, onSetCollapsed(false)).Times(1);
    org.setCollapsed(true);
    org.setCollapsed(true);
    org.setCollapsed(false);
    org.setCollapsed(false);

    CompositeProperty copy{org};

    MockCompositePropertyObserver obs2;
    copy.CompositePropertyObservable::addObserver(&obs2);

    EXPECT_CALL(obs2, onSetCollapsed).Times(0);
    EXPECT_CALL(obs2, onSetCollapsed(true)).Times(1);
    EXPECT_CALL(obs2, onSetCollapsed(false)).Times(1);
    copy.setCollapsed(true);
    copy.setCollapsed(true);
    copy.setCollapsed(false);
    copy.setCollapsed(false);
}

TEST(CompositeProperty, addProperty) {

    CompositeProperty org{"org", "name", "help"_help};

    FloatProperty f1{"f1", "f1"};

    MockPropertyOwnerObserver obs1;
    org.PropertyOwnerObservable::addObserver(&obs1);

    EXPECT_CALL(obs1, onWillAddProperty).Times(1);
    EXPECT_CALL(obs1, onDidAddProperty).Times(1);

    org.addProperty(f1);
}

TEST(CompositeProperty, ObserveMove) {

    MockPropertyOwnerObserver obs1;

    EXPECT_CALL(obs1, onWillAddProperty).Times(1);
    EXPECT_CALL(obs1, onDidAddProperty).Times(1);
    EXPECT_CALL(obs1, onWillRemoveProperty).Times(1);
    EXPECT_CALL(obs1, onDidRemoveProperty).Times(1);

    FloatProperty f1{"f1", "f1"};
    auto copy = [&]() {
        CompositeProperty org{"org", "name", "help"_help};
        org.PropertyOwnerObservable::addObserver(&obs1);
        org.addProperty(f1);
        return static_cast<CompositeProperty&&>(org);
    }();

    // The copy will only copy "owned" properties
    EXPECT_EQ(copy.size(), 0);
}

TEST(CompositeProperty, ObserveMoveOwned) {

    MockPropertyOwnerObserver obs1;

    const ::testing::InSequence dummy;
    EXPECT_CALL(obs1, onWillAddProperty).Times(1);
    EXPECT_CALL(obs1, onDidAddProperty).Times(1);
    EXPECT_CALL(obs1, onWillRemoveProperty).Times(1);
    EXPECT_CALL(obs1, onDidRemoveProperty).Times(1);

    auto copy = [&]() {
        CompositeProperty org{"org", "name", "help"_help};
        org.PropertyOwnerObservable::addObserver(&obs1);
        org.addProperty(std::make_unique<FloatProperty>("f1", "f1"));

        EXPECT_TRUE(org.PropertyOwnerObservable::isObservedBy(&obs1));

        return static_cast<CompositeProperty&&>(org);
    }();

    ASSERT_EQ(copy.size(), 1);
    EXPECT_EQ(copy.getProperties()[0]->getIdentifier(), "f1");
    EXPECT_EQ(copy.getProperties()[0]->getOwner(), &copy);

    EXPECT_TRUE(copy.PropertyOwnerObservable::isObservedBy(&obs1));
}

}  // namespace inviwo
