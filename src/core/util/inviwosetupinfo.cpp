/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
InviwoSetupInfo::ModuleSetupInfo::ModuleSetupInfo(const InviwoModule* module)
    : name_(module->getIdentifier()), version_(module->getVersion()) {
    for (const auto& processor : module->getProcessors()) {
        processors_.push_back((processor)->getClassIdentifier());
    }
}

void InviwoSetupInfo::ModuleSetupInfo::serialize(Serializer& s) const {
    s.serialize("name", name_, SerializationTarget::Attribute);
    s.serialize("version", version_, SerializationTarget::Attribute);
    s.serialize("Processors", processors_, "Processor");
}
void InviwoSetupInfo::ModuleSetupInfo::deserialize(Deserializer& d) {
    d.deserialize("name", name_, SerializationTarget::Attribute);
    d.deserialize("version", version_, SerializationTarget::Attribute);
    d.deserialize("Processors", processors_, "Processor");
}

InviwoSetupInfo::InviwoSetupInfo(const InviwoApplication* app) {
    auto& modules = app->getModules();
    for (auto& module : modules) {
        modules_.push_back(ModuleSetupInfo(module.get()));
    }
}

void InviwoSetupInfo::serialize(Serializer& s) const { s.serialize("Modules", modules_, "Module"); }
void InviwoSetupInfo::deserialize(Deserializer& d) { d.deserialize("Modules", modules_, "Module"); }

const InviwoSetupInfo::ModuleSetupInfo* InviwoSetupInfo::getModuleInfo(
    const std::string& module) const {
    auto it =
        util::find_if(modules_, [&](const ModuleSetupInfo& info) { return info.name_ == module; });
    if (it != modules_.end()) {
        return &(*it);
    } else {
        return nullptr;
    }
}

std::string InviwoSetupInfo::getModuleForProcessor(const std::string& processor) const {
    for (const auto& elem : modules_) {
        for (auto pit = elem.processors_.cbegin(); pit != elem.processors_.cend(); ++pit) {
            if (*pit == processor) {
                return elem.name_;
            }
        }
    }
    return "";
}

}  // namespace inviwo
