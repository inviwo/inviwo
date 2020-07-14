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

#include <inviwo/propertybasedtesting/testproperty.h>

namespace inviwo {

// TestPropertyComposite

BoolCompositeProperty* TestPropertyComposite::getBoolComp() const {
	return boolComp;
}

std::string TestPropertyComposite::getValueString(std::shared_ptr<TestResult> testResult) const {
	std::stringstream str;
	size_t n = 0;
	str << "(";
	for(const auto& subProp : subProperties)
		str << (n++ > 0 ? ", " : "") << subProp->getValueString(testResult);
	str << ")";
	return str.str();
}
	
std::string TestPropertyComposite::getDisplayName() const {
	return displayName;
}
std::string TestPropertyComposite::getIdentifier() const {
	return identifier;
}

std::optional<util::PropertyEffect> TestPropertyComposite::getPropertyEffect(
		std::shared_ptr<TestResult> newTestResult,
		std::shared_ptr<TestResult> oldTestResult) const {
	std::vector<util::PropertyEffect> subEffects;

	std::optional<util::PropertyEffect> res = {util::PropertyEffect::ANY};
	for(const auto& subProp : subProperties) {
		if(!res)
			break;
		auto tmp = subProp->getPropertyEffect(newTestResult, oldTestResult);
		if(!tmp) {
			res = std::nullopt;
			break;
		}
		res = util::combine(*res, *tmp);
	}
	return res;
}

std::ostream& TestPropertyComposite::ostr(std::ostream& out,
		std::shared_ptr<TestResult> testResult) const {
	out << "(";
	size_t n = 0;
	for(const auto& subProp : subProperties) {
		if(n++ > 0) out << ", ";
		subProp->ostr(out, testResult);
	}
	return out << ")";
}

std::ostream& TestPropertyComposite::ostr(std::ostream& out,
		std::shared_ptr<TestResult> newTestResult,
		std::shared_ptr<TestResult> oldTestResult) const {
	out << getDisplayName() << ":" << std::endl;
	for(const auto& subProp : subProperties)
		subProp->ostr(out << "\t", newTestResult, oldTestResult) << std::endl;
	return out;
}

TestPropertyComposite::TestPropertyComposite(PropertyOwner* original,
	const std::string& displayName, const std::string& identifier)
		: TestProperty()
		, propertyOwner(original)
		, displayName(displayName)
		, identifier(identifier)
		, boolComp(new BoolCompositeProperty(identifier+"Comp", displayName, false)) {
	for(Property* prop : original->getProperties())
		if(auto p = testableProperty(prop); p != std::nullopt) {
			subProperties.emplace_back(*p);
			boolComp->addProperty((*p)->getBoolComp());
		}
	boolComp->setCollapsed(true);
	makeOnChange(boolComp);
}
void TestPropertyComposite::setToDefault() const {
	for(const auto& subProp : subProperties)
		subProp->setToDefault();
}

void TestPropertyComposite::storeDefault() {
	for(const auto& subProp : subProperties)
		subProp->storeDefault();
}
std::vector<std::shared_ptr<PropertyAssignment>> TestPropertyComposite::generateAssignments() const {
	std::vector<std::shared_ptr<PropertyAssignment>> res;
	for(const auto& subProp : subProperties) {
		auto tmp = subProp->generateAssignments();
		res.insert(res.end(), tmp.begin(), tmp.end());
	}
	return res;
}

void TestPropertyComposite::serialize(Serializer& s) const {
	std::cerr << "\tserializing TestPropertyComposite" << std::endl;
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);

	s.serialize("PropertyOwner", propertyOwner);
	s.serialize("DisplayName", displayName);
	s.serialize("Identifier", identifier);
	s.serialize("BoolComp", boolComp);

	{
		std::vector<TestProperty*> subProps;
		for(const auto& sp : subProperties)
			subProps.emplace_back(sp.get());
		s.serialize("SubProperties", subProps);
	}
}
void TestPropertyComposite::deserialize(Deserializer& d) {
	std::cerr << "\tdeserializing TestPropertyComposite" << std::endl;
	d.deserialize("PropertyOwner", propertyOwner);
	d.deserialize("DisplayName", displayName);
	d.deserialize("Identifier", identifier);
	d.deserialize("BoolComp", boolComp);

	{
		std::vector<TestProperty*> subProps;
		d.deserialize("SubProperties", subProps);
		for(const auto& sp : subProps)
			subProperties.emplace_back(std::shared_ptr<TestProperty>(sp));
	}
}

