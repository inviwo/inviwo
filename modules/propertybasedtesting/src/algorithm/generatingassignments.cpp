#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>

#include <inviwo/propertybasedtesting/testproperty.h>

namespace inviwo {

namespace pbt {

bool PropertyAssignment::isDeactivated() const { return *deactivated_; }
void PropertyAssignment::apply() const {
    if (!isDeactivated()) {
        doApply();
        IVW_ASSERT(isApplied(), "PropertyAssignment could not be applied!");
    }
}
bool PropertyAssignment::isApplied() const { return isDeactivated() || doIsApplied(); }

PropertyAssignment::PropertyAssignment(const bool* deactivated) : deactivated_(deactivated) {}

}  // namespace pbt

}  // namespace inviwo
