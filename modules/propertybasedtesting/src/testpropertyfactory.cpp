/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOr
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/propertybasedtesting/testpropertyfactory.h>

namespace inviwo {

namespace pbt {

// TestPropertyFactoryObject
TestPropertyFactoryObject::TestPropertyFactoryObject(const std::string& classIdentifier)
		: classIdentifier_(classIdentifier) {
}
const std::string& TestPropertyFactoryObject::getClassIdentifier() const {
	return classIdentifier_;
}
    
// TestPropertyFactoryObjectTemplate
template<typename T>
std::unique_ptr<TestProperty> TestPropertyFactoryObjectTemplate<T>::create() const {
	return std::unique_ptr<T>(new T());
   }

struct TestPropertyFactoryHelper {
	template<typename T>
	auto operator()(std::vector<std::unique_ptr<TestPropertyFactoryObject>>& factoryObjects) {
		factoryObjects.emplace_back(std::make_unique<TestPropertyFactoryObjectTemplate<TestPropertyTyped<T>>>());
	}
};

// TestPropertyFactory
TestPropertyFactory::TestPropertyFactory() {
	util::for_each_type<PropertyTypes>{}(TestPropertyFactoryHelper{}, factoryObjects_);
	factoryObjects_.emplace_back(std::make_unique<TestPropertyFactoryObjectTemplate<TestPropertyComposite>>());
	for(auto& obj : factoryObjects_)
		registerObject(obj.get());
}

// TestPropertyCompositeFactoryObject
std::unique_ptr<TestPropertyComposite> TestPropertyCompositeFactoryObject::create() const {
	return std::unique_ptr<TestPropertyComposite>(new TestPropertyComposite());
}
const std::string& TestPropertyCompositeFactoryObject::getClassIdentifier() const {
	return TestPropertyComposite::classIdentifier();
}

// TestPropertyCompositeFactory
TestPropertyCompositeFactory::TestPropertyCompositeFactory()
		: obj_(new TestPropertyCompositeFactoryObject()) {
	registerObject(obj_.get());
}

} // namespace pbt

} // namespace inviwo
