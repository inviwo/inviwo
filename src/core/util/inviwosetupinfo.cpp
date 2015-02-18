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

#include <inviwo/core/util/inviwosetupinfo.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {
InviwoSetupInfo::ModuleSetupInfo::ModuleSetupInfo(const InviwoModule* module) {
    name_ = module->getIdentifier();
    const std::vector<ProcessorFactoryObject*>& processors = module->getProcessors();
    for (std::vector<ProcessorFactoryObject*>::const_iterator it = processors.begin();
         it != processors.end(); ++it) {
        processors_.push_back((*it)->getClassIdentifier());
    }
}

void InviwoSetupInfo::ModuleSetupInfo::serialize(IvwSerializer& s) const {
    s.serialize("name", name_, true);
    s.serialize("Processors", processors_, "Processor");
}
void InviwoSetupInfo::ModuleSetupInfo::deserialize(IvwDeserializer& d) {
    d.deserialize("name", name_, true);
    d.deserialize("Processors", processors_, "Processor");
}

InviwoSetupInfo::InviwoSetupInfo(const InviwoApplication* app) {
    const std::vector<InviwoModule*>& modules = app->getModules();
    for (std::vector<InviwoModule*>::const_iterator it = modules.begin(); it != modules.end();
         ++it) {
        modules_.push_back(ModuleSetupInfo(*it));
    }
}

void InviwoSetupInfo::serialize(IvwSerializer& s) const {
    s.serialize("Modules", modules_, "Module");
}
void InviwoSetupInfo::deserialize(IvwDeserializer& d) {
    d.deserialize("Modules", modules_, "Module");
}

std::string InviwoSetupInfo::getModuleForProcessor(const std::string& processor) const {
    for (std::vector<ModuleSetupInfo>::const_iterator mit = modules_.begin(); mit != modules_.end();
         ++mit) {
        for (std::vector<std::string>::const_iterator pit = mit->processors_.begin();
             pit != mit->processors_.end(); ++pit) {
            if (*pit == processor) {
                return mit->name_;
            }
        }
    }
    return "";
}

}  // namespace
