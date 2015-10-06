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

#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

bool PortFactory::registerObject(PortFactoryObject *port) {
    if (!util::insert_unique(map_, port->getClassIdentifier(), port)) {
        LogWarn("Port with class name: " << port->getClassIdentifier() << " already registered");
        return false;
    }
    return true;
}

bool PortFactory::unRegisterObject(PortFactoryObject *port) {
    size_t removed = util::map_erase_remove_if(
        map_, [port](Map::value_type &elem) { return elem.second == port; });

    return removed > 0;
}

std::unique_ptr<Port> PortFactory::create(const std::string &className) const {
    return create(className, "");
}

std::unique_ptr<Port> PortFactory::create(const std::string &className,
                                          const std::string &identifier) const {
    return std::unique_ptr<Port>(util::map_find_or_null(
        map_, className, [&identifier](PortFactoryObject *o) { return o->create(identifier); }));
}

bool PortFactory::hasKey(const std::string &className) const {
    return util::has_key(map_, className);
}

std::vector<std::string> PortFactory::getKeys() const {
    auto res = std::vector<std::string>();
    for (auto &elem : map_) res.push_back(elem.first);
    return res;
}

}  // namespace
