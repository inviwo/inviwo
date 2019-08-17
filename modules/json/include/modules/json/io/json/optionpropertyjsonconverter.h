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

#include <inviwo/core/properties/optionproperty.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {

/**
 * Converts an OptionPropertyOption to a JSON object.
 * Produces layout according to the members of OptionPropertyOption:
 * { {"value": val}, {"id": id},
 *   {"name": name}
 * }
 * @see OptionPropertyOption
 *
 * Usage example:
 * \code{.cpp}
 * OptionPropertyOption<double> p;
 * json j = p;
 * \endcode
 */
template <typename T>
void to_json(json& j, const OptionPropertyOption<T>& o) {
    j = json{{"value", o.value_}, {"id", o.id_}, {"name", o.name_}};
}

/**
 * Converts a JSON object to OptionPropertyOption.
 * Expects object layout according to the members of OptionPropertyOption:
 * { {"value": val}, {"id": id},
 *   {"name": name}
 * }
 * @see OptionPropertyOption
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<OrdinalProperty<double>>();
 * \endcode
 */
template <typename T>
void from_json(const json& j, OptionPropertyOption<T>& o) {
    if (j.empty() || !j.front().is_object()) {
        // Only support object types, i.e. [ {key: value} ]
        return;
    }
    if (j.count("value") > 0) {
        o.value_ = j.at("value").get<T>();
    }
    if (j.count("id") > 0) {
        o.id_ = j.at("id").get<std::string>();
    }
    if (j.count("name") > 0) {
        o.name_ = j.at("name").get<std::string>();
    }
}

/**
 * Converts an TemplateOptionProperty to a JSON object.
 * Produces layout according to the members of TemplateOptionProperty:
 * { {"value": val}, {"selectedIndex": selectedIndex},
 *   {"options": [OptionPropertyOption ... ]}
 * }
 * @see TemplateOptionProperty
 *
 * Usage example:
 * \code{.cpp}
 * TemplateOptionProperty<double> p;
 * json j = p;
 * \endcode
 */
template <typename T>
void to_json(json& j, const TemplateOptionProperty<T>& p) {
    j = json{
        {"value", p.get()}, {"selectedIndex", p.getSelectedIndex()}, {"options", p.getOptions()}};
}

/**
 * Converts a JSON object to an TemplateOptionProperty.
 * Expects object layout according to the members of TemplateOptionProperty:
 * { {"value": val}, {"selectedIndex": selectedIndex},
 *   {"options": [OptionPropertyOption ... ]}
 * }
 * @see TemplateOptionProperty
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<TemplateOptionProperty<double>>();
 * \endcode
 */
template <typename T>
void from_json(const json& j, TemplateOptionProperty<T>& p) {
    if (j.count("options") > 0) {
        auto options = j.at("options").get<std::vector<OptionPropertyOption<T>>>();
        p.replaceOptions(options);
    }
    if (j.count("value") > 0) {
        T value = j.at("value").get<T>();
        p.set(value);
    }
    if (j.count("selectedIndex") > 0) {
        auto selectedIndex = j.at("selectedIndex").get<size_t>();
        p.setSelectedIndex(selectedIndex);
    }
    if (j.count("selectedDisplayName") > 0) {
        auto selectedName = j.at("selectedDisplayName").get<std::string>();
        p.setSelectedDisplayName(selectedName);
    }
}

}  // namespace inviwo
