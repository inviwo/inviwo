#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>

#include <inviwo/propertybasedtesting/testproperty.h>

namespace inviwo {

bool PropertyAssignment::isDeactivated() const {
	return *m_deactivated;
}
void PropertyAssignment::apply() const {
	if(!isDeactivated())
		m_apply();
}
bool PropertyAssignment::isApplied() const {
	return isDeactivated() || m_isApplied();
}

PropertyAssignment::PropertyAssignment(const bool* deactivated)
		: m_deactivated(deactivated) {
}
	
}
