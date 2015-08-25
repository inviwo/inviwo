/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_REPRESENTATIONCONVERTERFACTORY_H
#define IVW_REPRESENTATIONCONVERTERFACTORY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/singleton.h>

namespace inviwo {

class IVW_CORE_API RepresentationConverterFactory
    : public Singleton<RepresentationConverterFactory> {
public:
    RepresentationConverterFactory() = default;
    virtual ~RepresentationConverterFactory() = default;

    void registerObject(RepresentationConverter* representationConverter);

    // Get best converter that can convert from the source to T
    template <typename T>
    RepresentationConverter* getRepresentationConverter(DataRepresentation* source);

    // Get best converter package that can convert from the source to T
    template <typename T>
    RepresentationConverterPackage<T>* getRepresentationConverterPackage(
        DataRepresentation* source);

    // Get all converters that can convert from the source
    std::vector<RepresentationConverter*> getRepresentationConvertersFrom(
        DataRepresentation* source);

    // Get best converter that can convert to T
    template <typename T>
    std::vector<RepresentationConverter*> getRepresentationConvertersTo();

private:
    std::vector<RepresentationConverter*> representationConverters_;
};

// Get all converters that can convert from the source
inline std::vector<RepresentationConverter*>
RepresentationConverterFactory::getRepresentationConvertersFrom(DataRepresentation* source) {
    std::vector<RepresentationConverter*> srcConverters;
    for (auto converter : representationConverters_) {
        if (converter->canConvertFrom(source)) {
            srcConverters.push_back(converter);
        }
    }
    return srcConverters;
}

template <typename T>
std::vector<RepresentationConverter*>
RepresentationConverterFactory::getRepresentationConvertersTo() {
    std::vector<RepresentationConverter*> tConverters;
    for (auto converter : representationConverters_) {
        if (dynamic_cast<RepresentationConverterType<T>*>(converter)) {
            tConverters.push_back(converter);
        }
    }
    return tConverters;
}

template <typename T>
RepresentationConverter* RepresentationConverterFactory::getRepresentationConverter(
    DataRepresentation* source) {
    // TODO: optimize performance, e.g., by using a hash table
    for (auto converter : representationConverters_) {
        if (auto converterTyped = dynamic_cast<RepresentationConverterType<T>*>(converter)) {
            if (converterTyped->canConvertFrom(source)) return converter;
        }
    }
    return nullptr;
}

// Get best converter package that can convert from the source to T

template <typename T>
inline RepresentationConverterPackage<T>*
RepresentationConverterFactory::getRepresentationConverterPackage(DataRepresentation* source) {
    // TODO: optimize performance, e.g., by using a hash table
    RepresentationConverterPackage<T>* result = nullptr;

    for (auto converter : representationConverters_) {
        if (auto package = dynamic_cast<RepresentationConverterPackage<T>*>(converter)) {
            if (package->canConvertFrom(source)) {
                if (result) {
                    result = (package->getNumberOfConverters() < result->getNumberOfConverters()
                                  ? package
                                  : result);
                } else {
                    result = package;
                }
            }
        }
    }

    return result;
}

}  // namespace

#endif  // IVW_REPRESENTATIONCONVERTERFACTORY_H
