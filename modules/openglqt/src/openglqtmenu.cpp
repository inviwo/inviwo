/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <modules/openglqt/openglqtmenu.h>

#include <inviwo/core/util/iterrange.h>
#include <inviwo/core/util/stdextensions.h>

#include <inviwo/core/util/logcentral.h>

#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/openglqt/shaderwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <functional>
#include <utility>
#include <vector>
#include <ranges>

#include <QAction>
#include <QIcon>
#include <QMainWindow>  // IWYU pragma: keep
#include <QString>
#include <Qt>

class QWidget;

namespace inviwo {

OpenGLQtMenu::OpenGLQtMenu(QWidget* parent) : QMenu(tr("&Shaders"), parent) {
    /*
    onAddShader_ = ShaderManager::getPtr()->onDidAddShader([this](GLuint id) {
        const auto& shaders = ShaderManager::getPtr()->getShaders();
        auto it = util::find_if(shaders, [id](Shader* s) { return s->getID() == id; });
        if (it != shaders.end()) {
            auto* shader = *it;
            auto* menuItem = addMenu("");
            shadersItems_[id] = menuItem;

            addShaderObjects(shader, menuItem);

            shader->onReload([this, shader, menuItem]() {
                menuItem->clear();
                addShaderObjects(shader, menuItem);
            });
        }
    });
    */

    onRemoveShader_ = ShaderManager::getPtr()->onWillRemoveShader([this](GLuint id) {
        // Close any open editors.
        const auto& shaders = ShaderManager::getPtr()->getShaders();
        auto it = util::find_if(shaders, [id](Shader* s) { return s->getID() == id; });
        for (auto& obj : (*it)->getShaderObjects()) {
            auto eit = editors_.find(obj.getID());
            if (eit != editors_.end()) {
                eit->second->close();
                editors_.erase(eit);
            }
        }
    });

    connect(this, &QMenu::aboutToShow, this, [this]() {
        clear();

        auto* reloadShaders = addAction(QIcon(":/svgicons/revert.svg"), "&Reload All");
        connect(reloadShaders, &QAction::triggered,
                []() { ShaderManager::getPtr()->rebuildAllShaders(); });
        for (auto* shader : ShaderManager::getPtr()->getShaders()) {
            addShaderObjects(shader);
        }
    });
}

OpenGLQtMenu::~OpenGLQtMenu() = default;
void OpenGLQtMenu::addShaderObjects(Shader* shader) {
    const auto title =
        fmt::format("Id {:2}: {}{}", shader->getID(),
                    fmt::join(shader->getShaderObjects() |
                                  std::views::transform([](const auto& obj) -> const std::string& {
                                      return obj.getFileName();
                                  }),
                              ", "),
                    !shader->getLabel().empty() ? fmt::format(" ({})", shader->getLabel()) : "");

    auto* item = addMenu(utilqt::toQString(title));
    for (auto& obj : shader->getShaderObjects()) {
        auto* action = item->addAction(utilqt::toQString(obj.getFileName()));
        connect(action, &QAction::triggered,
                [this, shaderID = shader->getID(), objID = obj.getID()]() {
                    showShader(shaderID, objID);
                });
    }
}

void OpenGLQtMenu::showShader(GLuint shaderID, GLuint objectID) {
    auto* mainWindow = utilqt::getApplicationMainWindow();

    auto it = editors_.find(objectID);
    if (it != editors_.end()) {
        it->second->show();
        it->second->raise();
        it->second->activateWindow();
    } else {
        const auto& shaders = ShaderManager::getPtr()->getShaders();
        auto sit =
            std::ranges::find_if(shaders, [&](const Shader* s) { return s->getID() == shaderID; });
        if (sit == shaders.end()) return;

        auto objs = (*sit)->getShaderObjects();
        auto oit = std::ranges::find_if(
            objs, [&](const ShaderObject& obj) { return obj.getID() == objectID; });
        if (oit == objs.end()) return;

        ShaderObject& obj = *oit;

        auto editor = std::make_unique<ShaderWidget>(&obj, mainWindow);
        editor->setAttribute(Qt::WA_DeleteOnClose);
        connect(editor.get(), &ShaderWidget::destroyed, this, [this, objectID]() {
            auto i = editors_.find(objectID);
            if (i != editors_.end()) {
                i->second.release();
                editors_.erase(i);
            }
        });
        editor->show();
        editor->raise();
        editor->activateWindow();
        editors_[objectID] = std::move(editor);
    }
}

}  // namespace inviwo
