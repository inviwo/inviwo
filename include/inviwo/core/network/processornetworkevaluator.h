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

#ifndef IVW_PROCESSORNETWORKEVALUATOR_H
#define IVW_PROCESSORNETWORKEVALUATOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/network/processornetworkevaluationobserver.h>

namespace inviwo {

class Processor;
class ProcessorNetwork;

class IVW_CORE_API ProcessorNetworkEvaluator : public ProcessorNetworkObserver,
                                               public ProcessorNetworkEvaluationObservable {
    friend class Processor;

public:
    ProcessorNetworkEvaluator(ProcessorNetwork* processorNetwork);
    virtual ~ProcessorNetworkEvaluator() = default;
    void setExceptionHandler(ExceptionHandler handler);

private:
    virtual void onProcessorNetworkEvaluateRequest() override;
    virtual void onProcessorNetworkUnlocked() override;
    virtual void onProcessorNetworkDidAddProcessor(Processor* processor) override;
    virtual void onProcessorNetworkDidRemoveProcessor(Processor* processor) override;
    virtual void onProcessorNetworkDidAddConnection(const PortConnection& connection) override;
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection& connection) override;

    void requestEvaluate();
    void evaluate();

    ProcessorNetwork* processorNetwork_;
    // the sorted list of processors obtained through topological sorting
    std::vector<Processor*> processorsSorted_;
    bool evaulationQueued_;
    ExceptionHandler exceptionHandler_;
};

}  // namespace

#endif  // IVW_PROCESSORNETWORKEVALUATOR_H
