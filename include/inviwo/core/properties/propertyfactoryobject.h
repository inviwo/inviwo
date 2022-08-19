/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/stringconversion.h>
#include <string_view>

namespace inviwo {

class IVW_CORE_API PropertyFactoryObject {
public:
    PropertyFactoryObject(std::string_view className, std::string_view typeName);
    virtual ~PropertyFactoryObject();

    virtual std::unique_ptr<Property> create(std::string_view identifier,
                                             std::string_view displayName) = 0;

    const std::string& getClassIdentifier() const;
    const std::string& getTypeName() const;

private:
    std::string classIdentifier_;
    std::string typeName_;
};

template <typename T>
class PropertyFactoryObjectTemplate : public PropertyFactoryObject {
public:
    PropertyFactoryObjectTemplate()
        : PropertyFactoryObject(PropertyTraits<T>::classIdentifier(),
                                util::demangle(typeid(T).name())) {}

    virtual ~PropertyFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Property> create(std::string_view identifier,
                                             std::string_view displayName) {
        if constexpr (std::is_constructible_v<T, std::string_view, std::string_view>) {
            return std::make_unique<T>(identifier, displayName);
        } else {
            return std::make_unique<T>(std::string(identifier), std::string(displayName));
        }
    }
};

}  // namespace inviwo
