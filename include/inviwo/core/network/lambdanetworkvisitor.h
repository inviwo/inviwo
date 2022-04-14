/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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
 * ```{.cpp}
 * LambdaNetworkVisitor visitor{[](Property& property) {
 *                                  // Do something for a properties
 *                                  return true;
 *                              },
 *                              [](Processor&) {
 *                                  // Do something for a processor
 *                                  return true;
 *                              }};
 * processorNetwork->accept(visitor);
 * ```
 */
template <typename... Funcs>
struct LambdaNetworkVisitor : NetworkVisitor, Funcs... {
    LambdaNetworkVisitor(Funcs&&... args) : NetworkVisitor{}, Funcs{std::forward<Funcs>(args)}... {}
    using Funcs::operator()...;

    template <typename T>
    using ProcessorEnter = decltype(
        std::declval<T>()(std::declval<Processor&>(), std::declval<NetworkVisitorEnter>()));
    template <typename T>
    using ProcessorExit =
        decltype(std::declval<T>()(std::declval<Processor&>(), std::declval<NetworkVisitorExit>()));

    template <typename T>
    using PropertyOverload = decltype(std::declval<T>()(std::declval<Property&>()));

    template <typename T>
    using CompositeEnter = decltype(
        std::declval<T>()(std::declval<CompositeProperty&>(), std::declval<NetworkVisitorEnter>()));

    template <typename T>
    using CompositeExit = decltype(
        std::declval<T>()(std::declval<CompositeProperty&>(), std::declval<NetworkVisitorExit>()));

    virtual bool enter([[maybe_unused]] Processor& processor) override {
        if constexpr (util::is_detected_exact_v<bool, ProcessorEnter, decltype(*this)>) {
            return this->operator()(processor, NetworkVisitorEnter{});
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
        } else if constexpr (util::is_detected_exact_v<bool, PropertyOverload, decltype(*this)>) {
            return this->operator()(compositeProperty);
        } else {
            return true;
        }
    }

    virtual void exit([[maybe_unused]] CompositeProperty& compositeProperty) override {
        if constexpr (util::is_detected_exact_v<void, CompositeExit, decltype(*this)>) {
            this->operator()(compositeProperty, NetworkVisitorExit{});
        }
    }

    virtual void visit([[maybe_unused]] Property& property) override {
        if constexpr (util::is_detected_exact_v<void, PropertyOverload, decltype(*this)>) {
            this->operator()(property);
        }
    }
};
template <class... Funcs>
LambdaNetworkVisitor(Funcs...) -> LambdaNetworkVisitor<Funcs...>;
}  // namespace inviwo
