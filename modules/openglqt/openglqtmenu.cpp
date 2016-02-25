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

#include <modules/opengl/shader/shadermanager.h>
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

 OpenGLQtMenu::OpenGLQtMenu() : shadersItem_(nullptr) {
    if (auto qtApp = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())) {
        if (auto win = qtApp->getMainWindow()) {
            qtApp->getProcessorNetwork()->addObserver(this);

            shadersItem_ = win->menuBar()->addMenu(tr("&Shaders"));

            QAction* reloadShaders = shadersItem_->addAction("Reload All");
            connect(reloadShaders, &QAction::triggered,[&](){ shadersReload(); });
        }
    }
}

void OpenGLQtMenu::updateShadersMenu() {
    if (!shadersItem_) return;

    const auto shaders = ShaderManager::getPtr()->getShaders();
    std::vector<QMenu*> reused;


    auto unusedEditors = util::transform(editors_, [](std::pair<const unsigned int, ShaderWidget*> item){
        return item.first;
    });

    for (auto shader : shaders) {
        QMenu*& shaderSubMenu = shadersItems_[shader->getID()];

        if (!shaderSubMenu) {
            shaderSubMenu = shadersItem_->addMenu(QString("Id %1").arg(shader->getID(), 2));

            for (auto& item : shader->getShaderObjects()) {
                auto action = shaderSubMenu->addAction(
                    QString::fromStdString(item.second->getResource()->key()));
                shaderSubMenu->setTitle(shaderSubMenu->title() + QString(", ") +
                                        QString::fromStdString(item.second->getFileName()));
                connect(action, &QAction::triggered, [&]() { showShader(item.second.get()); });
                
                util::erase_remove(unusedEditors, item.second->getID());
            }
        }
        reused.push_back(shaderSubMenu);
    }

    util::map_erase_remove_if(shadersItems_, [&](std::pair<const unsigned int, QMenu*> item) {
        if (!util::contains(reused, item.second)) {
            shadersItem_->removeAction(item.second->menuAction());
            delete item.second;
            return true;
        } else {
            return false;
        }
    });

    for(auto id : unusedEditors) editors_[id]->close();
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

void OpenGLQtMenu::onProcessorNetworkDidAddProcessor(Processor* processor) { updateShadersMenu(); }
void OpenGLQtMenu::onProcessorNetworkDidRemoveProcessor(Processor* processor) {
    updateShadersMenu();
}

}  // namespace
