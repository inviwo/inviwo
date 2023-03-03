/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <vector>
#include <memory>

namespace inviwo {

TEST(SerializerPolymorphicTest, NullUniquePtr) {

    std::unique_ptr<Property> prop1 = std::make_unique<FloatProperty>("float1", "float1", 0.123f);
    prop1->setSerializationMode(PropertySerializationMode::All);

    std::stringstream ss;
    Serializer s("");

    s.serialize("prop", prop1);
    s.writeFile(ss);

    std::unique_ptr<Property> prop2;

    Deserializer d(ss, "");
    d.registerFactory(InviwoApplication::getPtr()->getPropertyFactory());

    d.deserialize("prop", prop2);

    EXPECT_EQ(prop1->getClassIdentifier(), prop2->getClassIdentifier());
    EXPECT_EQ(prop1->getIdentifier(), prop2->getIdentifier());

    auto fprop1 = dynamic_cast<FloatProperty*>(prop1.get());
    EXPECT_TRUE(fprop1);
    auto fprop2 = dynamic_cast<FloatProperty*>(prop2.get());
    EXPECT_TRUE(fprop2);

    EXPECT_EQ(fprop1->get(), fprop2->get());
}

TEST(SerializerPolymorphicTest, NonNullUniquePtr) {
    std::unique_ptr<Property> prop1 = std::make_unique<FloatProperty>("float1", "float1", 0.123f);
    prop1->setSerializationMode(PropertySerializationMode::All);

    std::stringstream ss;
    Serializer s("");

    s.serialize("prop", prop1);
    s.writeFile(ss);

    std::unique_ptr<Property> prop2 = std::make_unique<FloatProperty>("float2", "float2", 0.234f);
    ;

    Deserializer d(ss, "");
    d.registerFactory(InviwoApplication::getPtr()->getPropertyFactory());

    d.deserialize("prop", prop2);

    EXPECT_EQ(prop1->getClassIdentifier(), prop2->getClassIdentifier());
    EXPECT_EQ(prop1->getIdentifier(), prop2->getIdentifier());

    auto fprop1 = dynamic_cast<FloatProperty*>(prop1.get());
    EXPECT_TRUE(fprop1);
    auto fprop2 = dynamic_cast<FloatProperty*>(prop2.get());
    EXPECT_TRUE(fprop2);

    EXPECT_EQ(fprop1->get(), fprop2->get());
}

TEST(SerializerPolymorphicTest, DifferentTypeUniquePtr) {
    std::unique_ptr<Property> prop1 = std::make_unique<FloatProperty>("float1", "float1", 0.123f);
    prop1->setSerializationMode(PropertySerializationMode::All);

    std::stringstream ss;
    Serializer s("");

    s.serialize("prop", prop1);
    s.writeFile(ss);

    std::unique_ptr<Property> prop2 =
        std::make_unique<DoubleProperty>("double2", "double2", 0.234f);
    ;

    Deserializer d(ss, "");
    d.registerFactory(InviwoApplication::getPtr()->getPropertyFactory());

    d.deserialize("prop", prop2);

    EXPECT_EQ(prop1->getClassIdentifier(), prop2->getClassIdentifier());
    EXPECT_EQ(prop1->getIdentifier(), prop2->getIdentifier());

    auto fprop1 = dynamic_cast<FloatProperty*>(prop1.get());
    EXPECT_TRUE(fprop1);
    auto fprop2 = dynamic_cast<FloatProperty*>(prop2.get());
    EXPECT_TRUE(fprop2);

    EXPECT_EQ(fprop1->get(), fprop2->get());
}

TEST(SerializerPolymorphicTest, NullVectorUniquePtr) {
    std::vector<std::unique_ptr<Property>> props1;
    props1.emplace_back(std::make_unique<FloatProperty>("float1", "float1", 0.123f));
    props1.emplace_back(std::make_unique<DoubleProperty>("double2", "double2", 0.234));

    props1[0]->setSerializationMode(PropertySerializationMode::All);
    props1[1]->setSerializationMode(PropertySerializationMode::All);

    std::stringstream ss;
    Serializer s("");

    s.serialize("props", props1);
    s.writeFile(ss);

    std::vector<std::unique_ptr<Property>> props2;

    Deserializer d(ss, "");
    d.registerFactory(InviwoApplication::getPtr()->getPropertyFactory());

    d.deserialize("props", props2);

    EXPECT_EQ(props2[0]->getClassIdentifier(), PropertyTraits<FloatProperty>::classIdentifier());
    EXPECT_EQ(props2[1]->getClassIdentifier(), PropertyTraits<DoubleProperty>::classIdentifier());

    EXPECT_EQ(props2[0]->getIdentifier(), "float1");
    EXPECT_EQ(props2[1]->getIdentifier(), "double2");

    auto fprop = dynamic_cast<FloatProperty*>(props2[0].get());
    EXPECT_TRUE(fprop);
    EXPECT_EQ(fprop->get(), 0.123f);

    auto dprop = dynamic_cast<DoubleProperty*>(props2[1].get());
    EXPECT_TRUE(dprop);
    EXPECT_EQ(dprop->get(), 0.234);
}

TEST(SerializerPolymorphicTest, NonNullVectorUniquePtr) {
    std::vector<std::unique_ptr<Property>> props1;
    props1.emplace_back(std::make_unique<FloatProperty>("float1", "float1", 0.123f));
    props1.emplace_back(std::make_unique<DoubleProperty>("double2", "double2", 0.234));

    props1[0]->setSerializationMode(PropertySerializationMode::All);
    props1[1]->setSerializationMode(PropertySerializationMode::All);

    std::stringstream ss;
    Serializer s("");

    s.serialize("props", props1);
    s.writeFile(ss);

    std::vector<std::unique_ptr<Property>> props2;
    props2.emplace_back(std::make_unique<FloatProperty>("float1", "float1", 0.0123f));
    props2.emplace_back(std::make_unique<DoubleProperty>("double2", "double2", 0.0234));

    Deserializer d(ss, "");
    d.registerFactory(InviwoApplication::getPtr()->getPropertyFactory());

    d.deserialize("props", props2);

    EXPECT_EQ(props2[0]->getClassIdentifier(), PropertyTraits<FloatProperty>::classIdentifier());
    EXPECT_EQ(props2[1]->getClassIdentifier(), PropertyTraits<DoubleProperty>::classIdentifier());

    EXPECT_EQ(props2[0]->getIdentifier(), "float1");
    EXPECT_EQ(props2[1]->getIdentifier(), "double2");

    auto fprop = dynamic_cast<FloatProperty*>(props2[0].get());
    EXPECT_TRUE(fprop);
    EXPECT_EQ(fprop->get(), 0.123f);

    auto dprop = dynamic_cast<DoubleProperty*>(props2[1].get());
    EXPECT_TRUE(dprop);
    EXPECT_EQ(dprop->get(), 0.234);
}

TEST(SerializerPolymorphicTest, DifferentTypeVectorUniquePtr) {
    std::vector<std::unique_ptr<Property>> props1;
    props1.emplace_back(std::make_unique<FloatProperty>("float1", "float1", 0.123f));
    props1.emplace_back(std::make_unique<DoubleProperty>("double2", "double2", 0.234));

    props1[0]->setSerializationMode(PropertySerializationMode::All);
    props1[1]->setSerializationMode(PropertySerializationMode::All);

    std::stringstream ss;
    Serializer s("");

    s.serialize("props", props1);
    s.writeFile(ss);

    std::vector<std::unique_ptr<Property>> props2;
    props2.emplace_back(std::make_unique<IntProperty>("int1", "int1", 1));
    props2.emplace_back(std::make_unique<IntProperty>("int2", "int2", 2));

    Deserializer d(ss, "");
    d.registerFactory(InviwoApplication::getPtr()->getPropertyFactory());

    d.deserialize("props", props2);

    EXPECT_EQ(props2[0]->getClassIdentifier(), PropertyTraits<FloatProperty>::classIdentifier());
    EXPECT_EQ(props2[1]->getClassIdentifier(), PropertyTraits<DoubleProperty>::classIdentifier());

    EXPECT_EQ(props2[0]->getIdentifier(), "float1");
    EXPECT_EQ(props2[1]->getIdentifier(), "double2");

    auto fprop = dynamic_cast<FloatProperty*>(props2[0].get());
    EXPECT_TRUE(fprop);
    EXPECT_EQ(fprop->get(), 0.123f);

    auto dprop = dynamic_cast<DoubleProperty*>(props2[1].get());
    EXPECT_TRUE(dprop);
    EXPECT_EQ(dprop->get(), 0.234);
}

}  // namespace inviwo
