/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

    const auto* newScript =
        menu_->addAction(QIcon(":/svgicons/newfile.svg"), "New &Workspace Script ...");
    QObject::connect(newScript, &QAction::triggered, [this] {
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
}
void PythonWorkspaceScriptMenu::onScriptRemoved(std::string_view key, std::string_view) {
    if (const auto it = menuItems_.find(key); it != menuItems_.end()) {
        menuItems_.erase(it);
    }
    if (const auto it = scriptEditors_.find(key); it != scriptEditors_.end() && it->second) {
        scriptEditors_.erase(it);
    }
}
void PythonWorkspaceScriptMenu::onScriptSaved(std::string_view key, std::string_view) {
    if (const auto it = scriptEditors_.find(key); it != scriptEditors_.end() && it->second) {
        it->second->setWindowModified(false);
    }
}
void PythonWorkspaceScriptMenu::onScriptUpdate(std::string_view key, std::string_view script) {
    if (const auto it = scriptEditors_.find(key); it != scriptEditors_.end() && it->second) {
        it->second->setSource(script);
    }
}

void PythonWorkspaceScriptMenu::addScriptMenuItem(std::string_view key) {
    const auto qKey = utilqt::toQString(key);
    auto* openScript = menu_->addAction(qKey);
    menuItems_.emplace(key, openScript);
    QObject::connect(openScript, &QAction::triggered,
                     [this, key = std::string{key}] { openEditor(key); });
}
PythonEditorWidget* PythonWorkspaceScriptMenu::newScriptEditor(const std::string& key) {
    auto editor = util::make_qptr<PythonEditorWidget>(
        win_, app_,
        [this, key](PythonEditorWidget&, const std::string& source) {
            scripts_.updateScript(key, source);
        },
        [this, key](UnorderedStringMap<QVariant>& state) {
            if (auto m = scripts_.getMetaData<bool>(key, "sticky")) {
                state["sticky"] = *m;
            }
            if (auto m = scripts_.getMetaData<bool>(key, "floating")) {
                state["floating"] = *m;
            }
            if (auto m = scripts_.getMetaData<bool>(key, "visible")) {
                state["visible"] = *m;
            }
            if (auto m = scripts_.getMetaData<ivec2>(key, "size")) {
                state["size"] = utilqt::toQSize(*m);
            }
            if (auto m = scripts_.getMetaData<ivec2>(key, "pos")) {
                state["pos"] = utilqt::toQPoint(*m);
            }
        },
        [this, key](const UnorderedStringMap<QVariant>& state) {
            if (state.contains("sticky")) {
                scripts_.setMetaData(key, "sticky", state.at("sticky").toBool());
            }
            if (state.contains("floating")) {
                scripts_.setMetaData(key, "floating", state.at("floating").toBool());
            }
            if (state.contains("visible")) {
                scripts_.setMetaData(key, "visible", state.at("visible").toBool());
            }
            if (state.contains("size")) {
                scripts_.setMetaData(key, "size", utilqt::toGLM(state.at("size").toSize()));
            }
            if (state.contains("pos")) {
                scripts_.setMetaData(key, "pos", utilqt::toGLM(state.at("pos").toPoint()));
            }
        });

    editor->setAttribute(Qt::WA_DeleteOnClose);
    utilqt::addDockWidgetTabify(editor.get(), win_, Qt::RightDockWidgetArea);
    editor->setSticky(false);
    editor->setFloating(true);
    editor->setVisible(true);
    editor->loadState();

    for (auto&& [k, e] : scriptEditors_) {
        if (e->pos() == editor->pos()) {
            const auto newPos =
                utilqt::movePointOntoDesktop(editor->pos() + QPoint{50, 50}, editor->size());
            editor->move(newPos);
        }
    }

    // Connect this after the state is loaded!
    PythonEditorWidget::connect(editor.get(), &PythonEditorWidget::stateChanged, editor.get(),
                                &PythonEditorWidget::saveState);

    scriptEditors_[key] = std::move(editor);
    return scriptEditors_[key].get();
}

void PythonWorkspaceScriptMenu::openEditor(const std::string& key) {
    if (const auto it = scriptEditors_.find(key); it != scriptEditors_.end() && it->second) {
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
