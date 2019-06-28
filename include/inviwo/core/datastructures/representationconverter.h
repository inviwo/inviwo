/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
    virtual ~ConverterException() noexcept = default;
};

/**
 * A base type for all RepresentationConverters
 * @see RepresentationConverter
 */
class IVW_CORE_API BaseRepresentationConverter {
public:
    BaseRepresentationConverter() = default;
    virtual ~BaseRepresentationConverter() = default;
};

/**
 * A RepresentationConverter creates or updates a DataRepresentation from an other
 * DataRepresentation using the createFrom() or update() functions
 * @see DataRepresentation
 * @see RepresentationConverterFactory
 * @see InviwoApplication::getRepresentationConverterFactory()
 * @see InviwoModule::registerRepresentationConverter()
 */
template <typename BaseRepr>
class RepresentationConverter : public BaseRepresentationConverter {
public:
    RepresentationConverter() = default;
    virtual ~RepresentationConverter() = default;
    using ConverterID = std::pair<std::type_index, std::type_index>;
    virtual ConverterID getConverterID() const = 0;

    virtual std::shared_ptr<BaseRepr> createFrom(std::shared_ptr<const BaseRepr> source) const = 0;
    virtual void update(std::shared_ptr<const BaseRepr> source,
                        std::shared_ptr<BaseRepr> destination) const = 0;
};

/**
 * A helper type to make it easier to create RepresentationConverters for specific types
 * @see RepresentationConverter
 */
template <typename BaseRepr, typename From, typename To>
class RepresentationConverterType : public RepresentationConverter<BaseRepr> {
public:
    using ConverterID = typename RepresentationConverter<BaseRepr>::ConverterID;
    virtual ConverterID getConverterID() const override;

    virtual std::shared_ptr<BaseRepr> createFrom(
        std::shared_ptr<const BaseRepr> source) const override final {
        return createFrom(std::static_pointer_cast<const From>(source));
    }

    virtual void update(std::shared_ptr<const BaseRepr> source,
                        std::shared_ptr<BaseRepr> destination) const override final {
        update(std::static_pointer_cast<const From>(source),
               std::static_pointer_cast<To>(destination));
    };

    virtual std::shared_ptr<To> createFrom(std::shared_ptr<const From> source) const = 0;
    virtual void update(std::shared_ptr<const From> source,
                        std::shared_ptr<To> destination) const = 0;
};

/**
 * A set of RepresentationConverters to convert from one kind of representation to a different kind
 * through multiple steps.
 * @see RepresentationConverter
 * @see DataRepresenation
 * @see Data
 */
template <typename BaseRepr>
class RepresentationConverterPackage {
public:
    using ConverterID = typename RepresentationConverter<BaseRepr>::ConverterID;
    using ConverterList = std::vector<const RepresentationConverter<BaseRepr>*>;

    size_t steps() const;
    ConverterID getConverterID() const;

    void addConverter(const RepresentationConverter<BaseRepr>* converter);
    const ConverterList& getConverters() const;

private:
    ConverterList converters_;
};

template <typename BaseRepr, typename From, typename To>
auto RepresentationConverterType<BaseRepr, From, To>::getConverterID() const -> ConverterID {
    return ConverterID(typeid(From), typeid(To));
}

template <typename BaseRepr>
auto RepresentationConverterPackage<BaseRepr>::getConverters() const -> const ConverterList& {
    return converters_;
}

template <typename BaseRepr>
size_t RepresentationConverterPackage<BaseRepr>::steps() const {
    return converters_.size();
}

template <typename BaseRepr>
auto RepresentationConverterPackage<BaseRepr>::getConverterID() const -> ConverterID {
    return ConverterID(converters_.front()->getConverterID().first,
                       converters_.back()->getConverterID().second);
}

template <typename BaseRepr>
void RepresentationConverterPackage<BaseRepr>::addConverter(
    const RepresentationConverter<BaseRepr>* converter) {
    converters_.push_back(converter);
}

}  // namespace inviwo

#endif  // IVW_REPRESENTATIONCONVERTER_H
