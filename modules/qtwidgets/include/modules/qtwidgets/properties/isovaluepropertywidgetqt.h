/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_ISOVALUEPROPERTYWIDGETQT_H
#define IVW_ISOVALUEPROPERTYWIDGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/tf/tfpropertydialog.h>

namespace inviwo {

class IsoValueProperty;
class TFPushButton;
class EditableLabelQt;

class IVW_MODULE_QTWIDGETS_API IsoValuePropertyWidgetQt : public PropertyWidgetQt {
public:
    IsoValuePropertyWidgetQt(IsoValueProperty* property);
    virtual ~IsoValuePropertyWidgetQt() = default;

    virtual TFPropertyDialog* getEditorWidget() const override;
    virtual bool hasEditorWidget() const override;

    virtual void updateFromProperty() override;

    virtual void setReadOnly(bool readonly) override;

protected:
    virtual std::unique_ptr<QMenu> getContextMenu() override;

private:
    EditableLabelQt* label_;
    TFPushButton* btnOpenTF_;
    mutable std::unique_ptr<TFPropertyDialog> tfDialog_ = nullptr;
};

}  // namespace inviwo

#endif  // IVW_ISOVALUEPROPERTYWIDGETQT_H
