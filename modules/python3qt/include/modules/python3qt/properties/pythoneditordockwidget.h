/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/python3qt/python3qtmoduledefine.h>  // for IVW_MODULE_PYTHON3QT_API

#include <modules/qtwidgets/properties/texteditorwidgetqt.h>  // for TextEditorDockWidget

#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <vector>      // for vector

namespace inviwo {
class Property;

/**
 * @brief A text editor with Python syntax highlighting
 * @see TextEditorDockWidget
 * @see PythonSyntaxHighlight
 */
class IVW_MODULE_PYTHON3QT_API PythonEditorDockWidget : public TextEditorDockWidget {
public:
    /**
     * @brief Create a text editor for @p property
     * @pre Property has to be of type FileProperty or StringProperty
     */
    PythonEditorDockWidget(Property* property);

private:
    std::vector<std::shared_ptr<std::function<void()>>> callbacks_;
};

}  // namespace inviwo
