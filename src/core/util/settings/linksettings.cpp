/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

LinkSettings::LinkSettings(const std::string& id, PropertyFactory* factory)
    : Settings(id)
    , FactoryObserver<PropertyFactoryObject>()
    , linkProperties_("autoLinkProperties", "Auto Link Properties") {

    addProperty(linkProperties_);

    auto properties = factory->getKeys();
    std::sort(properties.begin(), properties.end());

    for (auto& property : properties) {
        //enable camera prop linking by default.
        bool enabled = (property == CameraProperty::CLASS_IDENTIFIER) != 0 ? true : false;
        auto ids = dotSeperatedToPascalCase(property);
        auto linkPropery = util::make_unique<BoolProperty>("link" + ids, property, enabled);
        linkProperties_.addProperty(linkPropery.get(), false);
        propertyMap_[ids] = std::move(linkPropery);
    }
    
    factory->addObserver(this);
}

void LinkSettings::onRegister(PropertyFactoryObject* p) {
    auto property = p->getClassIdentifier();
    bool enabled = (property == CameraProperty::CLASS_IDENTIFIER) != 0 ? true : false;
    // Have to check we we already have a property from deserialization.
    auto id = dotSeperatedToPascalCase(property);
    if (linkProperties_.getPropertyByIdentifier("link" + id) == nullptr) {
        auto linkPropery = util::make_unique<BoolProperty>("link" + id, property, enabled);
        linkProperties_.addProperty(linkPropery.get(), false);
        propertyMap_[id] = std::move(linkPropery);
    }
}

void LinkSettings::onUnRegister(PropertyFactoryObject* p) {
    auto id = dotSeperatedToPascalCase(p->getClassIdentifier());
    linkProperties_.removeProperty("link" + id);
    propertyMap_.erase(id);
}

bool LinkSettings::isLinkable(const Property* property) const {
    auto id = dotSeperatedToPascalCase(property->getClassIdentifier());
    auto it  = propertyMap_.find(id);
    if (it != propertyMap_.end()) {
        return it->second->get();
    }
    return false;
}

} // namespace
