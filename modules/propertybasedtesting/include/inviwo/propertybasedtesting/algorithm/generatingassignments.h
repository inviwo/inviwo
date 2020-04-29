#pragma once

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <vector>

namespace inviwo {

class PropertyAssignment {
public:
	virtual ~PropertyAssignment() = default;
	virtual void apply() const = 0;
	virtual bool isApplied() const = 0;
};
template<typename T>
class PropertyAssignmentTyped : public PropertyAssignment {
	OrdinalProperty<T>* const prop;
	const T value;
public:
	PropertyAssignmentTyped(OrdinalProperty<T>* const prop, const T& value)
		: prop(prop)
		, value(value) {
	}
	~PropertyAssignmentTyped() = default;
	OrdinalProperty<T>* getProperty() const {
		return prop;
	}
	const T& getValue() const {
		return value;
	}
	void apply() const override {
		prop->set(value);
	}
	bool isApplied() const override {
		return prop->get() == value;
	}
};

template<typename T>
std::vector<std::shared_ptr<PropertyAssignment>> _generateAssignments(T* const);
template<>
std::vector<std::shared_ptr<PropertyAssignment>> _generateAssignments<> (OrdinalProperty<int>* const prop);
template<>
std::vector<std::shared_ptr<PropertyAssignment>> _generateAssignments<> (IntMinMaxProperty* const prop);

}
