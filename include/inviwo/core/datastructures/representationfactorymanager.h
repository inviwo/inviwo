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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/representationfactory.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/representationmetafactory.h>
#include <inviwo/core/datastructures/representationconvertermetafactory.h>

#include <vector>
#include <memory>

namespace inviwo {

/**
 * The RepresentationFactoryManager is a indirection layer between Data and the
 * Representation/RepresentationConverter Factories. The RepresentationFactoryManager is responsible
 * for providing the factories to Data, by default it will redirect the request to the
 * InviwoApplication. But in the case where when we are not running the InviwoApplication, like in
 * some unit tests, a instance of the RepresentationFactoryManager can be constructed and the
 * relevant factories initialized directly, without having to initialize all of InviwoApplication.
 * We use this in the base module unit tests for example.
 * @see Data
 * @see RepresentationFactory
 * @see RepresentationConverterFactory
 * @see InviwoApplication
 */
class IVW_CORE_API RepresentationFactoryManager {
public:
    RepresentationFactoryManager();
    ~RepresentationFactoryManager();

    /**
     * Get a Representation factory for a specific kind of representation (Volume Representation,
     * Layer Representation, Buffer Representation, etc)
     * This function will use the local representation converter factory if available, otherwise it
     * will forward to the InviwoApplication
     * @see Data
     * @see DataRepresentation
     * @see RepresentationFactory
     * @see InviwoApplication::getRepresentationFactory()
     */
    template <typename BaseRepr>
    static RepresentationFactory<BaseRepr>* getRepresentationFactory();

    /**
     * Get a Representation converter factory for a specific kind of representation (Volume
     * Representation, Layer Representation, Buffer Representation, etc)
     * This function will use the local representation factory if available, otherwise it will
     * forward to the InviwoApplication
     * @see Data
     * @see RepresentationConverter
     * @see RepresentationConverterFactory
     * @see InviwoApplication::getRepresentationConverterFactory()
     */
    template <typename BaseRepr>
    static RepresentationConverterFactory<BaseRepr>* getRepresentationConverterFactory();

    /**
     * Get the local RepresentationMetaFactory
     */
    RepresentationMetaFactory* getRepresentationMetaFactory() {
        return localRepresentationMetaFactory_.get();
    }

    /**
     * Get the local RepresentationConverterMetaFactory
     */
    RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory() {
        return localRepresentationConverterMetaFactory_.get();
    };

    /**
     * Register a representation factory object for creating representations with the respective
     * representation factory. The template type BaseRepr is used to select representation
     * factory. A representation factory object should implement RepresentationFactoryObject
     * @see RepresentationFactory
     * @see RepresentationFactoryObject
     * @see DataRepresentation
     * @see RepresentationFactoryManager::getRepresentationFactory()
     */
    template <typename BaseRepr>
    void registerRepresentationFactoryObject(
        std::unique_ptr<RepresentationFactoryObject<BaseRepr>> representation);

    /**
     * Register a factory for representations. Each base representation (Volume
     * Representation, Layer Representation, Buffer Representation, etc) has its own representation
     * factory. A representation factory should implement RepresentationFactory
     * @see RepresentationFactory
     * @see DataRepresentation
     * @see RepresentationFactoryManager::getRepresentationMetaFactory()
     */
    void registerRepresentationFactory(
        std::unique_ptr<BaseRepresentationFactory> representationFactory);

    /**
     * Register a representation converter with the respective representation converter factory.
     * The template type BaseRepr is used to select representation converter factory.
     * A representation converter should implement RepresentationConverterType
     * @see RepresentationConverterFactory
     * @see RepresentationConverter
     * @see DataRepresentation
     * @see RepresentationFactoryManager::getRepresentationConverterFactory()
     */
    template <typename BaseRepr>
    void registerRepresentationConverter(
        std::unique_ptr<RepresentationConverter<BaseRepr>> converter);

