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

#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <warn/pop>

#include <string>

namespace inviwo {

using ::testing::ElementsAre;

TEST(OptionProperty, ConstructInt) {

    const OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};

    EXPECT_EQ(prop.size(), 5);
    EXPECT_EQ(prop.getSelectedValue(), 2);
    EXPECT_EQ(prop.getSelectedIndex(), 1);
    EXPECT_EQ(prop.getSelectedIdentifier(), "2");
    EXPECT_EQ(prop.getSelectedDisplayName(), "2");

    EXPECT_EQ(prop.getOptionIdentifier(2), "3");
    EXPECT_EQ(prop.getOptionDisplayName(2), "3");
    EXPECT_EQ(prop.getOptionValue(2), 3);

    EXPECT_TRUE(prop.isDefaultState());

    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5));
    EXPECT_THAT(prop.getIdentifiers(), ElementsAre("1", "2", "3", "4", "5"));
    EXPECT_THAT(prop.getDisplayNames(), ElementsAre("1", "2", "3", "4", "5"));

    EXPECT_EQ(prop, 2);
    EXPECT_EQ([](const int& val) { return val; }(prop), 2);

    EXPECT_TRUE(prop.isSelectedIndex(1));
    EXPECT_TRUE(prop.isSelectedIdentifier("2"));
    EXPECT_TRUE(prop.isSelectedDisplayName("2"));
    EXPECT_TRUE(prop.isSelectedValue(2));

    EXPECT_FALSE(prop.isSelectedIndex(2));
    EXPECT_FALSE(prop.isSelectedIdentifier("3"));
    EXPECT_FALSE(prop.isSelectedDisplayName("3"));
    EXPECT_FALSE(prop.isSelectedValue(3));
}

TEST(OptionProperty, ConstructString) {

    const OptionProperty<std::string> prop{"test", "test", {"a", "b", "c", "d", "e"}, 1};

    EXPECT_EQ(prop.size(), 5);
    EXPECT_EQ(prop.getSelectedValue(), "b");
    EXPECT_EQ(prop.getSelectedIndex(), 1);
    EXPECT_EQ(prop.getSelectedIdentifier(), "b");
    EXPECT_EQ(prop.getSelectedDisplayName(), "B");

    EXPECT_EQ(prop.getOptionIdentifier(2), "c");
    EXPECT_EQ(prop.getOptionDisplayName(2), "C");
    EXPECT_EQ(prop.getOptionValue(2), "c");

    EXPECT_TRUE(prop.isDefaultState());

    EXPECT_THAT(prop.getValues(), ElementsAre("a", "b", "c", "d", "e"));
    EXPECT_THAT(prop.getIdentifiers(), ElementsAre("a", "b", "c", "d", "e"));
    EXPECT_THAT(prop.getDisplayNames(), ElementsAre("A", "B", "C", "D", "E"));

    EXPECT_EQ(prop, "b");

    EXPECT_EQ([](std::string_view val) { return val; }(prop), "b");
    EXPECT_EQ([](const std::string& val) { return val; }(prop), "b");

    EXPECT_TRUE(prop.isSelectedIndex(1));
    EXPECT_TRUE(prop.isSelectedIdentifier("b"));
    EXPECT_TRUE(prop.isSelectedDisplayName("B"));
    EXPECT_TRUE(prop.isSelectedValue("b"));

    EXPECT_FALSE(prop.isSelectedIndex(2));
    EXPECT_FALSE(prop.isSelectedIdentifier("c"));
    EXPECT_FALSE(prop.isSelectedDisplayName("C"));
    EXPECT_FALSE(prop.isSelectedValue("c"));
}

TEST(OptionProperty, Set) {

    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};
    prop.set(3);

    EXPECT_EQ(prop.getSelectedValue(), 3);
    EXPECT_EQ(prop.getSelectedIndex(), 2);
    EXPECT_EQ(prop.getSelectedIdentifier(), "3");
    EXPECT_EQ(prop.getSelectedDisplayName(), "3");
}

