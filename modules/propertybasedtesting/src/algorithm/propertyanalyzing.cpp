#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>

namespace inviwo {

namespace pbt {

std::ostream& operator<<(std::ostream& out, const PropertyEffect& a) {
    static const std::string names[] = {"EQUAL",   "NOT_EQUAL",     "LESS", "LESS_EQUAL",
                                        "GREATER", "GREATER_EQUAL", "ANY",  "NOT_COMPARABLE"};
    IVW_ASSERT(static_cast<size_t>(a) < numPropertyEffects,
			"ostream& operator<< for PropertyEffect: given PropertyEffect is invalid");
    return out << names[static_cast<size_t>(a)];
}

PropertyEffect combine(const PropertyEffect& a, const PropertyEffect& b) {
    const static std::array<std::array<bool, 5>, numPropertyEffects> compatibility{{
        {false, false, true, false, false},  // EQUAL
        {true, false, false, false, true},   // NOT_EQUAL
        {true, false, false, false, false},  // LESS
        {true, true, false, false, false},   // LESS_EQUAL
        {false, false, false, false, true},  // GREATER
        {false, false, false, true, true},   // GREATER_EQUAL
        {true, true, true, true, true},      // ANY
        {false, false, false, false, false}  // NOT_COMPARABLE
    }};
    auto resAll = compatibility[static_cast<size_t>(a)];
    for (size_t i = 0; i < resAll.size(); i++) {
		resAll[i] &= compatibility[static_cast<size_t>(b)][i];
	}

    for (size_t i = 0; i < numPropertyEffects; i++) {
        if (resAll == compatibility[i]) {
            PropertyEffect res = PropertyEffect(i);
            if (res != PropertyEffect::NOT_COMPARABLE) return PropertyEffect(i);
        }
    }

    return PropertyEffect::NOT_COMPARABLE;
}

const PropertyEffect& reverseEffect(const PropertyEffect& pe) {
    IVW_ASSERT(static_cast<size_t>(pe) < numPropertyEffects,
			"reverseEffect: given PropertyEffect is invalid");
    const static std::array<PropertyEffect, numPropertyEffects> reverseEffects{
        PropertyEffect::EQUAL,          // EQUAL
        PropertyEffect::NOT_EQUAL,      // NOT_EQUAL
        PropertyEffect::GREATER,        // LESS
        PropertyEffect::GREATER_EQUAL,  // LESS_EQUAL
        PropertyEffect::LESS,           // GREATER
        PropertyEffect::LESS_EQUAL,     // GREATER_EQUAL
        PropertyEffect::ANY,            // ANY
        PropertyEffect::NOT_COMPARABLE  // NOT_COMPARABLE
    };
    return reverseEffects[(size_t)pe];
}

Processor* getOwningProcessor(Property* const prop) {
    PropertyOwner* const owner = prop->getOwner();
    if (Processor* const proc = dynamic_cast<Processor*>(owner)) {
		return proc;
	}
    if (Property* const owningProp = dynamic_cast<Property*>(owner)) {
        return getOwningProcessor(owningProp);
	}
    return nullptr;
}

}  // namespace pbt

}  // namespace inviwo
