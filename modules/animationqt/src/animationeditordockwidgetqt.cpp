/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/util/raiiutils.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>
#include <modules/qtwidgets/textlabeloverlay.h>

#include <modules/animation/animationmanager.h>
#include <modules/animation/animationcontroller.h>
#include <modules/animation/datastructures/controlkeyframe.h>
#include <modules/animation/datastructures/controlkeyframesequence.h>

#include <modules/animationqt/widgets/keyframewidgetqt.h>
#include <modules/animationqt/widgets/keyframesequencewidgetqt.h>
#include <modules/animationqt/widgets/trackwidgetqt.h>
#include <modules/animationqt/animationeditorqt.h>
#include <modules/animationqt/animationviewqt.h>

#include <modules/animationqt/sequenceeditor/sequenceeditorpanel.h>
#include <modules/animationqt/animationlabelviewqt.h>

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
#include <QScrollBar>
#include <warn/pop>

namespace inviwo {

namespace animation {

AnimationEditorDockWidgetQt::AnimationEditorDockWidgetQt(AnimationManager& manager,
                                                         const std::string& widgetName,
                                                         TrackWidgetQtFactory& widgetFactory,
                                                         SequenceEditorFactory& editorFactory,
                                                         QWidget* parent)
    : InviwoDockWidget(utilqt::toQString(widgetName), parent, "AnimationEditorWidget")
    , controller_{manager.getAnimationController()} {

    resize(utilqt::emToPx(this, QSizeF(100, 40)));  // default size
    setAllowedAreas(Qt::BottomDockWidgetArea);

    setFloating(true);
    setSticky(true);

    setWindowIcon(
        QIcon(":/animation/icons/arrow_next_player_previous_recording_right_icon_128.png"));

    // right part
    sequenceEditorView_ = new SequenceEditorPanel(manager, editorFactory, this);

    auto optionLayout = sequenceEditorView_->getOptionLayout();
    // Settings for the controller
    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    for (auto property : controller_.getProperties()) {
        auto propWidget = factory->create(property);
        auto propWidgetQt = static_cast<PropertyWidgetQt*>(propWidget.release());
        optionLayout->addWidget(propWidgetQt);
        propWidgetQt->initState();
    }

    // Entire mid part
    auto overlay = new TextLabelOverlay(nullptr);
    animationEditor_ = std::make_unique<AnimationEditorQt>(controller_, widgetFactory, *overlay);
    animationView_ = new AnimationViewQt(controller_, animationEditor_.get());
    animationView_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    {
        overlay->setParent(animationView_->viewport());
        auto grid = new QGridLayout(animationView_->viewport());
        auto const space = utilqt::refSpacePx(this);
        grid->setContentsMargins(space, space, space, space);
        grid->addWidget(overlay, 0, 0, Qt::AlignTop | Qt::AlignLeft);
        auto sp = overlay->sizePolicy();
        sp.setHorizontalStretch(10);
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        overlay->setSizePolicy(sp);
    }

    mainWindow_ = new QMainWindow();
    mainWindow_->setContextMenuPolicy(Qt::NoContextMenu);
    mainWindow_->setCentralWidget(sequenceEditorView_);

    // left part List widget of track labels
    auto animationLabelView = new AnimationLabelViewQt(controller_);
    animationLabelView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    animationLabelView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    animationLabelView->verticalScrollBar()->setTracking(true);

    auto splitter = new QSplitter();
    splitter->setMidLineWidth(0);
    splitter->setHandleWidth(1);
    splitter->setLineWidth(0);
    splitter->setFrameStyle(QFrame::NoFrame);
    splitter->addWidget(animationLabelView);
    splitter->addWidget(animationView_);
    splitter->addWidget(mainWindow_);

    setWidget(splitter);

    connect(animationView_->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this, animationLabelView](auto val) {
                if (vScrolling_) return;
                util::KeepTrueWhileInScope scrolling(&vScrolling_);
                auto vs = animationView_->verticalScrollBar();
                auto ls = animationLabelView->verticalScrollBar();

                const double vSize = vs->maximum() - vs->minimum() + vs->pageStep();
                const double lSize = ls->maximum() - ls->minimum() + ls->pageStep();
                const auto lval = static_cast<int>(std::round(val * lSize / vSize));

                ls->setValue(lval);
            });
    connect(animationLabelView->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this, animationLabelView](auto val) {
                if (vScrolling_) return;
                util::KeepTrueWhileInScope scrolling(&vScrolling_);
                auto vs = animationView_->verticalScrollBar();
                auto ls = animationLabelView->verticalScrollBar();

                const double vSize = vs->maximum() - vs->minimum() + vs->pageStep();
                const double lSize = ls->maximum() - ls->minimum() + ls->pageStep();
                const auto vval = static_cast<int>(std::round(val * vSize / lSize));

                vs->setValue(vval);
            });

    {
        auto policy = animationLabelView->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        policy.setHorizontalStretch(0);
        animationLabelView->setSizePolicy(policy);
        animationLabelView->setMinimumWidth(150);
    }
    {
        auto policy = animationView_->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        policy.setHorizontalStretch(5);
        animationView_->setSizePolicy(policy);
        animationView_->setMinimumWidth(400);
    }
    {
        auto policy = sequenceEditorView_->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        policy.setHorizontalStretch(0);
        sequenceEditorView_->setSizePolicy(policy);
        sequenceEditorView_->setMinimumWidth(320);  // same as PropertyListWidget
    }

    auto toolBar = new QToolBar();
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    mainWindow_->addToolBar(toolBar);
    {
        auto begin = toolBar->addAction(
            QIcon(":/animation/icons/arrow_media_next_player_previous_song_icon_32.png"),
            "To Beginning");
        begin->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        begin->setToolTip("To Beginning");
        mainWindow_->addAction(begin);
        connect(begin, &QAction::triggered,
                [&]() { controller_.eval(controller_.getCurrentTime(), Seconds(0.0)); });
    }

    {
        auto prev = toolBar->addAction(
            QIcon(":/animation/icons/arrow_arrows_direction_previous_icon_32.png"), "Prev Key");
        prev->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        prev->setToolTip("Prev Key");
        mainWindow_->addAction(prev);
        connect(prev, &QAction::triggered, [&]() {
            auto times = controller_.getAnimation().getAllTimes();
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
        mainWindow_->addAction(btnPlayPause_);

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
        mainWindow_->addAction(next);
        connect(next, &QAction::triggered, [&]() {
            auto times = controller_.getAnimation().getAllTimes();
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
        mainWindow_->addAction(end);
        connect(end, &QAction::triggered, [&]() {
            auto endTime = controller_.getAnimation().getLastTime();
            controller_.eval(controller_.getCurrentTime(), endTime);
        });
    }

    toolBar->addSeparator();
    controller_.AnimationControllerObservable::addObserver(this);
}

AnimationEditorDockWidgetQt::~AnimationEditorDockWidgetQt() = default;

void AnimationEditorDockWidgetQt::onStateChanged(AnimationController*, AnimationState,
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
