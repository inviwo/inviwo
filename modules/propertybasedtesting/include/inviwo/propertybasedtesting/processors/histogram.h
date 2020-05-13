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
#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>
#include <inviwo/propertybasedtesting/algorithm/histogramtesting.h>
#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>


#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <inviwo/core/ports/imageport.h>

#include <filesystem>

namespace inviwo {

class TestResult;

class TestProperty {
private:
	Property* const prop;
public:
	TestProperty(Property* const prop)
		: prop(prop) {
	}
	
	virtual void withOptionProperties(std::function<void(OptionPropertyInt*)>) const = 0;
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
		return prop;
	}
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
	void withOptionProperties(std::function<void(OptionPropertyInt*)> f) const {
		for(auto& x : effectOption)
			f(x);
	}
	std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const;
	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>) const;
	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>,
			std::shared_ptr<TestResult>) const;

	TestPropertyTyped(T* prop)
		: TestProperty(prop)
		, typedProperty(prop)
		, defaultValue(prop->get())
		, effectOption([prop](){
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
					std::string identifier = prop->getIdentifier() + "_selector_" + std::to_string(i);
					res[i] = new OptionPropertyInt(identifier, "Comparator for increasing values", options, options.size() - 1);
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
	public:
		size_t getNumberOfPixels() {
			return pixels;
		}
		template<typename T>
		const typename T::value_type& getValue(const T* prop) const {
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
		TestResult(const std::vector<std::shared_ptr<TestProperty>>& defaultValues, const Test& t, size_t val)
			  : defaultValues(defaultValues)
			  , test(t)
			  , pixels(val) {
			  }
};
	
template<typename T>
std::optional<util::PropertyEffect> TestPropertyTyped<T>::getPropertyEffect(
		std::shared_ptr<TestResult> newTestResult,
		std::shared_ptr<TestResult> oldTestResult) const {
	const value_type& valNew = newTestResult->getValue(this->typedProperty);
	const value_type& valOld = oldTestResult->getValue(this->typedProperty);

	std::array<util::PropertyEffect, numComponents> selectedEffects;
	for(size_t i = 0; i < numComponents; i++)
		selectedEffects[i] = util::PropertyEffect(effectOption[i]->getSelectedValue());

	std::optional<util::PropertyEffect> res = {util::PropertyEffect::ANY};
	for(size_t i = 0; res && i < numComponents; i++) {
		auto compEff = util::propertyEffect(selectedEffects[i],
				util::GetComponent<value_type, numComponents>::get(valNew, i),
				util::GetComponent<value_type, numComponents>::get(valOld, i));
		res = util::combine(*res, compEff);
	}
	return res;
}

template<typename T>
std::ostream& TestPropertyTyped<T>::ostr(std::ostream& out,
			std::shared_ptr<TestResult> testResult) const {
	const value_type& val = testResult->getValue(this->typedProperty);
	
	std::array<util::PropertyEffect, numComponents> selectedEffects;
	for(size_t i = 0; i < numComponents; i++)
		selectedEffects[i] = util::PropertyEffect(effectOption[i]->getSelectedValue());
	
	return out << '\"' << getProperty()->getDisplayName() << "\" with identifier \"" << getProperty()->getIdentifier() << "\": "
				<< val;
}
template<typename T>
std::ostream& TestPropertyTyped<T>::ostr(std::ostream& out,
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const {
	const value_type& valNew = newTestResult->getValue(this->typedProperty);
	const value_type& valOld = oldTestResult->getValue(this->typedProperty);
	
	return out << '\"' << getProperty()->getDisplayName() << "\" with identifier \"" << getProperty()->getIdentifier() << "\": "
				<< valNew << ", " << valOld << " ; comparator set to  "
				<< getPropertyEffect(newTestResult, oldTestResult);
}

/** \docpage{org.inviwo.Histogram, Histogram}
 * ![](org.inviwo.Histogram.png?classIdentifier=org.inviwo.Histogram)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API Histogram : public Processor {
public:
    Histogram(InviwoApplication*);
    virtual ~Histogram() {
		std::filesystem::remove_all(tempDir_);
	}

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
	enum TestingState {
		NONE,
		GATHERING,
		SINGLE_COUNT
	};
	TestingState testingState;

	InviwoApplication* const app_;
    std::filesystem::path tempDir_;

	ImageInport inport_;
	
	BoolProperty useDepth_;
    FloatVec4Property color_;
	ButtonProperty countPixelsButton_;
	ButtonProperty startButton_;
	IntSizeTProperty numTests_;
	ButtonProperty collectButton_;

	std::vector<CompositeProperty*> compositeProperties_;

	std::vector<std::shared_ptr<TestProperty>> props_;
	void resetAllProps() {
		for(auto prop : props_) {
			prop->setToDefault();
		}
	}

	// Testing stuff
	void initTesting();

	bool testIsSetUp(const Test& test);
	void setupTest(const Test& test);

	std::queue<Test> remainingTests;
	std::vector<std::shared_ptr<TestResult>> testResults;
	void checkTestResults();
};

}  // namespace inviwo
