/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PROPERTYCONVERTER_H
#define IVW_PROPERTYCONVERTER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {

class IVW_CORE_API PropertyConverter {
public:
    PropertyConverter(const std::string &srcClassIdentifier, const std::string &dstClassIdentifier);
    virtual ~PropertyConverter();

    std::string getSourcePropertyClassIdenetifier() const;
    std::string getDestinationPropertyClassIdenetifier() const;

    virtual void convert(const Property *srcProperty, Property *dstProperty) const;

protected:
    std::string srcClassIdentifier_;
    std::string dstClassIdentifier_;
};

template <typename srcProperty, typename dstProperty>
class TemplatePropertyConverter : public PropertyConverter {
public:
    TemplatePropertyConverter()
        : PropertyConverter(srcProperty::CLASS_IDENTIFIER, dstProperty::CLASS_IDENTIFIER) {}
    virtual ~TemplatePropertyConverter() {}

    virtual void convert(const Property *src, Property *dst) const override {
        const srcProperty *s = dynamic_cast<const srcProperty *>(src);
        dstProperty *d = dynamic_cast<dstProperty *>(dst);
        if (s && d) {
            convertimpl(s, d);
        } else {
            LogWarn("Cant convert from " << src->getClassIdentifier() << " to "
                                         << dst->getClassIdentifier() << " using this convert");
        }
    }

protected:
    virtual void convertimpl(const srcProperty *src, dstProperty *dst) const = 0;
};

template <typename srcProperty, typename dstProperty>
class OrdinalPropertyConverter : public TemplatePropertyConverter<srcProperty, dstProperty> {
public:
    OrdinalPropertyConverter() : TemplatePropertyConverter<srcProperty, dstProperty>() {}
    virtual ~OrdinalPropertyConverter() {}


protected:
    virtual void convertimpl(const srcProperty *src, dstProperty *dst) const override {
        dst->setMinValue(static_cast<typename dstProperty::value_type>(src->getMinValue()));
        dst->setMaxValue(static_cast<typename dstProperty::value_type>(src->getMaxValue()));
        dst->setIncrement(static_cast<typename dstProperty::value_type>(src->getIncrement()));
        dst->set(static_cast<typename dstProperty::value_type>(src->get()));
    }
};

template <typename srcProperty>
class ScalarToStringConverter : public TemplatePropertyConverter<srcProperty, StringProperty> {
public:
    ScalarToStringConverter() : TemplatePropertyConverter<srcProperty, StringProperty>() {}
    virtual ~ScalarToStringConverter() {}

protected:
    virtual void convertimpl(const srcProperty *src, StringProperty *dst) const override {
        dst->set(toString(src->get()));
    }
};

template <typename srcProperty>
class VectorToStringConverter : public TemplatePropertyConverter<srcProperty, StringProperty> {
public:
    VectorToStringConverter() : TemplatePropertyConverter<srcProperty, StringProperty>() {}
    virtual ~VectorToStringConverter() {}

protected:
    virtual void convertimpl(const srcProperty *src, StringProperty *dst) const override {
        dst->set(glm::to_string(src->get()));
    }
};

}  // namespace

#endif  // IVW_PROPERTYCONVERTER_H
