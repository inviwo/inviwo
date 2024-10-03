/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/python3/polymorphictypehooks.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/core/processors/canvasprocessor.h>

namespace inviwo::detail {

const void* castProcessor(const ::inviwo::Processor* processor, const std::type_info*& type) {
    if (!processor) {  // default implementation if processor is null
        type = nullptr;
        return dynamic_cast<const void*>(processor);
    }

    // check if the exact type is registered in python, then return that.
    const auto& id = typeid(*processor);
    if (::pybind11::detail::get_type_info(id)) {
        type = &id;
        return dynamic_cast<const void*>(processor);
    }

    // else check if we know a more derived base then Processor and return that.
    if (auto cp = dynamic_cast<const ::inviwo::CanvasProcessor*>(processor)) {
        type = &typeid(::inviwo::CanvasProcessor);
        return cp;
    } else {  // default implementation for processor != null
        type = &id;
        return dynamic_cast<const void*>(processor);
    }
}

const void* castProperty(const ::inviwo::Property* property, const std::type_info*& type) {
    if (!property) {  // default implementation if property is null
        type = nullptr;
        return dynamic_cast<const void*>(property);
    }

    // check if the exact type is registered in python, then return that.
    const auto& id = typeid(*property);
    if (::pybind11::detail::get_type_info(id)) {
        type = &id;
        return dynamic_cast<const void*>(property);
    }

    // else check if we know a more derived base then Property and return that.
    if (auto cp = dynamic_cast<const inviwo::BoolCompositeProperty*>(property)) {
        type = &typeid(inviwo::BoolCompositeProperty);
        return cp;
    } else if (auto lp = dynamic_cast<const inviwo::ListProperty*>(property)) {
        type = &typeid(inviwo::ListProperty);
        return lp;
    } else if (auto bp = dynamic_cast<const inviwo::CompositeProperty*>(property)) {
        type = &typeid(inviwo::CompositeProperty);
        return bp;
    } else if (auto op = dynamic_cast<const inviwo::BaseOptionProperty*>(property)) {
        type = &typeid(inviwo::BaseOptionProperty);
        return op;
    } else {  // default implementation for property != null
        type = &id;
        return dynamic_cast<const void*>(property);
    }
}

}  // namespace inviwo::detail
