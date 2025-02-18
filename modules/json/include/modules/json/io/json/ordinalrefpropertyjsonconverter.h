/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <modules/json/json.h>

namespace inviwo {

template <typename T>
class OrdinalRefProperty;

/**
 * Converts an OrdinalRefProperty to a JSON object.
 * Produces layout according to the members of OrdinalRefProperty:
 * { {"value": val}, {"increment": increment},
 *   {"minValue": minVal}, {"maxValue": maxVal}
 * }
 * @see OrdinalRefProperty
 *
 * Usage example:
 * \code{.cpp}
 * OrdinalRefProperty<double> p;
 * json j = p;
 * \endcode
 */
template <typename T>
void to_json(json& j, const OrdinalRefProperty<T>& p) {
    j = json{{"value", p.get()},
             {"minValue", p.getMinValue()},
             {"maxValue", p.getMaxValue()},
             {"increment", p.getIncrement()}};
}

/**
 * Converts a JSON object to an OrdinalRefProperty.
 * Expects object layout according to the members of OrdinalRefProperty:
 * { {"value": val}, {"increment": increment},
 *   {"minValue": minVal}, {"maxValue": maxVal}
 * }
 * @see OrdinalRefProperty
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<OrdinalRefProperty<double>>();
 * \endcode
 */
template <typename T>
void from_json(const json& j, OrdinalRefProperty<T>& p) {

    auto value = j.value("value", p.get());

    // Optional parameters
    auto minVal = j.value("minValue", p.getMinValue());
    auto maxVal = j.value("maxValue", p.getMaxValue());
    auto increment = j.value("increment", p.getIncrement());

    p.set(value, minVal, maxVal, increment);
}

}  // namespace inviwo
