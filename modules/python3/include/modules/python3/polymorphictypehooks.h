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
#pragma once

#include <modules/python3/python3moduledefine.h>

#include <pybind11/pybind11.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/property.h>

#include <typeinfo>

namespace inviwo::detail {

IVW_MODULE_PYTHON3_API const void* castProcessor(const ::inviwo::Processor* processor,
                                                 const std::type_info*& type);
IVW_MODULE_PYTHON3_API const void* castProperty(const ::inviwo::Property* property,
                                                const std::type_info*& type);

}  // namespace inviwo::detail

/*
 * The python type caster for polymorphic types can only lookup exact
 * matches. I.e. if we have a property Basis that derives from CompositeProperty
 * which derives from Property, and we try to cast a Property pointer pointing to a Basis
 * and only Property and CompositeProperty are exposed to python and not Basis. Python will
 * not find a exact match, and the returned wrapper will be of the Pointer class used.
 * In this cases Property. To get at least some support for unexposed properties
 * derived from CompositeProperty and BaseOptionProperty we add a specialization here
 * that will mean that in the example above we will get a CompositeProperty wrapper instead of
 * of a Property wrapper.
 * see polymorphic_type_hook at the end of include/pybind11/detail/type_caster_base.h
 */
template <>
struct pybind11::polymorphic_type_hook<inviwo::Property> {
    static const void* get(const inviwo::Property* property, const std::type_info*& type) {
        return inviwo::detail::castProperty(property, type);
    }
};

template <>
struct pybind11::polymorphic_type_hook<inviwo::Processor> {
    static const void* get(const inviwo::Processor* processor, const std::type_info*& type) {
        return inviwo::detail::castProcessor(processor, type);
    }
};
