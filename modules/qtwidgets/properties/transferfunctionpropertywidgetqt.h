/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

#ifndef IVW_TRANSFERFUNCTIONPROPERTYWIDGET_H
#define IVW_TRANSFERFUNCTIONPROPERTYWIDGET_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/transferfunctionpropertydialog.h>
#include <modules/qtwidgets/inviwowidgetsqt.h>

namespace inviwo {

class EditableLabelQt;
class TransferFunctionProperty;
class TFPushButton;

class IVW_MODULE_QTWIDGETS_API TransferFunctionPropertyWidgetQt : public PropertyWidgetQt {
public:
    TransferFunctionPropertyWidgetQt(TransferFunctionProperty* property);
    virtual ~TransferFunctionPropertyWidgetQt();

    virtual void updateFromProperty() override;
    virtual TransferFunctionPropertyDialog* getEditorWidget() const override;
    virtual bool hasEditorWidget() const override;

private:
    EditableLabelQt* label_ = nullptr;
    TFPushButton* btnOpenTF_ = nullptr;
    mutable std::unique_ptr<TransferFunctionPropertyDialog> transferFunctionDialog_ = nullptr;

    void generateWidget();
};

class IVW_MODULE_QTWIDGETS_API TFPushButton : public IvwPushButton {
public:
    TFPushButton(TransferFunctionProperty* property, QWidget* parent = nullptr);
    virtual ~TFPushButton() = default;
    void updateFromProperty();

private:
    void resizeEvent(QResizeEvent* event) override;

    TransferFunctionProperty* tfProperty_ = nullptr;
};

}  // namespace

#endif  // IVW_TRANSFERFUNCTIONPROPERTYWIDGET_H
