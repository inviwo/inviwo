/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertyfactoryobject.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

namespace {

enum class MyEnumA { a, b };

enum class MyEnumB { a, b };

}  // namespace

template <>
struct EnumTraits<MyEnumB> {
    static std::string name() { return "MyEnumB"; }
};

TEST(EnumOptionProperty, Test1) {
    TemplateOptionProperty<MyEnumA> pA("test", "test");
    TemplateOptionProperty<MyEnumB> pB("test", "test");

    auto idA = pA.getClassIdentifier();
    auto idB = pB.getClassIdentifier();

    EXPECT_NE(idA, idB);

    PropertyFactory factory;
    PropertyFactoryObjectTemplate<TemplateOptionProperty<MyEnumB>> factoryObj;
    factory.registerObject(&factoryObj);

    std::unique_ptr<Property> propA =
        std::make_unique<TemplateOptionProperty<MyEnumB>>("test", "test");
    propA->setSerializationMode(PropertySerializationMode::All);
    std::stringstream ss;
    Serializer s("");
    s.serialize("property", propA);
    s.writeFile(ss);
    Deserializer d(ss, "");
    d.registerFactory(&factory);

    std::unique_ptr<Property> propB;
    d.deserialize("property", propB);

    EXPECT_TRUE(propB != nullptr);
}

}  // namespace inviwo
