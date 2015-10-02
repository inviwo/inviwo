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

bool PropertyFactory::registerObject(PropertyFactoryObject *property) {
    if (!util::insert_unique(map_, property->getClassIdentifier(), property)) {
        LogWarn("Property with class name: " << property->getClassIdentifier()
                                             << " already registed");
        return false;
    }
    notifyObserversOnRegister(property);
    return true;
}

bool PropertyFactory::unRegisterObject(PropertyFactoryObject *property) {
    size_t removed = map_.erase(property->getClassIdentifier());
    if (removed > 0) {
        notifyObserversOnUnRegister(property);
        return true;
    }
    LogWarn("Property with class name: " << property->getClassIdentifier()
                                         << " could not be unregisted");
    return false;
}

std::unique_ptr<Property> PropertyFactory::create(const std::string &className) const {
    return create(className, "", "");
}

std::unique_ptr<Property> PropertyFactory::create(const std::string &className,
                                                  const std::string &identifier,
                                                  const std::string &displayName) const {
    return std::unique_ptr<Property>(util::map_find_or_null(
        map_, className, [&identifier, &displayName](PropertyFactoryObject *o) {
            return o->create(identifier, displayName);
        }));
}

bool PropertyFactory::hasKey(const std::string &className) const {
    return util::has_key(map_, className);
}

std::vector<std::string> PropertyFactory::getKeys() const {
    auto res = std::vector<std::string>();
    for (auto &elem : map_) res.push_back(elem.first);
    return res;
}

}  // namespace
