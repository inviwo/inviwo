/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/json/jsonmoduledefine.h>  // for IVW_MODULE_JSON_API

#include <inviwo/core/common/inviwomodule.h>  // for InviwoModule
#include <inviwo/core/properties/property.h>

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/porttraits.h>

#include <modules/json/jsonconverter.h>
#include <modules/json/jsonconverterregistry.h>
#include <modules/json/jsonsupplier.h>

#include <modules/json/jsoninportconverter.h>
#include <modules/json/jsonpropertyconverter.h>

#include <nlohmann/json.hpp>

namespace inviwo {

using json = ::nlohmann::json;

class InviwoApplication;

class IVW_MODULE_JSON_API JSONModule : public InviwoModule,
                                       public JSONSupplier<Inport, PortTraits>,
                                       public JSONSupplier<Property, PropertyTraits> {
    using JSONSupplier<Inport, PortTraits>::registerJSONConverter;
    using JSONSupplier<Property, PropertyTraits>::registerJSONConverter;

public:
    JSONModule(InviwoApplication* app);
    virtual ~JSONModule();

    const JSONPropertyConverter& getJSONPropertyConverter() const { return propertyConverter_; }
    const JSONInportConverter& getJSONInportConverter() const { return inportConverter_; }

    template <typename Base>
    JSONConverterRegistry<Base>& getRegistry() {
        if constexpr (std::is_base_of_v<Property, Base>) {
            return propertyConverter_;
        } else if constexpr (std::is_base_of_v<Inport, Base>) {
            return inportConverter_;
        } else {
            throw Exception("Invalid type for JSONModule::getRegistry", IVW_CONTEXT);
        }
    }

protected:
    JSONInportConverter inportConverter_;
    JSONPropertyConverter propertyConverter_;
};

}  // namespace inviwo
