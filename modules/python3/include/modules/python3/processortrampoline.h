/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/python3/python3moduledefine.h>  // for IVW_MODULE_PYTHON3_API

#include <pybind11/trampoline_self_life_support.h>  // for trampoline_self_life_support

#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel

namespace inviwo {

class Event;
class Outport;
class Property;

/*
 * We need to export this class other wise pybind11 will not see that it derives from
 * trampoline_self_life_support and we get runtime errors like "Alias class (also known as
 * trampoline) does not inherit from py::trampoline_self_life_support, therefore the ownership of
 * this instance cannot safely be transferred to C++.". Down side it that this will cause warnings:
 * "inviwo::ProcessorTrampoline’ declared with greater visibility than its base
 * ‘pybind11::trampoline_self_life_support’" for now we will ignore those.
 *
 * Im currently not 100% sure if we need do this for all trampolines.
 */

#include <warn/push>
#include <warn/ignore/dll-interface-base>
#include <warn/ignore/attributes>
class IVW_MODULE_PYTHON3_API ProcessorTrampoline : public Processor,
                                                   public pybind11::trampoline_self_life_support {
public:
    // Inherit the constructors
    using Processor::Processor;

    // Trampoline (need one for each virtual function)
    virtual void initializeResources() override;
    virtual void process() override;
    virtual void doIfNotReady() override;
    virtual void setValid() override;
    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* modifiedProperty = nullptr) override;
    virtual const ProcessorInfo& getProcessorInfo() const override;
    virtual void invokeEvent(Event* event) override;
    virtual void propagateEvent(Event* event, Outport* source) override;
};
#include <warn/pop>

}  // namespace inviwo
