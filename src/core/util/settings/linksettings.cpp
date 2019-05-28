/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

    load();

    for (auto prop : linkProperties_.getProperties()) {
        propertyMap_[prop->getIdentifier()] = static_cast<BoolProperty*>(prop);
        prop->setVisible(false);
    }

    auto properties = factory->getKeys();
    std::sort(properties.begin(), properties.end());
    for (auto& property : properties) registerProperty(property);

    factory->addObserver(this);
}

void LinkSettings::onRegister(PropertyFactoryObject* p) {
    auto property = p->getClassIdentifier();
    registerProperty(property);
}

void LinkSettings::registerProperty(std::string property) {
    const bool enabled =
        (property == PropertyTraits<CameraProperty>::classIdentifier()) != 0 ? true : false;
    // Have to check we already have a property from deserialization.
    auto ids = "link" + dotSeperatedToPascalCase(property);

    auto it = propertyMap_.find(ids);
    if (it != propertyMap_.end()) {
        it->second->setVisible(true);
    } else {
        auto linkPropery = std::make_unique<BoolProperty>(ids, property, enabled);
        linkPropery->setSerializationMode(PropertySerializationMode::All);
        propertyMap_[ids] = linkPropery.get();
        linkProperties_.addProperty(linkPropery.release(), true);
    }
}

void LinkSettings::onUnRegister(PropertyFactoryObject* p) {
    auto ids = "link" + dotSeperatedToPascalCase(p->getClassIdentifier());
    auto it = propertyMap_.find(ids);
    if (it != propertyMap_.end()) {
        it->second->setVisible(false);
    }
}

bool LinkSettings::isLinkable(const Property* property) const {
    auto ids = "link" + dotSeperatedToPascalCase(property->getClassIdentifier());
    auto it = propertyMap_.find(ids);
    if (it != propertyMap_.end()) {
        return it->second->get();
    }
    return false;
}

}  // namespace inviwo
