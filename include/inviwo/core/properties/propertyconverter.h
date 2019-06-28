/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo {

class IVW_CORE_API PropertyConverter {
public:
    PropertyConverter(const std::string& srcClassIdentifier, const std::string& dstClassIdentifier);
    virtual ~PropertyConverter();

    std::string getSourcePropertyClassIdenetifier() const;
    std::string getDestinationPropertyClassIdenetifier() const;

    /**
     * convert requires that srcProperty and dstProperty has the same class identifiers as
     * srcClassIdentifier and dstClassIdentifier, given in the constructor, i.e. the same types.
     */
    virtual void convert(const Property* srcProperty, Property* dstProperty) const;

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

    virtual void convert(const Property* src, Property* dst) const override {
        // Static cast will work here since we will only use the converter for its registered
        // property types
        convertimpl(static_cast<const SrcProperty*>(src), static_cast<DstProperty*>(dst));
    }

protected:
    virtual void convertimpl(const SrcProperty* src, DstProperty* dst) const = 0;
};

template <typename SrcProperty, typename DstProperty>
class OrdinalPropertyConverter : public TemplatePropertyConverter<SrcProperty, DstProperty> {
protected:
    virtual void convertimpl(const SrcProperty* src, DstProperty* dst) const override {
        dst->setMinValue(static_cast<typename DstProperty::value_type>(src->getMinValue()));
        dst->setMaxValue(static_cast<typename DstProperty::value_type>(src->getMaxValue()));
        dst->setIncrement(static_cast<typename DstProperty::value_type>(src->getIncrement()));
        dst->set(static_cast<typename DstProperty::value_type>(src->get()));
    }
};

template <typename SrcProperty>
class ScalarToStringConverter : public TemplatePropertyConverter<SrcProperty, StringProperty> {
protected:
    virtual void convertimpl(const SrcProperty* src, StringProperty* dst) const override {
        dst->set(toString(src->get()));
    }
};

template <typename SrcProperty>
class VectorToStringConverter : public TemplatePropertyConverter<SrcProperty, StringProperty> {
protected:
    virtual void convertimpl(const SrcProperty* src, StringProperty* dst) const override {
        dst->set(glm::to_string(src->get()));
    }
};

template <typename OptionProperty>
class OptionToStringConverter : public TemplatePropertyConverter<OptionProperty, StringProperty> {
protected:
    virtual void convertimpl(const OptionProperty* src, StringProperty* dst) const override {
        if (src->size() > 0) {
            dst->set(src->getSelectedDisplayName());
        } else {
            dst->set("");
        }
    }
};

template <typename OptionProperty>
class OptionToIntConverter : public TemplatePropertyConverter<OptionProperty, IntProperty> {
protected:
    virtual void convertimpl(const OptionProperty* src, IntProperty* dst) const override {
        dst->set(static_cast<int>(src->getSelectedIndex()), 0, static_cast<int>(src->size()) - 1,
                 1);
    }
};

template <typename OptionProperty>
class IntToOptionConverter : public TemplatePropertyConverter<IntProperty, OptionProperty> {
protected:
    virtual void convertimpl(const IntProperty* src, OptionProperty* dst) const override {
        dst->setSelectedIndex(src->get());
    }
};

// conversion between String and File/Directory properties
class FileToStringConverter : public TemplatePropertyConverter<FileProperty, StringProperty> {
protected:
    virtual void convertimpl(const FileProperty* src, StringProperty* dst) const override {
        dst->set(src->get());
    }
};

class StringToFileConverter : public TemplatePropertyConverter<StringProperty, FileProperty> {
protected:
    virtual void convertimpl(const StringProperty* src, FileProperty* dst) const override {
        dst->set(src->get());
    }
};

class DirectoryToStringConverter
    : public TemplatePropertyConverter<DirectoryProperty, StringProperty> {
protected:
    virtual void convertimpl(const DirectoryProperty* src, StringProperty* dst) const override {
        dst->set(src->get());
    }
};

class StringToDirectoryConverter
    : public TemplatePropertyConverter<StringProperty, DirectoryProperty> {
protected:
    virtual void convertimpl(const StringProperty* src, DirectoryProperty* dst) const override {
        dst->set(src->get());
    }
};

// conversion between various TF and isovalue properties
class TransferfunctionToIsoTFConverter
    : public TemplatePropertyConverter<TransferFunctionProperty, IsoTFProperty> {
protected:
    virtual void convertimpl(const TransferFunctionProperty* src,
                             IsoTFProperty* dst) const override {
        dst->set(*src);
    }
};

class IsoTFToTransferfunctionConverter
    : public TemplatePropertyConverter<IsoTFProperty, TransferFunctionProperty> {
protected:
    virtual void convertimpl(const IsoTFProperty* src,
                             TransferFunctionProperty* dst) const override {
        dst->set(*src);
    }
};

class IsovalueToIsoTFConverter : public TemplatePropertyConverter<IsoValueProperty, IsoTFProperty> {
protected:
    virtual void convertimpl(const IsoValueProperty* src, IsoTFProperty* dst) const override {
        dst->set(*src);
    }
};

class IsoTFToIsovalueConverter : public TemplatePropertyConverter<IsoTFProperty, IsoValueProperty> {
protected:
    virtual void convertimpl(const IsoTFProperty* src, IsoValueProperty* dst) const override {
        dst->set(*src);
    }
};

}  // namespace inviwo

#endif  // IVW_PROPERTYCONVERTER_H
