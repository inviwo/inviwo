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
#include <inviwo/core/datastructures/representationfactory.h>

namespace inviwo {

/**
 * The Representation Meta Factory holds RepresentationFactories for
 * various kinds of representations (Volume Representation, Layer Representation, Buffer
 * Representation, etc)
 * @see Data
 * @see DataRepresentation
 * @see RepresentationFactoryObject
 * @see RepresentationFactory
 * @see InviwoApplication::getRepresentationMetaFactory()
 * @see InviwoModule::registerRepresentationFactory()
 */
class IVW_CORE_API RepresentationMetaFactory {
public:
    using BaseReprId = BaseRepresentationFactory::BaseReprId;
    using FactoryMap = std::unordered_map<BaseReprId, BaseRepresentationFactory*>;

    RepresentationMetaFactory() = default;
    ~RepresentationMetaFactory() = default;

    // Does not take ownership
    bool registerObject(BaseRepresentationFactory* factory);
    bool unRegisterObject(BaseRepresentationFactory* factory);

    template <typename BaseRepr>
    RepresentationFactory<BaseRepr>* getRepresentationFactory() const;

private:
    FactoryMap map_;
};

template <typename BaseRepr>
RepresentationFactory<BaseRepr>* RepresentationMetaFactory::getRepresentationFactory() const {
    const auto it = map_.find(BaseReprId(typeid(BaseRepr)));
    if (it != map_.end()) {
        return static_cast<RepresentationFactory<BaseRepr>*>(it->second);
    }
    return nullptr;
}

}  // namespace inviwo
