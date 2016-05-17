/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PORTFACTORYOBJECT_H
#define IVW_PORTFACTORYOBJECT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>

namespace inviwo {

class IVW_CORE_API InportFactoryObject {
public:
    InportFactoryObject(const std::string& className);
    virtual ~InportFactoryObject() = default;

    virtual std::unique_ptr<Inport> create() = 0;
    virtual std::unique_ptr<Inport> create(const std::string& identifier) = 0;
    const std::string& getClassIdentifier() const;

protected:
    std::string className_;
};

template <typename T>
class InportFactoryObjectTemplate : public InportFactoryObject {
public:
    InportFactoryObjectTemplate(const std::string& className) : InportFactoryObject(className) {
        static_assert(std::is_base_of<Port, T>::value, "All ports must derive from Port");
    }

    virtual ~InportFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Inport> create() override {
        return util::make_unique<T>(className_);
    }

    virtual std::unique_ptr<Inport> create(const std::string& identifier) override {
        return util::make_unique<T>(identifier);
    }
};

class IVW_CORE_API OutportFactoryObject {
public:
    OutportFactoryObject(const std::string& className);
    virtual ~OutportFactoryObject() = default;

    virtual std::unique_ptr<Outport> create() = 0;
    virtual std::unique_ptr<Outport> create(const std::string& identifier) = 0;
    const std::string& getClassIdentifier() const;

protected:
    std::string className_;
};

template <typename T>
class OutportFactoryObjectTemplate : public OutportFactoryObject {
public:
    OutportFactoryObjectTemplate(const std::string& className) : OutportFactoryObject(className) {
        static_assert(std::is_base_of<Port, T>::value, "All ports must derive from Port");
    }

    virtual ~OutportFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Outport> create() override {
        return util::make_unique<T>(className_);
    }

    virtual std::unique_ptr<Outport> create(const std::string& identifier) override {
        return util::make_unique<T>(identifier);
    }
};

}  // namespace

#endif  // IVW_PORTFACTORYOBJECT_H
