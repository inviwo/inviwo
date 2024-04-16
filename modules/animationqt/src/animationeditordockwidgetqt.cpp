/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>                    // for InviwoApplication
#include <inviwo/core/network/workspacemanager.h>                    // for WorkspaceManager
#include <inviwo/core/properties/propertywidgetfactory.h>            // for PropertyWidgetFactory
#include <inviwo/core/util/filedialogstate.h>                        // for FileMode, FileMode::...
#include <inviwo/core/util/filesystem.h>                             // for cleanupPath, fileExists
#include <inviwo/core/util/logcentral.h>                             // for LogCentral, LogError
#include <inviwo/core/util/pathtype.h>                               // for PathType, PathType::...
#include <inviwo/core/util/raiiutils.h>                              // for KeepTrueWhileInScope
#include <modules/animation/animationcontroller.h>                   // for AnimationController
#include <modules/animation/animationcontrollerobserver.h>           // for AnimationControllerO...
#include <modules/animation/animationmanager.h>                      // for AnimationManager
#include <modules/animation/datastructures/animation.h>              // for Animation
#include <modules/animation/datastructures/animationstate.h>         // for AnimationState, Anim...
#include <modules/animation/datastructures/animationtime.h>          // for Seconds
#include <modules/animation/mainanimation.h>                         // for MainAnimation
#include <modules/animation/workspaceanimations.h>                   // for WorkspaceAnimations
#include <modules/animationqt/animationeditorqt.h>                   // for AnimationEditorQt
#include <modules/animationqt/animationlabelviewqt.h>                // for AnimationLabelViewQt
#include <modules/animationqt/animationviewqt.h>                     // for AnimationViewQt
#include <modules/animationqt/sequenceeditor/sequenceeditorpanel.h>  // for SequenceEditorPanel
#include <modules/animationqt/workspaceanimationsmodel.h>            // for AnimationsModel
#include <modules/qtwidgets/inviwodockwidget.h>                      // for InviwoDockWidget
#include <modules/qtwidgets/inviwofiledialog.h>                      // for InviwoFileDialog
#include <modules/qtwidgets/inviwoqtutils.h>                         // for fromQString, toQString
#include <modules/qtwidgets/properties/propertywidgetqt.h>           // for PropertyWidgetQt
#include <modules/qtwidgets/textlabeloverlay.h>                      // for TextLabelOverlay

#include <algorithm>  // for lower_bound, upper_b...
#include <chrono>     // for duration
#include <cmath>      // for round
#include <exception>  // for exception
#include <fstream>    // for operator<<, basic_if...
#include <iterator>   // for distance, prev
#include <utility>    // for move
#include <vector>     // for vector

#include <QAction>         // for QAction
#include <QComboBox>       // for QComboBox, QComboBox...
#include <QFrame>          // for QFrame, QFrame::NoFrame
#include <QGridLayout>     // for QGridLayout
#include <QIcon>           // for QIcon, QIcon::Normal
#include <QLayout>         // for QLayout
#include <QList>           // for QList
#include <QMainWindow>     // for QMainWindow
#include <QScrollBar>      // for QScrollBar
#include <QSignalBlocker>  // for QSignalBlocker
#include <QSize>           // for QSize
#include <QSizeF>          // for QSizeF
#include <QSizePolicy>     // for QSizePolicy, QSizePo...
#include <QSplitter>       // for QSplitter
#include <QString>         // for QString
#include <QStringList>     // for QStringList
#include <QToolBar>        // for QToolBar
#include <Qt>              // for operator|, WidgetWit...
#include <QtGlobal>        // for QNonConstOverload<>::of
#include <fmt/core.h>      // for format

class QWidget;

