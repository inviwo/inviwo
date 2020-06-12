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
#include <inviwo/propertybasedtesting/algorithm/histogramtesting.h>

#include <inviwo/core/properties/ordinalproperty.h>

#include <filesystem>

namespace inviwo {

class TestResult;

class TestProperty : public Serializable {
private:
	Property* const original;
public:
	TestProperty(Property* const original)
		: original(original) {
	}

	virtual void serialize(Serializer&) const override = 0;
	virtual void deserialize(Deserializer&) override = 0;
	
	virtual void withOptionProperties(std::function<void(OptionPropertyInt*)>) const = 0;

	virtual std::string getValueString(std::shared_ptr<TestResult>) const = 0;

	virtual std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult>, 
			std::shared_ptr<TestResult>) const = 0;

	virtual std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>) const = 0;

	virtual std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>,
			std::shared_ptr<TestResult>) const = 0;

	virtual void setToDefault() const = 0;
	virtual void storeDefault() = 0;
	virtual std::vector<std::shared_ptr<PropertyAssignment>> generateAssignments() = 0;
	Property* getProperty() const {
		return original;
	}
	// Property* getOriginalProperty() const {
	// 	return original;
	// }
	virtual ~TestProperty() = default;
};

template<typename T>
class TestPropertyTyped : public TestProperty {
	using value_type = typename T::value_type;
	static constexpr size_t numComponents = DataFormat<value_type>::components();
	T* const typedProperty;
	value_type defaultValue;
	const std::array<OptionPropertyInt*, numComponents> effectOption;
public:
	void serialize(Serializer& s) const override {
		// TODO
	}
	void deserialize(Deserializer& d) override {
		// TODO
	}

	void withOptionProperties(std::function<void(OptionPropertyInt*)> f) const {
		for(auto& x : effectOption)
			f(x);
	}

	std::string getValueString(std::shared_ptr<TestResult>) const override;

	std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const override;

	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>) const override;

	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>,
			std::shared_ptr<TestResult>) const override;

	TestPropertyTyped(T* original)
		: TestProperty(original)
		, typedProperty(original)
		, defaultValue(original->get())
		, effectOption([original](){
				std::array<OptionPropertyInt*, numComponents> res;
				for(size_t i = 0; i < numComponents; i++) {
					const static std::vector<OptionPropertyOption<int>> options{
							{"EQUAL",			"EQUAL",			0},
							{"NOT_EQUAL",		"NOT_EQUAL",		1},
							{"LESS",			"LESS",				2},
							{"LESS_EQUAL",		"LESS_EQUAL",		3},
							{"GREATER",			"GREATER",			4},
							{"GREATER_EQUAL",	"GREATER_EQUAL",	5},
							{"ANY",				"ANY",				6},
							{"NOT_COMPARABLE",	"NOT_COMPARABLE",	7}
						};
					std::string identifier = original->getIdentifier() + "_selector_" + std::to_string(i);
					res[i] = new OptionPropertyInt(
						identifier,
						"Comparator for increasing values",
						options,
						options.size() - 1);
				}
				return res;
			}()){
	}
	~TestPropertyTyped() = default;
	T* getTypedProperty() const {
		return typedProperty;
	}
	void setToDefault() const {
		typedProperty->set(defaultValue);
	}
	const value_type& getDefaultValue() {
		return defaultValue;
	}
	void storeDefault() {
		defaultValue = typedProperty->get();
	}
	std::vector<std::shared_ptr<PropertyAssignment>> generateAssignments() {
		return _generateAssignments<T>(typedProperty);
	}
};

class TestResult {
	private:
		const std::vector<std::shared_ptr<TestProperty>>& defaultValues;
		const Test test;
		const size_t pixels;
		const std::filesystem::path imgPath;
	public:
		size_t getNumberOfPixels() {
			return pixels;
		}
		const std::filesystem::path& getImagePath() {
			return imgPath;
		}
		template<typename T>
		const typename T::value_type& getValue(const T* prop) const;
		TestResult(const std::vector<std::shared_ptr<TestProperty>>& defaultValues
				, const Test& t
				, size_t val
				, const std::filesystem::path& imgPath)
			  : defaultValues(defaultValues)
			  , test(t)
			  , pixels(val)
			  , imgPath(imgPath) {
			  }
};
		
template<typename T>
const typename T::value_type& TestResult::getValue(const T* prop) const {
	for(const auto& t : test)
		if(auto p = std::dynamic_pointer_cast<PropertyAssignmentTyped<typename T::value_type>>(t);
				p && reinterpret_cast<T*>(p->getProperty()) == prop)
			return p->getValue();
	for(auto def : defaultValues)
		if(auto p = std::dynamic_pointer_cast<TestPropertyTyped<T>>(def);
				p && p->getProperty() == prop)
			return p->getDefaultValue();
	assert(false);
}

using PropertyTypes = std::tuple<OrdinalProperty<int>, IntMinMaxProperty>;

std::optional<std::shared_ptr<TestProperty>> testableProperty(Property* prop);

using TestingError = std::tuple<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>, util::PropertyEffect, size_t, size_t>;

} // namespace inviwo
