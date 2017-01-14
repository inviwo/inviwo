/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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
#include <QSettings>
#include <QToolBar>
#include <QMainWindow>
#include <warn/pop>

constexpr auto UnicodePlay = 9658;
constexpr auto UnicodeVerticalBar = 10073;
constexpr auto UnicodePause = 9208;  //= 10074;
constexpr auto UnicodeStop = 9724;

namespace inviwo {

namespace animation {

AnimationEditorDockWidgetQt::AnimationEditorDockWidgetQt(AnimationController& controller,
                                                         const std::string& widgetName,
                                                         QWidget* parent)
    : InviwoDockWidget(QString(widgetName.c_str()), parent), controller_(controller) {

    setObjectName("AnimationEditor");
    setFloating(true);
    setSticky(false);
    setWindowIcon(
        QIcon(":/animation/icons/arrow_next_player_previous_recording_right_icon_128.png"));

    // List widget of track labels
    animationLabelView_ = new AnimationLabelViewQt(*controller_.getAnimation());

    // Entire right half
    animationEditor_ = std::make_unique<AnimationEditorQt>(controller_);
    animationView_ = new AnimationViewQt(controller_);

    animationView_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    {
        animationView_->setMinimumSize(300, 200);
        animationView_->setScene(animationEditor_.get());
        auto policy = animationView_->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Expanding);
        policy.setHorizontalStretch(5);
        animationView_->setSizePolicy(policy);
    }

    auto leftWidget = new QMainWindow();
    QToolBar* toolBar = new QToolBar();
    {
        leftWidget->setContextMenuPolicy(Qt::NoContextMenu);
        leftWidget->addToolBar(toolBar);
        toolBar->setFloatable(false);
        toolBar->setMovable(false);
        leftWidget->setCentralWidget(animationLabelView_);
        auto policy = leftWidget->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Fixed);
        policy.setHorizontalStretch(0);
        leftWidget->setSizePolicy(policy);
        leftWidget->setMinimumWidth(160);
    }

    auto splitter = new QSplitter();
    splitter->setMidLineWidth(1);
    splitter->setHandleWidth(1);
    splitter->setLineWidth(1);
    splitter->addWidget(leftWidget);
    splitter->addWidget(animationView_);
    setWidget(splitter);

    {
        auto begin = toolBar->addAction(
            QIcon(":/animation/icons/arrow_media_next_player_previous_song_icon_32.png"),
            "To Beginning");
        begin->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        begin->setToolTip("To Beginning");
        leftWidget->addAction(begin);
        connect(begin, &QAction::triggered, [&]() {
            controller_.eval(controller_.getCurrentTime(), Seconds(0.0));
        });
    }

    {
        auto prev = toolBar->addAction(
            QIcon(":/animation/icons/arrow_arrows_direction_previous_icon_32.png"), "Prev Key");
        prev->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        prev->setToolTip("Prev Key");
        leftWidget->addAction(prev);
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
        leftWidget->addAction(btnPlayPause_);

        connect(btnPlayPause_, &QAction::triggered, [&]() {
            if (controller_.getState() == AnimationState::Playing) {
                controller_.pause();
            } else if (controller_.getState() == AnimationState::Paused) {
                controller_.play();
            }
        });
    }
    {
        btnStop_ = toolBar->addAction(
            QIcon(":/animation/icons/multimedia_off_recording_station_stop_icon_32.png"), "Stop");
        btnStop_->setShortcut(Qt::Key_S);
        btnStop_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        btnStop_->setToolTip("Stop");
        leftWidget->addAction(btnStop_);

        connect(btnStop_, &QAction::triggered, [&]() { 
            controller_.stop(); 
        });
    }

    {
        auto next = toolBar->addAction(
            QIcon(":/animation/icons/arrow_arrows_direction_next_previous_icon_32.png"), "Next Key");
        next->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        next->setToolTip("Next Key");
        leftWidget->addAction(next);
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
        leftWidget->addAction(end);
        connect(end, &QAction::triggered, [&]() {
            auto end = controller_.getAnimation()->lastTime();
            controller_.eval(controller_.getCurrentTime(), end);
        });
    }

    toolBar->addSeparator();

    {
        QIcon icon;
        icon.addFile(":/animation/icons/arrows_media_player_repeat_song_sound_video_icon_32.png", QSize(),
                     QIcon::Normal, QIcon::On);
        icon.addFile(":/animation/icons/arrow_direction_next_previous_right_icon_32.png", QSize(),
                     QIcon::Normal, QIcon::Off);
     
        loop_ = toolBar->addAction(
            icon, "Loop");
        loop_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        loop_->setCheckable(true);
        loop_->setChecked(controller_.getPlaybackMode() == PlaybackMode::Loop);
        loop_->setToolTip("Loop");
        leftWidget->addAction(loop_);
        connect(loop_, &QAction::triggered, [&](bool checked) {
            if (checked) {
                controller_.setPlaybackMode(PlaybackMode::Loop);
            } else {
                controller_.setPlaybackMode(PlaybackMode::Once);
            }
        });
    }


    addObservation(&controller_);

    {
        // Restore State
        QSettings settings("Inviwo", "Inviwo");
        settings.beginGroup("AnimationEditor");
        restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
        setSticky(settings.value("isSticky", InviwoDockWidget::isSticky()).toBool());
        settings.endGroup();
    }
}

AnimationEditorDockWidgetQt::~AnimationEditorDockWidgetQt() = default;

void AnimationEditorDockWidgetQt::onStateChanged(AnimationController* controller,
                                                 AnimationState prevState,
                                                 AnimationState newState) {
    if (newState == AnimationState::Playing) {
        QSignalBlocker block(btnPlayPause_);
        btnPlayPause_->setChecked(true);
    } else if (newState == AnimationState::Paused) {
        QSignalBlocker block(btnPlayPause_);
        btnPlayPause_->setChecked(false);
    }
}

void AnimationEditorDockWidgetQt::onPlaybackModeChanged(AnimationController* controller,
                                                        PlaybackMode prevMode,
                                                        PlaybackMode newMode) {
    QSignalBlocker block(btnPlayPause_);
    loop_->setChecked(newMode == PlaybackMode::Loop);
}

void AnimationEditorDockWidgetQt::closeEvent(QCloseEvent* event) {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("AnimationEditor");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("isSticky", isSticky());
    settings.endGroup();

    InviwoDockWidget::closeEvent(event);
}

}  // namespace

}  // namespace
