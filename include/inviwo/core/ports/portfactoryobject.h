/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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
#include <inviwo/core/ports/port.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/porttraits.h>
#include <inviwo/core/util/demangle.h>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace inviwo {

class IVW_CORE_API InportFactoryObject {
public:
    InportFactoryObject(std::string_view classIdentifier, std::string_view typeName);
    virtual ~InportFactoryObject() = default;

    virtual std::unique_ptr<Inport> create() = 0;
    virtual std::unique_ptr<Inport> create(std::string_view identifier) = 0;
    const std::string& getClassIdentifier() const;
    const std::string& getTypeName() const;

protected:
    std::string classIdentifier_;
    std::string typeName_;
};

template <typename T>
class InportFactoryObjectTemplate : public InportFactoryObject {
public:
    InportFactoryObjectTemplate()
        : InportFactoryObject(PortTraits<T>::classIdentifier(), util::demangle(typeid(T).name())) {
        static_assert(std::is_base_of<Inport, T>::value, "All inports must derive from Inport");
    }

    virtual ~InportFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Inport> create() override {
        return std::make_unique<T>(classIdentifier_);
    }

    virtual std::unique_ptr<Inport> create(std::string_view identifier) override {
        return std::make_unique<T>(identifier);
    }
};

class IVW_CORE_API OutportFactoryObject {
public:
    OutportFactoryObject(std::string_view classIdentifier, std::string_view typeName);
    virtual ~OutportFactoryObject() = default;

    virtual std::unique_ptr<Outport> create() = 0;
    virtual std::unique_ptr<Outport> create(std::string_view identifier) = 0;
    const std::string& getClassIdentifier() const;
    const std::string& getTypeName() const;

protected:
    std::string classIdentifier_;
    std::string typeName_;
};

template <typename T>
class OutportFactoryObjectTemplate : public OutportFactoryObject {
public:
    OutportFactoryObjectTemplate()
        : OutportFactoryObject(PortTraits<T>::classIdentifier(), util::demangle(typeid(T).name())) {
        static_assert(std::is_base_of<Outport, T>::value, "All outports must derive from Outport");
    }

    virtual ~OutportFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Outport> create() override {
        return std::make_unique<T>(classIdentifier_);
    }

    virtual std::unique_ptr<Outport> create(std::string_view identifier) override {
        if constexpr (std::is_constructible_v<T, std::string_view>) {
            return std::make_unique<T>(identifier);
        } else {
            return std::make_unique<T>(std::string(identifier));
        }
    }
};

}  // namespace inviwo
