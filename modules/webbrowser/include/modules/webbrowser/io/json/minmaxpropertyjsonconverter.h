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
#include <inviwo/core/properties/minmaxproperty.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace glm {
template <typename T, precision P, template <typename, precision> class VecType>
void from_json(const nlohmann::json& j, VecType<T, P>& v) {
    for (int i = 0; i < v.length(); i++) {
        v[i] = j[i];
    }
};

template <typename T, precision P, template <typename, precision> class VecType>
void to_json(nlohmann::json& j, const VecType<T, P>& v) {
    for (int i = 0; i < v.length(); i++) {
        j[i] = v[i];
    }
};
}  // namespace glm

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
    // Extract header and column types
    if (j.empty() || !j.front().is_object()) {
        // Only support object types, i.e. [ {key: value} ]
        return;
    }

    auto start = j.count("start") > 0 ? j.at("start").get<T>() : p.getStart();
    auto end = j.count("end") > 0 ? j.at("end").get<T>() : p.getEnd();

    auto rangeMin = j.count("rangeMin") > 0 ? j.at("rangeMin").get<T>() : p.getRangeMin();
    auto rangeMax = j.count("rangeMax") > 0 ? j.at("rangeMax").get<T>() : p.getRangeMax();

    auto increment = j.count("increment") > 0 ? j.at("increment").get<T>() : p.getIncrement();
    auto minSep =
        j.count("minSeparation") > 0 ? j.at("minSeparation").get<T>() : p.getMinSeparation();
    p.set(start, end, rangeMin, rangeMax, increment, minSep);
}

}  // namespace inviwo
