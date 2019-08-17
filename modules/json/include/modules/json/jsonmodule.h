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

#ifndef IVW_JSONMODULE_H
#define IVW_JSONMODULE_H

#include <modules/json/jsonmoduledefine.h>
#include <modules/json/io/json/propertyjsonconverterfactory.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo {

class IVW_MODULE_JSON_API JSONModule : public InviwoModule {
public:
    JSONModule(InviwoApplication* app);
    virtual ~JSONModule() = default;

    template <typename P>
    void registerPropertyJSONConverter();
    void registerPropertyJSONConverter(
        std::unique_ptr<PropertyJSONConverterFactoryObject> propertyConverter);
    inline const PropertyJSONConverterFactory* getPropertyJSONConverterFactory() const;

protected:
    // JSON Converter factory
    std::vector<std::unique_ptr<PropertyJSONConverterFactoryObject>> propertyJSONConverters_;
    PropertyJSONConverterFactory propertyJSONConverterFactory_;
};

inline const PropertyJSONConverterFactory* JSONModule::getPropertyJSONConverterFactory() const {
    return &propertyJSONConverterFactory_;
}

template <typename P>
void JSONModule::registerPropertyJSONConverter() {
    registerPropertyJSONConverter(
        std::make_unique<PropertyJSONConverterFactoryObjectTemplate<P>>());
}

}  // namespace inviwo

#endif  // IVW_JSONMODULE_H
