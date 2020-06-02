/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <utility>

namespace inviwo {

class Processor;
class PropertyOwner;
class Property;
class CompositeProperty;

/**
 * \brief A vistor base for visiting an inviwo network
 */
class IVW_CORE_API NetworkVisitor {
public:
    virtual ~NetworkVisitor() = default;

    virtual bool visit(Processor&) { return true; }
    virtual bool visit(PropertyOwner&) { return true; }
    virtual bool visit(Property&) { return true; }
    virtual bool visit(CompositeProperty&) { return true; }
};

template <typename... Funcs>
struct LambdaNetworkVisitor : NetworkVisitor, Funcs... {
    LambdaNetworkVisitor(Funcs&&... args) : NetworkVisitor{}, Funcs{std::forward<Funcs>(args)}... {}
    using Funcs::operator()...;

    virtual bool visit(Processor& processor) override { return operator()(processor); }
    virtual bool visit(PropertyOwner& propertyOwner) override { return operator()(propertyOwner); }
    virtual bool visit(Property& property) override { return operator()(property); }
    virtual bool visit(CompositeProperty& compositeProperty) override {
        return operator()(compositeProperty);
    }
};
template <class... Funcs>
LambdaNetworkVisitor(Funcs...) -> LambdaNetworkVisitor<Funcs...>;

}  // namespace inviwo
