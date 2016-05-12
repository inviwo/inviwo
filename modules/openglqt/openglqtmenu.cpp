/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "openglqtmenu.h"


#include <modules/opengl/shader/shaderresource.h>

#include <inviwo/qt/widgets/inviwoapplicationqt.h>
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
    QAction* reloadShaders = addAction("Reload All");
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
                auto editor = editors_.find(obj.second->getID());
                if (editor != editors_.end()) editor->second->close();
            }
        }

        auto it = shadersItems_.find(id);
        if (it != shadersItems_.end()) {
            removeAction(it->second->menuAction());
            shadersItems_.erase(it);
        }
    });
}

void OpenGLQtMenu::addShaderObjects(Shader* shader, QMenu* menuItem) {
    for (auto& item : shader->getShaderObjects()) {
        auto name = QString::fromStdString(item.second->getFileName());
        auto action = menuItem->addAction(name);
        menuItem->setTitle(menuItem->title() + QString(", ") + name);
        connect(action, &QAction::triggered, [&]() { showShader(item.second.get()); });
    }
}

void OpenGLQtMenu::showShader(const ShaderObject* obj) {
    auto win = static_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())->getMainWindow();

    auto editor = [&]() {
        if (auto res = util::map_find_or_null(editors_, obj->getID())) {
            return res;
        } else {
            res = new ShaderWidget(obj, win);
            editors_[obj->getID()] = res;
            res->resize(900, 800);
            return res;
        }
    }();
    editor->show();
    auto id = obj->getID();
    connect(editor, &ShaderWidget::widgetClosed, [this, id](){editors_.erase(id);});
}

void OpenGLQtMenu::shadersReload() { ShaderManager::getPtr()->rebuildAllShaders(); }


}  // namespace
