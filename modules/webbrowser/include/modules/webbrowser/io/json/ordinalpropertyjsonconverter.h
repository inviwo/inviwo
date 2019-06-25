/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/io/json/glmjsonconverter.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {

/**
 * Converts an OrdinalProperty to a JSON object.
 * Produces layout according to the members of OrdinalProperty:
 * { {"value": val}, {"increment": increment},
 *   {"minValue": minVal}, {"maxValue": maxVal}
 * }
 * @see OrdinalProperty
 *
 * Usage example:
 * \code{.cpp}
 * OrdinalProperty<double> p;
 * json j = p;
 * \endcode
 */
template <typename T>
void to_json(json& j, const OrdinalProperty<T>& p) {
    j = json{{"value", p.get()},
             {"minValue", p.getMinValue()},
             {"maxValue", p.getMaxValue()},
             {"increment", p.getIncrement()}};
}

/**
 * Converts a JSON object to an OrdinalProperty.
 * Expects object layout according to the members of OrdinalProperty:
 * { {"value": val}, {"increment": increment},
 *   {"minValue": minVal}, {"maxValue": maxVal}
 * }
 * @see OrdinalProperty
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<OrdinalProperty<double>>();
 * \endcode
 */
template <typename T>
void from_json(const json& j, OrdinalProperty<T>& p) {

    auto value = j.count("value") > 0 ? j.at("value").get<T>() : p.get();

    // Optional parameters
    auto minVal = j.count("minValue") > 0 ? j.at("minValue").get<T>() : p.getMinValue();
    auto maxVal = j.count("maxValue") > 0 ? j.at("maxValue").get<T>() : p.getMaxValue();
    auto increment = j.count("increment") > 0 ? j.at("increment").get<T>() : p.getIncrement();

    p.set(value, minVal, maxVal, increment);
}

}  // namespace inviwo

