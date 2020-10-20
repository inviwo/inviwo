#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <vector>

#include <iostream> // TODO: remove

namespace inviwo {

class PropertyAssignment {
private:
	const bool* m_deactivated;
protected:
	virtual void m_apply() const = 0;
	virtual bool m_isApplied() const = 0;
public:
	PropertyAssignment(const bool* deactivated);
	bool isDeactivated() const;
	
	void apply() const;
	bool isApplied() const;

	virtual ~PropertyAssignment() = default;
	virtual void print(std::ostream& out) const = 0;
};

template<typename T>
class PropertyAssignmentTyped : public PropertyAssignment {
private:
	OrdinalProperty<T>* const prop;
	const T value;
	
	void m_apply() const override {
		prop->set(value);
	}
	bool m_isApplied() const override {
		return prop->get() == value;
	}
public:
	PropertyAssignmentTyped(const bool* deactivated,
			OrdinalProperty<T>* const prop, const T& value)
		: PropertyAssignment(deactivated)
		, prop(prop)
		, value(value) {
	}
	~PropertyAssignmentTyped() = default;
	OrdinalProperty<T>* getProperty() const {
		return prop;
	}
	const T& getValue() const {
		return value;
	}
	
	void print(std::ostream& out) const override {
		out << "PropertyAssignment(" << prop->getIdentifier() << "," << getValue() << ")";
	}
};

using Test = std::vector<std::shared_ptr<PropertyAssignment>>;

template<typename T>
struct GenerateAssignments {
	std::pair<std::unique_ptr<bool>,
		std::vector<std::shared_ptr<PropertyAssignment>>> operator()(T* const) const;
};

}
