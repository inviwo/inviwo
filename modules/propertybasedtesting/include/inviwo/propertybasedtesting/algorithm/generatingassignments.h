#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <vector>

#include <ostream>

namespace inviwo {

/*
 * Interface to property assignments, enables to apply the assignment
 * (if it not deactivated).
 * Is used mainly by TestProperty
 */
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

/* 
 * Specialization of PropertyAssignment for OrdinalProperty<T>
 */
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

/*
 * A test is a set of assignments to properties. Should contain no
 * property more than once.
 */
using Test = std::vector<std::shared_ptr<PropertyAssignment>>;

/*
 * Helper for generating assignments.
 * If you want to be able to generate assignments for new properties, you
 * probably want to add a corresponding specialization for this.
 */
template<typename T>
struct GenerateAssignments {
	std::pair<std::unique_ptr<bool>,
		std::vector<std::shared_ptr<PropertyAssignment>>> operator()(T* const) const;
};


constexpr size_t maxStepsPerVal = 3;
constexpr size_t randomValues = 3;

template<typename T>
struct GenerateAssignments<OrdinalProperty<T>> {
	std::pair<std::unique_ptr<bool>,
		std::vector<std::shared_ptr<PropertyAssignment>>> operator()(
				OrdinalProperty<T>* const prop) const {

		std::unique_ptr<bool> deactivated = std::make_unique<bool>(false);

		using value_type = typename OrdinalProperty<T>::value_type;
		std::vector<std::shared_ptr<PropertyAssignment>> res;

		const auto increment = prop->getIncrement();
		const size_t maxSteps = (prop->getMaxValue() - prop->getMinValue()) / increment;

		for(size_t stepsFromMin = 0; stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMin++)
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(
						deactivated.get(),
						prop, 
						prop->getMinValue() + stepsFromMin * increment));

		for(size_t stepsFromMax = 0; stepsFromMax < std::min(maxSteps, maxStepsPerVal); stepsFromMax++)
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(
						deactivated.get(),
						prop, 
						prop->getMaxValue() - stepsFromMax * increment));

		if(maxSteps > 0) {
			for(size_t cnt = 0; cnt < randomValues; cnt++) {
				res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(
							deactivated.get(),
							prop, 
							prop->getMinValue() + (rand() % maxSteps) * increment));
			}
		}

		return std::make_pair(std::move(deactivated), res);
	}
};

// NOTE: We assume that a MinMaxProperty<T>* may be casted to
// OrdinalProperty<T>*
template<typename T>
struct GenerateAssignments<MinMaxProperty<T>> {
	std::pair<std::unique_ptr<bool>,
		std::vector<std::shared_ptr<PropertyAssignment>>> operator()(
				MinMaxProperty<T>* const prop) const {

		std::unique_ptr<bool> deactivated = std::make_unique<bool>(false);

		using value_type = typename MinMaxProperty<T>::value_type;
		std::vector<std::shared_ptr<PropertyAssignment>> res;

		const auto minSeparation = prop->getMinSeparation();
		const size_t maxSteps = (prop->getRangeMax() - prop->getRangeMin()) / minSeparation;

		for(size_t stepsFromMin = 0; stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMin++) {
			for(size_t stepsFromMax = 0; stepsFromMax + stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMax++) {
				res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(
							deactivated.get(),
							(OrdinalProperty<value_type>*)prop, 
							value_type(	prop->getRangeMin() + stepsFromMin * minSeparation,
								prop->getRangeMax() - stepsFromMax * minSeparation)));
			}
		}

		// add some random values
		for(size_t cnt = 0; cnt < randomValues; cnt++) {
			size_t stepsFromMin = rand() % maxSteps;
			size_t stepsFromMax = rand() % (maxSteps - stepsFromMin);
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(
						deactivated.get(),
						(OrdinalProperty<value_type>*)prop,
						value_type( prop->getRangeMin() + stepsFromMin * minSeparation,
							prop->getRangeMax() - stepsFromMax * minSeparation)));
		}

		return std::make_pair(std::move(deactivated), res);
	}
};

}
