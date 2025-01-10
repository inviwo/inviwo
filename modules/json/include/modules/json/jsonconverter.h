/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/json/jsonmoduledefine.h>

#include <string>
#include <nlohmann/json.hpp>

namespace inviwo {

using json = ::nlohmann::json;

template <typename Base>
class JSONConverter {
public:
    virtual ~JSONConverter() = default;
    virtual std::string_view getClassIdentifier() const = 0;
    virtual void toJSON(json& j, const Base& p) const = 0;
    virtual void fromJSON(const json& j, Base& p) const = 0;
};

/**
 * Convert between JSON and Property.
 * A TemplatePropertyJSONConverter requires implementations of to_json(json& j,
 * const TProperty& p) and from_json(const json& j, TProperty& p).
 */
template <typename Base, typename Derived, template <typename...> typename Traits>
    requires std::is_base_of_v<Base, Derived>
class TemplateJSONConverter : public JSONConverter<Base> {
public:
    virtual std::string_view getClassIdentifier() const override {
        return Traits<Derived>::classIdentifier();
    }
    /**
     * Converts a Derived to a JSON object.
     * Requires that to_json(json& j, const Derived& p) has been implemented.
     */
    virtual void toJSON(json& j, const Base& p) const override {
        to_json(j, static_cast<const Derived&>(p));
    }
    /**
     * Converts a JSON object to a Derived to a JSON object.
     * Requires that to_json(json& j, const Derived& p) has been implemented.
     */
    virtual void fromJSON(const json& j, Base& p) const override {
        j.get_to(static_cast<Derived&>(p));
    }
};

}  // namespace inviwo
