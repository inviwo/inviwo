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
#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

#include <filesystem>

namespace inviwo {

class TestResult;

class TestPropertyObservable;

class TestPropertyObserver : public Observer {
public:
	friend TestPropertyObservable;
	TestPropertyObserver() = default;
	virtual void onTestPropertyChange() {}
	virtual ~TestPropertyObserver() = default;
};

class TestPropertyObservable : public Observable<TestPropertyObserver> {
protected:
	TestPropertyObservable() = default;
	void notifyObserversAboutChange() {
		forEachObserver([](TestPropertyObserver* o) {
				o->onTestPropertyChange();
			});
	}
	virtual ~TestPropertyObservable() = default;
};

/** \docpage{org.inviwo.PropertyAnalyzer, PropertyAnalyzer}
 * ![](org.inviwo.TestProperty.png?classIdentifier=org.inviwo.TestProperty)
 *
 * A TestProperty holds a reference to a testable property and maintains the
 * respective BoolCompositeProperty and drop-down menu.
 * It is used to provide a general interface to all properties, regardless of
 * their value_type.
 * It enables
 *	- generation of assignments,
 *	- a textual description of the expected effects on the counted number of pixels,
 *	- the expected effect on the counted number of pixels given two testcases, and
 *  - storing and resetting the value of the property befor the tests have started.
 * Instantiation happes mainly by testableProperty (see below).
 */ 
class TestProperty : public Serializable, public TestPropertyObservable {
protected:
	std::string displayName, identifier;
	BoolCompositeProperty* boolComp = nullptr;
	TestProperty() = default;
	TestProperty(const std::string& displayName, const std::string& identifier);

	const auto& options() const {
		const static std::vector<OptionPropertyOption<int>> options {
				{"EQUAL",			"EQUAL",			0},
				{"NOT_EQUAL",		"NOT_EQUAL",		1},
				{"LESS",			"LESS",				2},
				{"LESS_EQUAL",		"LESS_EQUAL",		3},
				{"GREATER",			"GREATER",			4},
				{"GREATER_EQUAL",	"GREATER_EQUAL",	5},
				{"ANY",				"ANY",				6},
				{"NOT_COMPARABLE",	"NOT_COMPARABLE",	7}
			};
		return options;
	}
public:
	virtual BoolCompositeProperty* getBoolComp() const;

	const std::string& getDisplayName() const;
	const std::string& getIdentifier() const;

	virtual std::string textualDescription(unsigned int indent = 0) const = 0;

	virtual std::string getValueString(std::shared_ptr<TestResult>) const = 0;
	void traverse(std::function<void(const TestProperty*, const TestProperty*)>)
		const;
	virtual void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
			const TestProperty*) const = 0;

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
	virtual std::vector<std::tuple<std::unique_ptr<bool>,
			util::AssignmentComparator,
			std::vector<std::shared_ptr<PropertyAssignment>>>>
		generateAssignmentsCmp() const = 0;
	virtual ~TestProperty() = default;
};

/*
 * returns the combined expected effect (if any) of two values, given the
 * effect of each of the components.
 */
template<typename T,
	size_t numComp = DataFormat<T>::components()>
std::optional<util::PropertyEffect> propertyEffect(const T& val_old, const T& val_new,
		const std::array<util::PropertyEffect, numComp>& selectedEffects);

/*
 * Creates and returns a TestProperty for an arbitrary property, if possible.
 * That is, if the given property derives from CompositeProperty or one of
 * the properties in PropertyTypes (see below).
 */
std::optional<std::shared_ptr<TestProperty>> testableProperty(Property* prop);

/*
 * Adds necessary callbacks for the proper behavior of the BoolComps of/in a
 * BoolCompositeProperty, i.e. the BoolProperty of the BoolCompositeProperty
 * is checked, if and only if at least one of the contained
 * BoolCompositeProperties is checked.
 */
void makeOnChange(BoolCompositeProperty* const prop);

/*
 * Derived from TestProperty, for all PropertyOwners supporting
 * getDisplayName and getIdentifier, i.e. CompositeProperty and Processor
 * May only be instantiated by TestPropertyComposite::make
 */
class TestPropertyComposite : public TestProperty, public TestPropertyObserver {
	PropertyOwner* propertyOwner;
	std::vector<std::shared_ptr<TestProperty>> subProperties;
	
	TestPropertyComposite() = default;
	TestPropertyComposite(PropertyOwner* original, const std::string& displayName,
			const std::string& identifier);
	
	void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
			const TestProperty*) const override;
public:
	const static std::string& getClassIdentifier() {
		const static std::string name = "org.inviwo.TestPropertyComposite";
		return name;
	}
	friend class TestPropertyFactory;
	
	void onTestPropertyChange() override;
	
	std::string textualDescription(unsigned int indent = 0) const override;

	std::string getValueString(std::shared_ptr<TestResult> testResult) const override;

	std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const override;

	std::ostream& ostr(std::ostream& out,
			std::shared_ptr<TestResult> testResult) const override;

	std::ostream& ostr(std::ostream& out,
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const override;

	// Constructor for all Propertyowners with a display name and an identifier
	template<typename C, decltype(static_cast<PropertyOwner*>(std::declval<C*>()),
			std::declval<C>().getDisplayName(),
			std::declval<C>().getIdentifier(), int()) = 0>
	static std::shared_ptr<TestPropertyComposite> make(C* orig);

	virtual ~TestPropertyComposite() = default;
	void setToDefault() const override;

	void serialize(Serializer& s) const override;
	void deserialize(Deserializer& s) override;

	template<typename T>
	std::optional<typename T::value_type> getDefaultValue(const T* prop) const;

	void storeDefault();
	std::vector<std::tuple<std::unique_ptr<bool>,
			util::AssignmentComparator,
			std::vector<std::shared_ptr<PropertyAssignment>>>>
		generateAssignmentsCmp() const override;
};

