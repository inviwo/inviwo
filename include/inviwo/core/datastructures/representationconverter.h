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
#include <typeindex>

namespace inviwo {

class IVW_CORE_API ConverterException : public Exception {
public:
    ConverterException(const std::string& message = "",
                       ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
    virtual ~ConverterException() throw() {}
};

class DataRepresentation;

class IVW_CORE_API RepresentationConverter {
public:
    RepresentationConverter() = default;
    virtual ~RepresentationConverter() = default;
    using ConverterID = std::pair<std::type_index, std::type_index>;
    virtual ConverterID getConverterID() const = 0;

    virtual std::shared_ptr<DataRepresentation> createFrom(
        std::shared_ptr<const DataRepresentation> source) const = 0;
    virtual void update(std::shared_ptr<const DataRepresentation> source,
                        std::shared_ptr<DataRepresentation> destination) const = 0;
};

template <typename From, typename To>
class RepresentationConverterType : public RepresentationConverter {
public:
    virtual ConverterID getConverterID() const override;

    virtual std::shared_ptr<DataRepresentation> createFrom(
        std::shared_ptr<const DataRepresentation> source) const override final {
        return createFrom(std::static_pointer_cast<const From>(source));
    }

    virtual void update(std::shared_ptr<const DataRepresentation> source,
                        std::shared_ptr<DataRepresentation> destination) const override final {
        update(std::static_pointer_cast<const From>(source),
               std::static_pointer_cast<To>(destination));
    };

    virtual std::shared_ptr<To> createFrom(std::shared_ptr<const From> source) const = 0;
    virtual void update(std::shared_ptr<const From> source,
                        std::shared_ptr<To> destination) const = 0;
};

class IVW_CORE_API RepresentationConverterPackage {
public:
    using ConverterID = std::pair<std::type_index, std::type_index>;

    size_t steps() const;
    ConverterID getConverterID() const;

    void addConverter(const RepresentationConverter* converter);
    const std::vector<const RepresentationConverter*>& getConverters() const;

private:
    std::vector<const RepresentationConverter*> converters_;
};

template <typename From, typename To>
RepresentationConverter::ConverterID inviwo::RepresentationConverterType<From, To>::getConverterID()
    const {
    return ConverterID(typeid(From), typeid(To));
}

}  // namespace

#endif  // IVW_REPRESENTATIONCONVERTER_H
