/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>

namespace inviwo {

namespace pbt {

std::ostream& operator<<(std::ostream& out, const PropertyEffect& a) {
    static constexpr std::string_view names[] = {"EQUAL",      "NOT_EQUAL",     "LESS",
                                                 "LESS_EQUAL", "GREATER",       "GREATER_EQUAL",
                                                 "ANY",        "NOT_COMPARABLE"};
    IVW_ASSERT(static_cast<size_t>(a) < numPropertyEffects,
               "ostream& operator<< for PropertyEffect: given PropertyEffect is invalid");
    return out << names[static_cast<size_t>(a)];
}

PropertyEffect combine(const PropertyEffect& a, const PropertyEffect& b) {
    const static std::array<std::bitset<5>, numPropertyEffects> compatibility{{
        0b00100,  // EQUAL
        0b10001,  // NOT_EQUAL
        0b10000,  // LESS
        0b11000,  // LESS_EQUAL
        0b00001,  // GREATER
        0b00011,  // GREATER_EQUAL
        0b11111,  // ANY
        0b00000   // NOT_COMPARABLE
    }};
    const auto resAll =
        compatibility[static_cast<size_t>(a)] & compatibility[static_cast<size_t>(b)];

    // find the effect with the maximum number of set bits in compatibility that
    // is comparable with resAll (i.e. x & resAll == resAll
    std::pair<size_t, PropertyEffect> res(0, PropertyEffect::NOT_COMPARABLE);
    for (size_t i = 0; i < numPropertyEffects; i++) {
        const auto& comp = compatibility[i];
        if ((resAll & comp) == comp) {
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
    return reverseEffects[static_cast<size_t>(pe)];
}

Processor* getOwningProcessor(Property* const prop) {
    PropertyOwner* const owner = prop->getOwner();
    if (auto proc = dynamic_cast<Processor*>(owner)) {
        return proc;
    }
    if (auto owningProp = dynamic_cast<Property*>(owner)) {
        return getOwningProcessor(owningProp);
    }
    return nullptr;
}

}  // namespace pbt

}  // namespace inviwo