namespace inviwo {

class Property;

namespace animation {

AnimationEditorDockWidgetQt::AnimationEditorDockWidgetQt(
    WorkspaceAnimations& animations, AnimationManager& manager, const std::string& widgetName,
    TrackWidgetQtFactory& widgetFactory, SequenceEditorFactory& editorFactory, QWidget* parent)
    : InviwoDockWidget(utilqt::toQString(widgetName), parent, "AnimationEditorWidget")
    , animations_(animations)
    , controller_{animations_.getMainAnimation().getController()}
    , manager_{manager}
    , mainWindow_(new QMainWindow())
    , sequenceEditorView_(new SequenceEditorPanel(controller_, manager, editorFactory, this))
    , animationView_(new AnimationViewQt(controller_, animationEditor_.get())) {

    resize(utilqt::emToPx(this, QSizeF(100, 40)));  // default size
    setAllowedAreas(Qt::BottomDockWidgetArea);

    setFloating(true);
    setSticky(true);

    setWindowIcon(
        QIcon(":/animation/icons/arrow_next_player_previous_recording_right_icon_128.png"));

    mainWindow_->setContextMenuPolicy(Qt::NoContextMenu);
    setWidget(mainWindow_);

    // right part

    auto* optionLayout = sequenceEditorView_->getOptionLayout();
    // Settings for the controller
    auto* factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    for (auto* property : controller_.getProperties()) {
        auto propWidget = factory->create(property);
        auto propWidgetQt = static_cast<PropertyWidgetQt*>(propWidget.release());
        optionLayout->addWidget(propWidgetQt);
        propWidgetQt->initState();
    }

    // Entire mid part
    auto* overlay = new TextLabelOverlay(nullptr);
    animationEditor_ = std::make_unique<AnimationEditorQt>(controller_, widgetFactory, *overlay);

    animationView_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    {
        overlay->setParent(animationView_->viewport());
        auto* grid = new QGridLayout(animationView_->viewport());
        auto const space = utilqt::refSpacePx(this);
        grid->setContentsMargins(space, space, space, space);
        grid->addWidget(overlay, 0, 0, Qt::AlignTop | Qt::AlignLeft);
        auto sp = overlay->sizePolicy();
        sp.setHorizontalStretch(10);
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        overlay->setSizePolicy(sp);
    }

    // left part List widget of track labels
    auto* animationLabelView = new AnimationLabelViewQt(controller_);
    animationLabelView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    animationLabelView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    animationLabelView->verticalScrollBar()->setTracking(true);

    auto* splitter = new QSplitter();
    splitter->setMidLineWidth(0);
    splitter->setHandleWidth(1);
    splitter->setLineWidth(0);
    splitter->setFrameStyle(QFrame::NoFrame);
    splitter->addWidget(animationLabelView);
    splitter->addWidget(animationView_);
    splitter->addWidget(sequenceEditorView_);
    mainWindow_->setCentralWidget(splitter);

    connect(animationView_->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this, animationLabelView](auto val) {
                if (vScrolling_) return;
                util::KeepTrueWhileInScope const scrolling(&vScrolling_);
                auto* vs = animationView_->verticalScrollBar();
                auto* ls = animationLabelView->verticalScrollBar();

                const double vSize = vs->maximum() - vs->minimum() + vs->pageStep();
                const double lSize = ls->maximum() - ls->minimum() + ls->pageStep();
                const auto lval = static_cast<int>(std::round(val * lSize / vSize));

                ls->setValue(lval);
            });
    connect(animationLabelView->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this, animationLabelView](auto val) {
                if (vScrolling_) return;
                util::KeepTrueWhileInScope const scrolling(&vScrolling_);
                auto* vs = animationView_->verticalScrollBar();
                auto* ls = animationLabelView->verticalScrollBar();

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

    auto* toolBar = new QToolBar();
    toolBar->setObjectName("AnimationToolBar");
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    mainWindow_->addToolBar(toolBar);

    {
        auto* newAction = new QAction(QIcon(":/svgicons/newfile.svg"), tr("&New Animation"), this);
        newAction->setToolTip("New Animation");
        addAction(newAction);
        connect(newAction, &QAction::triggered, this, [this]() {
            animationsList_->addItem(
                utilqt::toQString(fmt::format("Animation {}", animations_.size() + 1)));
            animationsList_->setCurrentIndex(animationsList_->count() - 1);
        });
        toolBar->addAction(newAction);
    }

    {
        auto* importAction =
            new QAction(QIcon(":/svgicons/open.svg"), tr("&Import Animation"), this);
        addAction(importAction);
        connect(importAction, &QAction::triggered, this, [this]() { importAnimation(); });
        toolBar->addAction(importAction);
    }

    {
        auto* deleteAction =
            new QAction(QIcon(":/animation/icons/trashcan.svg"), tr("&Remove Animation"), this);
        deleteAction->setToolTip("Remove Animation");
        addAction(deleteAction);
        connect(deleteAction, &QAction::triggered, this,
                [this]() { animationsList_->removeItem(animationsList_->currentIndex()); });
        toolBar->addAction(deleteAction);
    }

    {
        animationsList_ = new QComboBox(toolBar);
        animationsList_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        animationsList_->setInsertPolicy(QComboBox::InsertAtCurrent);
        animationsList_->setEditable(true);
        animationsList_->setModel(new AnimationsModel(animations_, animationsList_));
        // Update currently selected index
        onAnimationChanged(&controller_, &animations_.getMainAnimation().get(),
                           &animations_.getMainAnimation().get());
        connect(animationsList_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this](int index) {
                    if (index >= 0 && index < static_cast<int>(animations_.size())) {
                        animations_.setMainAnimation(animations_[index]);
                    }
                });
        toolBar->addWidget(animationsList_);
    }

