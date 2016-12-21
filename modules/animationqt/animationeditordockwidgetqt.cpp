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
#include <modules/animationqt/animationlabelviewqt.h>
#include <modules/animationqt/trackqt.h>
#include <modules/animationqt/keyframesequenceqt.h>
#include <modules/animationqt/keyframeqt.h>

#include <modules/animation/animationcontroller.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <warn/pop>

constexpr auto UnicodePlay = 9658;
constexpr auto UnicodeVerticalBar = 10073;
constexpr auto UnicodePause = 9208;//= 10074;
constexpr auto UnicodeStop = 9724;

namespace inviwo {

namespace animation {

AnimationEditorDockWidgetQt::AnimationEditorDockWidgetQt(AnimationController& controller, const std::string& widgetName, QWidget* parent) 
    : InviwoDockWidget(QString(widgetName.c_str()), parent)
	, controller_(controller) {

    generateWidget();
    setFloating(true);
	addObservation(&controller_);
}

void AnimationEditorDockWidgetQt::generateWidget() {

	btnPlayPause_ = new QToolButton();
	btnPlayPause_->setText(QChar(UnicodePlay));
    connect(btnPlayPause_, &QToolButton::clicked, [&]() {
		if (controller_.getState() == AnimationState::Playing) {
			controller_.pause();
		}
		else if (controller_.getState() == AnimationState::Paused) {
			controller_.play();
		}
	});

	btnStop_ = new QToolButton();
	btnStop_->setText(QChar(UnicodeStop));
    connect(btnStop_, &QToolButton::clicked, [&]() {
		controller_.stop();
    });

	btnPlayPause_->setFixedSize(15, 15);
	btnStop_->setFixedSize(15, 15);

	// Exposes controller buttons
    auto controllerLayout = new QHBoxLayout();
    controllerLayout->addWidget(btnPlayPause_);
    controllerLayout->addWidget(btnStop_);
	controllerLayout->setSpacing(0);
	controllerLayout->setMargin(0);
	controllerLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	// Layout of all options (top left)
	auto optionsLayout = new QVBoxLayout();
	optionsLayout->addItem(controllerLayout);
	optionsLayout->setSpacing(0);
	optionsLayout->setMargin(0);

	auto optionsWidget = new QWidget();
	optionsWidget->setLayout(optionsLayout);
	optionsWidget->setMinimumHeight(TimelineHeight);
	optionsWidget->setMaximumHeight(TimelineHeight);

	// List widget of track labels
	animationLabelView_ = new AnimationLabelViewQt(*controller_.getAnimation());

	// Entire left half
    auto leftPanel = new QVBoxLayout();
    leftPanel->addWidget(optionsWidget);
    leftPanel->addWidget(animationLabelView_);
	leftPanel->setSpacing(0);
	leftPanel->setMargin(0);

	// Entire right half
    //auto rightPanel = new QVBoxLayout();

    animationEditor_ = new AnimationEditorQt(controller_);
    animationView_ = new AnimationViewQt(controller_);
	//timelineView_ = new TimelineViewQt(controller_);

	animationView_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	animationView_->setMinimumSize(200, 200);
	animationView_->setScene(animationEditor_);

	//timelineView_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	//timelineView_->setScene(animationEditor_);

	//rightPanel->addWidget(timelineView_);
    //rightPanel->addWidget(animationView_);

	auto leftWidget = new QWidget();
	leftWidget->setLayout(leftPanel);
	
	//auto rightWidget = new QWidget();
	//rightWidget->setLayout(rightPanel);

    auto splitter = new QSplitter();
	splitter->setMidLineWidth(1);
	splitter->setHandleWidth(1);
	splitter->setLineWidth(1);
	splitter->addWidget(leftWidget);
	splitter->addWidget(animationView_);

    //hLayout->addWidget(new QSplitter());
   // hLayout->addItem(rightPanel);

    //QWidget* mainPanel = new QWidget(this);
    //mainPanel->setLayout(splitter);
    setWidget(splitter);
}

void AnimationEditorDockWidgetQt::onStateChanged(AnimationController* controller, AnimationState prevState, AnimationState newState) {
	if (newState == AnimationState::Playing) {
		const QChar pause[2] = { QChar(UnicodeVerticalBar), QChar(UnicodeVerticalBar) };
		btnPlayPause_->setText(QChar(UnicodePause));
	}
	else if (newState == AnimationState::Paused) {
		btnPlayPause_->setText(QChar(UnicodePlay));
	}
}

} // namespace

} // namespace

