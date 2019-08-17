/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <typeindex>
#include <memory>

namespace inviwo {

/**
 * Base class for RepresentationFactoryObject
 * @see RepresentationFactoryObject
 */
class IVW_CORE_API BaseRepresentationFactoryObject {
public:
    BaseRepresentationFactoryObject() = default;
    virtual ~BaseRepresentationFactoryObject() = default;
};

/**
 * Factory object for creating DataRepresentations
 * @see Data
 * @see DataRepresentation
 * @see RepresentationFactory
 * @see InviwoModule::registerRepresentationFactoryObject()
 */
template <typename Representation>
class RepresentationFactoryObject : public BaseRepresentationFactoryObject {
public:
    using ReprId = std::type_index;

    RepresentationFactoryObject(ReprId classIdentifier) : classIdentifier_{classIdentifier} {};
    virtual ~RepresentationFactoryObject() = default;

    ReprId getClassIdentifier() const { return classIdentifier_; }

    virtual std::unique_ptr<Representation> create(const typename Representation::ReprOwner*) = 0;

private:
    ReprId classIdentifier_;
};

template <typename Representation, typename Derived>
class RepresentationFactoryObjectTemplate : public RepresentationFactoryObject<Representation> {
public:
    RepresentationFactoryObjectTemplate()
        : RepresentationFactoryObject<Representation>{std::type_index(typeid(Derived))} {}
    virtual ~RepresentationFactoryObjectTemplate() = default;
};

}  // namespace inviwo
