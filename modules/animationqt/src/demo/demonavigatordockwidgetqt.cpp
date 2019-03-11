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

#include <inviwo/qt/editor/inviwomainwindow.h>
#include <modules/animationqt/demo/demonavigatordockwidgetqt.h>

#include <modules/animation/demo/democontroller.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/qt/editor/inviwomainwindow.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QSettings>
#include <QToolBar>
#include <QLabel>
#include <QMainWindow>
#include <warn/pop>

namespace inviwo {

namespace animation {

DemoNavigatorDockWidgetQt::DemoNavigatorDockWidgetQt(DemoController& controller,
                                                     const std::string& widgetName, QWidget* parent)
    : InviwoDockWidget(utilqt::toQString(widgetName), parent, "DemoNavigatorWidget")
    , controller_(controller) {

    setFloating(true);
    setSticky(false);
    resize(utilqt::emToPx(this, QSizeF(30, 10)));  // default size
    setWindowIcon(
        QIcon(":/animation/icons/arrow_next_player_previous_recording_right_icon_128.png"));

    QWidget* mainWidget = new QWidget();
    QVBoxLayout* boxLayout = new QVBoxLayout();
    QMainWindow* toolWidget = new QMainWindow();

    mainWidget->setLayout(boxLayout);
    setWidget(mainWidget);

    {
        auto policy = toolWidget->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        policy.setHorizontalStretch(0);
        toolWidget->setSizePolicy(policy);
        toolWidget->setMinimumWidth(32 * 6);
    }

    // Add all properties of DemoController
    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    for (auto property : controller_.getProperties()) {
        auto propWidget = factory->create(property);
        auto propWidgetQt = static_cast<PropertyWidgetQt*>(propWidget.release());
        boxLayout->addWidget(propWidgetQt);
        propWidgetQt->initState();
    }

    QToolBar* toolBar = new QToolBar();
    {
        toolWidget->setContextMenuPolicy(Qt::NoContextMenu);
        toolWidget->addToolBar(toolBar);
        toolBar->setFloatable(false);
        toolBar->setMovable(false);
        auto policy = toolWidget->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Fixed);
        policy.setHorizontalStretch(0);
        toolWidget->setSizePolicy(policy);
        toolWidget->setMinimumWidth(160);
        boxLayout->addWidget(toolBar);
    }

    {
        auto begin = toolBar->addAction(
            QIcon(":/animation/icons/arrow_media_next_player_previous_song_icon_32.png"),
            "To Beginning");
        begin->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        begin->setToolTip("To Beginning");
        toolWidget->addAction(begin);
        connect(begin, &QAction::triggered,
                [&]() { controller_.onChangeSelection(DemoController::Offset::First); });
    }

    {
        auto prev = toolBar->addAction(
            QIcon(":/animation/icons/arrow_direction_left_next_previous_return_icon_32.png"),
            "Prev Key");
        prev->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        prev->setToolTip("Previous Demo");
        toolWidget->addAction(prev);
        connect(prev, &QAction::triggered,
                [&]() { controller_.onChangeSelection(DemoController::Offset::Previous); });
    }

    {
        auto reset = toolBar->addAction(
            QIcon(":/animation/icons/arrow_direction_refresh_repeat_restart_icon_32.png"),
            "Reset Workspace");
        reset->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        reset->setToolTip("Reset");
        toolWidget->addAction(reset);
        connect(reset, &QAction::triggered,
                [&]() { controller_.onChangeSelection(DemoController::Offset::Reload); });
    }
    {
        auto next = toolBar->addAction(
            QIcon(":/animation/icons/arrow_direction_previous_right_icon_32.png"), "Next Demo");
        next->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        next->setToolTip("Next Demo");
        toolWidget->addAction(next);
        connect(next, &QAction::triggered,
                [&]() { controller_.onChangeSelection(DemoController::Offset::Next); });
    }

    {
        auto end = toolBar->addAction(
            QIcon(":/animation/icons/arrow_next_player_previous_icon_32.png"), "To End");
        end->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        end->setToolTip("To End");
        toolWidget->addAction(end);
        connect(end, &QAction::triggered,
                [&]() { controller_.onChangeSelection(DemoController::Offset::Last); });
    }

    toolBar->addSeparator();

    controller_.DemoControllerObservable::addObserver(this);
}

DemoNavigatorDockWidgetQt::~DemoNavigatorDockWidgetQt() = default;

}  // namespace animation

}  // namespace inviwo
