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

#include <modules/json/io/json/glmjsonconverter.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {

/**
 * Converts an MinMaxProperty to a JSON object.
 * Produces layout according to the members of MinMaxProperty:
 * { {"start": val}, {"end": val},
 *   {"minValue": minVal}, {"maxValue": maxVal},
 *   {"increment": increment}, {"minSeparation": minSep}
 * }
 * @see MinMaxProperty
 *
 * Usage example:
 * \code{.cpp}
 * MinMaxProperty<double> p;
 * json j = p;
 * \endcode
 */
template <typename T>
void to_json(json& j, const MinMaxProperty<T>& p) {
    j = json{{"start", p.getStart()},         {"end", p.getEnd()},
             {"rangeMin", p.getRangeMin()},   {"rangeMax", p.getRangeMax()},
             {"increment", p.getIncrement()}, {"minSeparation", p.getMinSeparation()}};
}

/**
 * Converts a JSON object to an MinMaxProperty.
 * Expects object layout according to the members of MinMaxProperty:
 * { {"start": val}, {"end": val},
 *   {"minValue": minVal}, {"maxValue": maxVal},
 *   {"increment": increment}, {"minSeparation": minSep}
 * }
 * @see MinMaxProperty
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<MinMaxProperty<double>>();
 * \endcode
 */
template <typename T>
void from_json(const json& j, MinMaxProperty<T>& p) {
    auto start = j.value("start", p.getStart());
    auto end = j.value("end", p.getEnd());

    auto rangeMin = j.value("rangeMin", p.getRangeMin());
    auto rangeMax = j.value("rangeMax", p.getRangeMax());

    auto increment = j.value("increment", p.getIncrement());
    auto minSep = j.value("minSeparation", p.getMinSeparation());
    p.set(start, end, rangeMin, rangeMax, increment, minSep);
}

}  // namespace inviwo
