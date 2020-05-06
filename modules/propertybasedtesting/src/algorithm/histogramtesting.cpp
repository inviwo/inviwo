#include <inviwo/propertybasedtesting/algorithm/histogramtesting.h>

namespace inviwo {

namespace util {

std::ostream& operator<<(std::ostream& out, const PropertyEffect& a) {
	switch(a) {
		case PropertyEffect::EQUAL:
			return out << "EQUAL";
		case PropertyEffect::NOT_EQUAL:
			return out << "NOT_EQUAL";
		case PropertyEffect::LESS:
			return out << "LESS";
		case PropertyEffect::LESS_EQUAL:
			return out << "LESS_EQUAL";
		case PropertyEffect::GREATER:
			return out << "GREATER";
		case PropertyEffect::GREATER_EQUAL:
			return out << "GREATER_EQUAL";
		case PropertyEffect::ANY:
			return out << "ANY";
		case PropertyEffect::NOT_COMPARABLE:
			return out << "NOT_COMPARABLE";
	}
	return out << "ERROR<<PropertyEffect";
}

std::optional<PropertyEffect> combine(const PropertyEffect& a, const PropertyEffect& b) {
	const static std::array<std::array<bool,3>, numPropertyEffects> compatibility{{
			{false,true ,false}, // EQUAL
			{true ,false,true }, // NOT_EQUAL
			{true ,false,false}, // LESS
			{true ,true ,false}, // LESS_EQUAL
			{false,false,true }, // GREATER
			{false,true ,true }, // GREATER_EQUAL
			{true ,true ,true }, // ANY
			{false,false,false}  // NOT_COMPARABLE
		}};
	auto resAll = compatibility[(size_t)a];
	for(size_t i = 0; i < 3; i++)
		resAll[i] &= compatibility[(size_t)b][i];

	for(size_t i = 0; i < numPropertyEffects; i++) {
		if(resAll == compatibility[i]) {
			PropertyEffect res = PropertyEffect(i);
			if(res != PropertyEffect::NOT_COMPARABLE)
				return PropertyEffect(i);
		}
	}

	return std::nullopt;
}

const PropertyEffect& reverseEffect(const PropertyEffect& pe) {
	assert(pe != PropertyEffect::Count);
	const static std::array<PropertyEffect, numPropertyEffects> reverseEffects {
			PropertyEffect::EQUAL,			// EQUAL
			PropertyEffect::NOT_EQUAL,		// NOT_EQUAL
			PropertyEffect::GREATER,		// LESS
			PropertyEffect::GREATER_EQUAL,	// LESS_EQUAL
			PropertyEffect::LESS,			// GREATER
			PropertyEffect::LESS_EQUAL,		// GREATER_EQUAL
			PropertyEffect::ANY,			// ANY
			PropertyEffect::NOT_COMPARABLE	// NOT_COMPARABLE
		};
	return reverseEffects[(size_t)pe];
}

} // namespace util
	
} // namespace inviwo
