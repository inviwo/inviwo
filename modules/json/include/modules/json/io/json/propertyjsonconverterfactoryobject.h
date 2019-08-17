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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/property.h>
#include <modules/json/io/json/propertyjsonconverter.h>
#include <string>

namespace inviwo {

class Property;
class PropertyWidget;

class IVW_MODULE_JSON_API PropertyJSONConverterFactoryObject {
public:
    PropertyJSONConverterFactoryObject();
    virtual ~PropertyJSONConverterFactoryObject();

    virtual std::unique_ptr<PropertyJSONConverter> create(Property*) = 0;
    /**
     * Property class identifier.
     */
    virtual std::string getClassIdentifier() const = 0;

private:
};

template <typename P>
class PropertyJSONConverterFactoryObjectTemplate : public PropertyJSONConverterFactoryObject {
public:
    PropertyJSONConverterFactoryObjectTemplate() : PropertyJSONConverterFactoryObject() {}

    virtual ~PropertyJSONConverterFactoryObjectTemplate() {}

    virtual std::unique_ptr<PropertyJSONConverter> create(Property*) {
        return std::make_unique<TemplatePropertyJSONConverter<P>>();
    }

    virtual std::string getClassIdentifier() const { return PropertyTraits<P>::classIdentifier(); };
};

}  // namespace inviwo
