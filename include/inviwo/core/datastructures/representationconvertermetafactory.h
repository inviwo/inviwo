/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_REPRESENTATIONCONVERTERMETAFACTORY_H
#define IVW_REPRESENTATIONCONVERTERMETAFACTORY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>

namespace inviwo {

/**
 * The Representation Converter Meta Factory holds RepresentationConverterFactories for
 * various kinds of representations (Volume Representation, Layer Representation, Buffer
 * Representation, etc)
 * @see Data
 * @see DataRepresentation
 * @see RepresentationConverter
 * @see RepresentationConverterFactory
 * @see InviwoApplication::getRepresentationConverterMetaFactory()
 * @see InviwoModule::registerRepresentationConverterFactory()
 */
class IVW_CORE_API RepresentationConverterMetaFactory {
public:
    using BaseReprId = BaseRepresentationConverterFactory::BaseReprId;
    using FactoryMap = std::unordered_map<BaseReprId, BaseRepresentationConverterFactory*>;

    RepresentationConverterMetaFactory() = default;
    ~RepresentationConverterMetaFactory() = default;

    bool registerObject(BaseRepresentationConverterFactory* factory);
    bool unRegisterObject(BaseRepresentationConverterFactory* factory);

    template <typename BaseRepr>
    RepresentationConverterFactory<BaseRepr>* getConverterFactory() const;

private:
    FactoryMap map_;
};

template <typename BaseRepr>
RepresentationConverterFactory<BaseRepr>* RepresentationConverterMetaFactory::getConverterFactory()
    const {
    const auto it = map_.find(BaseReprId(typeid(BaseRepr)));
    if (it != map_.end()) {
        return static_cast<RepresentationConverterFactory<BaseRepr>*>(it->second);
    }
    return nullptr;
}

}  // namespace inviwo

#endif  // IVW_REPRESENTATIONCONVERTERMETAFACTORY_H