TEST(OptionProperty, SetSelectedValue) {

    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};
    prop.setSelectedValue(3);

    EXPECT_EQ(prop.getSelectedIndex(), 2);
    EXPECT_EQ(prop.getSelectedValue(), 3);
    EXPECT_EQ(prop.getSelectedIdentifier(), "3");
    EXPECT_EQ(prop.getSelectedDisplayName(), "3");
}

TEST(OptionProperty, SetSelectedIndex) {

    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};
    prop.setSelectedIndex(2);

    EXPECT_EQ(prop.getSelectedIndex(), 2);
    EXPECT_EQ(prop.getSelectedValue(), 3);
    EXPECT_EQ(prop.getSelectedIdentifier(), "3");
    EXPECT_EQ(prop.getSelectedDisplayName(), "3");
}

TEST(OptionProperty, SetSelectedIdentifier) {

    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};
    prop.setSelectedIdentifier("3");

    EXPECT_EQ(prop.getSelectedIndex(), 2);
    EXPECT_EQ(prop.getSelectedValue(), 3);
    EXPECT_EQ(prop.getSelectedIdentifier(), "3");
    EXPECT_EQ(prop.getSelectedDisplayName(), "3");
}

TEST(OptionProperty, SetSelectedDisplayName) {

    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};
    prop.setSelectedDisplayName("3");

    EXPECT_EQ(prop.getSelectedIndex(), 2);
    EXPECT_EQ(prop.getSelectedValue(), 3);
    EXPECT_EQ(prop.getSelectedIdentifier(), "3");
    EXPECT_EQ(prop.getSelectedDisplayName(), "3");
}

TEST(OptionProperty, SerializeCopyAll) {

    OptionProperty<int> src{"src", "src", {1, 2, 3, 4, 5}, 1};
    src.setSerializationMode(PropertySerializationMode::All);

    Serializer s{"dummy.inv"};
    s.serialize("Property", src);
    std::pmr::string xml;
    s.write(xml);

    OptionProperty<int> dst{"dst", "dst"};
    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", dst);

    ASSERT_EQ(dst.size(), 5);
    EXPECT_EQ(dst.getSelectedValue(), 2);
    EXPECT_EQ(dst.getSelectedIndex(), 1);
    EXPECT_EQ(dst.getSelectedIdentifier(), "2");
    EXPECT_EQ(dst.getSelectedDisplayName(), "2");
    EXPECT_THAT(dst.getValues(), ElementsAre(1, 2, 3, 4, 5));
    EXPECT_THAT(dst.getIdentifiers(), ElementsAre("1", "2", "3", "4", "5"));
    EXPECT_THAT(dst.getDisplayNames(), ElementsAre("1", "2", "3", "4", "5"));
}

TEST(OptionProperty, SerializeCopyDefault) {
    OptionProperty<int> src{"src", "src", {1, 2, 3, 4, 5}, 1};

    Serializer s{"dummy.inv"};
    s.serialize("Property", src);
    std::pmr::string xml;
    s.write(xml);

    OptionProperty<int> dst{"dst", "dst"};
    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", dst);

    ASSERT_EQ(dst.size(), 0);
}

TEST(OptionProperty, SerializeCopyNone) {
    OptionProperty<int> src{"src", "src", {1, 2, 3, 4, 5}, 1};
    src.setSerializationMode(PropertySerializationMode::None);

    Serializer s{"dummy.inv"};
    s.serialize("Property", src);
    std::pmr::string xml;
    s.write(xml);

    OptionProperty<int> dst{"dst", "dst"};
    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", dst);

    ASSERT_EQ(dst.size(), 0);
}

