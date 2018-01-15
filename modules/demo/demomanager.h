/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_DEMOMANAGER_H
#define IVW_DEMOMANAGER_H

#include <modules/demo/demomoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/propertyownerobserver.h>

#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/processornetworkobserver.h>

#include <modules/demo/democontroller.h>

namespace inviwo {

class InviwoApplication;
class DemoModule;

namespace demo {

/**
 * The DemoManager contains the demo state. It is also responsible for 
 * clearing, saving, and loading the animation when ever the workspace is cleared, saved, or loaded.
 *
 * @see DemoController
 */
class IVW_MODULE_DEMO_API DemoManager : public PropertyOwnerObserver,
                                        public ProcessorNetworkObserver {
public:
    DemoManager(InviwoApplication* app, DemoModule* demoModule);
    virtual ~DemoManager() = default;

    DemoController& getDemoController();
    const DemoController& getDemoController() const;

private:

    InviwoApplication* app_;

    DemoController controller_;

    WorkspaceManager::ClearHandle demoClearHandle_;
    WorkspaceManager::SerializationHandle demoSerializationHandle_;
    WorkspaceManager::DeserializationHandle demoDeserializationHandle_;
};

} // namespace demo

} // namespace inviwo

#endif // IVW_DEMOMANAGER_H

