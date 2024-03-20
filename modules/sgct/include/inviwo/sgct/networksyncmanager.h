/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#pragma once

#include <inviwo/sgct/sgctmoduledefine.h>

#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/sgct/io/communication.h>

#include <iostream>
#include <vector>
#include <functional>
#include <string_view>
#include <mutex>

namespace inviwo {

class ProcessorNetwork;
class WorkspaceManager;

class IVW_MODULE_SGCT_API NetworkSyncServer : public ProcessorNetworkObserver,
                                              public ProcessorObserver {
public:
    NetworkSyncServer(ProcessorNetwork& net);

    const std::vector<SgctCommand>& getCommands();
    void clearCommands();

    std::vector<std::byte> getEncodedCommandsAndClear();

    void showStats(bool show);

private:
    virtual void onProcessorNetworkDidAddProcessor(Processor*) override;
    virtual void onProcessorNetworkDidRemoveProcessor(Processor*) override;
    virtual void onProcessorNetworkDidAddConnection(const PortConnection&) override;
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection&) override;
    virtual void onProcessorNetworkDidAddLink(const PropertyLink&) override;
    virtual void onProcessorNetworkDidRemoveLink(const PropertyLink&) override;

    virtual void onAboutPropertyChange(Property*) override;

    void collectPropertyChanges();

    std::mutex modifiedMutex_;
    std::vector<Property*> modified_;

    std::mutex commandsMutex_;
    std::vector<SgctCommand> commands_;
    ProcessorNetwork& net_;
};

class IVW_MODULE_SGCT_API NetworkSyncClient {
public:
    NetworkSyncClient(ProcessorNetwork& net);

    void applyCommands(const std::vector<SgctCommand>& commands);

    std::function<void(bool)> onStats;

private:
    ProcessorNetwork& net_;
    WorkspaceManager& wm_;
};

}  // namespace inviwo
