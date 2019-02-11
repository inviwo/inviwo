/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <warn/push>
#include <warn/ignore/all>
#include <QKeyEvent>
#include <QSettings>
#include <warn/pop>

namespace inviwo {

InviwoDockWidget::InviwoDockWidget(QString title, QWidget *parent) : QDockWidget(title, parent) {

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

InviwoDockWidget::InviwoDockWidget(QString title, QWidget *parent, QString objName)
    : InviwoDockWidget(title, parent) {
    setObjectName(objName);
}

InviwoDockWidget::~InviwoDockWidget() = default;

void InviwoDockWidget::showEvent(QShowEvent *showEvent) {
    raise();
    QDockWidget::showEvent(showEvent);
}

void InviwoDockWidget::keyPressEvent(QKeyEvent *keyEvent) {
    if (keyEvent->key() == Qt::Key_Escape && isFloating()) {
        hide();
    } else {
        keyEvent->ignore();
    }
    QDockWidget::keyPressEvent(keyEvent);
}

void InviwoDockWidget::setSticky(bool sticky) { dockWidgetTitleBar_->setSticky(sticky); }

bool InviwoDockWidget::isSticky() const { return dockWidgetTitleBar_->isSticky(); }

void InviwoDockWidget::setContents(QWidget *widget) {
    QWidget *oldWidget = this->widget();
    if (oldWidget) {
        this->setWidget(nullptr);
        delete oldWidget;
    }

    this->setWidget(widget);
}

void InviwoDockWidget::setContents(QLayout *layout) {
    QWidget *oldWidget = this->widget();
    if (oldWidget) {
        this->setWidget(nullptr);
        delete oldWidget;
    }

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(layout);
    this->setWidget(centralWidget);
}

void InviwoDockWidget::saveState() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("sticky", isSticky());
    settings.setValue("floating", isFloating());
    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        settings.setValue("dockarea", static_cast<int>(mainWindow->dockWidgetArea(this)));
    }
    settings.setValue("size", size());
    settings.setValue("pos", pos());
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
        if (settings.contains("dockarea")) {
            auto dockarea = static_cast<Qt::DockWidgetArea>(settings.value("dockarea").toInt());
            if (dockarea == Qt::NoDockWidgetArea) {
                // take care of special case where dock area is not set due to floating status
                if (allowedAreas() & Qt::RightDockWidgetArea) {
                    dockarea = Qt::RightDockWidgetArea;
                } else if (allowedAreas() & Qt::LeftDockWidgetArea) {
                    dockarea = Qt::LeftDockWidgetArea;
                } else if (allowedAreas() & Qt::BottomDockWidgetArea) {
                    dockarea = Qt::BottomDockWidgetArea;
                } else if (allowedAreas() & Qt::TopDockWidgetArea) {
                    dockarea = Qt::TopDockWidgetArea;
                } else {
                    // fall-back: dock to right
                    dockarea = Qt::RightDockWidgetArea;
                }
                mainWindow->addDockWidget(dockarea, this);
                setFloating(true);
            } else {
                mainWindow->addDockWidget(dockarea, this);
            }
        }
    }

    if (settings.contains("size")) {
        resize(settings.value("size").toSize());
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

    if (settings.contains("visible")) {
        setVisible(settings.value("visible").toBool());
    }
    settings.endGroup();
}

void InviwoDockWidget::closeEvent(QCloseEvent *event) {
    saveState();
    QDockWidget::closeEvent(event);
}

}  // namespace inviwo
