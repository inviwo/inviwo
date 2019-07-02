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
#include <modules/json/io/json/propertyjsonconverterfactory.h>
#include <modules/webbrowser/properties/propertywidgetcef.h>
#include <string>

namespace inviwo {

class Property;
class PropertyWidget;

class IVW_MODULE_WEBBROWSER_API PropertyWidgetCEFFactoryObject {
public:
    PropertyWidgetCEFFactoryObject(const PropertyJSONConverterFactory* converterFactory);
    virtual ~PropertyWidgetCEFFactoryObject();

    virtual std::unique_ptr<PropertyWidgetCEF> create(Property*) = 0;

    virtual std::string getClassIdentifier() const = 0;

protected:
    const PropertyJSONConverterFactory* converterFactory_;
};

template <typename T, typename P>
class PropertyWidgetCEFFactoryObjectTemplate : public PropertyWidgetCEFFactoryObject {
public:
    PropertyWidgetCEFFactoryObjectTemplate(const PropertyJSONConverterFactory* converterFactory)
        : PropertyWidgetCEFFactoryObject(converterFactory) {}

    virtual ~PropertyWidgetCEFFactoryObjectTemplate() {}

    virtual std::unique_ptr<PropertyWidgetCEF> create(Property* prop) {
        return std::make_unique<T>(prop, converterFactory_->create(getClassIdentifier(), prop));
    }

    virtual std::string getClassIdentifier() const { return PropertyTraits<P>::classIdentifier(); };
};

}  // namespace inviwo
