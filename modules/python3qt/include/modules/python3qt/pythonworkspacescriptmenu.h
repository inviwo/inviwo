/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <modules/python3qt/pythoneditorwidget.h>
#include <modules/python3/pythonworkspacescripts.h>

// Ensure all python includes are before any Qt includes since qt defines slots
#include <modules/qtwidgets/qptr.h>

#include <map>

#include <QMenu>
#include <QMainWindow>

namespace inviwo {

class InviwoApplication;
class PythonEditorWidget;

class IVW_MODULE_PYTHON3QT_API PythonWorkspaceScriptMenu : public PythonWorkspaceScriptsObserver {
public:
    PythonWorkspaceScriptMenu(PythonWorkspaceScripts& scripts, QMenu* parent,
                              InviwoApplication* app, QMainWindow* win);
    virtual ~PythonWorkspaceScriptMenu();

    virtual void onScriptAdded(std::string_view key, std::string_view script) override;
    virtual void onScriptRemoved(std::string_view key, std::string_view script) override;
    virtual void onScriptSaved(std::string_view key, std::string_view script) override;
    virtual void onScriptUpdate(std::string_view key, std::string_view script) override;

private:
    void addScriptMenuItem(std::string_view key);
    PythonEditorWidget* newScriptEditor(const std::string& key);

    void openEditor(const std::string& key);

    std::optional<std::string> getScriptName(std::string_view suggestion);

    PythonWorkspaceScripts& scripts_;
    InviwoApplication* app_;
    QMainWindow* win_;
    std::map<std::string, QPtr<QAction>, std::less<>> menuItems_;
    std::map<std::string, QPtr<PythonEditorWidget>, std::less<>> scriptEditors_;
    QPtr<QMenu> menu_;
};

}  // namespace inviwo
