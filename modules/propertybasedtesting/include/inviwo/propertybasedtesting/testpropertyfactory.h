/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>

#include <inviwo/propertybasedtesting/testproperty.h>

#include <memory>
#include <vector>

namespace inviwo {

namespace pbt {

class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyFactoryObject {
public:
    TestPropertyFactoryObject(const std::string& classIdentifier);
    virtual ~TestPropertyFactoryObject() = default;

    virtual std::unique_ptr<TestProperty> create() const = 0;
    const std::string& getClassIdentifier() const;

protected:
    const std::string classIdentifier_;
};

template <typename T>
class TestPropertyFactoryObjectTemplate : public TestPropertyFactoryObject {
public:
    TestPropertyFactoryObjectTemplate() : TestPropertyFactoryObject(T::classIdentifier()) {}
    virtual ~TestPropertyFactoryObjectTemplate() = default;

    virtual std::unique_ptr<TestProperty> create() const override;
};

class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyFactory
    : public StandardFactory<TestProperty, TestPropertyFactoryObject> {
    std::vector<std::unique_ptr<TestPropertyFactoryObject>> factoryObjects_;

public:
    TestPropertyFactory();
    virtual ~TestPropertyFactory() = default;
};

class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyCompositeFactoryObject {
public:
    TestPropertyCompositeFactoryObject() = default;
    virtual ~TestPropertyCompositeFactoryObject() = default;

    virtual std::unique_ptr<TestPropertyComposite> create() const;
    const std::string& getClassIdentifier() const;
};

class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyCompositeFactory
    : public StandardFactory<TestPropertyComposite, TestPropertyCompositeFactoryObject> {
    std::unique_ptr<TestPropertyCompositeFactoryObject> obj_;

public:
    TestPropertyCompositeFactory();
    virtual ~TestPropertyCompositeFactory() = default;
};

}  // namespace pbt

}  // namespace inviwo
