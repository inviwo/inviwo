/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <warn/pop>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/optionproperty.h>

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
    static const void* get(const inviwo::Property* prop, const std::type_info*& type) {
        if (!prop) {  // default implementation if prop is null
            type = nullptr;
            return dynamic_cast<const void*>(prop);
        }

        // check if the exact type is registered in python, then return that.
        const auto& id = typeid(*prop);
        if (detail::get_type_info(id)) {
            type = &id;
            return dynamic_cast<const void*>(prop);
        }

        // else check if we know a more derived base then Property and return that.
        if (auto cp = dynamic_cast<const inviwo::BoolCompositeProperty*>(prop)) {
            type = &typeid(inviwo::BoolCompositeProperty);
            return cp;
        } else if (auto lp = dynamic_cast<const inviwo::ListProperty*>(prop)) {
            type = &typeid(inviwo::ListProperty);
            return lp;
        } else if (auto bp = dynamic_cast<const inviwo::CompositeProperty*>(prop)) {
            type = &typeid(inviwo::CompositeProperty);
            return bp;
        } else if (auto op = dynamic_cast<const inviwo::BaseOptionProperty*>(prop)) {
            type = &typeid(inviwo::BaseOptionProperty);
            return op;
        } else {  // default implementation for prop != null
            type = &id;
            return dynamic_cast<const void*>(prop);
        }
    }
};
