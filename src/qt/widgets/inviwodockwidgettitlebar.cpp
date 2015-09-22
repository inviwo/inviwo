/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/inviwodockwidgettitlebar.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QDockWidget>
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
{
    label_ = new QLabel(parent->windowTitle());
    label_->setStyleSheet("QWidget { padding-left: 5px; background-color: 'transparent'; }");

    stickyBtn_ = new QToolButton();
    QIcon icon;
    icon.addPixmap(QPixmap(":/stylesheets/images/dock-unsticky.png"), QIcon::Normal, QIcon::Off);
    icon.addPixmap(QPixmap(":/stylesheets/images/dock-sticky.png"), QIcon::Normal, QIcon::On);
    stickyBtn_->setIcon(icon);
    stickyBtn_->setCheckable(true);
    stickyBtn_->setChecked(true);
    stickyBtn_->setObjectName("dockBtn");

    floatBtn_ = new QToolButton();
    QIcon icon2;
    icon2.addPixmap(QPixmap(":/stylesheets/images/dock-docked.png"), QIcon::Normal, QIcon::Off);
    icon2.addPixmap(QPixmap(":/stylesheets/images/dock-floating.png"), QIcon::Normal, QIcon::On);
    floatBtn_->setIcon(icon2);
    floatBtn_->setCheckable(true);
    floatBtn_->setChecked(parent_->isFloating());
    floatBtn_->setObjectName("dockBtn");

    QToolButton *closeBtn = new QToolButton();
    closeBtn->setIcon(QIcon(":/stylesheets/images/close.png"));
    closeBtn->setObjectName("dockBtn");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(label_, 1);
    layout->addWidget(stickyBtn_);
    layout->addWidget(floatBtn_);
    layout->addWidget(closeBtn);
    layout->setSpacing(2);
    layout->setMargin(2);

    this->setLayout(layout);

    QObject::connect(stickyBtn_, SIGNAL(toggled(bool)), this, SLOT(stickyBtnToggled(bool)));
    QObject::connect(floatBtn_, SIGNAL(clicked()), this, SLOT(floatBtnClicked()));
    QObject::connect(closeBtn, SIGNAL(clicked()), parent_, SLOT(close()));
}

InviwoDockWidgetTitleBar::~InviwoDockWidgetTitleBar() {}

void InviwoDockWidgetTitleBar::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    //style()->drawControl(QStyle::CE_DockWidgetTitle, &opt, &p, this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void InviwoDockWidgetTitleBar::setLabel(const QString &str) {
    label_->setText(str);
}

void InviwoDockWidgetTitleBar::stickyBtnToggled(bool toggle) {
    if (toggle) {
        // docking allowed, restore docking areas
        parent_->setAllowedAreas(allowedDockAreas_);
    }
    else {
        // no docking, disable all areas
        parent_->setAllowedAreas(Qt::NoDockWidgetArea);
    }
}

void InviwoDockWidgetTitleBar::floatBtnClicked() {
    parent_->setFloating(!parent_->isFloating());
}

void InviwoDockWidgetTitleBar::floating(bool floating) {
    floatBtn_->setChecked(floating);
}

void InviwoDockWidgetTitleBar::setSticky(bool toggle) {
    stickyBtn_->setChecked(toggle);
}

bool InviwoDockWidgetTitleBar::isSticky() const {
    return stickyBtn_->isChecked();
}

} // namespace
