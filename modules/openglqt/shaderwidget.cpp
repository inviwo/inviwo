/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "shaderwidget.h"
#include <modules/opengl/shader/shaderobject.h>


#include <warn/push>
#include <warn/ignore/all>
#include <QTextEdit>
#include <QDialog>
#include <QToolBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QGridLayout>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <warn/pop>

namespace inviwo {

ShaderWidget::ShaderWidget(const ShaderObject* obj, QWidget* parent)
    : InviwoDockWidget(QString::fromStdString(obj->getFileName()), parent), obj_(obj) {
    setObjectName("ShaderEditor");

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFloating(true);
    setSticky(false);

    QMainWindow* mainWindow = new QMainWindow();
    mainWindow->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow);

    auto shadercode = new QTextEdit(nullptr);
    shadercode->setReadOnly(false);
    shadercode->setText(obj->print(false, false).c_str());
    shadercode->setStyleSheet("font: 10pt \"Courier\";");
    shadercode->setWordWrapMode(QTextOption::NoWrap);

    auto save = toolBar->addAction(QIcon(":/icons/save.png"), tr("&Save shader"));
    save->setShortcut(QKeySequence::Save); 
    save->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    mainWindow->addAction(save);
    connect(save, &QAction::triggered,[=]() {
        std::ofstream file(obj->getAbsoluteFileName());
        file << shadercode->toPlainText().toLocal8Bit().constData();
        file.close();
    });

    auto showSource = toolBar->addAction("Show Sources");
    showSource->setChecked(false);
    showSource->setCheckable(true);

    auto preprocess = toolBar->addAction("Show preprocess");
    preprocess->setChecked(false);
    preprocess->setCheckable(true);


    auto update = [=](int state) {
        shadercode->setText(obj->print(showSource->isChecked(), preprocess->isChecked()).c_str());
        shadercode->setReadOnly(showSource->isChecked() || preprocess->isChecked());
        save->setEnabled(!showSource->isChecked() && !preprocess->isChecked());
        showSource->setText(showSource->isChecked() ? "Hide Sources" : "Show Sources");
        preprocess->setText(preprocess->isChecked() ? "Hide Preprocessed" : "Show Preprocessed");
    };

    connect(showSource, &QAction::triggered, update);
    connect(preprocess, &QAction::triggered, update);

    mainWindow->setCentralWidget(shadercode);
}

ShaderWidget::~ShaderWidget() {}

void ShaderWidget::closeEvent(QCloseEvent* event) {
    event->accept();
    emit widgetClosed();
    this->deleteLater();
}

}  // namespace