template<typename C, decltype(static_cast<PropertyOwner*>(std::declval<C*>()),
			std::declval<C>().getDisplayName(),
			std::declval<C>().getIdentifier(), int()) = 0>
std::shared_ptr<TestPropertyComposite> TestPropertyComposite::make(C* orig) {
		std::string ident = orig->getIdentifier();
		std::replace(ident.begin(), ident.end(), ' ', '_');
		return std::shared_ptr<TestPropertyComposite>(
				new TestPropertyComposite(orig, orig->getDisplayName(), ident));
	}

/*
 * Derived from TestProperty, for all Properties having a value that is
 * supported by DataFormat<>.
 */
template<typename T>
class TestPropertyTyped : public TestProperty {
	using value_type = typename T::value_type;
	static constexpr size_t numComponents = DataFormat<value_type>::components();

	T* typedProperty;
	value_type defaultValue;
	std::array<OptionPropertyInt*, numComponents> effectOption;
	
	TestPropertyTyped() = default;

	std::array<util::PropertyEffect, numComponents> selectedEffects() const;
	
	void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
			const TestProperty*) const override;
public:
	const static std::string& getClassIdentifier() {
		const static std::string name = std::string("org.inviwo.TestPropertyTyped") + PropertyTraits<T>::classIdentifier();
		return name;
	}
	friend class TestPropertyFactoryHelper;
	
	std::string textualDescription(unsigned int indent) const override;

	std::string getValueString(std::shared_ptr<TestResult>) const override;

	std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const override;

	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>) const override;

	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>,
			std::shared_ptr<TestResult>) const override;

	TestPropertyTyped(T* original);
	virtual ~TestPropertyTyped() = default;
	T* getTypedProperty() const;
	void setToDefault() const override;
	const value_type& getDefaultValue() const;

	void serialize(Serializer& s) const override;
	void deserialize(Deserializer& s) override;
	
	void storeDefault();
	std::vector<std::tuple<std::unique_ptr<bool>,
			util::AssignmentComparator,
			std::vector<std::shared_ptr<PropertyAssignment>>>>
		generateAssignmentsCmp() const override;
};

/*
 * Container for a test result.
 * Allows access to the tested value of each property, as well as the
 * resulted number of pixels.
 */
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
		typename T::value_type getValue(const T* prop) const;

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

template<typename T,
	size_t numComp = DataFormat<T>::components()>
std::optional<util::PropertyEffect> propertyEffect(const T& val_old, const T& val_new,
		const std::array<util::PropertyEffect, numComp>& selectedEffects) {
	
	std::optional<util::PropertyEffect> res = {util::PropertyEffect::ANY};
	for(size_t i = 0; res && i < numComp; i++) {
		auto compEff = util::propertyEffect(selectedEffects[i],
				util::GetComponent<T, numComp>::get(val_new, i),
				util::GetComponent<T, numComp>::get(val_old, i));
		res = util::combine(*res, compEff);
	}
	return res;
	
}

class TestPropertyFactory : public Factory<TestProperty> {
	static const std::unordered_map<std::string,
				 std::function<std::unique_ptr<TestProperty>()>> members;
public:
	std::unique_ptr<TestProperty> create(const std::string& key) const override {
		assert(members.count(key) > 0);
		return members.at(key)();
	}
	bool hasKey(const std::string& key) const override {
		return members.find(key) != members.end();
	}
};

template<typename T>
std::optional<typename T::value_type> TestPropertyComposite::getDefaultValue(const T* prop) const {
	for(auto subProp : subProperties) {
		if(auto p = std::dynamic_pointer_cast<TestPropertyTyped<T>>(subProp); p != nullptr) {
			if(p->getTypedProperty() == prop)
				return p->getDefaultValue();
		} else if(auto p = std::dynamic_pointer_cast<TestPropertyComposite>(subProp); p != nullptr) {
			if(auto res = p->getDefaultValue(prop); res != std::nullopt)
				return res;
		}
	}
	return std::nullopt;
}
template<typename T>
typename T::value_type TestResult::getValue(const T* prop) const {
	for(const auto& t : test)
		if(auto p = std::dynamic_pointer_cast<PropertyAssignmentTyped<T>>(t);
				p && reinterpret_cast<T*>(p->getProperty()) == prop)
			return p->getValue();

	for(auto def : defaultValues) {
		if(auto p = std::dynamic_pointer_cast<TestPropertyTyped<T>>(def);
				p != nullptr) {
			if(p->getTypedProperty() == prop)
				return p->getDefaultValue();
		} else if(auto p = std::dynamic_pointer_cast<TestPropertyComposite>(def);
				p != nullptr) {
			if(auto res = p->getDefaultValue(prop); res != std::nullopt)
				return *res;
		}
	}
	std::cerr << "could not get value for " << prop << " " << typeid(T).name() << std::endl;
	exit(1);
}

/*
 * Tuple containing all testable property types except CompositeProperty.
 * Should only contain OrdinalProperties
 * If you want to add support for a new property type, you probably want to add
 * it to this list.
 */
using PropertyTypes = std::tuple<IntProperty, FloatProperty, DoubleProperty, IntMinMaxProperty>;

using TestingError = std::tuple<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>, util::PropertyEffect, size_t, size_t>;

void testingErrorToBinary(std::vector<unsigned char>&, const std::vector<std::shared_ptr<TestProperty>>&, const TestingError&);

} // namespace inviwo
