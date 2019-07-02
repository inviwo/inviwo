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

#include <modules/json/jsonmoduledefine.h>
#include <inviwo/core/properties/property.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {
/**
 * Interface for converting between JSON and Property.
 */
class IVW_MODULE_JSON_API PropertyJSONConverter {
public:
    PropertyJSONConverter() = default;
    virtual ~PropertyJSONConverter() = default;

    virtual std::string getPropClassIdentifier() const = 0;

    /**
     * Converts a Property to a JSON object.
     */
    virtual void toJSON(json& j, const Property& p) const = 0;
    /**
     * Converts a JSON object to a Property to a JSON object.
     */
    virtual void fromJSON(const json& j, Property& p) const = 0;
};
/**
 * Convert between JSON and Property.
 * A TemplatePropertyJSONConverter requires implementations of to_json(json& j,
 * const SrcProperty& p) and from_json(const json& j, SrcProperty& p).
 */
template <typename SrcProperty>
class TemplatePropertyJSONConverter : public PropertyJSONConverter {
public:
    TemplatePropertyJSONConverter() = default;
    virtual ~TemplatePropertyJSONConverter() = default;

    virtual std::string getPropClassIdentifier() const override {
        return PropertyTraits<SrcProperty>::classIdentifier();
    }
    /**
     * Converts a Property to a JSON object.
     * Requires that to_json(json& j, const SrcProperty& p) has been implemented.
     */
    virtual void toJSON(json& j, const Property& p) const override {
        to_json(j, static_cast<const SrcProperty&>(p));
    }
    /**
     * Converts a JSON object to a Property to a JSON object.
     * Requires that to_json(json& j, const SrcProperty& p) has been implemented.
     */
    virtual void fromJSON(const json& j, Property& p) const override {
        j.get_to(static_cast<SrcProperty&>(p));
    }
};

}  // namespace inviwo
