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
#include <inviwo/core/util/factory.h>
#include <inviwo/core/datastructures/representationfactoryobject.h>

#include <typeindex>

namespace inviwo {

class IVW_CORE_API BaseRepresentationFactory {
public:
    using BaseReprId = std::type_index;

    BaseRepresentationFactory() = default;
    virtual ~BaseRepresentationFactory() = default;
    virtual BaseReprId getBaseReprId() const = 0;
};

/**
 * Factory for representations of a specific base type (Volume Representation, Layer Representation,
 * Buffer Representation, etc)
 * @tparam Representation the base representation type for the factory. All registered
 * representation has to derive from this base class (for example @see VolumeRepresentation)
 * @see Data
 * @see DataRepresentation
 * @see RepresentationFactoryObject
 * @see RepresentationMetaFactory
 * @see InviwoApplication::getRepresentationFactory()
 * @see InviwoModule::registerRepresentationFactoryObject()
 * @see InviwoModule::registerRepresentationFactory()
 */
template <typename Representation>
class RepresentationFactory
    : public BaseRepresentationFactory,
      public StandardFactory<Representation, RepresentationFactoryObject<Representation>,
                             std::type_index, const typename Representation::ReprOwner*> {
public:
    /**
     * Create a RepresentationFactory
     * @param defaultRepresentation Id of the default representation type to use in the factory
     */
    RepresentationFactory(BaseReprId defaultRepresentation)
        : BaseRepresentationFactory{}, defaultRepresentation_{defaultRepresentation} {};
    virtual ~RepresentationFactory() = default;

    virtual BaseReprId getBaseReprId() const override { return BaseReprId{typeid(Representation)}; }

    /**
     * Try to create a representation of the requested type, if that type was not found, return a
     * representation of the default type.
     */
    std::unique_ptr<Representation> createOrDefault(
        BaseReprId id, const typename Representation::ReprOwner* owner) {
        if (auto repr = this->create(id, owner)) {
            return repr;
        } else {
            return this->create(defaultRepresentation_, owner);
        }
    }

private:
    BaseReprId defaultRepresentation_;
};

}  // namespace inviwo
