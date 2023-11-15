/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

void InviwoSetupInfo::ModuleSetupInfo::serialize(Serializer& s) const {
    s.serialize("name", name, SerializationTarget::Attribute);
    s.serialize("version", version, SerializationTarget::Attribute);
    s.serialize("Processors", processors, "Processor");
}
void InviwoSetupInfo::ModuleSetupInfo::deserialize(Deserializer& d) {
    d.deserialize("name", name, SerializationTarget::Attribute);
    d.deserialize("version", version, SerializationTarget::Attribute);
    d.deserialize("Processors", processors, "Processor");
}

InviwoSetupInfo::InviwoSetupInfo(const InviwoApplication& app, ProcessorNetwork& network) {
    std::unordered_set<std::string> usedProcessorClassIdentifiers;
    network.forEachProcessor(
        [&](const Processor* p) { usedProcessorClassIdentifiers.insert(p->getClassIdentifier()); });

    for (const auto& inviwoModule : app.getModuleManager().getInviwoModules()) {
        ModuleSetupInfo info{inviwoModule.getIdentifier(), inviwoModule.getVersion(), {}};
        for (const auto& processor : inviwoModule.getProcessors()) {
            const auto& id = processor->getClassIdentifier();
            if (usedProcessorClassIdentifiers.count(id) > 0) {
                info.processors.push_back(id);
            }
        }
        if (!info.processors.empty()) {
            modules_.push_back(std::move(info));
        }
    }
}

void InviwoSetupInfo::serialize(Serializer& s) const { s.serialize("Modules", modules_, "Module"); }
void InviwoSetupInfo::deserialize(Deserializer& d) { d.deserialize("Modules", modules_, "Module"); }

const InviwoSetupInfo::ModuleSetupInfo* InviwoSetupInfo::getModuleInfo(
    const std::string& module) const {
    auto it =
        util::find_if(modules_, [&](const ModuleSetupInfo& info) { return info.name == module; });
    if (it != modules_.end()) {
        return &(*it);
    } else {
        return nullptr;
    }
}

const InviwoSetupInfo::ModuleSetupInfo* InviwoSetupInfo::getModuleForProcessor(
    const std::string& processorClassIdentifier) const {
    for (const auto& module : modules_) {
        auto it =
            std::find(module.processors.begin(), module.processors.end(), processorClassIdentifier);
        if (it != module.processors.end()) {
            return &module;
        }
    }
    return nullptr;
}

}  // namespace inviwo
