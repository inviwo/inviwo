/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <modules/python3/processortrampoline.h>

#include <pybind11/pybind11.h>  // for get_override, PYBIND11_OVERLOAD

#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel
#include <inviwo/core/util/exception.h>

#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

namespace inviwo {

class Event;
class Outport;
class Property;

void ProcessorTrampoline::initializeResources() {
    PYBIND11_OVERLOAD(void, Processor, initializeResources, );
}
void ProcessorTrampoline::process() { PYBIND11_OVERLOAD(void, Processor, process, ); }
void ProcessorTrampoline::doIfNotReady() { PYBIND11_OVERLOAD(void, Processor, doIfNotReady, ); }
void ProcessorTrampoline::setValid() { PYBIND11_OVERLOAD(void, Processor, setValid, ); }
void ProcessorTrampoline::invalidate(InvalidationLevel invalidationLevel,
                                     Property* modifiedProperty) {
    PYBIND11_OVERLOAD(void, Processor, invalidate, invalidationLevel, modifiedProperty);
}
const ProcessorInfo& ProcessorTrampoline::getProcessorInfo() const {
    // We use a custom implementation here since PYBIND11_OVERLOAD_PURE struggles with
    // keeping the processor info object alive.
    // PYBIND11_OVERLOAD_PURE(const ProcessorInfo&, Processor, getProcessorInfo, );

    if (!info_) {
        const pybind11::gil_scoped_acquire gil;
        const pybind11::function f =
            pybind11::get_override(static_cast<const Processor*>(this), "getProcessorInfo");
        if (f) {
            info_ = f().cast<ProcessorInfo>();
        } else {
            throw Exception("Missing getProcessorInfo member function in python processor");
        }
    }
    return info_.value();
}

void ProcessorTrampoline::invokeEvent(Event* event) {
    PYBIND11_OVERLOAD(void, Processor, invokeEvent, event);
}
void ProcessorTrampoline::propagateEvent(Event* event, Outport* source) {
    PYBIND11_OVERLOAD(void, Processor, propagateEvent, event, source);
}

void ProcessorTrampoline::serialize(Serializer& s) const {

    const pybind11::gil_scoped_acquire gil;
    const auto override = pybind11::get_override(static_cast<const Processor*>(this), "serialize");
    if (override) {
        auto o = override.template operator()<pybind11::return_value_policy::reference>(s);
    }
    Processor::serialize(s);
}

void ProcessorTrampoline::deserialize(Deserializer& d) {

    const pybind11::gil_scoped_acquire gil;
    const auto override =
        pybind11::get_override(static_cast<const Processor*>(this), "deserialize");
    if (override) {
        override.template operator()<pybind11::return_value_policy::reference>(d);
    }
    Processor::deserialize(d);
}

}  // namespace inviwo
