/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <inviwo/core/util/raiiutils.h>       // for KeepTrueWhileInScope
#include <modules/qtwidgets/inviwoqtutils.h>  // for emToPx, windowTitleHelper
#include <inviwo/core/util/logcentral.h>

#include <QDockWidget>   // for QDockWidget
#include <QEvent>        // for QEvent, QEvent::ModifiedChange, QEvent::Win...
#include <QHBoxLayout>   // for QHBoxLayout
#include <QIcon>         // for QIcon, QIcon::Normal, QIcon::Off, QIcon::On
#include <QLabel>        // for QLabel
#include <QLayout>       // for QLayout
#include <QList>         // for QList, QList<>::iterator
#include <QPainter>      // for QPainter
#include <QSizeF>        // for QSizeF
#include <QStyle>        // for QStyle, QStyle::PE_Widget
#include <QStyleOption>  // for QStyleOption
#include <QToolButton>   // for QToolButton

class QHBoxLayout;
class QShowEvent;

namespace inviwo {

InviwoDockWidgetTitleBar::InviwoDockWidgetTitleBar(QWidget* parent)
    : QWidget(parent)
    , parent_(dynamic_cast<QDockWidget*>(parent))
    , allowedDockAreas_(parent_->allowedAreas())
    , internalStickyFlagUpdate_(false)
    , windowTitle_(parent->windowTitle()) {
    label_ = new QLabel(parent->windowTitle());
    label_->setObjectName("InviwoDockWidgetTitleBarLabel");
    label_->setMinimumWidth(utilqt::emToPx(fontMetrics(), 10));
    auto policy = label_->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    policy.setRetainSizeWhenHidden(true);
    label_->setSizePolicy(policy);

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

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(label_, 1);
    layout->addWidget(stickyBtn_);
    layout->addWidget(floatBtn_);
    layout->addWidget(closeBtn_);
    layout->setSpacing(0);
    auto margin = utilqt::emToPx(this, 0.2);
    layout->setContentsMargins(margin, margin, margin, margin);

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

void InviwoDockWidgetTitleBar::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
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

void InviwoDockWidgetTitleBar::updateTitle() {
    QString windowTitle = utilqt::windowTitleHelper(windowTitle_, parent_);
    // elide window title in case it is too long to fit the label
    QFontMetrics fontMetrics(label_->font());
    label_->setText(fontMetrics.elidedText(windowTitle, Qt::ElideMiddle, label_->width() - 4));
}

void InviwoDockWidgetTitleBar::showEvent(QShowEvent*) {
    if (isSticky()) {
        // docking allowed, restore docking areas
        parent_->setAllowedAreas(allowedDockAreas_);
    } else {
        // no docking, disable all areas
        parent_->setAllowedAreas(Qt::NoDockWidgetArea);
    }
}

bool InviwoDockWidgetTitleBar::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::WindowTitleChange) {
        if (auto window = qobject_cast<QWidget*>(obj); window) {
            // need to cache the window title, the parent widget might set its title to an empty
            // string due to the custom  title bar (observed in Qt 6.5.x and Qt 6.6.0)
            windowTitle_ = window->windowTitle();
        }
    }
    if ((event->type() == QEvent::ModifiedChange) || (event->type() == QEvent::WindowTitleChange)) {
        updateTitle();
    }
    return QObject::eventFilter(obj, event);
}

void InviwoDockWidgetTitleBar::resizeEvent(QResizeEvent*) { updateTitle(); }

void InviwoDockWidgetTitleBar::floating(bool floating) { floatBtn_->setChecked(floating); }

void InviwoDockWidgetTitleBar::setIconSize(double size) {
    iconSize_ = size;
    const auto iconsize = utilqt::emToPx(this, QSizeF(iconSize_, iconSize_));

    if (auto theLayout = layout()) {
        for (auto tb : theLayout->findChildren<QToolButton*>()) {
            tb->setIconSize(iconsize);
        }
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
