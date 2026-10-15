/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <modules/qtwidgets/inviwodockwidget.h>

#include <modules/qtwidgets/inviwodockwidgettitlebar.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QDebug>
#include <QFlags>
#include <QKeyEvent>
#include <QMainWindow>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <QGuiApplication>
#include <algorithm>

class QCloseEvent;
class QShowEvent;

namespace inviwo {

InviwoDockWidget::InviwoDockWidget(QString title, QWidget* parent) : QDockWidget(title, parent) {

    setObjectName(title);
#ifdef __APPLE__
    setStyleSheet("QDockWidget::title {padding-left: 45px; }");
#endif

    // adding custom title bar to dock widget
    dockWidgetTitleBar_ = new InviwoDockWidgetTitleBar(this);
    setTitleBarWidget(dockWidgetTitleBar_);

    QObject::connect(dockWidgetTitleBar_, &InviwoDockWidgetTitleBar::stickyFlagChanged, this,
                     &InviwoDockWidget::stickyFlagChanged);
    QObject::connect(this, &QDockWidget::allowedAreasChanged, dockWidgetTitleBar_,
                     &InviwoDockWidgetTitleBar::allowedAreasChanged);

    QObject::connect(this, &InviwoDockWidget::topLevelChanged, this,
                     &InviwoDockWidget::stateChanged);
    QObject::connect(dockWidgetTitleBar_, &InviwoDockWidgetTitleBar::stickyFlagChanged, this,
                     &InviwoDockWidget::stateChanged);
}

InviwoDockWidget::InviwoDockWidget(QString title, QWidget* parent, QString objName)
    : InviwoDockWidget(title, parent) {
    setObjectName(objName);
}

InviwoDockWidget::~InviwoDockWidget() = default;

void InviwoDockWidget::setSticky(bool sticky) { dockWidgetTitleBar_->setSticky(sticky); }

bool InviwoDockWidget::isSticky() const { return dockWidgetTitleBar_->isSticky(); }

void InviwoDockWidget::setContents(QWidget* widget) {
    const QWidget* oldWidget = this->widget();
    if (oldWidget) {
        setWidget(nullptr);
        delete oldWidget;
    }
    setWidget(widget);
}

void InviwoDockWidget::setContents(QLayout* layout) {
    const QWidget* oldWidget = this->widget();
    if (oldWidget) {
        setWidget(nullptr);
        delete oldWidget;
    }

    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(layout);
    setWidget(centralWidget);
}

void InviwoDockWidget::getState(UnorderedStringMap<QVariant>& state) const {
    state.insert_range(std::to_array<typename UnorderedStringMap<QVariant>::value_type>({
        {"sticky", isSticky()},
        {"floating", isFloating()},
        {"pos", pos()},
        {"size", size()},
        {"visible", isVisible()},
    }));
}
void InviwoDockWidget::setState(const UnorderedStringMap<QVariant>& state) {
    if (state.contains("sticky")) setSticky(state.at("sticky").toBool());

    if (state.contains("floating")) setFloating(state.at("floating").toBool());

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        mainWindow->restoreDockWidget(this);
    }

    if (state.contains("pos")) {
        const auto pos = state.at("pos").toPoint();
        const auto newPos = utilqt::movePointOntoDesktop(pos, InviwoDockWidget::size(), false);
        move(newPos);
    } else if (auto ivwMW = utilqt::getApplicationMainWindow()) {
        // We assume that this is a new widget and give it a new position
        auto newPos = ivwMW->pos();
        newPos += utilqt::offsetWidget();
        move(newPos);
    }

    if (state.contains("size")) resize(state.at("size").toSize());

    if (const auto* screen = QGuiApplication::screenAt(pos())) {
        const auto w = std::clamp(width(), 0, screen->availableSize().width());
        const auto h = std::clamp(height(), 0, screen->availableSize().height());
        if (QSize(w, h) != size()) {
            resize(w, h);
        }
    }

    if (state.contains("visible")) setVisible(state.at("visible").toBool());
}

void InviwoDockWidget::saveState() {
    UnorderedStringMap<QVariant> state;
    getState(state);
    saveStateDefault(state);
}

void InviwoDockWidget::loadState() {
    UnorderedStringMap<QVariant> state;
    loadStateDefault(state);
    setState(state);
}

void InviwoDockWidget::loadStateDefault(UnorderedStringMap<QVariant>& state) const {
    QSettings settings;
    settings.beginGroup(objectName());
    for (const auto& key : settings.childKeys()) {
        state.emplace(utilqt::fromQString(key), settings.value(key));
    }
    settings.endGroup();
}
void InviwoDockWidget::saveStateDefault(const UnorderedStringMap<QVariant>& state) {
    QSettings settings;
    settings.beginGroup(objectName());
    for (const auto& [key, value] : state) {
        settings.setValue(utilqt::toQString(key), value);
    }
    settings.endGroup();
}

void InviwoDockWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Escape && isFloating()) {
        hide();
    } else {
        keyEvent->ignore();
    }
    QDockWidget::keyPressEvent(keyEvent);
}

void InviwoDockWidget::showEvent(QShowEvent* event) {
    raise();
    QDockWidget::showEvent(event);
    // Workaround: set focus for floating dock widget
    if (isFloating()) {
        activateWindow();
    }
    stateChanged();
}

void InviwoDockWidget::hideEvent(QHideEvent* event) {
    QDockWidget::hideEvent(event);
    stateChanged();
}
void InviwoDockWidget::resizeEvent(QResizeEvent* event) {
    QDockWidget::resizeEvent(event);
    stateChanged();
}
void InviwoDockWidget::moveEvent(QMoveEvent* event) {
    QDockWidget::moveEvent(event);
    stateChanged();
}
void InviwoDockWidget::closeEvent(QCloseEvent* event) {
    saveState();
    QDockWidget::closeEvent(event);
}

}  // namespace inviwo
