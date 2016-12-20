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

constexpr auto UnicodePlay = 9658;
constexpr auto UnicodeVerticalBar = 10073;
constexpr auto UnicodePause = 0xfe0e;//= 10074;
constexpr auto UnicodeStop = 9724;

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
	addObservation(&controller_);
}

void AnimationEditorDockWidgetQt::setAnimation(Animation * animation) {
	controller_.setAnimation(animation);
}

void AnimationEditorDockWidgetQt::generateWidget() {

	btnPlayPause_ = new QPushButton(QChar(UnicodePlay));
    connect(btnPlayPause_, &QPushButton::clicked, [&]() {
		if (controller_.getState() == AnimationState::Playing) {
			controller_.pause();
		}
		else if (controller_.getState() == AnimationState::Paused) {
			controller_.play();
		}
	});
	btnStop_ = new QPushButton(QChar(UnicodeStop));
    connect(btnStop_, &QPushButton::clicked, [&]() {
		controller_.stop();
    });

	btnPlayPause_->setFixedSize(75, 25);
	btnStop_->setFixedSize(75, 25);

	// Exposes controller buttons
    auto controllerLayout = new QHBoxLayout();
    controllerLayout->addWidget(btnPlayPause_);
    controllerLayout->addWidget(btnStop_);

	// 'Window' of all options (top left)
	auto optionsLayout = new QVBoxLayout();
	optionsLayout->addItem(controllerLayout);

	// List widget of track names
	lstTrackNames_ = new QListWidget();

	// Entire left half
    auto leftPanel = new QVBoxLayout();
    leftPanel->addItem(optionsLayout);
    leftPanel->addWidget(lstTrackNames_);

	// Entire right half
    auto rightPanel = new QVBoxLayout();
    //rightPanel->addWidget(new QLabel("Tracks"));

    animationEditor_ = new AnimationEditorQt(*animation_);
    animationView_ = new AnimationViewQt(controller_);

	animationView_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	animationView_->setMinimumSize(200, 200);
	animationView_->setScene(animationEditor_);

	//animationView->setDragMode(QGraphicsView::ScrollHandDrag);
    rightPanel->addWidget(animationView_);

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

void AnimationEditorDockWidgetQt::onStateChanged(AnimationController* controller, AnimationState prevState, AnimationState newState) {
	if (newState == AnimationState::Playing) {
		const QChar Pause[2] = { QChar(UnicodeVerticalBar), QChar(UnicodeVerticalBar) };
		btnPlayPause_->setText(QString(Pause, 2));
	}
	else if (newState == AnimationState::Paused) {
		btnPlayPause_->setText(QChar(UnicodePlay));
	}
}

} // namespace

} // namespace

