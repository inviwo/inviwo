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
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/core/network/processornetwork.h>
#include <QTextBrowser>
#include <QDialog>
#include <QVariant>
#include <QMainWindow>
#include <QMenuBar>
#include <QGridLayout>

namespace inviwo {

 OpenGLQtMenu::OpenGLQtMenu() : shadersItem_(nullptr), shaderMapper_(nullptr) {
    InviwoApplicationQt* qtApp = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());

    if (qtApp) {
        QMainWindow* win = qtApp->getMainWindow();

        if (win) {
            InviwoApplication::getPtr()->getProcessorNetwork()->addObserver(this);

            shadersItem_ = win->menuBar()->addMenu(tr("&Shaders"));

            shaderMapper_ = new QSignalMapper(this);
            connect(shaderMapper_, SIGNAL(mapped(QObject*)), this, SLOT(shaderMenuCallback(QObject*)));

            QAction* reloadShaders = shadersItem_->addAction("Reload All");
            connect(reloadShaders, SIGNAL(triggered()), this, SLOT(shadersReload()));
        }
    }
}

void OpenGLQtMenu::updateShadersMenu() {
    if (!shadersItem_) return;

    const std::vector<Shader*> shaders{ShaderManager::getPtr()->getShaders()};

    for (Shader* shader : shaders) {
        QMenu*& shaderSubMenu = shadersItems_[shader->getID()];

        if (!shaderSubMenu) {
            shaderSubMenu =
                shadersItem_->addMenu(QString::fromStdString("Shader " + toString(shader->getID())));
            
            ShaderObject* obj;
            if ((obj = shader->getVertexShaderObject()) != nullptr) {
                QAction* action =
                    shaderSubMenu->addAction(QString::fromStdString(obj->getFileName()));
                action->setData(QSize(shader->getID(), 1));
                connect(action, SIGNAL(triggered()), shaderMapper_, SLOT(map()));
                shaderMapper_->setMapping(action, action);
            }
            if ((obj = shader->getGeometryShaderObject()) != nullptr) {
                QAction* action =
                    shaderSubMenu->addAction(QString::fromStdString(obj->getFileName()));
                action->setData(QSize(shader->getID(), 2));
                connect(action, SIGNAL(triggered()), shaderMapper_, SLOT(map()));
                shaderMapper_->setMapping(action, action);
                shaderSubMenu->addAction(action);
            }
            if ((obj = shader->getFragmentShaderObject()) != nullptr) {
                QAction* action =
                    shaderSubMenu->addAction(QString::fromStdString(obj->getFileName()));
                action->setData(QSize(shader->getID(), 3));
                connect(action, SIGNAL(triggered()), shaderMapper_, SLOT(map()));
                shaderMapper_->setMapping(action, action);
                shaderSubMenu->addAction(action);
            }
        }
    }

    auto it = shadersItems_.begin();
    while (it != shadersItems_.end()) {
        auto sit = std::find_if(shaders.begin(), shaders.end(), [&it](Shader* shader) -> bool {
            return shader->getID() == it->first;
        });
        if (sit == shaders.end()) {
            delete it->second;
            it = shadersItems_.erase(it);
        } else {
            ++it;
        }
    }
}

void OpenGLQtMenu::shaderMenuCallback(QObject* obj) {
    QSize id = qobject_cast<QAction*>(obj)->data().toSize();
    int shaderId = id.width();
    int type = id.height();

    const std::vector<Shader*> shaders{ShaderManager::getPtr()->getShaders()};
    auto it = std::find_if(shaders.begin(), shaders.end(), [shaderId](Shader* shader) -> bool {
        return static_cast<int>(shader->getID()) == shaderId;
    });

    if (it != shaders.end()) {
        ShaderObject* shaderObj;
        switch (type) {
            case 1:
                shaderObj = (*it)->getVertexShaderObject();
                break;
            case 2:
                shaderObj = (*it)->getGeometryShaderObject();
                break;
            case 3:
                shaderObj = (*it)->getFragmentShaderObject();
                break;
            default:
                return;
        }

        QMainWindow* win =
            static_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())->getMainWindow();
        
        QTextBrowser* shadercode = new QTextBrowser(nullptr);
        shadercode->setText(shaderObj->print(true).c_str());
        shadercode->setStyleSheet("font: 12pt \"Courier\";");

        QDialog* dialog = new QDialog(win);
        QGridLayout* layout = new QGridLayout(dialog);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(shadercode);
        dialog->setLayout(layout);
        dialog->resize(600, 800);
        dialog->exec();

        delete dialog;
    }
}

void OpenGLQtMenu::shadersReload() { ShaderManager::getPtr()->rebuildAllShaders(); }

void OpenGLQtMenu::onProcessorNetworkDidAddProcessor(Processor* processor) { updateShadersMenu(); }
void OpenGLQtMenu::onProcessorNetworkDidRemoveProcessor(Processor* processor) {
    updateShadersMenu();
}

}  // namespace
