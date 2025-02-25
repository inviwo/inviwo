/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <modules/qtwidgets/inviwodockwidgettitlebar.h>  // for InviwoDockWidgetTitleBar
#include <modules/qtwidgets/inviwoqtutils.h>             // for getApplicationMainWindow, movePo...

#include <QDebug>       // for operator<<
#include <QFlags>       // for QFlags, operator==
#include <QKeyEvent>    // for QKeyEvent
#include <QMainWindow>  // for QMainWindow
#include <QPoint>       // for QPoint
#include <QSettings>    // for QSettings
#include <QSize>        // for QSize
#include <QVariant>     // for QVariant
#include <QWidget>      // for QWidget
#include <Qt>           // for qt_getEnumName, RightDockWidgetArea

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
}

InviwoDockWidget::InviwoDockWidget(QString title, QWidget* parent, QString objName)
    : InviwoDockWidget(title, parent) {
    setObjectName(objName);
}

InviwoDockWidget::~InviwoDockWidget() = default;

void InviwoDockWidget::showEvent(QShowEvent* showEvent) {
    raise();
    QDockWidget::showEvent(showEvent);
    // Workaround: set focus for floating dock widget
    if (isFloating()) {
        activateWindow();
    }
}

void InviwoDockWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Escape && isFloating()) {
        hide();
    } else {
        keyEvent->ignore();
    }
    QDockWidget::keyPressEvent(keyEvent);
}

void InviwoDockWidget::setSticky(bool sticky) { dockWidgetTitleBar_->setSticky(sticky); }

bool InviwoDockWidget::isSticky() const { return dockWidgetTitleBar_->isSticky(); }

void InviwoDockWidget::setContents(QWidget* widget) {
    QWidget* oldWidget = this->widget();
    if (oldWidget) {
        setWidget(nullptr);
        delete oldWidget;
    }
    setWidget(widget);
}

void InviwoDockWidget::setContents(QLayout* layout) {
    QWidget* oldWidget = this->widget();
    if (oldWidget) {
        setWidget(nullptr);
        delete oldWidget;
    }

    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(layout);
    setWidget(centralWidget);
}

void InviwoDockWidget::saveState() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("sticky", isSticky());
    settings.setValue("floating", isFloating());
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("visible", isVisible());
    settings.endGroup();
}

void InviwoDockWidget::loadState() {
    QSettings settings;
    settings.beginGroup(objectName());

    if (settings.contains("sticky")) {
        setSticky(settings.value("sticky").toBool());
    }

    if (settings.contains("floating")) {
        setFloating(settings.value("floating").toBool());
    }

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        mainWindow->restoreDockWidget(this);
    }

    if (settings.contains("pos")) {
        auto pos = settings.value("pos").toPoint();
        auto newPos = utilqt::movePointOntoDesktop(pos, InviwoDockWidget::size(), false);
        move(newPos);
    } else if (auto ivwMW = utilqt::getApplicationMainWindow()) {
        // We assume that this is a new widget and give it a new position
        auto newPos = ivwMW->pos();
        newPos += utilqt::offsetWidget();
        move(newPos);
    }

    if (settings.contains("size")) {
        resize(settings.value("size").toSize());
    }

    if (auto* screen = QGuiApplication::screenAt(pos())) {
        const auto w = std::clamp(width(), 0, screen->availableSize().width());
        const auto h = std::clamp(height(), 0, screen->availableSize().height());
        if (QSize(w, h) != size()) {
            resize(w, h);
        }
    }

    if (settings.contains("visible")) {
        setVisible(settings.value("visible").toBool());
    }
    settings.endGroup();
}

void InviwoDockWidget::closeEvent(QCloseEvent* event) {
    saveState();
    QDockWidget::closeEvent(event);
}

}  // namespace inviwo
