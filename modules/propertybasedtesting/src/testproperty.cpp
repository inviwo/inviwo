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

BoolCompositeProperty* TestProperty::getBoolComp() const {
	return boolComp_;
}
TestProperty::TestProperty(const std::string& displayName, const std::string& identifier)
		: displayName_(displayName)
		, identifier_(identifier)
		, boolComp_(new BoolCompositeProperty(identifier_+"Comp", displayName_, false)) {
	boolComp_->setSerializationMode(PropertySerializationMode::All);
	boolComp_->setCollapsed(true);
	makeOnChange(boolComp_);
}

const std::string& TestProperty::getDisplayName() const {
	return displayName_;
}
const std::string& TestProperty::getIdentifier() const {
	return identifier_;
}
	
void TestProperty::traverse(
		std::function<void(const TestProperty*, const TestProperty*)> f) const {
	traverse(f, nullptr);
}

// TestPropertyComposite

std::string TestPropertyComposite::getValueString(std::shared_ptr<TestResult> testResult) const {
	std::stringstream str;
	size_t n = 0;
	str << "(";
	for(const auto& subProp : subProperties)
		str << (n++ > 0 ? ", " : "") << subProp->getValueString(testResult);
	str << ")";
	return str.str();
}

void TestPropertyComposite::traverse(
				std::function<void(const TestProperty*, const TestProperty*)> f,
				const TestProperty* pa
			) const {
	f(this, pa);
	for(auto prop : subProperties)
		prop->traverse(f, this);
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

size_t TestPropertyComposite::totalCheckedComponents() const {
	size_t result = 0;
	for_each_checked([&](auto prop) {
		result += prop->totalCheckedComponents();
	});
	return result;
}
bool* TestPropertyComposite::deactivated(size_t i) {
	bool* result = nullptr;
	for_each_checked([&](auto prop) {
		const size_t sc = prop->totalCheckedComponents();
		if(result == nullptr && i < sc) result = prop->deactivated(i);
		else i -= sc;
	});
	assert(result != nullptr);
	return result;
}
const bool* TestPropertyComposite::deactivated(size_t i) const {
	bool* result = nullptr;
	for_each_checked([&](auto prop) {
		const size_t sc = prop->totalCheckedComponents();
		if(result == nullptr && i < sc) result = prop->deactivated(i);
		else i -= sc;
	});
	assert(result != nullptr);
	return result;
}

void TestPropertyComposite::onTestPropertyChange() {
	notifyObserversAboutChange();
}

std::string TestPropertyComposite::textualDescription(unsigned int indent) const {
	const std::string indentation(indent, ' ');
	std::string res = indentation + getDisplayName() + ":";
	for_each_checked([&](auto prop) {
			res += std::string("\n") + prop->textualDescription(indent+2);
		});
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
		: TestProperty(displayName, identifier)
		, propertyOwner(original) {
	if(auto p = dynamic_cast<Property*>(propertyOwner); p != nullptr)
		p->setSerializationMode(PropertySerializationMode::All);
	for(Property* prop : original->getProperties())
		if(auto p = testableProperty(prop); p != std::nullopt) {
			subProperties.emplace_back(*p);
			boolComp_->addProperty((*p)->getBoolComp());

			(*p)->addObserver(this);
		}
	boolComp_->onChange([this]() { this->notifyObserversAboutChange(); });
}
void TestPropertyComposite::setToDefault() const {
	for(const auto& subProp : subProperties)
		subProp->setToDefault();
}

void TestPropertyComposite::storeDefault() {
	for(const auto& subProp : subProperties)
		subProp->storeDefault();
}
std::vector<std::pair<
		util::AssignmentComparator,
		std::vector<std::shared_ptr<PropertyAssignment>>>>
	TestPropertyComposite::generateAssignmentsCmp() const {

	std::vector<std::pair<
			util::AssignmentComparator,
			std::vector<std::shared_ptr<PropertyAssignment>>>>
		res;
	for_each_checked([&](auto subProp) {
			auto tmp = subProp->generateAssignmentsCmp();
			res.insert(res.end(),
					std::make_move_iterator(tmp.begin()),
					std::make_move_iterator(tmp.end()));
		});
	return res;
}

void TestPropertyComposite::serialize(Serializer& s) const {
	//std::cerr << "\tserializing TestPropertyComposite: " << getIdentifier() << std::endl;

    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
	s.serialize("identifier", identifier_, SerializationTarget::Attribute);

	{
		const Processor* propertyOwnerProc = dynamic_cast<Processor*>(propertyOwner);
		const bool propertyOwnerIsProcessor = (propertyOwnerProc != nullptr);
		s.serialize("PropertyOwnerIsProcessor", propertyOwnerIsProcessor);
		if(propertyOwnerIsProcessor) {
			s.serialize("PropertyOwner", propertyOwnerProc);
		} else {
			const Property* propertyOwnerProp = dynamic_cast<Property*>(propertyOwner);
			assert(propertyOwnerProp != nullptr);
			s.serialize("PropertyOwner", propertyOwnerProp);
		}
	}

	s.serialize("DisplayName", displayName_);
	assert(boolComp_ != nullptr);
	s.serialize("BoolComp", boolComp_);

	{
		std::vector<TestProperty*> subProps;
		for(const auto& sp : subProperties) {
			subProps.emplace_back(sp.get());
			assert(subProps.back() != nullptr);
		}
		s.serialize("SubProperties", subProps);
	}
}
void TestPropertyComposite::deserialize(Deserializer& d) {
	d.deserialize("identifier", identifier_, SerializationTarget::Attribute);

	bool propertyOwnerIsProcessor;
	d.deserialize("PropertyOwnerIsProcessor", propertyOwnerIsProcessor);
	if(propertyOwnerIsProcessor) {
		Processor* tmp = nullptr;
		d.deserialize("PropertyOwner", tmp);
		propertyOwner = static_cast<PropertyOwner*>(tmp);
	} else {
		Property* tmp = nullptr; // required for proper deserialization of properties
		d.deserialize("PropertyOwner", tmp);
		propertyOwner = dynamic_cast<PropertyOwner*>(tmp);
		assert((propertyOwner == nullptr) == (tmp == nullptr));
	}
	assert(propertyOwner != nullptr);

	d.deserialize("DisplayName", displayName_);
	d.deserialize("BoolComp", boolComp_);
	assert(boolComp_ != nullptr);
	makeOnChange(boolComp_);

	{
		std::vector<TestProperty*> subProps;
		std::map<TestProperty*, decltype(subProperties)::value_type> old;
		std::transform(subProperties.begin(), subProperties.end(), std::inserter(old, old.end()),
				[](const auto& a) { return std::make_pair(a.get(), a); });
		std::vector<decltype(subProperties)::value_type> des;

		d.deserialize("SubProperties", subProps);
		for(TestProperty* const sp : subProps) {
			if(old.count(sp) == 0)
				des.emplace_back(std::shared_ptr<TestProperty>(sp));
			else
				des.emplace_back(old.at(sp));

			sp->addObserver(this);
		}

		subProperties = des;
	}
	
	boolComp_->onChange([this]() { this->notifyObserversAboutChange(); });
}

// TestPropertyTyped

template<typename T>
TestPropertyTyped<T>::TestPropertyTyped(T* original)
	: TestProperty(original->getDisplayName(), original->getIdentifier())
	, typedProperty_(original)
	, defaultValue_(original->get())
	, effectOption_([this,original](){
			std::array<OptionPropertyInt*, numComponents> res;
			for(size_t i = 0; i < numComponents; i++) {
				std::string identifier = original->getIdentifier() + "_selector_" + std::to_string(i);
				res[i] = new OptionPropertyInt(
					identifier,
					"Comparator for increasing values",
					options(),
					options().size() - 1);
				res[i]->setSerializationMode(PropertySerializationMode::All);
			}
			return res;
		}())
	, deactivated_(std::make_unique<bool>(false)) {

	for(auto effectOpt : effectOption_) {
		boolComp_->addProperty(effectOpt);
		effectOpt->onChange([this]() { this->notifyObserversAboutChange(); });
		boolComp_->onChange([this]() { this->notifyObserversAboutChange(); });
	}
}
template<typename T>
T* TestPropertyTyped<T>::getTypedProperty() const {
	return typedProperty_;
}
template<typename T>
void TestPropertyTyped<T>::setToDefault() const {
	typedProperty_->set(defaultValue_);
}
template<typename T>
auto TestPropertyTyped<T>::getDefaultValue() const -> const value_type& {
	return defaultValue_;
}
template<typename T>
void TestPropertyTyped<T>::storeDefault() {
	defaultValue_ = typedProperty_->get();
}
template<typename T>
size_t TestPropertyTyped<T>::totalCheckedComponents() const {
	return getBoolComp()->isChecked();
}
template<typename T>
bool* TestPropertyTyped<T>::deactivated(size_t i) {
	assert(i == 0);
	return deactivated_.get();
}
template<typename T>
const bool* TestPropertyTyped<T>::deactivated(size_t i) const {
	assert(i == 0);
	return deactivated_.get();
}
template<typename T>
std::string TestPropertyTyped<T>::textualDescription(unsigned int indent) const {
	const std::string indentation(indent, ' ');

	static const std::string effStrings[] = {
			"equal",
			"different",
			"smaller",
			"smaller or equal",
			"greater",
			"greater or equal",
			"PLACEHOLDER",
			"not comparable"
		};
	
	const auto change = [](const util::PropertyEffect& e) {
			switch(e) {
				case util::PropertyEffect::EQUAL:
				case util::PropertyEffect::NOT_EQUAL:
				case util::PropertyEffect::NOT_COMPARABLE:
					return "Changing";
				default:
					return "Increasing";
			}
		};

	const auto eff = this->selectedEffects();
	if(numComponents == 1) {
		if(eff[0] == util::PropertyEffect::ANY || eff[0] == util::PropertyEffect::NOT_COMPARABLE) {
			return indentation
				+ getDisplayName()
				+ " will be tested with differing values, but "
				+ (eff[0]==util::PropertyEffect::ANY
						? "it has no influence over the score."
						: "tests with different values are not comparable.");
		} else {
			return indentation
				+ change(eff[0]) + " "
				+ getDisplayName()
				+ "'s value should lead to a "
				+ effStrings[static_cast<unsigned int>(eff[0])]
				+ " score";
		}
	} else {
		std::array<std::vector<unsigned int>, util::numPropertyEffects> comps;
		for(unsigned int i = 0; i < numComponents; i++)
			comps[static_cast<unsigned int>(eff[i])].emplace_back(i);
		
		std::string res = indentation + getDisplayName() + ":";
		for(unsigned int eI = 0; eI < util::numPropertyEffects; eI++) {
			if(comps[eI].empty())
				continue;
			const bool plural = comps[eI].size() > 1;
			const util::PropertyEffect e = static_cast<util::PropertyEffect>(eI);

			std::string line = std::string("Component") + (plural ? "s " : " ");
			for(unsigned int i = 0; i < comps[eI].size(); i++) {
				if(i > 0) {
					line += (i==comps[eI].size()-1 ? " and " : ", ");
				}
				line += std::to_string(comps[eI][i]);
			}
			if(e == util::PropertyEffect::ANY || e == util::PropertyEffect::NOT_COMPARABLE) {
				line += std::string(" will be tested with differing values, but ")
					+ (e==util::PropertyEffect::ANY
						? ((plural ? "they have" : "it has")
							+ std::string(" no influence over the score."))
						: "tests with different values are not comparable.");
			} else {
				line[0] = 'c';
				line = std::string(change(e))
					+ " the value" + (plural ? "s" : "") + " of " 
					+ line + " should lead to a "
					+ effStrings[eI]
					+ " score";
			}

			res += '\n' + indentation + "  " + line;
		}
		return res;
	}
}
template<typename T>
auto TestPropertyTyped<T>::selectedEffects() const -> std::array<util::PropertyEffect, numComponents> {
	std::array<util::PropertyEffect, numComponents> selectedEffects;
	for(size_t i = 0; i < numComponents; i++)
		selectedEffects[i] = util::PropertyEffect(effectOption_[i]->getSelectedValue());
	return selectedEffects;
}
template<typename T>
std::vector<std::pair<
		util::AssignmentComparator,
		std::vector<std::shared_ptr<PropertyAssignment>>>>
	TestPropertyTyped<T>::generateAssignmentsCmp() const {

	const GenerateAssignments<T> tmp;
	static const util::AssignmentComparator cmp = [this](const auto& oldA, const auto& newA) {
			const PropertyAssignmentTyped<T>* oldAptr =
				dynamic_cast<const PropertyAssignmentTyped<T>*>(oldA.get());
			const PropertyAssignmentTyped<T>* newAptr =
				dynamic_cast<const PropertyAssignmentTyped<T>*>(newA.get());
			assert(oldAptr != nullptr);
			assert(newAptr != nullptr);

			const value_type& oldV = oldAptr->getValue();
			const value_type& newV = newAptr->getValue();

			return propertyEffect<value_type>(oldV, newV, selectedEffects());
		};
	auto assignments = tmp(typedProperty_, deactivated_.get());

	std::vector<std::pair<
			util::AssignmentComparator,
			std::vector<std::shared_ptr<PropertyAssignment>>>>
		res;
	res.emplace_back(cmp, assignments);
	return res;
}

template<typename T>
std::optional<util::PropertyEffect> TestPropertyTyped<T>::getPropertyEffect(
		std::shared_ptr<TestResult> newTestResult,
		std::shared_ptr<TestResult> oldTestResult) const {
	const value_type& valNew = newTestResult->getValue(this->typedProperty_);
	const value_type& valOld = oldTestResult->getValue(this->typedProperty_);

	return propertyEffect<value_type>(valOld, valNew, selectedEffects());
}

template<typename T>
std::string TestPropertyTyped<T>::getValueString(std::shared_ptr<TestResult> testResult) const {
	std::stringstream str;
	str << testResult->getValue(this->typedProperty_);
	return str.str();
}
template<typename T>
void TestPropertyTyped<T>::traverse(
				std::function<void(const TestProperty*, const TestProperty*)> f,
				const TestProperty* pa
			) const {
	f(this, pa);
}

template<typename T>
std::ostream& TestPropertyTyped<T>::ostr(std::ostream& out,
			std::shared_ptr<TestResult> testResult) const {
	const value_type& val = testResult->getValue(this->typedProperty_);
	
	return out << '\"' << getDisplayName() << "\" with identifier \"" << getIdentifier() << "\": "
				<< val;
}
template<typename T>
std::ostream& TestPropertyTyped<T>::ostr(std::ostream& out,
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const {
	const value_type& valNew = newTestResult->getValue(this->typedProperty_);
	const value_type& valOld = oldTestResult->getValue(this->typedProperty_);
	
	return out << '\"' << getDisplayName() << "\" with identifier \"" << getIdentifier() << "\": "
				<< valNew << ", " << valOld << " ; comparator set to  "
				<< getPropertyEffect(newTestResult, oldTestResult);
}

template<typename T>
void TestPropertyTyped<T>::serialize(Serializer& s) const {
	//std::cerr << "\tserializing " << getClassIdentifier() << ": " << getIdentifier() << std::endl;

    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
	s.serialize("identifier", identifier_, SerializationTarget::Attribute);

	s.serialize("TypedProperty", typedProperty_);
	s.serialize("DefaultValue", defaultValue_);
	s.serialize("DisplayName", displayName_);
	s.serialize("BoolComp", boolComp_);
	s.serialize("EffectOption", effectOption_);
}
template<typename T>
void TestPropertyTyped<T>::deserialize(Deserializer& d) {
	d.deserialize("identifier", identifier_, SerializationTarget::Attribute);

	d.deserialize("TypedProperty", typedProperty_);
	d.deserialize("DefaultValue", defaultValue_);
	d.deserialize("DisplayName", displayName_);
	d.deserialize("BoolComp", boolComp_);
	d.deserialize("EffectOption", effectOption_);
	//std::cerr << "deserialized " << getClassIdentifier() << "@" << this << " : " << typedProperty_->getIdentifier() << std::endl;

	for(auto effectOpt : effectOption_) {
		boolComp_->addProperty(effectOpt);
		effectOpt->onChange([this]() { this->notifyObserversAboutChange(); });
		boolComp_->onChange([this]() { this->notifyObserversAboutChange(); });
	}
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
	using M = std::remove_const_t<decltype(TestPropertyFactory::members)>;
	M res;
	// add TestPropertyComposite
	res[TestPropertyComposite::getClassIdentifier()] = [](){
			return std::unique_ptr<TestProperty>(new TestPropertyComposite());
		};
	// add all TestPropertyTyped
	util::for_each_type<PropertyTypes>{}(TestPropertyFactoryHelper{}, res);
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

void makeOnChange(BoolCompositeProperty* const prop) {
	prop->onChange([prop](){
			std::vector<BoolProperty*> subProps;
			for(auto boolCompProp : prop->getPropertiesByType<BoolCompositeProperty>())
				subProps.emplace_back(boolCompProp->getBoolProperty());
			for(auto boolProp : prop->getPropertiesByType<BoolProperty>())
				if(boolProp != prop->getBoolProperty())
					subProps.emplace_back(boolProp);

			if(subProps.size() > 0) {
				bool checked = false;
				for(auto* boolProp : subProps)
					checked |= boolProp->get();
				prop->setChecked(checked);
			}
		});
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
