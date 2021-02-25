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
#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>

#include <filesystem>

namespace inviwo {

namespace pbt {

class IVW_MODULE_PROPERTYBASEDTESTING_API TestProperty;
class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyComposite;
template <typename T>
class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyTyped;

/*
 * Container for a test result.
 * Allows access to the tested value of each property, as well as the
 * resulting number of pixels.
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API TestResult {
private:
    const std::vector<TestProperty*>& testProperties;
    const Test test;
    const size_t pixels;
    const std::filesystem::path imgPath;

public:
    size_t getNumberOfPixels() { return pixels; }
    const std::filesystem::path& getImagePath() { return imgPath; }

    template <typename T>
    typename T::value_type getValue(const T* prop) const;

    TestResult(const std::vector<TestProperty*>& testProperties, const Test& t, size_t val,
               const std::filesystem::path& imgPath)
        : testProperties(testProperties), test(t), pixels(val), imgPath(imgPath) {}
};

using TestingError = std::tuple<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>,
                                pbt::PropertyEffect, size_t, size_t>;

void IVW_MODULE_PROPERTYBASEDTESTING_API testingErrorToBinary(std::vector<unsigned char>&,
                                                              const std::vector<TestProperty*>&,
                                                              const TestingError&);

template <typename T>
typename T::value_type TestResult::getValue(const T* prop) const {
    for (const auto& t : test) {
        if (auto p = std::dynamic_pointer_cast<PropertyAssignmentTyped<T>>(t);
            p && reinterpret_cast<T*>(p->getProperty()) == prop && !(p->isDeactivated()))
            return p->getValue();
    }

    for (auto def : testProperties) {
        if (auto p = dynamic_cast<TestPropertyTyped<T>*>(def); p != nullptr) {
            if (p->getTypedProperty() == prop) return p->getDefaultValue();
        } else if (auto p = dynamic_cast<TestPropertyComposite*>(def); p != nullptr) {
            if (auto res = p->getDefaultValue(prop); res != std::nullopt) return *res;
        }
    }

    IVW_ASSERT(
        false,
        (std::string("getValue<") + typeid(T).name() + "> could not get value for ").c_str());
}

}  // namespace pbt

}  // namespace inviwo