// TestPropertyTyped

template<typename T>
BoolCompositeProperty* TestPropertyTyped<T>::getBoolComp() const {
	return boolComp;
}

template<typename T>
TestPropertyTyped<T>::TestPropertyTyped(T* original)
	: TestProperty()
	, typedProperty(original)
	, defaultValue(original->get())
	, boolComp(new BoolCompositeProperty(original->getIdentifier()+"Comp", original->getDisplayName(), false))
	, effectOption([this,original](){
			std::array<OptionPropertyInt*, numComponents> res;
			for(size_t i = 0; i < numComponents; i++) {
				std::string identifier = original->getIdentifier() + "_selector_" + std::to_string(i);
				res[i] = new OptionPropertyInt(
					identifier,
					"Comparator for increasing values",
					options(),
					options().size() - 1);
			}
			return res;
		}()){

	boolComp->setCollapsed(true);
	for(auto effectOpt : effectOption)
		boolComp->addProperty(effectOpt);
	makeOnChange(boolComp);
}
template<typename T>
T* TestPropertyTyped<T>::getTypedProperty() const {
	return typedProperty;
}
template<typename T>
void TestPropertyTyped<T>::setToDefault() const {
	typedProperty->set(defaultValue);
}
template<typename T>
auto TestPropertyTyped<T>::getDefaultValue() const -> const val_type& {
	return defaultValue;
}
template<typename T>
void TestPropertyTyped<T>::storeDefault() {
	defaultValue = typedProperty->get();
}
template<typename T>
std::vector<std::shared_ptr<PropertyAssignment>> TestPropertyTyped<T>::generateAssignments() const {
	static const GenerateAssignments<T> tmp;
	return tmp(typedProperty);
}

template<typename T>
std::optional<util::PropertyEffect> TestPropertyTyped<T>::getPropertyEffect(
		std::shared_ptr<TestResult> newTestResult,
		std::shared_ptr<TestResult> oldTestResult) const {
	const val_type& valNew = newTestResult->getValue(this->typedProperty);
	const val_type& valOld = oldTestResult->getValue(this->typedProperty);

	std::array<util::PropertyEffect, numComponents> selectedEffects;
	for(size_t i = 0; i < numComponents; i++)
		selectedEffects[i] = util::PropertyEffect(effectOption[i]->getSelectedValue());

	std::optional<util::PropertyEffect> res = {util::PropertyEffect::ANY};
	for(size_t i = 0; res && i < numComponents; i++) {
		auto compEff = util::propertyEffect(selectedEffects[i],
				util::GetComponent<val_type, numComponents>::get(valNew, i),
				util::GetComponent<val_type, numComponents>::get(valOld, i));
		res = util::combine(*res, compEff);
	}
	return res;
}

template<typename T>
std::string TestPropertyTyped<T>::getValueString(std::shared_ptr<TestResult> testResult) const {
	std::stringstream str;
	str << testResult->getValue(this->typedProperty);
	return str.str();
}
template<typename T>
std::string TestPropertyTyped<T>::getDisplayName() const {
	return getTypedProperty()->getDisplayName();
}
template<typename T>
std::string TestPropertyTyped<T>::getIdentifier() const {
	return getTypedProperty()->getIdentifier();
}