    /**
     * Register a factory for representation converters. Each base representation (Volume
     * Representation, Layer Representation, Buffer Representation, etc) has its own representation
     * converter factory. A converter factory should implement RepresentationConverterFactory
     * @see RepresentationConverterFactory
     * @see RepresentationConverter
     * @see DataRepresentation
     * @see RepresentationFactoryManager::getRepresentationConverterMetaFactory()
     */
    void registerRepresentationConverterFactory(
        std::unique_ptr<BaseRepresentationConverterFactory> converterFactory);

private:
    std::vector<std::unique_ptr<BaseRepresentationFactory>> representationFactories_;
    std::vector<std::unique_ptr<BaseRepresentationFactoryObject,
                                std::function<void(BaseRepresentationFactoryObject*)>>>
        representationFactoryObjects_;

    std::vector<std::unique_ptr<BaseRepresentationConverterFactory>>
        representationConverterFactories_;
    std::vector<std::unique_ptr<BaseRepresentationConverter,
                                std::function<void(BaseRepresentationConverter*)>>>
        representationConverters_;

    RepresentationMetaFactory* oldRepresentationMetaFactory_ = nullptr;
    RepresentationConverterMetaFactory* oldRepresentationConverterMetaFactory_ = nullptr;
    std::unique_ptr<RepresentationMetaFactory> localRepresentationMetaFactory_;
    std::unique_ptr<RepresentationConverterMetaFactory> localRepresentationConverterMetaFactory_;

    static RepresentationMetaFactory* representationMetaFactory_;
    static RepresentationConverterMetaFactory* representationConverterMetaFactory_;
};

/**
 * Get a Representation factory for a specific kind of representation (Volume Representation,
 * Layer Representation, Buffer Representation, etc)
 * @see Data
 * @see DataRepresentation
 * @see RepresentationFactory
 * @see InviwoApplication::getRepresentationFactory()
 */

template <typename BaseRepr>
inline RepresentationFactory<BaseRepr>* RepresentationFactoryManager::getRepresentationFactory() {
    if (!representationMetaFactory_) {
        representationMetaFactory_ = InviwoApplication::getPtr()->getRepresentationMetaFactory();
    }
    return representationMetaFactory_->getRepresentationFactory<BaseRepr>();
}

/**
 * Get a Representation converter factory for a specific kind of representation (Volume
 * Representation, Layer Representation, Buffer Representation, etc)
 * @see Data
 * @see RepresentationConverter
 * @see RepresentationConverterFactory
 * @see InviwoApplication::getRepresentationConverterFactory()
 */

template <typename BaseRepr>
inline RepresentationConverterFactory<BaseRepr>*
RepresentationFactoryManager::getRepresentationConverterFactory() {
    if (!representationConverterMetaFactory_) {
        representationConverterMetaFactory_ =
            InviwoApplication::getPtr()->getRepresentationConverterMetaFactory();
    }
    return representationConverterMetaFactory_->getConverterFactory<BaseRepr>();
}

template <typename BaseRepr>
void RepresentationFactoryManager::registerRepresentationFactoryObject(
    std::unique_ptr<RepresentationFactoryObject<BaseRepr>> representation) {
    if (auto factory = localRepresentationMetaFactory_->getRepresentationFactory<BaseRepr>()) {
        if (factory->registerObject(representation.get())) {
            representationFactoryObjects_.emplace_back(
                representation.release(), [factory](BaseRepresentationFactoryObject* repr) {
                    factory->unRegisterObject(
                        static_cast<RepresentationFactoryObject<BaseRepr>*>(repr));
                    delete repr;
                });
        }
    }
}

template <typename BaseRepr>
void RepresentationFactoryManager::registerRepresentationConverter(
    std::unique_ptr<RepresentationConverter<BaseRepr>> converter) {

    if (auto factory = localRepresentationConverterMetaFactory_->getConverterFactory<BaseRepr>()) {
        if (factory->registerObject(converter.get())) {
            representationConverters_.emplace_back(
                converter.release(), [factory](BaseRepresentationConverter* conv) {
                    factory->unRegisterObject(
                        static_cast<RepresentationConverter<BaseRepr>*>(conv));
                    delete conv;
                });
        }
    }
}

}  // namespace inviwo
