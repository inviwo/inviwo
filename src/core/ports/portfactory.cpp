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

namespace inviwo {

PortFactory::PortFactory() {}

PortFactory::~PortFactory() {}


void PortFactory::registeryObject(PortFactoryObject* port) {
    std::string className = port->getClassIdentifier();
    PortClassMap::const_iterator it = portClassMap_.find(className);

    if (it == portClassMap_.end())
        portClassMap_.insert(std::make_pair(className, port));
    else
        LogWarn("Port with class name: " << className << " already registed");
}

IvwSerializable *PortFactory::create(const std::string &className) const { return nullptr; }

bool PortFactory::isValidType(const std::string &className) const {
    PortClassMap::const_iterator it = portClassMap_.find(className);

    if (it != portClassMap_.end())
        return true;
    else
        return false;
}

Port* PortFactory::getPort(const std::string &className,const std::string &identifier) {
    PortClassMap::const_iterator it = portClassMap_.find(className);

    if (it != portClassMap_.end())
        return it->second->create(identifier);
    else
        return nullptr;
}

std::vector<std::string> PortFactory::getRegistedPortClassNames() {
    std::vector<std::string> classNames;

    for (auto &elem : portClassMap_) classNames.push_back(elem.first);

    return classNames;
}

} // namespace

