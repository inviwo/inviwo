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
#include <inviwo/core/properties/templateproperty.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {

/**
 * Converts an TemplateProperty to a JSON object.
 * Produces layout according to the members of TemplateProperty:
 * { {"value": val} }
 * @see TemplateProperty
 *
 * Usage example:
 * \code{.cpp}
 * TemplateProperty<double> p;
 * json j = p;
 * \endcode
 */
template <typename T>
void to_json(json& j, const TemplateProperty<T>& p) {
    j = json{{"value", p.get()}};
}

/**
 * Converts a JSON object to an TemplateProperty.
 * Expects object layout according to the members of TemplateProperty:
 * { {"value": val} }
 * @see TemplateProperty
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<TemplateProperty<double>>();
 * \endcode
 */
template <typename T>
void from_json(const json& j, TemplateProperty<T>& p) {
    auto value = j.value("value", p.get());
    p.set(value);
}

}  // namespace inviwo
