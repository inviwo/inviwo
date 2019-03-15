/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/opengl/shader/shaderresource.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/openglqt/shaderwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMainWindow>
#include <QMenuBar>
#include <warn/pop>

namespace inviwo {

OpenGLQtMenu::OpenGLQtMenu(QWidget* parent) : QMenu(tr("&Shaders"), parent) {
    QAction* reloadShaders = addAction(QIcon(":/svgicons/revert.svg"), "&Reload All");
    connect(reloadShaders, &QAction::triggered, [&]() { shadersReload(); });

    onAddShader_ = ShaderManager::getPtr()->onDidAddShader([this](GLuint id) {
        const auto& shaders = ShaderManager::getPtr()->getShaders();
        auto it = util::find_if(shaders, [id](Shader* s) { return s->getID() == id; });
        if (it != shaders.end()) {
            auto shader = *it;
            auto menuItem = addMenu(QString("Id %1").arg(shader->getID(), 2));
            shadersItems_[id] = menuItem;

            addShaderObjects(shader, menuItem);

            shader->onReload([this, shader, menuItem]() {
                menuItem->clear();
                menuItem->setTitle(QString("Id %1").arg(shader->getID()));
                addShaderObjects(shader, menuItem);
            });
        }
    });

    onRemoveShader_ = ShaderManager::getPtr()->onWillRemoveShader([this](GLuint id) {
        {
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
        }
        {
            auto it = shadersItems_.find(id);
            if (it != shadersItems_.end()) {
                removeAction(it->second->menuAction());
                shadersItems_.erase(it);
            }
        }
    });
}

void OpenGLQtMenu::addShaderObjects(Shader* shader, QMenu* menuItem) {
    for (auto& obj : shader->getShaderObjects()) {
        auto name = QString::fromStdString(obj.getFileName());
        auto action = menuItem->addAction(name);
        menuItem->setTitle(menuItem->title() + QString(", ") + name);
        connect(action, &QAction::triggered, [&]() { showShader(&obj); });
    }
}

void OpenGLQtMenu::showShader(const ShaderObject* obj) {
    auto mainWindow = utilqt::getApplicationMainWindow();

    auto it = editors_.find(obj->getID());
    if (it != editors_.end()) {
        it->second->show();
        it->second->raise();
        it->second->activateWindow();
    } else {
        auto editor = std::make_unique<ShaderWidget>(obj, mainWindow);
        editor->setAttribute(Qt::WA_DeleteOnClose);
        auto id = obj->getID();

        connect(editor.get(), &ShaderWidget::destroyed, this, [this, id]() {
            auto i = editors_.find(id);
            if (i != editors_.end()) {
                i->second.release();
                editors_.erase(i);
            }
        });
        editor->resize(900, 800);
        editor->show();
        editor->raise();
        editor->activateWindow();
        editors_[id] = std::move(editor);
    }
}

void OpenGLQtMenu::shadersReload() { ShaderManager::getPtr()->rebuildAllShaders(); }

}  // namespace inviwo
