/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/python3qt/pythonworkspacescriptmenu.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <QInputDialog>
namespace inviwo {

namespace {
constexpr auto defaultSource = IVW_UNINDENT(R"(
    #Inviwo Python script
    import inviwopy

    app = inviwopy.app
    network = app.network
    )");
}  // namespace

PythonWorkspaceScriptMenu::PythonWorkspaceScriptMenu(PythonWorkspaceScripts& scripts, QMenu* parent,
                                                     InviwoApplication* app, QMainWindow* win)
    : scripts_{scripts}, app_{app}, win_{win}, menu_{parent->addMenu("Workspace Scripts")} {

    const auto* newScript = menu_->addAction(QIcon(":/svgicons/newfile.svg"), "&New Python Script");
    QObject::connect(newScript, &QAction::triggered, [this]() {
        const auto keySuggestion = fmt::format("Script {}", scripts_.getKeys().size() + 1);
        if (auto key = getScriptName(keySuggestion)) {
            scripts_.addScript(*key, defaultSource);
            openEditor(*key);
        }
    });
    menu_->addSeparator();

    scripts.addObserver(this);
}
PythonWorkspaceScriptMenu::~PythonWorkspaceScriptMenu() = default;

void PythonWorkspaceScriptMenu::onScriptAdded(std::string_view key, std::string_view) {
    addScriptMenuItem(key);
};
void PythonWorkspaceScriptMenu::onScriptRemoved(std::string_view key, std::string_view) {
    if (auto it = menuItems_.find(key); it != menuItems_.end()) {
        menuItems_.erase(it);
    }
};
void PythonWorkspaceScriptMenu::onScriptSaved(std::string_view key, std::string_view) {
    if (auto it = scriptEditors_.find(key); it != scriptEditors_.end() && it->second) {
        it->second->setWindowModified(false);
    }
};
void PythonWorkspaceScriptMenu::onScriptUpdate(std::string_view key, std::string_view script) {
    if (auto it = scriptEditors_.find(key); it != scriptEditors_.end() && it->second) {
        it->second->setSource(script);
    }
};

void PythonWorkspaceScriptMenu::addScriptMenuItem(std::string_view key) {
    const auto qKey = utilqt::toQString(key);
    auto* openScript = menu_->addAction(qKey);
    menuItems_.emplace(key, openScript);
    QObject::connect(openScript, &QAction::triggered,
                     [this, key = std::string{key}]() { openEditor(key); });
}

PythonEditorWidget* PythonWorkspaceScriptMenu::newScriptEditor(const std::string& key) {
    auto editor = util::make_qptr<PythonEditorWidget>(
        win_, app_, [this, key](const std::string& source) { scripts_.updateScript(key, source); });
    editor->loadState();
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->setVisible(true);
    scriptEditors_[key] = std::move(editor);
    return scriptEditors_[key].get();
}

void PythonWorkspaceScriptMenu::openEditor(const std::string& key) {
    if (auto it = scriptEditors_.find(key); it != scriptEditors_.end() && it->second) {
        it->second->show();
        it->second->raise();
    } else if (auto script = scripts_.getScript(key)) {
        auto* editor = newScriptEditor(key);
        editor->setName(key);
        editor->setSource(*script);
    }
}

std::optional<std::string> PythonWorkspaceScriptMenu::getScriptName(std::string_view suggestion) {
    bool ok = false;
    const auto qKey = QInputDialog::getText(nullptr, "Script name", "Name:", QLineEdit::Normal,
                                            utilqt::toQString(suggestion), &ok,
                                            Qt::WindowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    auto key = utilqt::fromQString(qKey);
    if (ok && !key.empty()) {
        return key;
    } else {
        return std::nullopt;
    }
}

}  // namespace inviwo
