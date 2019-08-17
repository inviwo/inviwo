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

#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <modules/json/io/json/optionpropertyjsonconverter.h>
#include <nlohmann/json.hpp>

namespace inviwo {

/**
 * Converts an DataFrameColumnProperty to a JSON object.
 * Produces layout according to the members of TemplateOptionProperty:
 * { {"value": val}, {"selectedIndex": selectedIndex},
 *   {"options": [OptionPropertyOption ... ]}
 * }
 * @see DataFrameColumnProperty
 *
 * Usage example:
 * \code{.cpp}
 * DataFrameColumnProperty p;
 * json j = p;
 * \endcode
 */
IVW_MODULE_DATAFRAME_API void to_json(nlohmann::json& j, const DataFrameColumnProperty& p);

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
IVW_MODULE_DATAFRAME_API void from_json(const nlohmann::json& j, DataFrameColumnProperty& p);

}  // namespace inviwo
