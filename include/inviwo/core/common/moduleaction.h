/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

class IVW_CORE_API ModuleCallBackActionState {
public:
    enum Status { Default =0,
                  Enabled =1,
                  Disabled =2,
                  Custom =3
                };
};

//TODO: several types of call back action required ???
class IVW_CORE_API ModuleCallbackAction {
public:
    ModuleCallbackAction(std::string actionName, InviwoModule* module,
                         ModuleCallBackActionState::Status state=ModuleCallBackActionState::Disabled);
    std::string getActionName();
    InviwoModule* getModule();
    ModuleCallback* getCallBack();
    void setActionState(ModuleCallBackActionState::Status state);
    ModuleCallBackActionState::Status getActionState();
private:
    InviwoModule* module_;
    std::string actionName_;
    // TODO: for now call backs with single argument is supported
    ModuleCallback callBack_;
    ModuleCallBackActionState::Status actionState_;
};

} // namespace

#endif // IVW_MODULE_ACTION_H
