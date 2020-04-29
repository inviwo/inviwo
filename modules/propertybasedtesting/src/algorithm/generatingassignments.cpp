#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>

namespace inviwo {

	
constexpr size_t maxStepsPerVal = 4;
constexpr size_t randomValues = 5;

template<>
std::vector<std::shared_ptr<PropertyAssignment>> _generateAssignments<> (OrdinalProperty<int>* const prop) {
	using value_type = OrdinalProperty<int>::value_type;
	std::vector<std::shared_ptr<PropertyAssignment>> res;

	const auto increment = prop->getIncrement();
	const size_t maxSteps = (prop->getMaxValue() - prop->getMinValue()) / increment;

	for(size_t stepsFromMin = 0; stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMin++)
		res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(prop, 
					prop->getMinValue() + stepsFromMin * increment));

	for(size_t stepsFromMax = 0; stepsFromMax < std::min(maxSteps, maxStepsPerVal); stepsFromMax++)
		res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(prop, 
					prop->getMaxValue() - stepsFromMax * increment));

	if(maxSteps > 0) {
		for(size_t cnt = 0; cnt < randomValues; cnt++) {
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>(prop, 
						prop->getMinValue() + (rand() % maxSteps) * increment));
		}
	}

	return res;
}

template<>
std::vector<std::shared_ptr<PropertyAssignment>> _generateAssignments<> (IntMinMaxProperty* const prop) {
	using value_type = IntMinMaxProperty::value_type;
	std::vector<std::shared_ptr<PropertyAssignment>> res;

	const auto minSeparation = prop->getMinSeparation();
	const size_t maxSteps = (prop->getRangeMax() - prop->getRangeMin()) / minSeparation;

	for(size_t stepsFromMin = 0; stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMin++) {
		for(size_t stepsFromMax = 0; stepsFromMax + stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMax++) {
			res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>( (OrdinalProperty<value_type>*)prop, 
						value_type(	prop->getRangeMin() + stepsFromMin * minSeparation,
									prop->getRangeMax() - stepsFromMax * minSeparation)));
		}
	}

	for(size_t cnt = 0; cnt < randomValues; cnt++) {
		size_t stepsFromMin = rand() % maxSteps;
		size_t stepsFromMax = rand() % (maxSteps - stepsFromMin);
		res.emplace_back(std::make_shared<PropertyAssignmentTyped<value_type>>( (OrdinalProperty<value_type>*)prop,
						value_type( prop->getRangeMin() + stepsFromMin * minSeparation,
									prop->getRangeMax() - stepsFromMax * minSeparation)));
	}

	return res;
}

}
