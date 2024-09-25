/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <modules/python3qt/properties/pythonpropertywidgetqt.h>

#include <inviwo/core/properties/propertysemantics.h>             // for operator==, PropertySem...
#include <inviwo/core/properties/stringproperty.h>                // for StringProperty
#include <inviwo/core/util/assertion.h>                           // for IVW_ASSERT
#include <modules/python3qt/properties/pythoneditordockwidget.h>  // for PythonEditorDockWidget
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>  // for StringPropertyWidgetQt
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>      // for TextEditorDockWidget

#include <memory>  // for make_unique, unique_ptr

namespace inviwo {

PythonPropertyWidgetQt::PythonPropertyWidgetQt(StringProperty* property)
    : StringPropertyWidgetQt(property) {

    if (property->getSemantics() != PropertySemantics::PythonEditor) {
        throw Exception(IVW_CONTEXT,
                        "Invalid semantics for HtmlPropertyWidgetQt, expected PythonEditor, got {}",
                        property->getSemantics().getString());
    }

    addEditor();
}

PythonPropertyWidgetQt::~PythonPropertyWidgetQt() = default;

void PythonPropertyWidgetQt::initEditor() {
    editor_ = std::make_unique<PythonEditorDockWidget>(property_);
}

}  // namespace inviwo
