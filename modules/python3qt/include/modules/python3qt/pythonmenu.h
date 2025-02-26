/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

// make sure we import python first. Qt slot macros can mess with python otherwise
#include <pybind11/pybind11.h>

#include <inviwo/core/network/workspacemanager.h>
#include <modules/python3/pythonworkspacescripts.h>
#include <modules/python3qt/pythonworkspacescriptmenu.h>

#include <modules/qtwidgets/qptr.h>

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <string_view>
#include <optional>

#include <QObject>

class QMenu;
class QToolBar;
class QMainWindow;

namespace inviwo {

class InviwoModule;
class PythonEditorWidget;
class InviwoApplication;

class IVW_MODULE_PYTHON3QT_API PythonMenu {
public:
    PythonMenu(const std::filesystem::path& modulePath, InviwoApplication* app, QMainWindow* win);
    PythonMenu(const PythonMenu&) = delete;
    PythonMenu(PythonMenu&&) = delete;
    PythonMenu& operator=(const PythonMenu&) = delete;
    PythonMenu& operator=(PythonMenu&&) = delete;
    ~PythonMenu();

private:
    PythonEditorWidget* newEditor();

    InviwoApplication* app_;
    QMainWindow* win_;
    std::vector<QPtr<PythonEditorWidget>> editors_;
    QPtr<QToolBar> toolbar_;
    QPtr<QMenu> menu_;
    PythonWorkspaceScriptMenu scriptMenu_;
};

}  // namespace inviwo
