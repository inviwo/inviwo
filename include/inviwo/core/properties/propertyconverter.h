/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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
#include <inviwo/core/properties/fileproperty.h>

namespace inviwo {

class IVW_CORE_API PropertyConverter {
public:
    PropertyConverter(const std::string &srcClassIdentifier, const std::string &dstClassIdentifier);
    virtual ~PropertyConverter();

    std::string getSourcePropertyClassIdenetifier() const;
    std::string getDestinationPropertyClassIdenetifier() const;

    /**
     * convert requires that srcProperty and dstProperty has the same class identifiers as
     * srcClassIdentifier and dstClassIdentifier, given in the constructor, i.e. the same types.
     */
    virtual void convert(const Property *srcProperty, Property *dstProperty) const;

protected:
    std::string srcClassIdentifier_;
    std::string dstClassIdentifier_;
};

template <typename SrcProperty, typename DstProperty>
class TemplatePropertyConverter : public PropertyConverter {
public:
    TemplatePropertyConverter()
        : PropertyConverter(PropertyTraits<SrcProperty>::classIdentifier(),
                            PropertyTraits<DstProperty>::classIdentifier()) {}
    virtual ~TemplatePropertyConverter() = default;

    virtual void convert(const Property *src, Property *dst) const override {
        // Static cast will work here since we will only use the converter for its registered
        // property types
        convertimpl(static_cast<const SrcProperty *>(src), static_cast<DstProperty *>(dst));
    }

protected:
    virtual void convertimpl(const SrcProperty *src, DstProperty *dst) const = 0;
};

template <typename SrcProperty, typename DstProperty>
class OrdinalPropertyConverter : public TemplatePropertyConverter<SrcProperty, DstProperty> {
protected:
    virtual void convertimpl(const SrcProperty *src, DstProperty *dst) const override {
        dst->setMinValue(static_cast<typename DstProperty::value_type>(src->getMinValue()));
        dst->setMaxValue(static_cast<typename DstProperty::value_type>(src->getMaxValue()));
        dst->setIncrement(static_cast<typename DstProperty::value_type>(src->getIncrement()));
        dst->set(static_cast<typename DstProperty::value_type>(src->get()));
    }
};

template <typename SrcProperty>
class ScalarToStringConverter : public TemplatePropertyConverter<SrcProperty, StringProperty> {
protected:
    virtual void convertimpl(const SrcProperty *src, StringProperty *dst) const override {
        dst->set(toString(src->get()));
    }
};

template <typename SrcProperty>
class VectorToStringConverter : public TemplatePropertyConverter<SrcProperty, StringProperty> {
protected:
    virtual void convertimpl(const SrcProperty *src, StringProperty *dst) const override {
        dst->set(glm::to_string(src->get()));
    }
};

template <typename OptionProperty>
class OptionToStringConverter : public TemplatePropertyConverter<OptionProperty, StringProperty> {
protected:
    virtual void convertimpl(const OptionProperty *src, StringProperty *dst) const override {
        if (src->size() > 0) {
            dst->set(src->getSelectedDisplayName());
        } else {
            dst->set("");
        }
    }
};

class FileToStringConverter : public TemplatePropertyConverter<FileProperty, StringProperty> {
protected:
    virtual void convertimpl(const FileProperty *src, StringProperty *dst) const override {
        dst->set(src->get());
    }
};

class StringToFileConverter : public TemplatePropertyConverter<StringProperty, FileProperty> {
protected:
    virtual void convertimpl(const StringProperty *src, FileProperty *dst) const override {
        dst->set(src->get());
    }
};

}  // namespace inviwo

#endif  // IVW_PROPERTYCONVERTER_H
