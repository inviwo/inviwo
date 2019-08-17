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

#include <inviwo/core/datastructures/representationfactorymanager.h>

namespace inviwo {

RepresentationMetaFactory* RepresentationFactoryManager::representationMetaFactory_ = nullptr;
RepresentationConverterMetaFactory*
    RepresentationFactoryManager::representationConverterMetaFactory_ = nullptr;

RepresentationFactoryManager::RepresentationFactoryManager()
    : oldRepresentationMetaFactory_{representationMetaFactory_}
    , oldRepresentationConverterMetaFactory_{representationConverterMetaFactory_}
    , localRepresentationMetaFactory_{std::make_unique<RepresentationMetaFactory>()}
    , localRepresentationConverterMetaFactory_{
          std::make_unique<RepresentationConverterMetaFactory>()} {

    representationMetaFactory_ = localRepresentationMetaFactory_.get();
    representationConverterMetaFactory_ = localRepresentationConverterMetaFactory_.get();
}

RepresentationFactoryManager::~RepresentationFactoryManager() {
    representationMetaFactory_ = oldRepresentationMetaFactory_;
    representationConverterMetaFactory_ = oldRepresentationConverterMetaFactory_;
}

void RepresentationFactoryManager::registerRepresentationFactory(
    std::unique_ptr<BaseRepresentationFactory> representationFactory) {
    if (localRepresentationMetaFactory_->registerObject(representationFactory.get())) {
        representationFactories_.push_back(std::move(representationFactory));
    }
}

void RepresentationFactoryManager::registerRepresentationConverterFactory(
    std::unique_ptr<BaseRepresentationConverterFactory> converterFactory) {
    if (localRepresentationConverterMetaFactory_->registerObject(converterFactory.get())) {
        representationConverterFactories_.push_back(std::move(converterFactory));
    }
}

}  // namespace inviwo
