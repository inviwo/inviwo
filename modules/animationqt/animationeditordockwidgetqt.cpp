/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertywidgetfactory.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>

#include <modules/animation/animationcontroller.h>
#include <modules/animation/datastructures/controlkeyframe.h>
#include <modules/animation/datastructures/controlkeyframesequence.h>

#include <modules/animationqt/widgets/keyframewidgetqt.h>
#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>
#include <modules/animationqt/widgets/trackwidgetqt.h>
#include <modules/animationqt/animationeditorqt.h>
#include <modules/animationqt/animationviewqt.h>
#include <modules/animationqt/animationlabelviewqt.h>
#include <modules/animationqt/sequenceeditorpanel/sequenceeditorpanel.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QSettings>
#include <QToolBar>
#include <QMainWindow>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

namespace animation {

AnimationEditorDockWidgetQt::AnimationEditorDockWidgetQt(AnimationController& controller,
                                                         const std::string& widgetName,
                                                         QWidget* parent)
    : InviwoDockWidget(utilqt::toQString(widgetName), parent, "AnimationEditorWidget")
    , controller_(controller) {

    resize(QSize(1000, 400));  // default size
    setAllowedAreas(Qt::BottomDockWidgetArea);

    setFloating(true);
    setSticky(true);

    setWindowIcon(
        QIcon(":/animation/icons/arrow_next_player_previous_recording_right_icon_128.png"));

    //////////////////////////////////////////////////////
    // Left part: Track labels and Controller properties

    QVBoxLayout* leftPanelLayout = new QVBoxLayout();

    // List widget of track labels
    animationLabelView_ = new AnimationLabelViewQt(controller_);
    animationLabelView_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    leftPanelLayout->addWidget(animationLabelView_);

    // Settings for the controller
    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    for (auto pThisProperty : controller_.getProperties()) {
        auto propWidget = factory->create(pThisProperty);
        auto propWidgetQt = static_cast<PropertyWidgetQt*>(propWidget.release());
        propWidgetQt->initState();
        leftPanelLayout->addWidget(propWidgetQt);
    }

    QWidget* leftPanelContent = new QWidget();
    leftPanelContent->setLayout(leftPanelLayout);

    QScrollArea* leftScroll = new QScrollArea();
    leftScroll->setWidget(leftPanelContent);
    leftScroll->setWidgetResizable(true);

    // Toolbar with play controls
    QToolBar* toolBar = new QToolBar();
    toolBar->setFloatable(false);
    toolBar->setMovable(false);

    // Container for left part
    leftPanel_ = new QMainWindow();
    leftPanel_->setContextMenuPolicy(Qt::NoContextMenu);
    leftPanel_->addToolBar(toolBar);
    leftPanel_->setCentralWidget(leftScroll);

    // Entire mid part
    animationEditor_ = std::make_unique<AnimationEditorQt>(controller_);
    animationView_ = new AnimationViewQt(controller_);
    animationView_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    animationView_->setScene(animationEditor_.get());

    // right part
    sequenceEditorView_ = new SequenceEditorPanel(controller_, this);

    auto splitter1 = new QSplitter();
    splitter1->setMidLineWidth(1);
    splitter1->setHandleWidth(1);
    splitter1->setLineWidth(1);
    splitter1->addWidget(leftPanel_);
    splitter1->addWidget(animationView_);
    splitter1->addWidget(sequenceEditorView_);
    setWidget(splitter1);

    {
        auto policy = leftPanel_->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        policy.setHorizontalStretch(0);
        leftPanel_->setSizePolicy(policy);
        leftPanel_->setMinimumWidth(270);  // width of the tool bar on my (Rickard's) machine
    }
    {
        auto policy = animationView_->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        policy.setHorizontalStretch(5);
        animationView_->setSizePolicy(policy);
        animationView_->setMinimumWidth(600);
    }
    {
        auto policy = sequenceEditorView_->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        policy.setHorizontalStretch(0);
        sequenceEditorView_->setSizePolicy(policy);
        sequenceEditorView_->setMinimumWidth(320);  // same as PropertyListWidget
    }

    {
        auto begin = toolBar->addAction(
            QIcon(":/animation/icons/arrow_media_next_player_previous_song_icon_32.png"),
            "To Beginning");
        begin->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        begin->setToolTip("To Beginning");
        leftPanel_->addAction(begin);
        connect(begin, &QAction::triggered,
                [&]() { controller_.eval(controller_.getCurrentTime(), Seconds(0.0)); });
    }

    {
        auto prev = toolBar->addAction(
            QIcon(":/animation/icons/arrow_arrows_direction_previous_icon_32.png"), "Prev Key");
        prev->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        prev->setToolTip("Prev Key");
        leftPanel_->addAction(prev);
        connect(prev, &QAction::triggered, [&]() {
            auto times = controller_.getAnimation()->getAllTimes();
            auto it = std::lower_bound(times.begin(), times.end(), controller_.getCurrentTime());
            if (it != times.begin()) {
                controller_.eval(controller_.getCurrentTime(), *std::prev(it));
            }
        });
    }

    {
        QIcon icon;
        icon.addFile(":/animation/icons/arrow_play_player_record_right_start_icon_32.png", QSize(),
                     QIcon::Normal, QIcon::Off);
        icon.addFile(":/animation/icons/film_movie_pause_player_sound_icon_32.png", QSize(),
                     QIcon::Normal, QIcon::On);
        btnPlayPause_ = toolBar->addAction(icon, "Play/Pause");
        btnPlayPause_->setShortcut(Qt::Key_P);
        btnPlayPause_->setCheckable(true);
        btnPlayPause_->setChecked(controller_.getState() == AnimationState::Playing);
        btnPlayPause_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        btnPlayPause_->setToolTip("Play/Pause");
        leftPanel_->addAction(btnPlayPause_);

        connect(btnPlayPause_, &QAction::triggered, [&]() {
            if (controller_.getState() == AnimationState::Playing) {
                controller_.pause();
            } else if (controller_.getState() == AnimationState::Paused) {
                controller_.play();
            }
        });
    }

    {
        auto next = toolBar->addAction(
            QIcon(":/animation/icons/arrow_arrows_direction_next_previous_icon_32.png"),
            "Next Key");
        next->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        next->setToolTip("Next Key");
        leftPanel_->addAction(next);
        connect(next, &QAction::triggered, [&]() {
            auto times = controller_.getAnimation()->getAllTimes();
            auto it = std::upper_bound(times.begin(), times.end(), controller_.getCurrentTime());
            if (it != times.end()) {
                controller_.eval(controller_.getCurrentTime(), *it);
            }
        });
    }

    {
        auto end = toolBar->addAction(
            QIcon(":/animation/icons/arrow_next_player_previous_icon_32.png"), "To End");
        end->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        end->setToolTip("To End");
        leftPanel_->addAction(end);
        connect(end, &QAction::triggered, [&]() {
            auto endTime = controller_.getAnimation()->getLastTime();
            controller_.eval(controller_.getCurrentTime(), endTime);
        });
    }

    toolBar->addSeparator();
    controller_.AnimationControllerObservable::addObserver(this);
}

AnimationEditorDockWidgetQt::~AnimationEditorDockWidgetQt() = default;

void AnimationEditorDockWidgetQt::onStateChanged(AnimationController*,
                                                 AnimationState,
                                                 AnimationState newState) {
    if (newState == AnimationState::Playing) {
        QSignalBlocker block(btnPlayPause_);
        btnPlayPause_->setChecked(true);
    } else if (newState == AnimationState::Paused) {
        QSignalBlocker block(btnPlayPause_);
        btnPlayPause_->setChecked(false);
    }
}

}  // namespace animation

}  // namespace inviwo
