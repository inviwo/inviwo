/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/ports/volumeport.h>                   // for VolumeInport
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

namespace inviwo {

class EditableLabelQt;
class IsoValueProperty;
class MultilineTextEdit;
class TFPrimitiveSet;
class TransferFunctionProperty;
struct TFPrimitiveData;
class TFData;

/**
 * \class TFPrimitiveSetWidgetQt
 * \brief text-based widget for editing TF primitives
 *
 * This is a text-based widget for editing a TFPrimitiveSet, i.e. a TransferFunction or
 * IsoValueCollection. The individual TF primitives are represented by "position alpha #RRGGBB".
 * If the type of the TFPrimitiveSet is relative, the positions are mapped to the value range of
 * the property unless the PropertySemantics are equal to "Text (Normalized)".
 */
class IVW_MODULE_QTWIDGETS_API TFPrimitiveSetWidgetQt : public PropertyWidgetQt {
public:
    explicit TFPrimitiveSetWidgetQt(IsoValueProperty* property);
    explicit TFPrimitiveSetWidgetQt(TransferFunctionProperty* property);
    virtual ~TFPrimitiveSetWidgetQt() = default;

    virtual void updateFromProperty() override;
    void setPropertyValue();

private:
    void initializeWidget();

    std::vector<TFPrimitiveData> extractPrimitiveData(const std::string& str) const;

    struct PropertyConcept {
        virtual ~PropertyConcept() = default;
        virtual TFPrimitiveSet& get() = 0;
        virtual const TFData& data() const = 0;
    };

    template <typename U>
    class PropertyModel : public PropertyConcept {
    public:
        explicit PropertyModel(U data) : data_(data) {}

        virtual TFPrimitiveSet& get() override { return data_->get(); }
        virtual const TFData& data() const override { return data_->data(); }

    private:
        U data_;
    };

    std::unique_ptr<PropertyConcept> propertyPtr_;

    MultilineTextEdit* textEdit_{};
    EditableLabelQt* label_{};
};

}  // namespace inviwo
