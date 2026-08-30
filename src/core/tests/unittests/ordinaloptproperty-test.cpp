/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/properties/ordinaloptproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <optional>

namespace inviwo {

TEST(OrdinalOptProperty, DefaultIsEmpty) {
    OrdinalOptProperty<float> prop{"test", "test"};
    EXPECT_FALSE(prop.hasValue());
    EXPECT_EQ(prop.get(), std::nullopt);
}

TEST(OrdinalOptProperty, SetAndClear) {
    OrdinalOptProperty<float> prop{"test",
                                   "test",
                                   std::nullopt,
                                   {-100.0f, ConstraintBehavior::Editable},
                                   {100.0f, ConstraintBehavior::Editable}};

    prop.set(3.0f);
    EXPECT_TRUE(prop.hasValue());
    ASSERT_TRUE(prop.get().has_value());
    EXPECT_EQ(*prop.get(), 3.0f);

    prop.clear();
    EXPECT_FALSE(prop.hasValue());
    EXPECT_EQ(prop.get(), std::nullopt);
}

TEST(OrdinalOptProperty, SetNulloptClears) {
    OrdinalOptProperty<float> prop{"test",
                                   "test",
                                   std::optional<float>{2.0f},
                                   {-100.0f, ConstraintBehavior::Editable},
                                   {100.0f, ConstraintBehavior::Editable}};
    ASSERT_TRUE(prop.hasValue());

    prop.set(std::nullopt);
    EXPECT_FALSE(prop.hasValue());
}

TEST(OrdinalOptProperty, ClampsWhenEngaged) {
    OrdinalOptProperty<float> prop{"test",
                                   "test",
                                   std::nullopt,
                                   {0.0f, ConstraintBehavior::Editable},
                                   {10.0f, ConstraintBehavior::Editable}};
    prop.set(20.0f);
    ASSERT_TRUE(prop.get().has_value());
    EXPECT_EQ(*prop.get(), 10.0f);
}

TEST(OrdinalOptProperty, ComponentSetEngages) {
    OrdinalOptProperty<vec2> prop{"test",
                                  "test",
                                  std::nullopt,
                                  {vec2{-100.0f}, ConstraintBehavior::Editable},
                                  {vec2{100.0f}, ConstraintBehavior::Editable}};
    ASSERT_FALSE(prop.hasValue());

    prop.set(5.0f, 0);
    ASSERT_TRUE(prop.hasValue());
    EXPECT_EQ(prop.get()->x, 5.0f);
}

TEST(OrdinalOptProperty, Linking) {
    OrdinalOptProperty<float> src{"src",
                                  "src",
                                  std::nullopt,
                                  {-100.0f, ConstraintBehavior::Editable},
                                  {100.0f, ConstraintBehavior::Editable}};
    OrdinalOptProperty<float> dst{"dst",
                                  "dst",
                                  std::nullopt,
                                  {-100.0f, ConstraintBehavior::Editable},
                                  {100.0f, ConstraintBehavior::Editable}};

    src.set(4.0f);
    dst.set(static_cast<const Property*>(&src));
    ASSERT_TRUE(dst.get().has_value());
    EXPECT_EQ(*dst.get(), 4.0f);

    src.clear();
    dst.set(static_cast<const Property*>(&src));
    EXPECT_FALSE(dst.hasValue());
}

TEST(OrdinalOptProperty, DefaultState) {
    OrdinalOptProperty<float> prop{"test", "test"};
    EXPECT_TRUE(prop.isDefaultState());

    prop.set(1.0f);
    EXPECT_FALSE(prop.isDefaultState());

    prop.resetToDefaultState();
    EXPECT_TRUE(prop.isDefaultState());
    EXPECT_FALSE(prop.hasValue());
}

TEST(OrdinalOptProperty, SerializeCopyEngaged) {
    OrdinalOptProperty<float> src{"src",
                                  "src",
                                  std::nullopt,
                                  {-100.0f, ConstraintBehavior::Editable},
                                  {100.0f, ConstraintBehavior::Editable}};
    src.set(7.0f);
    src.setSerializationMode(PropertySerializationMode::All);

    Serializer s{};
    s.serialize("Property", src);
    std::pmr::string xml;
    s.write(xml);

    OrdinalOptProperty<float> dst{"dst", "dst"};
    Deserializer d{xml};
    d.deserialize("Property", dst);

    ASSERT_TRUE(dst.get().has_value());
    EXPECT_EQ(*dst.get(), 7.0f);
}

TEST(OrdinalOptProperty, SerializeCopyEmpty) {
    OrdinalOptProperty<float> src{"src", "src"};
    src.setSerializationMode(PropertySerializationMode::All);

    Serializer s{};
    s.serialize("Property", src);
    std::pmr::string xml;
    s.write(xml);

    OrdinalOptProperty<float> dst{"dst",
                                  "dst",
                                  std::optional<float>{9.0f},
                                  {-100.0f, ConstraintBehavior::Editable},
                                  {100.0f, ConstraintBehavior::Editable}};
    Deserializer d{xml};
    d.deserialize("Property", dst);

    EXPECT_FALSE(dst.hasValue());
}

}  // namespace inviwo
