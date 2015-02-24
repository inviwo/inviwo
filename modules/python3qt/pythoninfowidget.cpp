/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "pythoninfowidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>

#include <modules/python3/pyinviwo.h>

namespace inviwo {
PythonInfoWidget::PythonInfoWidget(QWidget* parent)
    :  InviwoDockWidget(tr("Python API Documentation"),parent) {
    setObjectName("PythonInfoWidget");
    setVisible(false);
    buildWidget();
    resize(500, 900);

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFloating(true);
}

PythonInfoWidget::~PythonInfoWidget() {
}

void PythonInfoWidget::show(){
    InviwoDockWidget::show();
}

void PythonInfoWidget::buildWidget() {
    QWidget* content = new QWidget(this);
    QVBoxLayout* layout_ = new QVBoxLayout();
    tabWidget_ = new QTabWidget(content);
    layout_->addWidget(tabWidget_);
    std::vector<PyModule*> modules = PyInviwo::getPtr()->getAllPythonModules();

    for (auto& module : modules) {
        onModuleRegistered(module);
    }

    content->setLayout(layout_);
    setWidget(content);
}

void PythonInfoWidget::onModuleRegistered(PyModule* module) {
    QScrollArea* tab = new QScrollArea(tabWidget_);
    tab->setWidgetResizable(true);
    QWidget* content = new QWidget(tab);
    std::vector<PyMethod*> methods =  module->getPyMethods();
    QGridLayout* layout = new QGridLayout();
    layout->setColumnStretch(2,1);
    QLabel* funcLabel = new QLabel("Function");
    QLabel* paramLabel = new QLabel("Parameters");
    QLabel* descLabel = new QLabel("Description");
    QFont font = funcLabel->font();
    font.setPointSize(font.pointSize()+1);
    font.setBold(true);
    funcLabel->setFont(font);
    paramLabel->setFont(font);
    descLabel->setFont(font);
    layout->addWidget(funcLabel,0,0);
    layout->addWidget(paramLabel,0,1);
    layout->addWidget(descLabel,0,2);
    layout->addWidget(new QLabel("<hr />"),1,0,1,3);

    int row = 0;
    for (int i = 0; i<static_cast<int>(methods.size()); ++i) {
        row = i*2 + 2;
        QLabel* functionNameLabel = new QLabel(methods[i]->getName().c_str());
        functionNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        layout->addWidget(functionNameLabel,row,0);
        std::string params = methods[i]->getParamDesc();
        replaceInString(params," , ","<br />");
        layout->addWidget(new QLabel(params.c_str()),row,1);
        QLabel* desc = new QLabel(methods[i]->getDesc().c_str());
        desc->setWordWrap(true);
        layout->addWidget(desc,row,2);
        layout->addWidget(new QLabel("<hr />"),row+1,0,1,3);
    }

    if(row)
        layout->addItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding),row+2,0);

    content->setLayout(layout);
    tab->setWidget(content);
    tabWidget_->addTab(tab,module->getModuleName());
}

} // namespace