template<typename T>
std::ostream& TestPropertyTyped<T>::ostr(std::ostream& out,
			std::shared_ptr<TestResult> testResult) const {
	const val_type& val = testResult->getValue(this->typedProperty);
	
	std::array<util::PropertyEffect, numComponents> selectedEffects;
	for(size_t i = 0; i < numComponents; i++)
		selectedEffects[i] = util::PropertyEffect(effectOption[i]->getSelectedValue());
	
	return out << '\"' << getDisplayName() << "\" with identifier \"" << getIdentifier() << "\": "
				<< val;
}
template<typename T>
std::ostream& TestPropertyTyped<T>::ostr(std::ostream& out,
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const {
	const val_type& valNew = newTestResult->getValue(this->typedProperty);
	const val_type& valOld = oldTestResult->getValue(this->typedProperty);
	
	return out << '\"' << getDisplayName() << "\" with identifier \"" << getIdentifier() << "\": "
				<< valNew << ", " << valOld << " ; comparator set to  "
				<< getPropertyEffect(newTestResult, oldTestResult);
}

template<typename T>
void TestPropertyTyped<T>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);

	s.serialize("TypedProperty", typedProperty);
	s.serialize("DefaultValue", defaultValue);
	s.serialize("BoolComp", boolComp);
	s.serialize("EffectOption", effectOption);
}
template<typename T>
void TestPropertyTyped<T>::deserialize(Deserializer& d) {
	d.deserialize("TypedProperty", typedProperty);
	d.deserialize("DefaultValue", defaultValue);
	d.deserialize("BoolComp", boolComp);
	d.deserialize("EffectOption", effectOption);
}

// Helpers
struct TestPropertyFactoryHelper {
	template<typename T>
	auto operator()(std::unordered_map<std::string, std::function<std::unique_ptr<TestProperty>()>>& res) {
		res[TestPropertyTyped<T>::getClassIdentifier()] = [](){
				return std::unique_ptr<TestProperty>(new TestPropertyTyped<T>());
			};
	}
};

const std::unordered_map<std::string, std::function<std::unique_ptr<TestProperty>()>> TestPropertyFactory::members = [](){
	std::cerr << "filling TestPropertyFactory" << std::endl;
	using M = std::remove_const_t<decltype(TestPropertyFactory::members)>;
	M res;
	res[TestPropertyComposite::getClassIdentifier()] = [](){
			return std::unique_ptr<TestProperty>(new TestPropertyComposite());
		};
	util::for_each_type<PropertyTypes>{}(TestPropertyFactoryHelper{}, res);
	for(auto [key,value] : res)
		std::cerr << "\t" << key << std::endl;
	return res;
}();

struct TestablePropertyHelper {
	// for Properties with values
	template<typename T>
	auto operator()(std::optional<std::shared_ptr<TestProperty>>& res, Property* prop) -> decltype((typename T::value_type*)(nullptr), void()) {
		if(res != std::nullopt)
			return;
		if(auto tmp = dynamic_cast<T*>(prop); tmp != nullptr)
			res = {std::make_shared<TestPropertyTyped<T>>(tmp)};
	}
};

std::optional<std::shared_ptr<TestProperty>> testableProperty(Property* prop) {
	std::optional<std::shared_ptr<TestProperty>> res = std::nullopt;
	util::for_each_type<PropertyTypes>{}(TestablePropertyHelper{}, res, prop);
	if(!res) {
		if(auto tmp = dynamic_cast<CompositeProperty*>(prop); tmp != nullptr)
			return TestPropertyComposite::make<CompositeProperty>(tmp);
	}
	return res;
}

void makeOnChange(BoolCompositeProperty* prop) {
	std::vector<BoolProperty*> subProps;
	for(auto boolCompProp : prop->getPropertiesByType<BoolCompositeProperty>())
		subProps.emplace_back(boolCompProp->getBoolProperty());
	for(auto boolProp : prop->getPropertiesByType<BoolProperty>())
		if(boolProp != prop->getBoolProperty())
			subProps.emplace_back(boolProp);

	//prop->getBoolProperty()->setReadOnly(subProps.size() > 0);
	if(!subProps.empty()) {
		prop->onChange([prop, subProps](){
				bool checked = false;
				for(auto* boolProp : subProps)
					checked |= boolProp->get();
				prop->setChecked(checked);
			});
	}
}

void testingErrorToBinary(std::vector<unsigned char>& output,
		const std::vector<std::shared_ptr<TestProperty>>& props,
		const TestingError& err) {
	const auto&[testResult1, testResult2, effect, res1, res2] = err;

	std::stringstream str;
	for(auto prop : props)
		prop->ostr(str, testResult1, testResult2) << std::endl;
	str << res1 << res2 << effect;

	const std::string data = str.str();
	output.insert(output.end(), data.begin(), data.end());
}

} // namespace inviwo
