/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

PortInspectorFactory::PortInspectorFactory() {}

PortInspectorFactory::~PortInspectorFactory() {
    for (PortInsectorCache::iterator cit = cache_.begin(); cit != cache_.end(); ++cit) {
        for (std::vector<PortInspector*>::iterator vit = cit->second.begin(); vit != cit->second.end(); ++vit) {
            delete *vit;
        }
    }
}

void PortInspectorFactory::registerObject(PortInspectorFactoryObject* portInspectorObj) {
    std::string className = portInspectorObj->getClassIdentifier();

    if (portInspectors_.find(className) == portInspectors_.end())
        portInspectors_.insert(std::make_pair(className, portInspectorObj));
    else
        LogWarn("PortInspector for " << className << " already registered");
}

PortInspector* PortInspectorFactory::getPortInspectorForPortClass(const std::string &className) {
    // Look in cache for an inactive port insepctor.
    PortInsectorCache::iterator cit = cache_.find(className);
    if (cit != cache_.end()) {
        for (std::vector<PortInspector*>::iterator vit = cit->second.begin(); vit != cit->second.end(); ++vit) {
            if (!(*vit)->isActive()){
                return *vit;
            }
        }
    }
    
    // Create a new port inspector
    PortInspectorMap::iterator it = portInspectors_.find(className);
    if (it != portInspectors_.end()) {
        PortInspector* p = it->second->create();
        cache_[className].push_back(p);
        return p;
    }
    return NULL;
}

bool PortInspectorFactory::isValidType(const std::string &className) const {
    PortInspectorMap::const_iterator it = portInspectors_.find(className);

    if (it != portInspectors_.end())
        return true;
    else
        return false;
}



} // namespace

