/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_INVIWOSETUPINFO_H
#define IVW_INVIWOSETUPINFO_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo {

struct IVW_CORE_API InviwoSetupInfo : public IvwSerializable {
    struct ModuleSetupInfo : public IvwSerializable {
        ModuleSetupInfo() : name_("") {}
        ModuleSetupInfo(const InviwoModule* module) {
            name_ = module->getIdentifier();
            const std::vector<ProcessorFactoryObject*>& processors = module->getProcessors();
            for (std::vector<ProcessorFactoryObject*>::const_iterator it = processors.cbegin();
                 it != processors.cend(); ++it) {
                processors_.push_back((*it)->getClassIdentifier());
            }
        }
        virtual void serialize(IvwSerializer& s) const {
            s.serialize("name", name_, true);
            s.serialize("Processors", processors_, "Processor");
        }
        virtual void deserialize(IvwDeserializer& d) {
            d.deserialize("name", name_, true);
            d.deserialize("Processors", processors_, "Processor");
        }

        std::string name_;
        std::vector<std::string> processors_;
    };

    InviwoSetupInfo(){};
    InviwoSetupInfo(const InviwoApplication* app) {
        const std::vector<InviwoModule*>& modules = app->getModules();
        for (std::vector<InviwoModule*>::const_iterator it = modules.cbegin(); it != modules.cend(); ++it) {
            modules_.push_back(ModuleSetupInfo(*it));
        }
    };
    virtual void serialize(IvwSerializer& s) const { s.serialize("Modules", modules_, "Module"); }
    virtual void deserialize(IvwDeserializer& d) { d.deserialize("Modules", modules_, "Module"); }
    std::vector<ModuleSetupInfo> modules_;
};

}  // namespace

#endif  // IVW_INVIWOSETUPINFO_H
