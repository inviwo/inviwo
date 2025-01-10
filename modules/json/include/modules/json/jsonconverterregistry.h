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

#include <inviwo/core/util/factory.h>
#include <modules/json/jsonconverter.h>

#include <nlohmann/json.hpp>

namespace inviwo {

template <typename Base>
class JSONConverterRegistry
    : public FactoryRegister<JSONConverter<Base>, std::string, std::string_view> {
public:
    JSONConverterRegistry() = default;
    virtual ~JSONConverterRegistry() = default;

    JSONConverterRegistry(const JSONConverterRegistry& rhs) = delete;
    JSONConverterRegistry& operator=(const JSONConverterRegistry& that) = delete;

    void toJSON(json& j, const Base& p) const;
    json toJSON(const Base& p) const;
    void fromJSON(const json& j, Base& p) const;
};

template <typename Base>
void JSONConverterRegistry<Base>::toJSON(json& j, const Base& p) const {
    if (auto* converter = this->getFactoryObject(p.getClassIdentifier())) {
        converter->toJSON(j, p);
    } else {
        throw Exception(IVW_CONTEXT, "No json converter found for type: {}",
                        p.getClassIdentifier());
    }
}
template <typename Base>
json JSONConverterRegistry<Base>::toJSON(const Base& p) const {
    json j;
    toJSON(j, p);
    return j;
}
template <typename Base>
void JSONConverterRegistry<Base>::fromJSON(const json& j, Base& p) const {
    if (auto* converter = this->getFactoryObject(p.getClassIdentifier())) {
        converter->fromJSON(j, p);
    } else {
        throw Exception(IVW_CONTEXT, "No json converter found for type: {}",
                        p.getClassIdentifier());
    }
}

}  // namespace inviwo
