/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>          // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/properties/fileproperty.h>              // for FileRequestable
#include <modules/qtwidgets/properties/propertywidgetqt.h>    // for PropertyWidgetQt
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>  // for TextEditorDockWidget

#include <memory>                                             // for unique_ptr

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QHBoxLayout;

namespace inviwo {

class FilePathLineEditQt;
class PropertyEditorWidget;

class IVW_MODULE_QTWIDGETS_API FilePropertyWidgetQt : public PropertyWidgetQt,
                                                      public FileRequestable {
public:
    FilePropertyWidgetQt(FileProperty* property);
    virtual ~FilePropertyWidgetQt() = default;

    virtual void updateFromProperty() override;
    virtual bool requestFile() override;

    virtual PropertyEditorWidget* getEditorWidget() const override;
    virtual bool hasEditorWidget() const override;

protected:
    virtual void dropEvent(QDropEvent*) override;
    virtual void dragEnterEvent(QDragEnterEvent*) override;
    virtual void dragMoveEvent(QDragMoveEvent*) override;

    virtual void initEditor();
    void addEditor();
    void setPropertyValue();

    FileProperty* property_;
    FilePathLineEditQt* lineEdit_;
    QHBoxLayout* hWidgetLayout_;
    std::unique_ptr<TextEditorDockWidget> editor_;
};

}  // namespace inviwo
