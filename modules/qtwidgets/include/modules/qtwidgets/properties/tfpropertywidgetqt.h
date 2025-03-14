/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/inviwowidgetsqt.h>              // for IvwPushButton
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt
#include <modules/qtwidgets/tf/tfpropertydialog.h>          // for TFPropertyDialog

#include <memory>  // for unique_ptr
#include <variant>

class QMenu;
class QResizeEvent;
class QShowEvent;
class QWidget;

namespace inviwo {

class EditableLabelQt;
class IsoTFProperty;
class IsoValueProperty;
class TFPushButton;
class TransferFunctionProperty;

class IVW_MODULE_QTWIDGETS_API TFPropertyWidgetQt : public PropertyWidgetQt {
public:
    explicit TFPropertyWidgetQt(TransferFunctionProperty* property);
    virtual ~TFPropertyWidgetQt();

    virtual void updateFromProperty() override;
    virtual TFPropertyDialog* getEditorWidget() const override;
    virtual bool hasEditorWidget() const override;

    virtual void setReadOnly(bool readonly) override;

protected:
    virtual std::unique_ptr<QMenu> getContextMenu() override;

    TransferFunctionProperty* tfProperty() const;

private:
    EditableLabelQt* label_ = nullptr;
    TFPushButton* btnOpenTF_ = nullptr;
    mutable std::unique_ptr<TFPropertyDialog> transferFunctionDialog_ = nullptr;
};

class IVW_MODULE_QTWIDGETS_API TFPushButton : public IvwPushButton {
public:
    explicit TFPushButton(TransferFunctionProperty* property, QWidget* parent = nullptr);
    explicit TFPushButton(IsoValueProperty* property, QWidget* parent = nullptr);
    explicit TFPushButton(IsoTFProperty* property, QWidget* parent = nullptr);
    virtual ~TFPushButton() = default;
    void updateFromProperty();

private:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    std::variant<TransferFunctionProperty*, IsoValueProperty*, IsoTFProperty*> property_;
};

}  // namespace inviwo
