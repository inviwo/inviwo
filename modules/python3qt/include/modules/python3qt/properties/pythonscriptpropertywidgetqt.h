/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/python3qt/python3qtmoduledefine.h>

#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>

#include <memory>

class QHBoxLayout;

namespace inviwo {
class ScriptProperty;
class EditableLabelQt;
class LineEditQt;
class PropertyEditorWidget;

/**
 * @brief Widget for a ScriptProperty providing a Python script editor.
 *
 * Displays the script source in a line edit and offers a text editor button
 * to open a Python-syntax-highlighted editor dock widget.
 * When the Python3Qt module is loaded, it also sets a Python execution backend
 * on the ScriptProperty so that scripts can be called at runtime.
 *
 * @see ScriptProperty
 * @see PythonEditorDockWidget
 */
class IVW_MODULE_PYTHON3QT_API PythonScriptPropertyWidgetQt : public PropertyWidgetQt {
public:
    explicit PythonScriptPropertyWidgetQt(ScriptProperty* property);
    virtual ~PythonScriptPropertyWidgetQt();

    virtual void updateFromProperty() override;

    virtual bool hasEditorWidget() const override;
    virtual PropertyEditorWidget* getEditorWidget() override;

private:
    void setPropertyValue();
    void initEditor();
    void addEditor();
    void installPythonBackend();

    ScriptProperty* property_;
    LineEditQt* lineEdit_;
    QHBoxLayout* hWidgetLayout_;
    std::unique_ptr<TextEditorDockWidget> editor_;
};

}  // namespace inviwo
