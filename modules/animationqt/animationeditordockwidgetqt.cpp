/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/animationqt/animationeditordockwidgetqt.h>
#include <modules/animationqt/animationeditorqt.h>
#include <modules/animationqt/animationviewqt.h>
#include <modules/animationqt/trackqt.h>
#include <modules/animationqt/keyframesequenceqt.h>
#include <modules/animationqt/keyframeqt.h>

#include <modules/animation/animationcontroller.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QListWidget>
#include <warn/pop>

namespace inviwo {

namespace animation {

AnimationEditorDockWidgetQt::AnimationEditorDockWidgetQt(Animation* animation, const std::string& widgetName, QWidget* parent) 
    : InviwoDockWidget(QString(widgetName.c_str()), parent)
    , animation_(animation)
	, controller_(animation) {

    if (animation == nullptr) {
        throw Exception("Animation cannot be null", IvwContext);
    }

    generateWidget();

    setFloating(true);
}

void AnimationEditorDockWidgetQt::setAnimation(Animation * animation) {
	controller_.setAnimation(animation);
}

void AnimationEditorDockWidgetQt::generateWidget() {

    auto btnPlay_ = new QPushButton("Play");
    connect(btnPlay_, &QPushButton::clicked, [&]() {
        controller_.play();
    });
    auto btnPause_ = new QPushButton("Pause");
    connect(btnPause_, &QPushButton::clicked, [&]() {
		controller_.pause();
    });
    auto btnStop_ = new QPushButton("Stop");
    connect(btnStop_, &QPushButton::clicked, [&]() {
		controller_.stop();
    });

    auto controllerLayout = new QHBoxLayout();
    controllerLayout->addWidget(btnPlay_);
    controllerLayout->addWidget(btnPause_);
    controllerLayout->addWidget(btnStop_);

    auto leftPanel = new QVBoxLayout();
    leftPanel->addItem(controllerLayout);
    auto trackNames = new QListWidget();
    leftPanel->addWidget(trackNames);

    auto rightPanel = new QVBoxLayout();
    rightPanel->addWidget(new QLabel("Timeline"));
    //rightPanel->addWidget(new QLabel("Tracks"));

    auto animationEditor = new AnimationEditorQt(*animation_);
    auto animationView = new AnimationViewQt();

	animationView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	animationView->setMinimumSize(200, 200);
	animationView->setScene(animationEditor);

	//animationView->setDragMode(QGraphicsView::ScrollHandDrag);
    rightPanel->addWidget(animationView);

    auto hLayout = new QHBoxLayout();
    hLayout->addItem(leftPanel);
    //auto splitter = new QSplitter();
    //splitter->
    //hLayout->addWidget(new QSplitter());
    hLayout->addItem(rightPanel);

    QWidget* mainPanel = new QWidget(this);
    mainPanel->setLayout(hLayout);
    setWidget(mainPanel);
}

} // namespace

} // namespace

