/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PROPERTYCONVERTERMANAGER_H
#define IVW_PROPERTYCONVERTERMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/propertyconverter.h>

#include <unordered_map>

namespace inviwo {

class PropertyConverter;
class Property;

class IVW_CORE_API PropertyConverterManager : public Singleton<PropertyConverterManager> {
public:
    PropertyConverterManager();
    virtual ~PropertyConverterManager();

    template <typename T>
    void registerConvert();

    bool canConvert(const std::string &srcClassIdentifier,
                    const std::string &dstClassIdentifier) const;
    bool canConvert(const Property *srcProperty, const Property *dstProperty) const;

    const PropertyConverter *getConverter(const std::string &srcClassIdentifier,
                                          const std::string &dstClassIdentifier) const;

    const PropertyConverter *getConverter(const Property *srcProperty,
                                          const Property *dstProperty) const;

private:
    PropertyConverter identityConverter_;
    std::unordered_map<std::pair<std::string, std::string>, PropertyConverter *> converters_;
};

template <typename T>
void PropertyConverterManager::registerConvert() {
    T *converter = new T();
    std::string src = converter->getSourcePropertyClassIdenetifier();
    std::string dst = converter->getDestinationPropertyClassIdenetifier();
    if (canConvert(src, dst)) {
        LogWarn("Property Converter from type " << src << " to type " << dst
                                                << " already registered");
        delete converter;
        return;
    }
    converters_.insert(std::make_pair(std::make_pair(src, dst), converter));
}

}  // namespace

#endif  // IVW_PROPERTYCONVERTERMANAGER_H
