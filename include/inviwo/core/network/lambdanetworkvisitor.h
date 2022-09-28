/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <inviwo/core/network/networkvisitor.h>
#include <inviwo/core/util/detected.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/canvasprocessor.h>

namespace inviwo {

class NetworkVisitorEnter {};
class NetworkVisitorExit {};

/**
 * @brief A Helper class to construct a NetworkVisitor from a set of lambda functions
 *
 * A number of different overloads can be given.
 * Processors will be visited before children in order of priority:
 * * [](Processor&, NetworkVisitorEnter) -> bool
 * * [](Processor&, NetworkVisitorEnter)
 * * [](Processor&) -> bool
 * * [](Processor&)
 *
 * Processors will be visited after children:
 * * [](Processor&, NetworkVisitorExit)
 *
 * CompositeProperties will be visited before children in order of priority:
 * * [](CompositeProperty&, NetworkVisitorEnter) -> bool
 * * [](CompositeProperty&, NetworkVisitorEnter)
 * * [](CompositeProperty&) -> bool
 * * [](CompositeProperty&)
 * * [](Property&) -> bool
 * * [](Property&)
 *
 * CompositeProperties will be visited after children:
 * * [](CompositeProperty&, NetworkVisitorExit) -> void
 *
 * Properties will be visited by:
 * * [](Property&)
 *
 * ```{.cpp}
 * LambdaNetworkVisitor visitor{[](Property& property) {
 *                                  // Do something for all properties
 *                                  return true; // visit all children
 *                              },
 *                              [](Processor&, NetworkVisitorEnter) {
 *                                  // Do something for a processor before properties
 *                                  return true; // Visit children
 *                              },
 *                              [](Processor&, NetworkVisitorExit) {
 *                                  // Do something for a processor after properties
 *                              }};
 * processorNetwork->accept(visitor);
 * ```
 */
template <typename... Funcs>
struct LambdaNetworkVisitor : NetworkVisitor, Funcs... {
    LambdaNetworkVisitor(Funcs&&... args) : NetworkVisitor{}, Funcs{std::forward<Funcs>(args)}... {}
    using Funcs::operator()...;

    template <typename T>
    using ProcessorOverload = decltype(std::declval<T>()(std::declval<Processor&>()));

    template <typename T>
    using ProcessorEnter = decltype(
        std::declval<T>()(std::declval<Processor&>(), std::declval<NetworkVisitorEnter>()));
    template <typename T>
    using ProcessorExit =
        decltype(std::declval<T>()(std::declval<Processor&>(), std::declval<NetworkVisitorExit>()));

    template <typename T>
    using CompositeOverload = decltype(std::declval<T>()(std::declval<CompositeProperty&>()));

    template <typename T>
    using CompositeEnter = decltype(
        std::declval<T>()(std::declval<CompositeProperty&>(), std::declval<NetworkVisitorEnter>()));
    template <typename T>
    using CompositeExit = decltype(
        std::declval<T>()(std::declval<CompositeProperty&>(), std::declval<NetworkVisitorExit>()));

    template <typename T>
    using PropertyOverload = decltype(std::declval<T>()(std::declval<Property&>()));

    template <typename T>
    static constexpr auto isOverloadCalled =
        util::is_detected_v<ProcessorOverload, T> || util::is_detected_v<ProcessorEnter, T> ||
        util::is_detected_v<ProcessorExit, T> || util::is_detected_v<CompositeOverload, T> ||
        util::is_detected_v<CompositeEnter, T> || util::is_detected_v<CompositeExit, T> ||
        util::is_detected_v<PropertyOverload, T>;

    static_assert((isOverloadCalled<Funcs> && ...), "Some overloads will never be called");

    virtual bool enter([[maybe_unused]] Processor& processor) override {
        if constexpr (util::is_detected_exact_v<bool, ProcessorEnter, decltype(*this)>) {
            return this->operator()(processor, NetworkVisitorEnter{});
        } else if constexpr (util::is_detected_v<ProcessorEnter, decltype(*this)>) {
            this->operator()(processor, NetworkVisitorEnter{});
            return true;
        } else if constexpr (util::is_detected_exact_v<bool, ProcessorOverload, decltype(*this)>) {
            return this->operator()(processor);
        } else if constexpr (util::is_detected_v<ProcessorOverload, decltype(*this)>) {
            this->operator()(processor);
            return true;
        } else {
            return true;
        }
    }
    virtual void exit([[maybe_unused]] Processor& processor) override {
        if constexpr (util::is_detected_exact_v<void, ProcessorExit, decltype(*this)>) {
            this->operator()(processor, NetworkVisitorExit{});
        }
    }

    virtual bool enter([[maybe_unused]] CompositeProperty& compositeProperty) override {
        if constexpr (util::is_detected_exact_v<bool, CompositeEnter, decltype(*this)>) {
            return this->operator()(compositeProperty, NetworkVisitorEnter{});
        } else if constexpr (util::is_detected_v<CompositeEnter, decltype(*this)>) {
            this->operator()(compositeProperty, NetworkVisitorEnter{});
            return true;
        } else if constexpr (util::is_detected_exact_v<bool, CompositeOverload, decltype(*this)>) {
            return this->operator()(compositeProperty);
        } else if constexpr (util::is_detected_v<CompositeOverload, decltype(*this)>) {
            this->operator()(compositeProperty);
            return true;
        } else if constexpr (util::is_detected_exact_v<bool, PropertyOverload, decltype(*this)>) {
            return this->operator()(compositeProperty);
        } else if constexpr (util::is_detected_v<PropertyOverload, decltype(*this)>) {
            this->operator()(compositeProperty);
            return true;
        } else {
            return true;
        }
    }

    virtual void exit([[maybe_unused]] CompositeProperty& compositeProperty) override {
        if constexpr (util::is_detected_v<CompositeExit, decltype(*this)>) {
            this->operator()(compositeProperty, NetworkVisitorExit{});
        }
    }

    virtual void visit([[maybe_unused]] Property& property) override {
        if constexpr (util::is_detected_v<PropertyOverload, decltype(*this)>) {
            this->operator()(property);
        }
    }
};
template <class... Funcs>
LambdaNetworkVisitor(Funcs...) -> LambdaNetworkVisitor<Funcs...>;
}  // namespace inviwo
