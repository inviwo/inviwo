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
template<typename P, typename T = typename P::value_type>
class PropertyAssignmentTyped : public PropertyAssignment {
private:
	P* const prop;
	const T value;
	
	void m_apply() const override {
		prop->set(value);
		std::cerr << prop->getDisplayName() << " : " << value << " (" << prop->get() << ")" << std::endl;
	}
	bool m_isApplied() const override {
		std::cerr << prop->getDisplayName() << " : " << prop->get() << " vs. " << value << std::endl;
		return prop->get() == value;
	}
public:
	PropertyAssignmentTyped(const bool* deactivated,
			P* const prop, const T& value)
		: PropertyAssignment(deactivated)
		, prop(prop)
		, value(value) {
	}
	~PropertyAssignmentTyped() = default;
	P* getProperty() const {
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
	std::vector<std::shared_ptr<PropertyAssignment>> operator()(T* const, const bool*) const;
};


constexpr size_t maxStepsPerVal = 3;
constexpr size_t randomValues = 3;

template<typename T>
struct GenerateAssignments<OrdinalProperty<T>> {
	std::vector<std::shared_ptr<PropertyAssignment>> operator()(
				OrdinalProperty<T>* const prop, const bool* deactivated) const {

		using P = OrdinalProperty<T>;

		std::vector<std::shared_ptr<PropertyAssignment>> res;

		const auto increment = prop->getIncrement();
		const size_t maxSteps = (prop->getMaxValue() - prop->getMinValue()) / increment;

		for(size_t stepsFromMin = 0; stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMin++)
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(
						deactivated,
						prop, 
						prop->getMinValue() + stepsFromMin * increment));

		for(size_t stepsFromMax = 0; stepsFromMax < std::min(maxSteps, maxStepsPerVal); stepsFromMax++)
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(
						deactivated,
						prop, 
						prop->getMaxValue() - stepsFromMax * increment));

		if(maxSteps > 0) {
			for(size_t cnt = 0; cnt < randomValues; cnt++) {
				res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(
							deactivated,
							prop, 
							prop->getMinValue() + (rand() % maxSteps) * increment));
			}
		}

		return res;
	}
};

// NOTE: We assume that a MinMaxProperty<T>* may be casted to
// OrdinalProperty<T>*
template<typename T>
struct GenerateAssignments<MinMaxProperty<T>> {
	std::vector<std::shared_ptr<PropertyAssignment>> operator()(
				MinMaxProperty<T>* const prop, const bool* deactivated) const {

		using P = MinMaxProperty<T>;
		using value_type = typename P::value_type;
		std::vector<std::shared_ptr<PropertyAssignment>> res;

		const auto minSeparation = prop->getMinSeparation();
		const size_t maxSteps = (prop->getRangeMax() - prop->getRangeMin()) / minSeparation;

		for(size_t stepsFromMin = 0; stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMin++) {
			for(size_t stepsFromMax = 0; stepsFromMax + stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMax++) {
				res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(
							deactivated,
							prop, 
							value_type(	prop->getRangeMin() + stepsFromMin * minSeparation,
								prop->getRangeMax() - stepsFromMax * minSeparation)));
			}
		}

		// add some random values
		for(size_t cnt = 0; cnt < randomValues; cnt++) {
			size_t stepsFromMin = rand() % maxSteps;
			size_t stepsFromMax = rand() % (maxSteps - stepsFromMin);
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<P>>(
						deactivated,
						prop,
						value_type( prop->getRangeMin() + stepsFromMin * minSeparation,
							prop->getRangeMax() - stepsFromMax * minSeparation)));
		}

		return res;
	}
};

}