TEST(OptionProperty, SerializeClear) {
    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};

    Serializer s{"dummy.inv"};
    s.serialize("Property", prop);
    std::pmr::string xml;
    s.write(xml);

    EXPECT_TRUE(prop.isDefaultState());
    prop.clearOptions();

    EXPECT_FALSE(prop.isDefaultState());
    EXPECT_EQ(prop.size(), 0);

    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", prop);

    EXPECT_TRUE(prop.isDefaultState());

    ASSERT_GT(prop.size(), 0);
    EXPECT_EQ(prop.size(), 5);
    EXPECT_EQ(prop.getSelectedValue(), 2);
    EXPECT_EQ(prop.getSelectedIndex(), 1);
    EXPECT_EQ(prop.getSelectedIdentifier(), "2");
    EXPECT_EQ(prop.getSelectedDisplayName(), "2");
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5));
    EXPECT_THAT(prop.getIdentifiers(), ElementsAre("1", "2", "3", "4", "5"));
    EXPECT_THAT(prop.getDisplayNames(), ElementsAre("1", "2", "3", "4", "5"));
}

TEST(OptionProperty, SerializeSelectedIndex) {
    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};

    prop.setSelectedIndex(4);
    Serializer s{"dummy.inv"};
    s.serialize("Property", prop);
    std::pmr::string xml;
    s.write(xml);

    EXPECT_FALSE(prop.isDefaultState());
    prop.resetToDefaultState();
    EXPECT_TRUE(prop.isDefaultState());

    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", prop);

    EXPECT_FALSE(prop.isDefaultState());

    ASSERT_GT(prop.size(), 0);
    EXPECT_EQ(prop.size(), 5);
    EXPECT_EQ(prop.getSelectedValue(), 5);
    EXPECT_EQ(prop.getSelectedIndex(), 4);
    EXPECT_EQ(prop.getSelectedIdentifier(), "5");
    EXPECT_EQ(prop.getSelectedDisplayName(), "5");
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5));
    EXPECT_THAT(prop.getIdentifiers(), ElementsAre("1", "2", "3", "4", "5"));
    EXPECT_THAT(prop.getDisplayNames(), ElementsAre("1", "2", "3", "4", "5"));
}

TEST(OptionProperty, SerializeOptions1) {
    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};

    prop.addOption(6);
    prop.addOption(7);
    prop.addOption(8);
    EXPECT_EQ(prop.size(), 8);
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));

    Serializer s{"dummy.inv"};
    s.serialize("Property", prop);
    std::pmr::string xml;
    s.write(xml);

    EXPECT_FALSE(prop.isDefaultState());
    prop.resetToDefaultState();
    EXPECT_TRUE(prop.isDefaultState());

    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", prop);

    EXPECT_FALSE(prop.isDefaultState());

    EXPECT_EQ(prop.size(), 8);
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));
}

TEST(OptionProperty, SerializeOptions2) {
    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};

    Serializer s{"dummy.inv"};
    s.serialize("Property", prop);
    std::pmr::string xml;
    s.write(xml);

    prop.addOption(6);
    prop.addOption(7);
    prop.addOption(8);
    EXPECT_EQ(prop.size(), 8);
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));

    EXPECT_FALSE(prop.isDefaultState());

    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", prop);

    EXPECT_TRUE(prop.isDefaultState());

    ASSERT_EQ(prop.size(), 5);
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5));
    EXPECT_EQ(prop.getSelectedValue(), 2);
}

TEST(OptionProperty, SerializeOptions3) {
    OptionProperty<int> prop{"test", "test", {1, 2, 3, 4, 5}, 1};

    prop.addOption(6);
    EXPECT_EQ(prop.size(), 6);

    Serializer s{"dummy.inv"};
    s.serialize("Property", prop);
    std::pmr::string xml;
    s.write(xml);

    prop.addOption(7);
    prop.addOption(8);
    EXPECT_EQ(prop.size(), 8);
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));

    EXPECT_FALSE(prop.isDefaultState());

    Deserializer d{xml, "dummy.inv"};
    d.deserialize("Property", prop);

    EXPECT_FALSE(prop.isDefaultState());

    ASSERT_EQ(prop.size(), 6);
    EXPECT_THAT(prop.getValues(), ElementsAre(1, 2, 3, 4, 5, 6));
    EXPECT_EQ(prop.getSelectedValue(), 2);
}

}  // namespace inviwo
