#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>

namespace inviwo {

namespace pbt {

std::ostream& operator<<(std::ostream& out, const PropertyEffect& a) {
    static constexpr std::string_view names[] = {"EQUAL",   "NOT_EQUAL",     "LESS", "LESS_EQUAL",
                                        "GREATER", "GREATER_EQUAL", "ANY",  "NOT_COMPARABLE"};
    IVW_ASSERT(static_cast<size_t>(a) < numPropertyEffects,
			"ostream& operator<< for PropertyEffect: given PropertyEffect is invalid");
    return out << names[static_cast<size_t>(a)];
}

PropertyEffect combine(const PropertyEffect& a, const PropertyEffect& b) {
	const static std::array<std::bitset<5>, numPropertyEffects> compatibility{{
		0b00100, // EQUAL
		0b10001, // NOT_EQUAL
		0b10000, // LESS
		0b11000, // LESS_EQUAL
		0b00001, // GREATER
		0b00011, // GREATER_EQUAL
		0b11111, // ANY
		0b00000  // NOT_COMPARABLE
	}};
	const auto resAll =
		compatibility[static_cast<size_t>(a)] & compatibility[static_cast<size_t>(b)];

	// find the effect with the maximum number of set bits in compatibility that
	// is comparable with resAll (i.e. x & resAll == resAll
	std::pair<size_t, PropertyEffect> res(0, PropertyEffect::NOT_COMPARABLE);
	for(size_t i = 0; i < numPropertyEffects; i++) {
		const auto& comp = compatibility[i];
		if((resAll & comp) == comp) {
			res = std::max(res, std::make_pair(comp.count(), PropertyEffect(i)));
		}
	}
	return res.second;
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
