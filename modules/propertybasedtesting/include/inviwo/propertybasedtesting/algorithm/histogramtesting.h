/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2020 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>
#include <optional>

namespace inviwo {

namespace util {

// how the number of non-background pixels should change when the value is
// increased
enum class PropertyEffect {
	EQUAL = 0,
	NOT_EQUAL,
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	ANY,
	NOT_COMPARABLE
};
constexpr size_t numPropertyEffects = 1 +
	static_cast<size_t>(PropertyEffect::NOT_COMPARABLE);

template<typename A, typename B>
bool propertyEffectComparator(const PropertyEffect& e, const A& a, const B& b) {
	assert(static_cast<size_t>(e) < numPropertyEffects);
	switch(e) {
		case PropertyEffect::NOT_COMPARABLE:
			return false;
		case PropertyEffect::ANY:
			return true;
		case PropertyEffect::NOT_EQUAL:
			return a != b;
		case PropertyEffect::EQUAL:
			return a == b;
		case util::PropertyEffect::LESS:
			return a < b;
		case util::PropertyEffect::LESS_EQUAL:
			return a <= b;
		case util::PropertyEffect::GREATER:
			return a > b;
		case util::PropertyEffect::GREATER_EQUAL:
			return a >= b;
	}
	assert(false);
}

using AssignmentComparator = std::function<std::optional<util::PropertyEffect>(
						const std::shared_ptr<PropertyAssignment>& oldVal,
						const std::shared_ptr<PropertyAssignment>& newVal)>;

std::ostream& operator<<(std::ostream& out, const PropertyEffect& a);

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::optional<T>& a) {
	if(!a) return out << "{}";
	return out << "{" << *a << "}";
}

std::optional<PropertyEffect> combine(const PropertyEffect& a, const PropertyEffect& b);

const PropertyEffect& reverseEffect(const PropertyEffect& pe);

/**
 * returns the desired effect on the number of non-background pixels,
 * ANY if no preference
 */
template<typename T>
PropertyEffect propertyEffect(const PropertyEffect& selectedEffect,
		const T& newVal,
		const T& oldVal) {
	if(newVal < oldVal)
		return selectedEffect;
	else if(newVal == oldVal)
		return PropertyEffect::ANY;
	else
		return reverseEffect(selectedEffect);
}

template<typename T, size_t N>
struct GetComponent {
	static typename T::value_type get(const T& v, size_t i) {
		return v[i];
	}
};
template<typename T>
struct GetComponent<T,1> {
	static T get(const T& v, size_t i) {
		assert(i == 0);
		return v;
	}
};

std::optional<Processor*> getOwningProcessor(Property* const prop);

} // util

} // inviwo
