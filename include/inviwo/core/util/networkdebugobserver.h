/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/network/processornetworkevaluationobserver.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/processors/processorobserver.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/properties/property.h>

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>

#include <inviwo/core/util/logcentral.h>

namespace inviwo {

/**
 * \brief Observes the network and logs
 */
struct IVW_CORE_API NetworkDebugObserver : ProcessorNetworkObserver,
                                           ProcessorNetworkEvaluationObserver,
                                           ProcessorObserver {

    void log(const SourceLocation& loc) {
        LogInfo(fmt::format("{:33}", loc.getFunction().substr(2)));
    }
    void log(const SourceLocation& loc, std::string_view msg) {
        LogInfo(fmt::format("{:33} {}", loc.getFunction().substr(2), msg));
    }
    void log(const SourceLocation& loc, std::string_view msg1, std::string_view msg2) {
        LogInfo(fmt::format("{:33} {} - {}", loc.getFunction().substr(2), msg1, msg2));
    }

    // Network
    virtual void onProcessorNetworkChange() override {}
    virtual void onProcessorNetworkEvaluateRequest() override {}
    virtual void onProcessorNetworkUnlocked() override {}

    virtual void onProcessorNetworkWillAddProcessor(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    }
    virtual void onProcessorNetworkDidAddProcessor(Processor* p) override {
        p->ProcessorObservable::addObserver(this);
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    }
    virtual void onProcessorNetworkWillRemoveProcessor(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    }
    virtual void onProcessorNetworkDidRemoveProcessor(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    }

    virtual void onProcessorNetworkWillAddConnection(const PortConnection& p) override {
        log(IVW_SOURCE_LOCATION, p.getInport()->getIdentifier(), p.getOutport()->getIdentifier());
    }
    virtual void onProcessorNetworkDidAddConnection(const PortConnection& p) override {
        log(IVW_SOURCE_LOCATION, p.getInport()->getIdentifier(), p.getOutport()->getIdentifier());
    }
    virtual void onProcessorNetworkWillRemoveConnection(const PortConnection& p) override {
        log(IVW_SOURCE_LOCATION, p.getInport()->getIdentifier(), p.getOutport()->getIdentifier());
    }
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection& p) override {
        log(IVW_SOURCE_LOCATION, p.getInport()->getIdentifier(), p.getOutport()->getIdentifier());
    }

    virtual void onProcessorNetworkWillAddLink(const PropertyLink& l) override {
        log(IVW_SOURCE_LOCATION, l.getSource()->getIdentifier(),
            l.getDestination()->getIdentifier());
    }
    virtual void onProcessorNetworkDidAddLink(const PropertyLink& l) override {
        log(IVW_SOURCE_LOCATION, l.getSource()->getIdentifier(),
            l.getDestination()->getIdentifier());
    }
    virtual void onProcessorNetworkWillRemoveLink(const PropertyLink& l) override {
        log(IVW_SOURCE_LOCATION, l.getSource()->getIdentifier(),
            l.getDestination()->getIdentifier());
    }
    virtual void onProcessorNetworkDidRemoveLink(const PropertyLink& l) override {
        log(IVW_SOURCE_LOCATION, l.getSource()->getIdentifier(),
            l.getDestination()->getIdentifier());
    }

    virtual void onProcessorBackgroundJobsChanged(Processor* p, int diff, int total) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    }

    // Evaluator
    virtual void onProcessorNetworkEvaluationBegin() override {
        LogWarn("ProcessorNetworkEvaluationBegin");
    };
    virtual void onProcessorNetworkEvaluationEnd() override {
        LogWarn("ProcessorNetworkEvaluationEnd");
    };

    // Processor
    virtual void onAboutPropertyChange(Property* p) override {
        log(IVW_SOURCE_LOCATION, p ? p->getPath() : "null");
    };
    virtual void onProcessorInvalidationBegin(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorInvalidationEnd(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorPortAdded(Processor* p, Port* port) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorPortRemoved(Processor* p, Port* port) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorAboutToProcess(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorFinishedProcess(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorSourceChanged(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorSinkChanged(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorReadyChanged(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorActiveConnectionsChanged(Processor* p) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorStartBackgroundWork(Processor* p, size_t jobs) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
    virtual void onProcessorFinishBackgroundWork(Processor* p, size_t jobs) override {
        log(IVW_SOURCE_LOCATION, p->getIdentifier());
    };
};

}  // namespace inviwo
