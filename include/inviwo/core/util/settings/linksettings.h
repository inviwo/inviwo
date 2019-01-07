/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_LINKSETTINGS_H
#define IVW_LINKSETTINGS_H

#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/boolproperty.h>

#include <unordered_map>

namespace inviwo {

class IVW_CORE_API LinkSettings : public Settings, public FactoryObserver<PropertyFactoryObject> {
public:
    LinkSettings(const std::string& id, PropertyFactory* factory);
    LinkSettings(const LinkSettings&) = delete;
    LinkSettings& operator=(const LinkSettings&) = delete;
    virtual bool isLinkable(const Property* property) const;

    virtual void onRegister(PropertyFactoryObject* p) override;

    void registerProperty(std::string property);

    virtual void onUnRegister(PropertyFactoryObject* p) override;

private:
    CompositeProperty linkProperties_;
    std::unordered_map<std::string, BoolProperty*> propertyMap_;
};

}  // namespace inviwo

#endif  // IVW_LINKSETTINGS_H
