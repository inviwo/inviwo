/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_MODULE_ACTION_H
#define IVW_MODULE_ACTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/modulecallback.h>

namespace inviwo {

class InviwoModule;

enum class ModuleCallBackActionState { Default = 0, Enabled = 1, Disabled = 2, Custom = 3 };

/**
 * A Module can register ModuleCallbackActions with the InviwoApplication. These action will the
 * Be added a some places, for example as property widget context menu options.
 */
class IVW_CORE_API ModuleCallbackAction {
public:
    ModuleCallbackAction(const std::string& actionName, InviwoModule* module,
                         ModuleCallBackActionState state = ModuleCallBackActionState::Disabled);
    const std::string& getActionName() const;
    InviwoModule* getModule() const;
    ModuleCallback& getCallBack();
    const ModuleCallback& getCallBack() const;
    void setActionState(ModuleCallBackActionState state);
    ModuleCallBackActionState getActionState() const;

private:
    InviwoModule* module_;
    std::string actionName_;

    ModuleCallback callBack_;
    ModuleCallBackActionState actionState_;
};

}  // namespace inviwo

#endif  // IVW_MODULE_ACTION_H
