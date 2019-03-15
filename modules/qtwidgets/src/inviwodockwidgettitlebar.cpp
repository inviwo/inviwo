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

#include <modules/qtwidgets/inviwodockwidgettitlebar.h>

#include <inviwo/core/util/raiiutils.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QDockWidget>
#include <QEvent>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QToolButton>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

InviwoDockWidgetTitleBar::InviwoDockWidgetTitleBar(QWidget *parent)
    : QWidget(parent)
    , parent_(dynamic_cast<QDockWidget *>(parent))
    , allowedDockAreas_(parent_->allowedAreas())
    , internalStickyFlagUpdate_(false) {
    label_ = new QLabel(parent->windowTitle());
    label_->setObjectName("InviwoDockWidgetTitleBarLabel");

    const auto iconsize = utilqt::emToPx(this, QSizeF(iconSize_, iconSize_));

    {
        stickyBtn_ = new QToolButton();
        QIcon icon;
        icon.addFile(":/svgicons/dock-unsticky.svg", iconsize, QIcon::Normal, QIcon::Off);
        icon.addFile(":/svgicons/dock-sticky.svg", iconsize, QIcon::Normal, QIcon::On);
        stickyBtn_->setIcon(icon);
        stickyBtn_->setCheckable(true);
        stickyBtn_->setChecked(true);
        stickyBtn_->setIconSize(iconsize);
    }
    {
        floatBtn_ = new QToolButton();
        QIcon icon;
        icon.addFile(":/svgicons/dock-docked.svg", iconsize, QIcon::Normal, QIcon::Off);
        icon.addFile(":/svgicons/dock-floating.svg", iconsize, QIcon::Normal, QIcon::On);
        floatBtn_->setIcon(icon);
        floatBtn_->setCheckable(true);
        floatBtn_->setChecked(parent_->isFloating());
        floatBtn_->setIconSize(iconsize);
    }

    {
        closeBtn_ = new QToolButton();
        QIcon icon;
        icon.addFile(":/svgicons/close.svg", iconsize);
        closeBtn_->setIcon(icon);
        closeBtn_->setIconSize(iconsize);
    }

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(label_, 1);
    layout->addWidget(stickyBtn_);
    layout->addWidget(floatBtn_);
    layout->addWidget(closeBtn_);
    layout->setSpacing(0);
    layout->setMargin(utilqt::emToPx(this, 0.2));

    setLayout(layout);

    QObject::connect(parent_, &QDockWidget::topLevelChanged, this,
                     &InviwoDockWidgetTitleBar::floating);
    QObject::connect(stickyBtn_, &QToolButton::toggled, this,
                     &InviwoDockWidgetTitleBar::stickyBtnToggled);
    QObject::connect(floatBtn_, &QToolButton::clicked, this,
                     [&]() { parent_->setFloating(!parent_->isFloating()); });
    QObject::connect(closeBtn_, &QToolButton::clicked, parent_, &QDockWidget::close);

    parent_->installEventFilter(this);
}

InviwoDockWidgetTitleBar::~InviwoDockWidgetTitleBar() = default;

void InviwoDockWidgetTitleBar::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    // style()->drawControl(QStyle::CE_DockWidgetTitle, &opt, &p, this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void InviwoDockWidgetTitleBar::stickyBtnToggled(bool toggle) {
    util::KeepTrueWhileInScope guard(&internalStickyFlagUpdate_);
    if (toggle) {
        // docking allowed, restore docking areas
        parent_->setAllowedAreas(allowedDockAreas_);
    } else {
        // no docking, disable all areas
        parent_->setAllowedAreas(Qt::NoDockWidgetArea);
    }
    emit stickyFlagChanged(toggle);
}

void InviwoDockWidgetTitleBar::showEvent(QShowEvent *) {
    if (isSticky()) {
        // docking allowed, restore docking areas
        parent_->setAllowedAreas(allowedDockAreas_);
    } else {
        // no docking, disable all areas
        parent_->setAllowedAreas(Qt::NoDockWidgetArea);
    }
}

bool InviwoDockWidgetTitleBar::eventFilter(QObject *obj, QEvent *event) {
    if ((event->type() == QEvent::ModifiedChange) || (event->type() == QEvent::WindowTitleChange)) {
        label_->setText(utilqt::windowTitleHelper(parent_->windowTitle(), parent_));
    }
    return QObject::eventFilter(obj, event);
}

void InviwoDockWidgetTitleBar::floating(bool floating) { floatBtn_->setChecked(floating); }

void InviwoDockWidgetTitleBar::setIconSize(double size) {
    iconSize_ = size;
    const auto iconsize = utilqt::emToPx(this, QSizeF(iconSize_, iconSize_));

    for (auto tb : layout()->findChildren<QToolButton *>()) {
        tb->setIconSize(iconsize);
    }
}

void InviwoDockWidgetTitleBar::setSticky(bool toggle) { stickyBtn_->setChecked(toggle); }

bool InviwoDockWidgetTitleBar::isSticky() const { return stickyBtn_->isChecked(); }

void InviwoDockWidgetTitleBar::allowedAreasChanged(Qt::DockWidgetAreas areas) {
    if (!internalStickyFlagUpdate_) {
        // save currently set docking areas
        allowedDockAreas_ = areas;
        if (!isSticky()) {
            // dockwidget is non-sticky, reset allowed areas to none
            util::KeepTrueWhileInScope guard(&internalStickyFlagUpdate_);
            parent_->setAllowedAreas(Qt::NoDockWidgetArea);
        }
    }
}

}  // namespace inviwo