    toolBar->addSeparator();

    {
        auto* begin = toolBar->addAction(
            QIcon(":/animation/icons/arrow_media_next_player_previous_song_icon_128.svg"),
            "To Beginning");
        begin->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        begin->setToolTip("To Beginning");
        mainWindow_->addAction(begin);
        connect(begin, &QAction::triggered,
                [&]() { controller_.eval(controller_.getCurrentTime(), Seconds(0.0)); });
    }

    {
        auto* prev = toolBar->addAction(
            QIcon(":/animation/icons/arrow_arrows_direction_previous_icon_128.svg"), "Prev Key");
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
        icon.addFile(":/animation/icons/arrow_play_player_record_right_start_icon_128.svg", QSize(),
                     QIcon::Normal, QIcon::Off);
        icon.addFile(":/animation/icons/film_movie_pause_player_sound_icon_128.svg", QSize(),
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
        auto* next = toolBar->addAction(
            QIcon(":/animation/icons/arrow_arrows_direction_next_previous_icon_128.svg"),
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
        auto* end = toolBar->addAction(
            QIcon(":/animation/icons/arrow_next_player_previous_icon_128.svg"), "To End");
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

void AnimationEditorDockWidgetQt::importAnimation() {
    InviwoFileDialog openFileDialog(this, "Import Animation ...", "animation");
    openFileDialog.addSidebarPath(PathType::Workspaces);
    openFileDialog.addExtension("inv", "Inviwo File");
    openFileDialog.setFileMode(FileMode::AnyFile);

    if (openFileDialog.exec() != 0) {
        const QString path = openFileDialog.selectedFiles().at(0);
        const std::filesystem::path fileName{utilqt::toPath(path)};
        if (!std::filesystem::is_regular_file(fileName)) {
            LogError("Could not find file: " << fileName);
            return;
        }
        try {
            auto anim = std::ifstream(fileName);
            auto* app = InviwoApplication::getPtr();
            auto deserializer =
                app->getWorkspaceManager()->createWorkspaceDeserializer(anim, fileName);
            controller_.pause();
            auto animations = manager_.import(deserializer);
            for (auto animation : animations) {
                animations_.add(std::move(animation));
            }
        } catch (const std::exception& ex) {
            LogError(ex.what());
        }
    }
}

void AnimationEditorDockWidgetQt::closeEvent(QCloseEvent*) { controller_.pause(); }

void AnimationEditorDockWidgetQt::onStateChanged(AnimationController*, AnimationState,
                                                 AnimationState newState) {
    if (newState == AnimationState::Playing) {
        const QSignalBlocker block(btnPlayPause_);
        btnPlayPause_->setChecked(true);
    } else if (newState == AnimationState::Paused) {
        const QSignalBlocker block(btnPlayPause_);
        btnPlayPause_->setChecked(false);
    }
}

void AnimationEditorDockWidgetQt::onAnimationChanged(AnimationController*, Animation*,
                                                     Animation* newAnim) {

    if (auto it = animations_.find(newAnim); it != animations_.end()) {
        auto selectedIndex = static_cast<int>(std::distance(animations_.begin(), it));
        if (animationsList_->currentIndex() != selectedIndex) {
            animationsList_->setCurrentIndex(selectedIndex);
        }
    }
}

}  // namespace animation

}  // namespace inviwo
