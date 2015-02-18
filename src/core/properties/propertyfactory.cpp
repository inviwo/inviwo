/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/properties/propertyfactory.h>


namespace inviwo {

PropertyFactory::PropertyFactory() {}

PropertyFactory::~PropertyFactory() {}


void PropertyFactory::registeryObject(PropertyFactoryObject* property) {
    std::string className = property->getClassIdentifier();
    PropertyClassMap::const_iterator it = propertyClassMap_.find(className);

    if (it == propertyClassMap_.end())
        propertyClassMap_.insert(std::make_pair(className, property));
    else
        LogWarn("Property with class name: " << className << " already registed");
        
}

IvwSerializable* PropertyFactory::create(const std::string &className) const {
    return NULL;
}

bool PropertyFactory::isValidType(const std::string &className) const {
    PropertyClassMap::const_iterator it = propertyClassMap_.find(className);

    if (it != propertyClassMap_.end())
        return true;
    else
        return false;
}

Property* PropertyFactory::getProperty(const std::string &className, const std::string &identifier, const std::string &displayName) {
    PropertyClassMap::const_iterator it = propertyClassMap_.find(className);

    if (it != propertyClassMap_.end())
        return it->second->create(identifier, displayName);
    else
        return NULL;
}

std::vector<std::string> PropertyFactory::getRegistedPropertyClassNames() {
    std::vector<std::string> classNames;

    for (PropertyClassMap::iterator it = propertyClassMap_.begin(); it != propertyClassMap_.end(); ++it)
        classNames.push_back(it->first);

    return classNames;
}

} // namespace
