#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

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

using Test = std::vector<std::shared_ptr<PropertyAssignment>>;

template<typename T>
struct GenerateAssignments {
	std::vector<std::shared_ptr<PropertyAssignment>> operator()(T* const) const;
};

}