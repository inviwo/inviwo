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

#ifndef IVW_REPRESENTATIONCONVERTER_H
#define IVW_REPRESENTATIONCONVERTER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <vector>

namespace inviwo {

class DataRepresentation;

class IVW_CORE_API RepresentationConverter {
public:
    RepresentationConverter() = default;
    virtual ~RepresentationConverter() = default;

    /**
     * Checks if it is possible to convert from the data representation.
     * @param source is the DataRepresentation to test for conversion possibility.
     * @return boolean True if possible, false otherwise.
     */
    virtual bool canConvertFrom(const DataRepresentation* source) const = 0;
    virtual bool canConvertTo(const DataRepresentation* destination) const = 0;

    virtual bool isConverterReverse(RepresentationConverter*);

    virtual DataRepresentation* createFrom(const DataRepresentation* source) = 0;
    virtual void update(const DataRepresentation* source, DataRepresentation* destination) = 0;
};

template <typename TO>
class RepresentationConverterType : public RepresentationConverter {
public:
    RepresentationConverterType() = default;
    virtual ~RepresentationConverterType() = default;
    virtual bool canConvertTo(const DataRepresentation* destination) const override;
};

template <typename T>
class RepresentationConverterPackage : public RepresentationConverter {
public:
    RepresentationConverterPackage() = default;
    virtual ~RepresentationConverterPackage();
    virtual bool canConvertFrom(const DataRepresentation* source) const override;
    virtual bool canConvertTo(const DataRepresentation* destination) const override;
    virtual DataRepresentation* createFrom(const DataRepresentation* source) override;
    virtual void update(const DataRepresentation* source, DataRepresentation* destination) override;

    void addConverter(RepresentationConverter* converter);
    size_t getNumberOfConverters();

    const std::vector<RepresentationConverter*>& getConverters() const;

private:
    std::vector<RepresentationConverter*> converters_;
};

class IVW_CORE_API ConverterException : public Exception {
public:
    ConverterException(const std::string& message = "",
                       ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~ConverterException() throw() {}
};

template <typename TO>
bool RepresentationConverterType<TO>::canConvertTo(const DataRepresentation* destination) const {
    return dynamic_cast<const TO*>(destination) != nullptr;
}

template <typename T>
const std::vector<RepresentationConverter*>& RepresentationConverterPackage<T>::getConverters()
    const {
    return converters_;
}

template <typename T>
size_t RepresentationConverterPackage<T>::getNumberOfConverters() {
    return converters_.size();
}

template <typename T>
void RepresentationConverterPackage<T>::addConverter(RepresentationConverter* converter) {
    converters_.push_back(converter);
}

template <typename T>
void RepresentationConverterPackage<T>::update(const DataRepresentation* source,
                                               DataRepresentation* destination) {
    for (auto converter : converters_) {
        if (converter->canConvertFrom(source)) converter->update(source, destination);
    }
}

template <typename T>
DataRepresentation* RepresentationConverterPackage<T>::createFrom(
    const DataRepresentation* source) {
    for (auto converter : converters_) {
        if (converter->canConvertFrom(source)) return converter->createFrom(source);
    }

    return nullptr;
}

template <typename T>
bool RepresentationConverterPackage<T>::canConvertTo(const DataRepresentation* destination) const {
    for (auto converter : converters_) {
        if (converter->canConvertTo(destination)) return true;
    }

    return false;
}

template <typename T>
bool RepresentationConverterPackage<T>::canConvertFrom(const DataRepresentation* source) const {
    for (auto converter : converters_) {
        if (converter->canConvertFrom(source)) return true;
    }

    return false;
}

template <typename T>
RepresentationConverterPackage<T>::~RepresentationConverterPackage() {
    for (auto converter : converters_) delete converter;
}

}  // namespace

#endif  // IVW_REPRESENTATIONCONVERTER_H
